# Project Rules — STRICT RULES (NO EXCEPTIONS)

## Use The Local AI Tools

This repository uses the same agent workflow style as the DevEmu server, adapted
for Medicon paths. The tools live in `ToolsForAI/`, not `tools/` or `AITools/`.

Required commands:

| Goal | Command |
|------|---------|
| Build/update index | `python ToolsForAI/source_graph.py build -i` |
| Project overview | `python ToolsForAI/source_graph.py summary` |
| Find symbol/function/class | `python ToolsForAI/source_graph.py func <name>` / `class <name>` |
| Find file by pattern | `python ToolsForAI/source_graph.py file <pattern>` |
| Search indexed text | `python ToolsForAI/source_graph.py find <term>` |
| Inspect file context | `python ToolsForAI/source_graph.py context <file>` |
| Trace/impact | `python ToolsForAI/source_graph.py trace <term>` / `impact <term>` |
| Tests/coverage signals | `python ToolsForAI/source_graph.py testmap <term>` / `coverage <term>` |
| Risk scans | `python ToolsForAI/source_graph.py crashes` / `nullrisks` / `reviewqueue` |
| Memory | `python ToolsForAI/ai_memory.py recent --limit 10` / `context <topic>` |
| Session | `python ToolsForAI/session_mgr.py list` / `status` / `start` / `event` |
| Worker tasks | `python ToolsForAI/taskctl.py ...` |

## Mandatory Startup

At the beginning of a code investigation or review:

```powershell
python ToolsForAI/ai_memory.py recent --limit 10
python ToolsForAI/ai_memory.py context "<topic>"
python ToolsForAI/source_graph.py build -i
python ToolsForAI/source_graph.py summary
python ToolsForAI/session_mgr.py list
python ToolsForAI/session_mgr.py status
```

Start a session for non-trivial work:

```powershell
python ToolsForAI/session_mgr.py start "<title>" --goal "<goal>"
```

Log significant findings, decisions, progress, blockers, and false positives
immediately with `session_mgr.py event`.

## Source Graph First

Use `source_graph.py` before broad shell discovery for project code. The graph is
the first-pass navigation layer for:

- functions, methods, classes, structs, enums, defines
- protobuf/gRPC flow
- SQL usage and applet-driven behavior
- Qt UI/controller/model paths
- tests and coverage
- change-impact and review-risk scans

If results look stale:

```powershell
python ToolsForAI/source_graph.py build
```

Then retry the graph query.

## Medicon-Specific Review Focus

- Backend: SQLAPI wrappers, SQL applets, PostgreSQL connection/query behavior,
  gRPC provider service, app-data consistency.
- Frontend: Qt widgets, table models, form/navigation controllers, generated
  gRPC client code, `.ui` files, integration tests.
- Shared/core: config parsing, include utilities, JSON parameter formatting,
  XML/Markup parsing, generated protobuf surface.
- Assets/config: `assets/app-data/provider/sql-applets/` should match backend
  expectations.

For DB or gRPC changes, inspect source, generated interface, app-data/config,
and tests together before editing.

## Completion Checklist

- Relevant source graph queries were used.
- Durable project facts were stored in `ai_memory`.
- Session state was updated or a handoff was generated for larger tasks.
- `python ToolsForAI/source_graph.py build -i` was run after meaningful changes.
- Tests were reviewed, updated, or explicitly noted as not run/not applicable.

## Git And Scratch Rules

- Do not commit or push unless the user explicitly asks.
- Do not use root-level temporary files.
- Put disposable outputs under `_scratch/`.
- Do not stage unrelated files.

## Model Worker Task System

Worker contracts are coordinated through:

```powershell
python ToolsForAI/taskctl.py auto-pickup --runner <runner> --topic <topic>
python ToolsForAI/taskctl.py contract <TASK_ID>
python ToolsForAI/taskctl.py review <TASK_ID> --runner <runner> --notes "<summary>"
python ToolsForAI/taskctl.py guard-staged <TASK_ID>
python ToolsForAI/taskctl.py done <TASK_ID>
python ToolsForAI/taskctl.py verify
```

External worker copy block:

```text
გაუშვი პარალელური სუბაგენტები და რომ დასრულდება დაუბრუნე კოდექსს რევიუსთვის
```
