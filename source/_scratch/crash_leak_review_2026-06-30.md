# Crash / Memory-Leak Review — Parameterized-SQL Refactor

**Scope:** uncommitted diff on `main` (8 files) refactoring `SQLApplet`/`SqlCommand`/`SqlQuery`
from string-substitution to SQLAPI++ `Param()` binding.
**Method:** 3 parallel review agents (exception-safety, memory/lifetime, test-gaps) →
each candidate adversarially re-verified by an independent agent.
**Result:** 10 candidates → **9 confirmed, 1 refuted.** No process crash / UB / leak found;
all defects are caught exceptions producing wrong behavior. Two distinct root causes.

---

## ROOT CAUSE A — HIGH — FIELD-typed `FILTER_VALUE` breaks all normal company searches

`company_select.xml` and `company_count.xml` declare `FILTER_VALUE` as `Type=FIELD`, but the
`<Code>` block uses it as a free-text value inside a quoted LIKE pattern:

    "...":FILTER_FIELD:" LIKE '%:FILTER_VALUE:%'..."

The new `SQLApplet::parse()` routes every FIELD param through `isValidIdentifier()`
(regex `^[a-zA-Z_][a-zA-Z0-9_]{0,62}$`, empty string rejected). So any ordinary search term —
`"Acme Corp"` (space), `"O'Brien"` (apostrophe), accented text, `%` — throws `SQLAppletException`
inside `cmd.query()`. Caught by `catch(const SQLAppletException&)` in `QueryCompanies` /
`QueryCompanyTotalCount` → `Status(StatusCode::INTERNAL)`. **No crash, but the core
search + total-count features are broken for virtually all real input, and the empty/no-filter
case fails too.**

- `assets/app-data/provider/sql-applets/company_select.xml:21` (FILTER_VALUE used at line 46)
- `assets/app-data/provider/sql-applets/company_count.xml:21` (used at line 30)
- driver: `backend/source/sqlapplet.cpp:171-176` (FIELD branch), `:235-241` (isValidIdentifier)
- handlers: `backend/grpc/company_server.hpp:375-378`, `:470-473`

**Fix options:** (a) change `FILTER_VALUE` to `Type=STRING` so it binds as a parameter
(but note it sits inside `'%...%'` literal — needs applet support for LIKE-wrapping, or change
SQL to `LIKE '%' || :FILTER_VALUE || '%'`); (b) keep FIELD only for `FILTER_FIELD` (a real
column name) and treat `FILTER_VALUE` as a bound value. `FILTER_FIELD` itself is fine — it
resolves to a quoted identifier `"NAME"`.

## ROOT CAUSE B — MEDIUM — non-numeric client JSON → uncaught `std::invalid_argument` → no rollback

`std::stod`/`std::stoll(binding.value)` are called unguarded for NUMERIC bindings in four places.
For `QueryCompanies`/`QueryCompanyTotalCount`, `binding.value` comes straight from client JSON
(`JsonParameterFormatter::fromJsonString(params->jsonparams())`) with no numeric validation, and
`company_select.xml` declares `SERVER_UID`, `OFFSET`, `LIMIT` as NUMERIC. A request like
`{"SERVER_UID":"abc"}` (or out-of-range `"1e400"`) throws `std::invalid_argument` /
`std::out_of_range`. These derive from `std::logic_error` — **not** `SAException`, **not**
`SQLAppletException` — so they skip both specific catches and land in the generic `catch(...)`,
which returns `Status::ABORTED "Unknown error!"` **and omits the `con.rollback()`** that the
`catch(SAException&)` block performs.

- `backend/source/sqlquery.cpp:76,78,83,87`
- `backend/source/sqlcommand.cpp:76,78,83,87` (only reachable via DeleteCompany today, whose
  sole param `UID` is STRING — not exploitable now, but structurally fragile)
- `backend/grpc/company_server.hpp:101-114` (AddCompany) and `:204-217` (EditCompany) —
  same inline loop; currently safe only because values are typed protobuf ints
- doc-contract mismatch: `sqlquery.h:146-152` documents only SQLAppletException/SAException

**Fix:** wrap the numeric conversions in try/catch and rethrow as `SQLAppletException`
(or validate up front), so callers get `INVALID_ARGUMENT` with a useful message; and/or add a
`catch(const std::exception&)` that rolls back before the bare `catch(...)`.

## Test gaps (confirmed)

- `SqlAppletTests.cpp` — no test exercises a FIELD param as free-text LIKE input (test.xml has
  zero FIELD params). Add an end-to-end test against company_select.xml with
  `FILTER_VALUE="O'Brien"` asserting current behavior. **(high)**
- No test feeds a non-numeric string into a NUMERIC param through `execute()` (every numeric
  test uses the typed overload, always well-formed). **(low)**
- No test for empty-string FIELD value (`isValidIdentifier("")==false`). **(medium)**

## Refuted (1)

- `SqlCommandTests.cpp:152` "DeleteCompany silently hits catch(...) on malformed NUMERIC" —
  **false:** `company_delete.xml` has a single STRING param (`UID`); DeleteCompany never reaches
  the stod/stoll branch. The realistic path is via SqlQuery, not SqlCommand/DeleteCompany.

## Bottom line

No crash, UB, double-free, dangling reference, or leak was found in this diff. The two real
defects are **functional regressions**: (A) the FIELD/LIKE-value type mismatch breaks company
search and count for normal input — fix before merge; (B) malformed numeric client input
produces a generic ABORTED + skipped rollback instead of a clean INVALID_ARGUMENT.
