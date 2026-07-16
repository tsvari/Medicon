#!/usr/bin/env python3
"""SQLite-backed task queue for model-worker orchestration."""

from __future__ import annotations

import json
import os
import sqlite3
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
TASKING_DIR = ROOT / "project_tasks" / "tasking"

CARDS_PATH = Path(os.environ.get("PROJECT_TASK_CARDS_PATH", TASKING_DIR / "machine_task_cards_v1.jsonl"))
MANIFEST_PATH = Path(os.environ.get("PROJECT_TASK_CARDS_MANIFEST", TASKING_DIR / "machine_task_cards_manifest_v1.json"))
DEFAULT_DB = Path(os.environ.get("PROJECT_TASK_QUEUE_DB", TASKING_DIR / "task_queue_v1.sqlite"))
PROTOCOL_PATH = Path(os.environ.get("PROJECT_TASK_WORKER_PROTOCOL", TASKING_DIR / "worker_low_token_protocol_v1.json"))

VALID_STATUS = {"pending", "processing", "review", "finished", "blocked", "future_backlog", "blocked_data"}
ACTIVE_STATUS = {"pending", "processing", "review", "blocked", "blocked_data"}
VALID_WORKER_STATUS = {"unclaimed", "claimed", "in_progress", "review", "done", "blocked"}
REQUIRED_FIELDS = {
    "schema_id",
    "task_id",
    "runner",
    "topic",
    "status",
    "worker_status",
    "priority",
    "queue_order",
    "objective",
    "read_first",
    "allowed_writes",
    "forbidden",
    "acceptance",
    "validation",
}


SCHEMA = """
CREATE TABLE IF NOT EXISTS tasks (
    task_id       TEXT PRIMARY KEY,
    runner        TEXT NOT NULL,
    topic         TEXT NOT NULL,
    status        TEXT NOT NULL,
    worker_status TEXT NOT NULL,
    priority      TEXT NOT NULL,
    queue_order   INTEGER NOT NULL,
    card_json     TEXT NOT NULL,
    claimed_by    TEXT DEFAULT '',
    review_notes  TEXT DEFAULT '',
    created_at    TEXT NOT NULL,
    updated_at    TEXT NOT NULL,
    finished_at   TEXT DEFAULT ''
);

CREATE INDEX IF NOT EXISTS idx_tasks_status ON tasks(status, worker_status);
CREATE INDEX IF NOT EXISTS idx_tasks_runner_topic ON tasks(runner, topic, status);

CREATE TABLE IF NOT EXISTS task_usage (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    task_id       TEXT NOT NULL,
    runner        TEXT NOT NULL,
    model         TEXT NOT NULL,
    input_tokens  INTEGER NOT NULL DEFAULT 0,
    output_tokens INTEGER NOT NULL DEFAULT 0,
    cost_usd      REAL NOT NULL DEFAULT 0,
    created_at    TEXT NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_task_usage_task ON task_usage(task_id);
CREATE INDEX IF NOT EXISTS idx_task_usage_runner ON task_usage(runner, model);
"""


class TaskError(RuntimeError):
    pass


def now() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="seconds")


def connect(db_path: Path = DEFAULT_DB) -> sqlite3.Connection:
    db_path.parent.mkdir(parents=True, exist_ok=True)
    conn = sqlite3.connect(str(db_path), timeout=15)
    conn.row_factory = sqlite3.Row
    conn.execute("PRAGMA journal_mode=WAL")
    conn.executescript(SCHEMA)
    return conn


def normalize_path(path: str | Path) -> str:
    return Path(path).as_posix()


def load_json(path: str | Path) -> dict[str, Any]:
    return json.loads(Path(path).read_text(encoding="utf-8"))


def read_jsonl(path: Path = CARDS_PATH) -> list[dict[str, Any]]:
    if not path.exists():
        return []
    cards: list[dict[str, Any]] = []
    for line_no, raw in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        line = raw.strip()
        if not line:
            continue
        try:
            cards.append(json.loads(line))
        except json.JSONDecodeError as exc:
            raise TaskError(f"{path}:{line_no}: invalid JSONL: {exc}") from exc
    return cards


def validate_card(card: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    missing = sorted(REQUIRED_FIELDS - set(card))
    if missing:
        errors.append(f"missing fields: {', '.join(missing)}")
    task_id = str(card.get("task_id", ""))
    if not task_id:
        errors.append("task_id is empty")
    if card.get("status") not in VALID_STATUS:
        errors.append(f"invalid status: {card.get('status')}")
    if card.get("worker_status") not in VALID_WORKER_STATUS:
        errors.append(f"invalid worker_status: {card.get('worker_status')}")
    for key in ("read_first", "allowed_writes", "forbidden", "acceptance", "validation", "handoff_prompt_lines"):
        if key in card and not isinstance(card[key], list):
            errors.append(f"{key} must be a list")
    if "human_brief" in card and not isinstance(card["human_brief"], dict):
        errors.append("human_brief must be an object")
    if "markdown_budget" in card and not isinstance(card["markdown_budget"], dict):
        errors.append("markdown_budget must be an object")
    if "allowed_writes" in card:
        for item in card["allowed_writes"]:
            p = str(item).replace("\\", "/")
            parts = Path(p).parts
            if not p or p == "." or not parts:
                errors.append(f"allowed_writes path is empty: {item}")
            elif p.startswith("/") or ":" in parts[0]:
                errors.append(f"allowed_writes must be repo-relative: {item}")
    return errors


def card_from_row(row: sqlite3.Row) -> dict[str, Any]:
    return json.loads(row["card_json"])


def upsert_card(conn: sqlite3.Connection, card: dict[str, Any]) -> None:
    errors = validate_card(card)
    if errors:
        raise TaskError(f"{card.get('task_id', '<unknown>')}: " + "; ".join(errors))
    current = now()
    existing = conn.execute("SELECT created_at FROM tasks WHERE task_id=?", (card["task_id"],)).fetchone()
    created = existing["created_at"] if existing else current
    conn.execute(
        """
        INSERT OR REPLACE INTO tasks(
            task_id, runner, topic, status, worker_status, priority, queue_order,
            card_json, claimed_by, review_notes, created_at, updated_at, finished_at
        ) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?)
        """,
        (
            card["task_id"],
            card["runner"],
            card["topic"],
            card["status"],
            card["worker_status"],
            card["priority"],
            int(card["queue_order"]),
            json.dumps(card, ensure_ascii=False, sort_keys=True, separators=(",", ":")),
            "",
            "",
            created,
            current,
            "",
        ),
    )
    conn.commit()


def update_card(conn: sqlite3.Connection, card: dict[str, Any], **meta: str) -> None:
    errors = validate_card(card)
    if errors:
        raise TaskError(f"{card.get('task_id', '<unknown>')}: " + "; ".join(errors))
    row = conn.execute("SELECT created_at, claimed_by, review_notes, finished_at FROM tasks WHERE task_id=?", (card["task_id"],)).fetchone()
    if not row:
        raise TaskError(f"task not found: {card['task_id']}")
    conn.execute(
        """
        UPDATE tasks
        SET runner=?, topic=?, status=?, worker_status=?, priority=?, queue_order=?,
            card_json=?, claimed_by=?, review_notes=?, updated_at=?, finished_at=?
        WHERE task_id=?
        """,
        (
            card["runner"],
            card["topic"],
            card["status"],
            card["worker_status"],
            card["priority"],
            int(card["queue_order"]),
            json.dumps(card, ensure_ascii=False, sort_keys=True, separators=(",", ":")),
            meta.get("claimed_by", row["claimed_by"]),
            meta.get("review_notes", row["review_notes"]),
            now(),
            meta.get("finished_at", row["finished_at"]),
            card["task_id"],
        ),
    )
    conn.commit()


def get_card(conn: sqlite3.Connection, task_id: str) -> dict[str, Any]:
    row = conn.execute("SELECT card_json FROM tasks WHERE task_id=?", (task_id,)).fetchone()
    if not row:
        rows = conn.execute("SELECT task_id, card_json FROM tasks WHERE task_id LIKE ?", (task_id + "%",)).fetchall()
        if len(rows) == 1:
            return json.loads(rows[0]["card_json"])
        if len(rows) > 1:
            raise TaskError("ambiguous task id prefix: " + ", ".join(r["task_id"] for r in rows))
        raise TaskError(f"task not found: {task_id}")
    return json.loads(row["card_json"])


def list_rows(conn: sqlite3.Connection, status: str | None = None, limit: int | None = None) -> list[sqlite3.Row]:
    limit_clause = ""
    params: list[Any] = []
    if limit is not None:
        if limit <= 0:
            raise TaskError("limit must be positive")
        limit_clause = " LIMIT ?"
        params.append(limit)
    if status:
        params.insert(0, status)
        return conn.execute(
            "SELECT * FROM tasks WHERE status=? ORDER BY queue_order, task_id" + limit_clause,
            params,
        ).fetchall()
    return conn.execute("SELECT * FROM tasks ORDER BY status, queue_order, task_id" + limit_clause, params).fetchall()


def active_rows(conn: sqlite3.Connection) -> list[sqlite3.Row]:
    placeholders = ",".join("?" for _ in ACTIVE_STATUS)
    return conn.execute(
        f"SELECT * FROM tasks WHERE status IN ({placeholders}) ORDER BY queue_order, task_id",
        sorted(ACTIVE_STATUS),
    ).fetchall()


def processing_rows_for_runner(conn: sqlite3.Connection, runner: str) -> list[sqlite3.Row]:
    return conn.execute(
        """
        SELECT * FROM tasks
        WHERE runner=? AND status='processing'
        ORDER BY queue_order, task_id
        """,
        (runner,),
    ).fetchall()


def claim_next(conn: sqlite3.Connection, runner: str, topic: str) -> dict[str, Any] | None:
    row = conn.execute(
        """
        SELECT * FROM tasks
        WHERE status='pending' AND worker_status='unclaimed' AND runner=? AND topic=?
        ORDER BY queue_order, task_id
        LIMIT 1
        """,
        (runner, topic),
    ).fetchone()
    if not row:
        return None
    card = card_from_row(row)
    current = now()
    card["status"] = "processing"
    card["worker_status"] = "in_progress"
    card["claimed_by"] = runner
    card["claimed_at"] = card.get("claimed_at") or current
    card["started_at"] = card.get("started_at") or current
    update_card(conn, card, claimed_by=runner)
    return card


def allowed_write_collisions(conn: sqlite3.Connection) -> list[dict[str, Any]]:
    owners: dict[str, list[str]] = {}
    for row in active_rows(conn):
        card = card_from_row(row)
        task_id = card["task_id"]
        for raw in card.get("allowed_writes", []):
            path = normalize_path(str(raw)).strip("/")
            if not path:
                continue
            owners.setdefault(path, []).append(task_id)
    collisions: list[dict[str, Any]] = []
    for path, task_ids in sorted(owners.items()):
        unique_ids = sorted(set(task_ids))
        if len(unique_ids) > 1:
            collisions.append({"path": path, "task_ids": unique_ids})
    return collisions


def record_usage(
    conn: sqlite3.Connection,
    task_id: str,
    runner: str,
    model: str,
    input_tokens: int,
    output_tokens: int,
    cost_usd: float,
) -> None:
    if input_tokens < 0 or output_tokens < 0 or cost_usd < 0:
        raise TaskError("usage values must be non-negative")
    conn.execute(
        """
        INSERT INTO task_usage(task_id, runner, model, input_tokens, output_tokens, cost_usd, created_at)
        VALUES(?,?,?,?,?,?,?)
        """,
        (task_id, runner, model, int(input_tokens), int(output_tokens), float(cost_usd), now()),
    )
    conn.commit()


def usage_rows(conn: sqlite3.Connection) -> list[sqlite3.Row]:
    return conn.execute(
        """
        SELECT
            runner,
            model,
            COUNT(*) AS entries,
            SUM(input_tokens) AS input_tokens,
            SUM(output_tokens) AS output_tokens,
            SUM(cost_usd) AS cost_usd
        FROM task_usage
        GROUP BY runner, model
        ORDER BY runner, model
        """
    ).fetchall()


def all_cards(conn: sqlite3.Connection) -> list[dict[str, Any]]:
    rows = conn.execute("SELECT card_json FROM tasks ORDER BY queue_order, task_id").fetchall()
    return [json.loads(row["card_json"]) for row in rows]


def manifest_for(cards: list[dict[str, Any]]) -> dict[str, Any]:
    status_counts: dict[str, int] = {}
    by_runner: dict[str, int] = {}
    ids: list[str] = []
    for card in cards:
        status_counts[card["status"]] = status_counts.get(card["status"], 0) + 1
        by_runner[card["runner"]] = by_runner.get(card["runner"], 0) + 1
        ids.append(card["task_id"])
    return {
        "schema_id": "machine_task_cards_manifest_v1",
        "total_cards": len(cards),
        "status_counts": status_counts,
        "by_runner": by_runner,
        "card_ids": ids,
    }


def write_jsonl(cards: list[dict[str, Any]], path: Path = CARDS_PATH) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    lines = [json.dumps(card, ensure_ascii=False, sort_keys=True, separators=(",", ":")) for card in cards]
    path.write_text("\n".join(lines) + ("\n" if lines else ""), encoding="utf-8")
    MANIFEST_PATH.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST_PATH.write_text(json.dumps(manifest_for(cards), ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def ensure_initial_files() -> None:
    TASKING_DIR.mkdir(parents=True, exist_ok=True)
    CARDS_PATH.touch(exist_ok=True)
    if not MANIFEST_PATH.exists():
        MANIFEST_PATH.write_text(
            json.dumps(manifest_for([]), ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
