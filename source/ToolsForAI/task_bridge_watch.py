#!/usr/bin/env python3
"""Emit compact Codex inbox events for tasks waiting in review."""

from __future__ import annotations

import argparse
import json
import time
from pathlib import Path

import taskdb


INBOX_PATH = Path("project_tasks/tasking/codex_inbox_v1.jsonl")
STATE_DIR = Path(".project_task_bridge")
SEEN_PATH = STATE_DIR / "seen_review_tasks.json"


def _load_seen() -> set[str]:
    if not SEEN_PATH.exists():
        return set()
    return set(json.loads(SEEN_PATH.read_text(encoding="utf-8")))


def _save_seen(seen: set[str]) -> None:
    STATE_DIR.mkdir(parents=True, exist_ok=True)
    SEEN_PATH.write_text(json.dumps(sorted(seen), indent=2) + "\n", encoding="utf-8")


def emit_once() -> int:
    conn = taskdb.connect()
    seen = _load_seen()
    emitted = 0
    INBOX_PATH.parent.mkdir(parents=True, exist_ok=True)
    with INBOX_PATH.open("a", encoding="utf-8") as out:
        for row in taskdb.list_rows(conn, "review"):
            if row["task_id"] in seen:
                continue
            card = taskdb.card_from_row(row)
            event = {
                "schema_id": "project.codex_inbox_event.v1",
                "event": "task_review_ready",
                "task_id": card["task_id"],
                "runner": card["runner"],
                "topic": card["topic"],
                "allowed_writes": card.get("allowed_writes", []),
                "validation": card.get("validation", []),
                "updated_at": row["updated_at"],
            }
            out.write(json.dumps(event, ensure_ascii=False, sort_keys=True) + "\n")
            seen.add(row["task_id"])
            emitted += 1
    _save_seen(seen)
    print(f"INBOX_EMITTED: {emitted}")
    return emitted


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--once", action="store_true")
    parser.add_argument("--daemon", action="store_true")
    parser.add_argument("--interval", type=int, default=180)
    args = parser.parse_args()

    if args.daemon:
        while True:
            emit_once()
            time.sleep(args.interval)
    emit_once()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
