# AGENTS.md — STRICT RULES (NO EXCEPTIONS)

## Project

Medicon is a C++/Qt/gRPC/PostgreSQL application. Treat `source/` as the primary
code tree and `assets/app-data/` as the local application data/config mirror.

---

## 🖥️ PLATFORM — Python Invocation (READ FIRST)

All ToolsForAI commands are run **from the repository root** with the
platform's Python binary. The two machines use different binaries:

| Machine | Python binary | Example |
|---------|--------------|---------|
| **Linux** | `python3` | `python3 source/ToolsForAI/source_graph.py summary` |
| **Windows** | `python` (or `py -3`) | `python source/ToolsForAI/source_graph.py summary` |

All command examples below use the **Linux form** (`python3`). On Windows,
replace `python3` with `python` and keep the rest of the command identical.
The path `source/ToolsForAI/...` is the same on both machines.

> 💡 **Working directory:** examples assume the **repository root**
> (e.g. `C:/projects/Medicon` / `~/Medicon`). If your shell is already inside
> `source/` (the VS Code workspace folder), DROP the `source/` prefix and use
> `ToolsForAI/...` instead — do NOT use `source/ToolsForAI/...` from there.

---

## ⚠️ ZERO-TOLERANCE RULES

These rules are **ABSOLUTE**. Violation voids all trust in the agent.

1. **SOURCE GRAPH IS THE ONLY NAVIGATION TOOL.** You MUST use `source_graph.py`
   for ALL project file/symbol/code lookups. You are FORBIDDEN from using
   direct `grep_search`, `file_search`, `semantic_search`, or raw `Get-ChildItem`
   for project code discovery. The ONLY exception is when `source_graph.py build`
   fails or does not exist — and even then, you must note the failure in a
   session event first.

2. **STARTUP SEQUENCE IS MANDATORY.** Before ANY investigation or code change,
   you SHALL execute the full startup block below. NO SKIPPING.

3. **SESSION MANAGER IS THE HANDOFF PROTOCOL.** Every non-trivial task REQUIRES
   a session. Log ALL progress, decisions, findings, false positives, and
   blockers as they happen — not at the end. The session handoff is the ONLY
   way other models can continue work without starting from zero.

4. **AI MEMORY IS THE PERSISTENT BRAIN.** Store ALL durable facts — decisions,
   patterns, config gotchas, bug locations, architecture notes — in `ai_memory`.
   This is your permanent working memory. Other agents read it to avoid
   repeating mistakes.

5. **DEVIATION REQUIREMENTS.** If you believe a deviation from these rules is
   absolutely necessary, you MUST:
   a. Log the reason as a session event (`event decision`)
   b. Ask the user for explicit permission
   c. Only proceed after receiving clear approval

---

## 🔧 Mandatory AI Tooling

All agent work in this repository MUST use the local tools under `ToolsForAI/`.
Using any alternative approach (raw shell, grep, manual search) for project
navigation is FORBIDDEN.

| Tool | Command | Purpose |
|------|---------|---------|
| source graph | `python3 source/ToolsForAI/source_graph.py` | **ONLY** allowed project navigation tool |
| ai memory | `python3 source/ToolsForAI/ai_memory.py` | Agent's persistent working memory |
| session manager | `python3 source/ToolsForAI/session_mgr.py` | Cross-model session state and handoff |
| task cards | `python3 source/ToolsForAI/taskctl.py` | Multi-model task queue and worker contracts |

> NOTE: run all commands from the repo root; `source/ToolsForAI/` is the same
> path on both Linux and Windows. Only the Python binary differs (see the
> PLATFORM section above: Linux `python3` / Windows `python`).

---

## 🚀 STRICT STARTUP SEQUENCE

This sequence SHALL be executed at the BEGINNING of every investigation or
code change. NOT OPTIONAL.

```bash
python3 source/ToolsForAI/ai_memory.py recent --limit 10
python3 source/ToolsForAI/ai_memory.py context "<topic>"
python3 source/ToolsForAI/source_graph.py build -i
python3 source/ToolsForAI/source_graph.py summary
python3 source/ToolsForAI/session_mgr.py list
python3 source/ToolsForAI/session_mgr.py status
```

### Session Creation Rule

If there is no active session AND the task is more than a quick read-only
check (e.g., any code change, multi-step investigation, debugging), you MUST
create one:

```bash
python3 source/ToolsForAI/session_mgr.py start "<short title>" --goal "<goal>"
```

### Event Logging Rule (LIVE, NOT BATCH)

You SHALL log events **as they happen**, NOT at the end. Every meaningful
step requires an event:

```bash
python3 source/ToolsForAI/session_mgr.py event progress "<what changed>" --files source/path.cpp
python3 source/ToolsForAI/session_mgr.py event decision "<decision and reason>"
python3 source/ToolsForAI/session_mgr.py event finding "<bug or risk found>" --files source/path.cpp
python3 source/ToolsForAI/session_mgr.py event falsepos "<finding ruled out and why>"
python3 source/ToolsForAI/session_mgr.py event blocker "<what blocks progress>"
```

---

## 🗺️ SOURCE GRAPH — THE ONLY NAVIGATION TOOL

**YOU ARE FORBIDDEN** from using agent `grep`/`glob` tools (e.g. Continue's
`grep_search`/`file_search`, Zoo Code's `grep`/`glob`, or any other),
`Get-ChildItem`, `Select-String`, or any other direct filesystem search for
project code discovery. Source graph is the **SOLE** navigation layer.

### Allowed Commands

```bash
python3 source/ToolsForAI/source_graph.py func <name>       # Find function/method
python3 source/ToolsForAI/source_graph.py class <name>       # Find class/struct/enum
python3 source/ToolsForAI/source_graph.py file <pattern>     # Find file by name
python3 source/ToolsForAI/source_graph.py find <term>        # Search indexed text
python3 source/ToolsForAI/source_graph.py context <file>     # File context overview
python3 source/ToolsForAI/source_graph.py trace <term>       # Trace call flow
python3 source/ToolsForAI/source_graph.py impact <term>      # Change impact analysis
python3 source/ToolsForAI/source_graph.py testmap <term>     # Find related tests
python3 source/ToolsForAI/source_graph.py coverage <term>    # Coverage signals
python3 source/ToolsForAI/source_graph.py reviewqueue <term> # Review queue
python3 source/ToolsForAI/source_graph.py crashes            # Crash patterns
python3 source/ToolsForAI/source_graph.py nullrisks          # Null safety risks
python3 source/ToolsForAI/source_graph.py stats              # Index statistics
```

### Stale Index Recovery

If the graph returns stale or missing results, rebuild it ONCE:

```bash
python3 source/ToolsForAI/source_graph.py build
```

Then retry the graph query. If it STILL fails, log a session event and ONLY
then fall back to `read_file` with a known path. Do NOT use grep/find/search.

---

## 🧠 AI MEMORY — PERMANENT WORKING MEMORY

`ai_memory` is your **persistent brain**. Other agents (including different
model types) read it. Use it liberally.

### When to Store

- **Decisions**: Why you chose approach X over Y
- **Patterns**: Repeating code structures, conventions, idioms
- **Config**: Non-obvious config keys, flags, paths
- **Gotchas**: Bugs you found, things to watch out for
- **Architecture**: Component relationships, data flow notes
- **Testing**: Test patterns, known test gaps

### When NOT to Store

- Temporary/obvious state — that belongs in session events
- Information that changes every session

### Storage Command

```bash
python3 source/ToolsForAI/ai_memory.py store "<key>" "<finding>" --tag decision,pattern,config
```

---

## 📋 SESSION MANAGER — CROSS-MODEL HANDSHAKE

Session manager exists so that **any model, any provider, any agent** can pick
up where another left off. You SHALL keep it populated.

### Completion / Handoff Rule

Before finishing work, you MUST execute:

```bash
python3 source/ToolsForAI/ai_memory.py store "<key>" "<finding>" --tag decision,pattern,config
python3 source/ToolsForAI/source_graph.py build -i
python3 source/ToolsForAI/session_mgr.py handoff
```

The handoff is what lets the next agent continue seamlessly. Without it, the
next agent starts blind.

---

## 📝 Review And Change Rules

- Keep edits scoped to the confirmed subsystem.
- Prefer existing CMake, Qt, gRPC, SQLAPI, and local helper patterns.
- For database behavior, inspect BOTH the SQL/query code AND the app-data SQL
  applet or config file that drives it.
- For protobuf/gRPC behavior, inspect ALL of: `.proto` file, generated C++
  surface, server implementation, client implementation, and related tests.
- For UI changes, inspect ALL of: `.ui` file, controller/model code, and
  integration tests as one workflow.
- When changing behavior, review related unit/integration tests and update or
  justify test coverage.
- Do NOT commit or push unless the user explicitly asks.

---

## 🗑️ Scratch Files

- Do NOT create temporary investigation files in the repository root.
- Use `_scratch/` for command output, experiments, and disposable notes.
- Durable findings go in `ai_memory`; scratch files are disposable.

```bash
python3 source/ToolsForAI/source_graph.py find company --out _scratch/sg_company.txt
```

---

## 🤖 Model Worker Task System

Use task cards ONLY when work is split across external/helper AI workers.

```bash
python3 source/ToolsForAI/taskctl.py auto-pickup --runner <runner> --topic <topic>
python3 source/ToolsForAI/taskctl.py contract <TASK_ID>
python3 source/ToolsForAI/taskctl.py review <TASK_ID> --runner <runner> --notes "<summary>"
python3 source/ToolsForAI/taskctl.py guard-staged <TASK_ID>
python3 source/ToolsForAI/taskctl.py done <TASK_ID>
python3 source/ToolsForAI/taskctl.py verify
```

External worker copy block:

```text
გაუშვი პარალელური სუბაგენტები და რომ დასრულდება დაუბრუნე კოდექსს რევიუსთვის
```
