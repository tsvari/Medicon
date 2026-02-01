# Medicon Security Audit Notes (2026-01-31)

This document captures security hotspots identified during a codebase scan and provides prioritized remediation guidance.

> Scope note: This is a static code review snapshot focused on obvious design/pattern risks. It is not a penetration test.

## Executive Summary

**Highest risk issues**

1. **SQL injection risk via string substitution templating**
   - SQL templates are parsed from XML and placeholders of the form `:Param:` are substituted by doing string replacement.
   - String/date/time values are wrapped in `'...'` but are not safely escaped.
   - Parameters can be sourced from request-provided JSON.

2. **gRPC transport security/auth hardening gaps**
   - Server uses insecure credentials (plaintext, no authentication). Binding is loopback, which lowers exposure but does not eliminate local attacker risk.
   - Reflection is enabled.

3. **Sensitive data leakage via logs**
   - SQL text is logged on exceptions, which may include substituted values (PII/secrets) depending on usage.

## Evidence / Hotspots

### 1) SQL templating + substitution (Injection surface)

**Where it happens**

- XML applet parsing and placeholder replacement:
  - [backend/source/sqlapplet.cpp](backend/source/sqlapplet.cpp)
- Execution of generated SQL text:
  - [backend/source/sqlquery.cpp](backend/source/sqlquery.cpp)

**Why it’s risky**

- Placeholder substitution is done by raw string replace into SQL text.
- Values are quoted for string/date/time but **not escaped** (e.g., input containing `'` breaks out of the literal).
- JSON parameters are converted to strings, and even complex JSON objects/arrays get serialized via `dump()`.

**Direct request path observed**

- gRPC server accepts JSON parameters and forwards into SQL query construction:
  - [backend/grpc/company_server.hpp](backend/grpc/company_server.hpp)

**Security impact**

- Potential SQL injection if any user-controlled value reaches the templating layer (directly or indirectly).
- Depending on DB permissions, could allow data exfiltration, modification, privilege escalation.

### 2) gRPC insecure credentials / reflection

**Where it happens**

- Insecure server credentials:
  - [backend/grpc/company_server.hpp](backend/grpc/company_server.hpp)
- Insecure client credentials exist in example/test code:
  - [frontend/grpc/company_client.hpp](frontend/grpc/company_client.hpp)

**Why it’s risky**

- Insecure credentials = plaintext transport, no peer authentication.
- Reflection enabled: helpful for development, but increases API discovery surface.

### 3) Logging sensitive SQL text

**Where it happens**

- SQL text logged when DB errors occur:
  - [backend/grpc/company_server.hpp](backend/grpc/company_server.hpp)

**Why it’s risky**

- Logs may contain substituted parameter values.
- Logs are often copied/aggregated and accessible to broader audiences than production databases.

### 4) Secrets in repo/tests

**Where it happens**

- Test JSON contains `host/user/pass` placeholders:
  - [source/tests/app-data/GlobalTestProject/GlobalTestProject.json](source/tests/app-data/GlobalTestProject/GlobalTestProject.json)

**Risk note**

- This file contains dummy values, but ensure real credentials never land in source control.

## Recommendations (Prioritized)

### P0 (Immediate / Blocker)

1. **Stop building executable SQL via string replacement**
   - Prefer **prepared statements / bound parameters** in SQLAPI.
   - Keep templates if needed, but only substitute identifiers from an allowlist; bind user data.

2. **Interim mitigation if full refactor must wait**
   - Enforce strict **type validation** for each `DataInfo::Type` before substitution.
     - Numeric: only allow `[-+]?[0-9]+(\.[0-9]+)?` (or stricter based on DB expectations).
     - Date/Time: validate format strictly (ISO-like, no extra characters).
    - Escape string literals safely for your DB dialect at minimum.

       **Postgres specifics**
       - Escape single quotes in string literals by doubling them: `'` → `''`.
       - Reject NUL (`\0`) in text values (Postgres does not allow NUL in `text/varchar`).
       - Avoid `E'...'` (backslash escapes) unless you absolutely need it; with default `standard_conforming_strings=on`, backslashes are just characters.
       - For `LIKE` queries, if you allow user-provided patterns, consider escaping `%` and `_` and using `... LIKE :pattern ESCAPE '\\'` (or provide separate “contains/startsWith” params and build the pattern server-side).
       - Never substitute identifiers (table/column/order-by) from user input; if you must, use a strict allowlist and quote identifiers by doubling `"` inside `"identifier"`.

       **Safe baseline**
       - Replace `'` with `''` before wrapping in quotes.
   - Reject or sanitize values that contain control characters or comment tokens if applicable.

3. **Reduce blast radius (defense-in-depth)**
   - Ensure the DB user used by the service has least privilege (no DDL, no superuser).

### P1 (Near-term)

1. **gRPC security hardening**
   - Use TLS (`SslServerCredentials`) at minimum; consider mTLS if internal service-to-service.
   - Add authn/authz (token-based metadata, mTLS identity mapping, etc.).
   - Disable reflection in production builds (compile-time flag or config).

2. **Request/response limits and timeouts**
   - Set max message sizes (server builder / channel args) appropriate to your app.
   - Enforce per-RPC deadlines/timeouts and propagate to DB layer.

3. **Logging hygiene**
   - Do not log full SQL text with substituted values.
   - Log:
     - an operation name / applet name,
     - a correlation id,
     - and safe error codes.

### P2 (Medium-term)

1. **Secrets management**
   - Load DB creds from secure storage (Windows Credential Manager, Vault, env vars in CI, etc.).
   - Avoid long-lived global credential storage if possible.

2. **Monitoring and auditing**
   - Add structured security logging (auth failures, rate limit triggers, etc.).
   - Add metrics for request sizes, error rates, and query timing.

## Suggested Implementation Path

1. Implement strict validation + escaping in the SQL applet pipeline as an immediate guardrail.
2. Introduce a new query mechanism that uses parameter binding in SQLAPI.
3. Migrate the most exposed endpoints first (those accepting JSON parameters).
4. Enable TLS and disable reflection in production.

## Open Questions (to confirm scope)

- Is the gRPC server ever exposed beyond localhost (container, remote host, port-forwarding, etc.)?
- Do SQL applets allow substitution in identifiers (table/column names), or only values?
- What DB vendor(s) are targeted? Escaping rules differ.
