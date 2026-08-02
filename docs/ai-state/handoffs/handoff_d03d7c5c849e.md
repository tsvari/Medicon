## Session Handoff
**Session:** fix-sql-injection-parameterized (`d03d7c5c849e`)
**Branch:** main
**Goal:** Refactor SQLApplet/SqlCommand/SqlQuery to use SQLAPI++ parameterized queries (Param() API) instead of string substitution. Validate FIELD-type identifiers. Update company_server.hpp and tests.
**Last updated:** 2026-06-30T20:49:04+00:00

### Work completed
- Implemented parameterized SQL: SQLApplet::parse() now generates :name markers for SQLAPI++ Param() binding instead of inline string substitution. FIELD params validated via regex. SqlCommand/SqlQuery execute() bind via Param(). company_server.hpp updated. 8 files changed.

### Decisions made (do not re-litigate these)
- SQL injection fix: Changed from inline string substitution to SQLAPI++ parameterized queries. FIELD types validated via regex (isValidIdentifier). STRING/NUMERIC/DATE/TIME types bound via SACommand::Param() API.

### Restore context
```bash
python tools/session_mgr.py resume d03d7c5c849e
python tools/session_mgr.py status
```