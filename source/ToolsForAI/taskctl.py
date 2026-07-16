#!/usr/bin/env python3
"""Task control CLI for project-local model-worker orchestration."""

from __future__ import annotations

import argparse
import fnmatch
import json
import subprocess
import sys
from pathlib import Path
from typing import Any

import taskdb


if sys.stdout.encoding and sys.stdout.encoding.lower() not in ("utf-8", "utf8"):
    sys.stdout = open(sys.stdout.fileno(), mode="w", encoding="utf-8", errors="replace", closefd=False)
if sys.stderr.encoding and sys.stderr.encoding.lower() not in ("utf-8", "utf8"):
    sys.stderr = open(sys.stderr.fileno(), mode="w", encoding="utf-8", errors="replace", closefd=False)


ROOT = Path(__file__).resolve().parents[1]


def _print_json(data: Any) -> None:
    print(json.dumps(data, ensure_ascii=False, indent=2))


def _load_card(path: str) -> dict[str, Any]:
    return taskdb.load_json(path)


def _contract(card: dict[str, Any]) -> dict[str, Any]:
    return {
        "schema_id": "project.worker_contract.v1",
        "task_id": card["task_id"],
        "runner": card["runner"],
        "topic": card["topic"],
        "status": card["status"],
        "worker_status": card["worker_status"],
        "mode": card.get("mode", ""),
        "human_brief": card.get("human_brief", {}),
        "objective": card["objective"],
        "read_first": card["read_first"],
        "allowed_writes": card["allowed_writes"],
        "forbidden": card["forbidden"],
        "acceptance": card["acceptance"],
        "validation": card["validation"],
        "handoff_note": card.get("handoff_note", "Return to Codex review."),
        "markdown_budget": card.get("markdown_budget", {"allow_markdown": False, "max_lines_total": 0}),
        "commit_contract": card.get("commit_contract", "NO_COMMIT preferred. Coordinator finalizes."),
        "rules": [
            "Work exactly one task.",
            "Read only this contract and read_first paths.",
            "Write only allowed_writes paths.",
            "Run listed validation.",
            "Finish with taskctl review; coordinator runs done.",
            "Never use git add -A, git add ., or mixed-task staging.",
            "Emit machine-readable JSON/JSONL artifacts instead of long markdown unless allowed.",
        ],
    }


def cmd_init_db(_args: argparse.Namespace) -> int:
    taskdb.ensure_initial_files()
    taskdb.connect().close()
    print(f"INIT: {taskdb.DEFAULT_DB}")
    return 0


def cmd_import_jsonl(_args: argparse.Namespace) -> int:
    taskdb.ensure_initial_files()
    conn = taskdb.connect()
    count = 0
    for card in taskdb.read_jsonl():
        taskdb.upsert_card(conn, card)
        count += 1
    print(f"IMPORTED: {count}")
    return 0


def cmd_export_jsonl(_args: argparse.Namespace) -> int:
    conn = taskdb.connect()
    cards = taskdb.all_cards(conn)
    taskdb.write_jsonl(cards)
    print(f"EXPORTED: {len(cards)}")
    return 0


def cmd_verify(_args: argparse.Namespace) -> int:
    errors: list[str] = []
    taskdb.ensure_initial_files()

    try:
        cards = taskdb.read_jsonl()
    except taskdb.TaskError as exc:
        errors.append(str(exc))
        cards = []

    seen: set[str] = set()
    for card in cards:
        task_id = str(card.get("task_id", "<unknown>"))
        if task_id in seen:
            errors.append(f"duplicate task_id in JSONL: {task_id}")
        seen.add(task_id)
        for err in taskdb.validate_card(card):
            errors.append(f"{task_id}: {err}")

    conn = taskdb.connect()
    db_ids = {row["task_id"] for row in conn.execute("SELECT task_id FROM tasks").fetchall()}
    jsonl_ids = {str(card.get("task_id", "")) for card in cards}
    if db_ids and db_ids != jsonl_ids:
        errors.append(f"db/jsonl mismatch: db_only={sorted(db_ids-jsonl_ids)} jsonl_only={sorted(jsonl_ids-db_ids)}")

    manifest = taskdb.manifest_for(cards)
    if taskdb.MANIFEST_PATH.exists():
        try:
            existing = json.loads(taskdb.MANIFEST_PATH.read_text(encoding="utf-8"))
            if existing.get("total_cards") != manifest["total_cards"]:
                errors.append("manifest total_cards mismatch")
            if existing.get("card_ids") != manifest["card_ids"]:
                errors.append("manifest card_ids mismatch")
        except json.JSONDecodeError as exc:
            errors.append(f"manifest invalid JSON: {exc}")

    if errors:
        print("VERIFY: FAIL")
        for err in errors:
            print(f"- {err}")
        return 1
    print("VERIFY: PASS")
    return 0


def cmd_add_card(args: argparse.Namespace) -> int:
    conn = taskdb.connect()
    card = _load_card(args.path)
    taskdb.upsert_card(conn, card)
    taskdb.write_jsonl(taskdb.all_cards(conn))
    print(f"ADDED: {card['task_id']}")
    return 0


def cmd_list(args: argparse.Namespace) -> int:
    conn = taskdb.connect()
    rows = taskdb.list_rows(conn, args.status, args.limit)
    for row in rows:
        print(f"{row['task_id']}\t{row['status']}\t{row['worker_status']}\t{row['runner']}\t{row['topic']}\t{row['priority']}\t{row['queue_order']}")
    if not rows:
        print("(no tasks)")
    return 0


def cmd_review_queue(_args: argparse.Namespace) -> int:
    conn = taskdb.connect()
    rows = taskdb.list_rows(conn, "review")
    for row in rows:
        print(f"{row['task_id']}\t{row['runner']}\t{row['topic']}\t{row['updated_at']}")
    if not rows:
        print("(review queue empty)")
    return 0


def cmd_show(args: argparse.Namespace) -> int:
    conn = taskdb.connect()
    _print_json(taskdb.get_card(conn, args.task_id))
    return 0


def cmd_auto_pickup(args: argparse.Namespace) -> int:
    conn = taskdb.connect()
    existing = taskdb.processing_rows_for_runner(conn, args.runner)
    if existing:
        ids = ", ".join(row["task_id"] for row in existing)
        print(f"WARNING runner_has_processing: {ids}")
    card = taskdb.claim_next(conn, args.runner, args.topic)
    if not card:
        print("NO_TASK")
        return 0
    taskdb.write_jsonl(taskdb.all_cards(conn))
    print(f"CLAIMED {card['task_id']}")
    _print_json(_contract(card))
    return 0


def cmd_contract(args: argparse.Namespace) -> int:
    conn = taskdb.connect()
    card = taskdb.get_card(conn, args.task_id)
    if card["worker_status"] == "claimed":
        card["worker_status"] = "in_progress"
        taskdb.update_card(conn, card)
        taskdb.write_jsonl(taskdb.all_cards(conn))
    _print_json(_contract(card))
    return 0


def cmd_review(args: argparse.Namespace) -> int:
    conn = taskdb.connect()
    card = taskdb.get_card(conn, args.task_id)
    if args.runner and card["runner"] != args.runner:
        raise taskdb.TaskError(f"runner mismatch: card={card['runner']} arg={args.runner}")
    card["status"] = "review"
    card["worker_status"] = "review"
    notes = args.notes or ""
    taskdb.update_card(conn, card, claimed_by=args.runner or card["runner"], review_notes=notes)
    taskdb.write_jsonl(taskdb.all_cards(conn))
    print(f"REVIEW: {card['task_id']}")
    return 0


def cmd_done(args: argparse.Namespace) -> int:
    conn = taskdb.connect()
    card = taskdb.get_card(conn, args.task_id)
    card["status"] = "finished"
    card["worker_status"] = "done"
    taskdb.update_card(conn, card, finished_at=taskdb.now())
    taskdb.write_jsonl(taskdb.all_cards(conn))
    print(f"DONE: {card['task_id']}")
    return 0


def cmd_defer(args: argparse.Namespace) -> int:
    conn = taskdb.connect()
    card = taskdb.get_card(conn, args.task_id)
    card["status"] = "future_backlog"
    card["worker_status"] = "blocked"
    taskdb.update_card(conn, card, review_notes=args.reason)
    taskdb.write_jsonl(taskdb.all_cards(conn))
    print(f"DEFERRED: {card['task_id']}")
    return 0


def cmd_blocked(args: argparse.Namespace) -> int:
    conn = taskdb.connect()
    if not args.task_id:
        rows = taskdb.list_rows(conn, "blocked")
        data_rows = taskdb.list_rows(conn, "blocked_data")
        for row in [*rows, *data_rows]:
            print(f"{row['task_id']}\t{row['status']}\t{row['worker_status']}\t{row['runner']}\t{row['topic']}\t{row['updated_at']}")
        if not rows and not data_rows:
            print("(blocked queue empty)")
        return 0
    if not args.reason:
        raise taskdb.TaskError("--reason is required when marking a task blocked")
    card = taskdb.get_card(conn, args.task_id)
    card["status"] = "blocked"
    card["worker_status"] = "blocked"
    taskdb.update_card(conn, card, review_notes=args.reason)
    taskdb.write_jsonl(taskdb.all_cards(conn))
    print(f"BLOCKED: {card['task_id']}")
    return 0


def _git_lines(args: list[str]) -> list[str]:
    result = subprocess.run(["git", *args], cwd=ROOT, capture_output=True, text=True, encoding="utf-8", errors="replace")
    if result.returncode != 0:
        raise taskdb.TaskError(result.stderr.strip() or "git command failed")
    return [line.strip() for line in result.stdout.splitlines() if line.strip()]


def _allowed_set(card: dict[str, Any]) -> set[str]:
    return {str(Path(p).as_posix()).strip("/") for p in card.get("allowed_writes", [])}


def _assert_allowed(card: dict[str, Any], paths: list[str]) -> None:
    allowed = _allowed_set(card)
    bad = []
    for path in paths:
        rel = path.replace("\\", "/").strip("/")
        if not _path_allowed(rel, allowed):
            bad.append(rel)
    if bad:
        raise taskdb.TaskError("paths outside allowed_writes: " + ", ".join(bad))


def _path_allowed(rel: str, allowed: set[str]) -> bool:
    for pattern in allowed:
        if rel == pattern:
            return True
        if pattern.endswith("/**") and rel.startswith(pattern[:-3].rstrip("/") + "/"):
            return True
        if fnmatch.fnmatch(rel, pattern):
            return True
    return False


def cmd_stage(args: argparse.Namespace) -> int:
    conn = taskdb.connect()
    card = taskdb.get_card(conn, args.task_id)
    allowed = sorted(_allowed_set(card))
    if not allowed:
        raise taskdb.TaskError("allowed_writes is empty; refusing to stage")
    subprocess.run(["git", "add", "--", *allowed], cwd=ROOT, check=True)
    cmd_guard_staged(args)
    print(f"STAGED: {card['task_id']}")
    return 0


def cmd_guard_staged(args: argparse.Namespace) -> int:
    conn = taskdb.connect()
    card = taskdb.get_card(conn, args.task_id)
    staged = _git_lines(["diff", "--cached", "--name-only"])
    _assert_allowed(card, staged)
    print("GUARD-STAGED: PASS")
    return 0


def cmd_owner_brief(args: argparse.Namespace) -> int:
    conn = taskdb.connect()
    card = taskdb.get_card(conn, args.task_id)
    print(f"{card['task_id']} [{card['runner']}/{card['topic']}] {card['priority']}")
    brief = card.get("human_brief") or {}
    if brief:
        print(f"intent: {brief.get('intent', '')}")
        print(f"risk: {brief.get('risk', '')}")
        print(f"expected_win: {brief.get('expected_win', '')}")
        print(f"reviewer_focus: {brief.get('reviewer_focus', '')}")
    print(card["objective"])
    print("allowed_writes:")
    for path in card["allowed_writes"]:
        print(f"- {path}")
    return 0


def cmd_collision_guard(args: argparse.Namespace) -> int:
    conn = taskdb.connect()
    collisions = taskdb.allowed_write_collisions(conn)
    if collisions:
        print("COLLISION-GUARD: FAIL")
        for collision in collisions:
            print(f"- {collision['path']}: {', '.join(collision['task_ids'])}")
        return 1
    if args.print:
        active = taskdb.active_rows(conn)
        print(f"COLLISION-GUARD: PASS active_tasks={len(active)}")
    else:
        print("COLLISION-GUARD: PASS")
    return 0


def cmd_usage(args: argparse.Namespace) -> int:
    conn = taskdb.connect()
    taskdb.get_card(conn, args.task_id)
    taskdb.record_usage(
        conn,
        args.task_id,
        args.runner,
        args.model,
        args.input_tokens,
        args.output_tokens,
        args.cost_usd,
    )
    print(f"USAGE: {args.task_id}\t{args.runner}\t{args.model}")
    return 0


def cmd_usage_report(_args: argparse.Namespace) -> int:
    conn = taskdb.connect()
    rows = taskdb.usage_rows(conn)
    if not rows:
        print("(usage report empty)")
        return 0
    for row in rows:
        print(
            f"{row['runner']}\t{row['model']}\tentries={row['entries']}\t"
            f"input={row['input_tokens'] or 0}\toutput={row['output_tokens'] or 0}\tcost={row['cost_usd'] or 0:.6f}"
        )
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(prog="taskctl")
    sub = parser.add_subparsers(dest="cmd", required=True)

    sub.add_parser("init-db")
    sub.add_parser("import-jsonl")
    sub.add_parser("export-jsonl")
    sub.add_parser("verify")

    p = sub.add_parser("add-card")
    p.add_argument("path")

    p = sub.add_parser("list")
    p.add_argument("--status")
    p.add_argument("--limit", type=int)

    sub.add_parser("review-queue")

    p = sub.add_parser("show")
    p.add_argument("task_id")

    p = sub.add_parser("auto-pickup")
    p.add_argument("--runner", required=True)
    p.add_argument("--topic", required=True)

    p = sub.add_parser("contract")
    p.add_argument("task_id")

    p = sub.add_parser("review")
    p.add_argument("task_id")
    p.add_argument("--runner", default="")
    p.add_argument("--notes", default="")

    p = sub.add_parser("done")
    p.add_argument("task_id")

    p = sub.add_parser("defer")
    p.add_argument("task_id")
    p.add_argument("--reason", required=True)

    p = sub.add_parser("blocked")
    p.add_argument("task_id", nargs="?")
    p.add_argument("--reason", default="")

    p = sub.add_parser("stage")
    p.add_argument("task_id")

    p = sub.add_parser("guard-staged")
    p.add_argument("task_id")

    p = sub.add_parser("owner-brief")
    p.add_argument("task_id")

    p = sub.add_parser("brief")
    p.add_argument("task_id")

    p = sub.add_parser("collision-guard")
    p.add_argument("--print", action="store_true")

    p = sub.add_parser("usage")
    p.add_argument("task_id")
    p.add_argument("--runner", required=True)
    p.add_argument("--model", required=True)
    p.add_argument("--input-tokens", type=int, required=True)
    p.add_argument("--output-tokens", type=int, required=True)
    p.add_argument("--cost-usd", type=float, required=True)

    sub.add_parser("usage-report")

    args = parser.parse_args()
    dispatch = {
        "init-db": cmd_init_db,
        "import-jsonl": cmd_import_jsonl,
        "export-jsonl": cmd_export_jsonl,
        "verify": cmd_verify,
        "add-card": cmd_add_card,
        "list": cmd_list,
        "review-queue": cmd_review_queue,
        "show": cmd_show,
        "auto-pickup": cmd_auto_pickup,
        "contract": cmd_contract,
        "review": cmd_review,
        "done": cmd_done,
        "defer": cmd_defer,
        "blocked": cmd_blocked,
        "stage": cmd_stage,
        "guard-staged": cmd_guard_staged,
        "owner-brief": cmd_owner_brief,
        "brief": cmd_owner_brief,
        "collision-guard": cmd_collision_guard,
        "usage": cmd_usage,
        "usage-report": cmd_usage_report,
    }
    try:
        return dispatch[args.cmd](args)
    except taskdb.TaskError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
