#!/usr/bin/env python3
"""
AI Session Manager — SQLite-backed session state for AI agents.

Lets any model/agent record work-in-progress and restore full context in a
new conversation without reading the entire git log or codebase from scratch.

QUICK REFERENCE
---------------
Start a session:
    python tools/session_mgr.py start "Fix trade atomicity" --goal "wrap both SaveDBAllItemBox in one tx"

Log progress (call often):
    python tools/session_mgr.py event progress "Fixed LoginQueue strcpy — strncpy+null-terminator" --files LoginServer/LoginQueue.cpp
    python tools/session_mgr.py event decision "Skipped MySQLPreparedStatement destructor — m_stmt_guard already closes it"
    python tools/session_mgr.py event commit "5af2db9a fix: bound account strcpy"
    python tools/session_mgr.py event blocker "SaveDBAllItemBox is 700 lines — full tx refactor risky"
    python tools/session_mgr.py event finding "NDEBUG in Debug builds disables all assert()"
    python tools/session_mgr.py event note "user prefers separate commits per fix"

Get handoff brief (paste into next model's first message):
    python tools/session_mgr.py handoff

Show active session status:
    python tools/session_mgr.py status

Close session:
    python tools/session_mgr.py close --summary "9 commits, all critical fixes done"

List recent sessions:
    python tools/session_mgr.py list

Resume a previous session:
    python tools/session_mgr.py resume <session_id>

EVENT TYPES
-----------
  progress  Work completed, step done
  decision  A choice made (and why, including why NOT to do something)
  commit    A git commit recorded (hash + message)
  finding   Bug or issue found (not yet fixed)
  blocker   Something blocking progress
  note      User preference, constraint, or context
  falsepos  Audit/review finding ruled out as false positive

DB: tools/ai_session.db
"""

from __future__ import annotations

import argparse
import json
import os
import sqlite3
import sys
import textwrap
import uuid
from datetime import datetime, timezone
from pathlib import Path
from typing import Optional

# Force UTF-8 output on Windows consoles that default to cp1251/cp1252
if sys.stdout.encoding and sys.stdout.encoding.lower() not in ("utf-8", "utf8"):
    sys.stdout = open(sys.stdout.fileno(), mode="w", encoding="utf-8", errors="replace", closefd=False)
if sys.stderr.encoding and sys.stderr.encoding.lower() not in ("utf-8", "utf8"):
    sys.stderr = open(sys.stderr.fileno(), mode="w", encoding="utf-8", errors="replace", closefd=False)

# ---------------------------------------------------------------------------
SCRIPT_DIR = Path(__file__).resolve().parent
DB_PATH = SCRIPT_DIR / "ai_session.db"

VALID_EVENT_TYPES = {"progress", "decision", "commit", "finding", "blocker", "note", "falsepos"}

# ---------------------------------------------------------------------------
# Schema
# ---------------------------------------------------------------------------
_SCHEMA = """
CREATE TABLE IF NOT EXISTS sessions (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    session_id  TEXT    NOT NULL UNIQUE,
    title       TEXT    NOT NULL,
    goal        TEXT    DEFAULT '',
    branch      TEXT    DEFAULT '',
    status      TEXT    DEFAULT 'active'
                        CHECK(status IN ('active','closed','handoff')),
    summary     TEXT    DEFAULT '',
    created_at  TEXT    NOT NULL,
    updated_at  TEXT    NOT NULL,
    closed_at   TEXT    DEFAULT ''
);

CREATE TABLE IF NOT EXISTS events (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    session_id  TEXT    NOT NULL REFERENCES sessions(session_id),
    type        TEXT    NOT NULL,
    content     TEXT    NOT NULL,
    files       TEXT    DEFAULT '',
    timestamp   TEXT    NOT NULL
);

CREATE TABLE IF NOT EXISTS active_session (
    singleton   INTEGER PRIMARY KEY DEFAULT 1,
    session_id  TEXT
);

CREATE INDEX IF NOT EXISTS idx_events_session ON events(session_id);
CREATE INDEX IF NOT EXISTS idx_events_type    ON events(type);
"""


def _now() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="seconds")


def _connect() -> sqlite3.Connection:
    DB_PATH.parent.mkdir(parents=True, exist_ok=True)
    conn = sqlite3.connect(str(DB_PATH), timeout=10)
    conn.row_factory = sqlite3.Row
    conn.execute("PRAGMA journal_mode=WAL")
    conn.execute("PRAGMA foreign_keys=ON")
    conn.executescript(_SCHEMA)
    return conn


def _get_active_id(conn: sqlite3.Connection) -> Optional[str]:
    row = conn.execute("SELECT session_id FROM active_session WHERE singleton=1").fetchone()
    return row["session_id"] if row else None


def _set_active_id(conn: sqlite3.Connection, session_id: Optional[str]) -> None:
    if session_id is None:
        conn.execute("DELETE FROM active_session WHERE singleton=1")
    else:
        conn.execute(
            "INSERT OR REPLACE INTO active_session(singleton, session_id) VALUES(1, ?)",
            (session_id,),
        )
    conn.commit()


def _git_branch() -> str:
    try:
        import subprocess
        result = subprocess.run(
            ["git", "rev-parse", "--abbrev-ref", "HEAD"],
            capture_output=True, text=True, timeout=3,
            cwd=SCRIPT_DIR.parent,
        )
        return result.stdout.strip() if result.returncode == 0 else ""
    except Exception:
        return ""


def _require_active(conn: sqlite3.Connection) -> str:
    sid = _get_active_id(conn)
    if not sid:
        print("ERROR: no active session. Run: python tools/session_mgr.py start <title>", file=sys.stderr)
        sys.exit(1)
    return sid


# ---------------------------------------------------------------------------
# Commands
# ---------------------------------------------------------------------------

def cmd_start(args: argparse.Namespace) -> int:
    conn = _connect()
    existing = _get_active_id(conn)
    if existing:
        row = conn.execute("SELECT title FROM sessions WHERE session_id=?", (existing,)).fetchone()
        print(f"WARNING: closing previous active session '{row['title']}' ({existing[:8]})")
        conn.execute(
            "UPDATE sessions SET status='closed', closed_at=?, updated_at=? WHERE session_id=?",
            (_now(), _now(), existing),
        )
        conn.commit()

    sid = uuid.uuid4().hex[:12]
    branch = _git_branch()
    now = _now()
    conn.execute(
        "INSERT INTO sessions(session_id, title, goal, branch, status, created_at, updated_at) VALUES(?,?,?,?,?,?,?)",
        (sid, args.title, args.goal or "", branch, "active", now, now),
    )
    conn.commit()
    _set_active_id(conn, sid)
    print(f"Session started: {sid}")
    print(f"  title : {args.title}")
    if args.goal:
        print(f"  goal  : {args.goal}")
    if branch:
        print(f"  branch: {branch}")
    return 0


def cmd_event(args: argparse.Namespace) -> int:
    etype = args.type.lower()
    if etype not in VALID_EVENT_TYPES:
        print(f"ERROR: unknown event type '{etype}'. Valid: {', '.join(sorted(VALID_EVENT_TYPES))}", file=sys.stderr)
        return 1

    conn = _connect()
    sid = _require_active(conn)

    files = args.files or ""
    now = _now()
    conn.execute(
        "INSERT INTO events(session_id, type, content, files, timestamp) VALUES(?,?,?,?,?)",
        (sid, etype, args.content, files, now),
    )
    conn.execute("UPDATE sessions SET updated_at=? WHERE session_id=?", (now, sid))
    conn.commit()

    icon = {
        "progress": "+", "decision": "!", "commit": "*", "finding": "?",
        "blocker": "X", "note": ">", "falsepos": "-",
    }.get(etype, ".")
    print(f"{icon} [{etype}] {args.content[:80]}")
    return 0


def cmd_status(args: argparse.Namespace) -> int:
    conn = _connect()
    sid = args.session_id or _get_active_id(conn)
    if not sid:
        print("No active session.")
        return 0

    row = conn.execute("SELECT * FROM sessions WHERE session_id=?", (sid,)).fetchone()
    if not row:
        print(f"Session {sid} not found.")
        return 1

    events = conn.execute(
        "SELECT type, content, files, timestamp FROM events WHERE session_id=? ORDER BY id",
        (sid,),
    ).fetchall()

    print(f"\n{'='*60}")
    print(f"SESSION: {row['title']}")
    print(f"  ID    : {row['session_id']}")
    print(f"  status: {row['status']}")
    print(f"  branch: {row['branch'] or '(unknown)'}")
    if row["goal"]:
        print(f"  goal  : {row['goal']}")
    if row["summary"]:
        print(f"  summary: {row['summary']}")
    print(f"  started : {row['created_at']}")
    print(f"  updated : {row['updated_at']}")
    print(f"{'='*60}")

    if not events:
        print("  (no events yet)")
    else:
        by_type: dict[str, list] = {}
        for e in events:
            by_type.setdefault(e["type"], []).append(e)

        order = ["commit", "progress", "decision", "finding", "blocker", "falsepos", "note"]
        for t in order:
            if t not in by_type:
                continue
            label = {
                "commit": "COMMITS", "progress": "PROGRESS", "decision": "DECISIONS",
                "finding": "FINDINGS", "blocker": "BLOCKERS", "falsepos": "FALSE POSITIVES", "note": "NOTES",
            }[t]
            print(f"\n{label}:")
            for e in by_type[t]:
                files = f"  [{e['files']}]" if e["files"] else ""
                print(f"  · {e['content']}{files}")

    print()
    return 0


def cmd_handoff(args: argparse.Namespace) -> int:
    """Generate a handoff brief — paste this as the first message to the next model."""
    conn = _connect()
    sid = _get_active_id(conn)
    if not sid:
        print("No active session to hand off.")
        return 1

    row = conn.execute("SELECT * FROM sessions WHERE session_id=?", (sid,)).fetchone()
    events = conn.execute(
        "SELECT type, content, files, timestamp FROM events WHERE session_id=? ORDER BY id",
        (sid,),
    ).fetchall()

    lines: list[str] = []
    lines.append("## Session Handoff")
    lines.append(f"**Session:** {row['title']} (`{row['session_id']}`)")
    lines.append(f"**Branch:** {row['branch'] or '(unknown)'}")
    if row["goal"]:
        lines.append(f"**Goal:** {row['goal']}")
    if row["summary"]:
        lines.append(f"**Summary so far:** {row['summary']}")
    lines.append(f"**Last updated:** {row['updated_at']}")
    lines.append("")

    commits = [e for e in events if e["type"] == "commit"]
    if commits:
        lines.append("### Commits made this session")
        for e in commits:
            lines.append(f"- {e['content']}")
        lines.append("")

    progress = [e for e in events if e["type"] == "progress"]
    if progress:
        lines.append("### Work completed")
        for e in progress:
            files = f" `{e['files']}`" if e["files"] else ""
            lines.append(f"- {e['content']}{files}")
        lines.append("")

    decisions = [e for e in events if e["type"] == "decision"]
    if decisions:
        lines.append("### Decisions made (do not re-litigate these)")
        for e in decisions:
            lines.append(f"- {e['content']}")
        lines.append("")

    falsepos = [e for e in events if e["type"] == "falsepos"]
    if falsepos:
        lines.append("### False positives already ruled out (skip these)")
        for e in falsepos:
            lines.append(f"- {e['content']}")
        lines.append("")

    findings = [e for e in events if e["type"] == "finding"]
    if findings:
        lines.append("### Open findings (not yet fixed)")
        for e in findings:
            files = f" `{e['files']}`" if e["files"] else ""
            lines.append(f"- {e['content']}{files}")
        lines.append("")

    blockers = [e for e in events if e["type"] == "blocker"]
    if blockers:
        lines.append("### Blockers")
        for e in blockers:
            lines.append(f"- {e['content']}")
        lines.append("")

    notes = [e for e in events if e["type"] == "note"]
    if notes:
        lines.append("### Notes / constraints")
        for e in notes:
            lines.append(f"- {e['content']}")
        lines.append("")

    lines.append("### Restore context")
    lines.append("```bash")
    lines.append(f"python tools/session_mgr.py resume {row['session_id']}")
    lines.append(f"python tools/session_mgr.py status")
    lines.append("```")

    output = "\n".join(lines)
    print(output)

    # also write to file for easy copy
    out_path = SCRIPT_DIR / f"handoff_{row['session_id']}.md"
    out_path.write_text(output, encoding="utf-8")
    print(f"\n(written to {out_path})", file=sys.stderr)

    conn.execute(
        "UPDATE sessions SET status='handoff', updated_at=? WHERE session_id=?",
        (_now(), sid),
    )
    conn.commit()
    return 0


def cmd_close(args: argparse.Namespace) -> int:
    conn = _connect()
    sid = _require_active(conn)
    row = conn.execute("SELECT title FROM sessions WHERE session_id=?", (sid,)).fetchone()
    now = _now()
    summary = args.summary or ""
    conn.execute(
        "UPDATE sessions SET status='closed', summary=?, closed_at=?, updated_at=? WHERE session_id=?",
        (summary, now, now, sid),
    )
    conn.commit()
    _set_active_id(conn, None)
    print(f"Session '{row['title']}' ({sid[:8]}) closed.")
    return 0


def cmd_resume(args: argparse.Namespace) -> int:
    conn = _connect()
    sid = args.session_id
    row = conn.execute("SELECT * FROM sessions WHERE session_id=?", (sid,)).fetchone()
    if not row:
        # try prefix match
        rows = conn.execute(
            "SELECT * FROM sessions WHERE session_id LIKE ?", (sid + "%",)
        ).fetchall()
        if len(rows) == 1:
            row = rows[0]
            sid = row["session_id"]
        elif len(rows) > 1:
            print(f"Ambiguous prefix '{sid}' — matches:")
            for r in rows:
                print(f"  {r['session_id']} — {r['title']}")
            return 1
        else:
            print(f"Session '{sid}' not found.")
            return 1

    existing = _get_active_id(conn)
    if existing and existing != sid:
        r2 = conn.execute("SELECT title FROM sessions WHERE session_id=?", (existing,)).fetchone()
        print(f"WARNING: closing current active session '{r2['title']}' ({existing[:8]})")
        conn.execute(
            "UPDATE sessions SET status='closed', closed_at=?, updated_at=? WHERE session_id=?",
            (_now(), _now(), existing),
        )
        conn.commit()

    conn.execute("UPDATE sessions SET status='active', updated_at=? WHERE session_id=?", (_now(), sid))
    conn.commit()
    _set_active_id(conn, sid)
    print(f"Resumed session '{row['title']}' ({sid[:8]})")
    return 0


def cmd_list(args: argparse.Namespace) -> int:
    conn = _connect()
    limit = args.limit or 20
    rows = conn.execute(
        "SELECT s.*, COUNT(e.id) as event_count FROM sessions s "
        "LEFT JOIN events e ON s.session_id=e.session_id "
        "GROUP BY s.session_id ORDER BY s.updated_at DESC LIMIT ?",
        (limit,),
    ).fetchall()

    if not rows:
        print("No sessions found.")
        return 0

    active = _get_active_id(conn)
    print(f"\n{'ID':12}  {'STATUS':8}  {'EVENTS':6}  {'UPDATED':20}  TITLE")
    print("-" * 80)
    for r in rows:
        marker = "▶ " if r["session_id"] == active else "  "
        print(f"{marker}{r['session_id']:12}  {r['status']:8}  {r['event_count']:6}  {r['updated_at'][:19]:20}  {r['title']}")
    print()
    return 0


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main() -> int:
    p = argparse.ArgumentParser(
        prog="session_mgr",
        description="AI Session Manager — persist and restore work context across model conversations.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    sub = p.add_subparsers(dest="cmd", required=True)

    # start
    s = sub.add_parser("start", help="Start a new session")
    s.add_argument("title", help="Short description of this work session")
    s.add_argument("--goal", help="What we are trying to achieve")

    # event
    s = sub.add_parser("event", help="Log an event to the active session")
    s.add_argument("type", choices=sorted(VALID_EVENT_TYPES), help="Event type")
    s.add_argument("content", help="Event description")
    s.add_argument("--files", help="Comma-separated affected file paths", default="")

    # status
    s = sub.add_parser("status", help="Show active session state")
    s.add_argument("session_id", nargs="?", help="Session ID (default: active)")

    # handoff
    s = sub.add_parser("handoff", help="Generate handoff brief for the next model")

    # close
    s = sub.add_parser("close", help="Close the active session")
    s.add_argument("--summary", help="Final summary of what was done")

    # resume
    s = sub.add_parser("resume", help="Resume a previous session")
    s.add_argument("session_id", help="Session ID or prefix")

    # list
    s = sub.add_parser("list", help="List recent sessions")
    s.add_argument("--limit", type=int, default=20)

    args = p.parse_args()
    dispatch = {
        "start": cmd_start, "event": cmd_event, "status": cmd_status,
        "handoff": cmd_handoff, "close": cmd_close, "resume": cmd_resume,
        "list": cmd_list,
    }
    return dispatch[args.cmd](args)


if __name__ == "__main__":
    sys.exit(main())
