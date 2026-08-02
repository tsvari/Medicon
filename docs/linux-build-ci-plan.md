# Medicon — Linux Backend Build + CI Plan

**Date:** 2026-08-02
**Status:** Step 1 in progress (user switching to Linux machine)

---

## Target Environments

| App | Production OS | Build hosts |
|-----|--------------|-------------|
| Backend (provider) | **Linux** | Linux (primary) |
| Frontend (ProviderDesktop) | Windows first → Android/iOS later | Windows, later Mac |
| Desktop (Mac) | macOS | Mac (same code as Windows) |

---

## STEP 1 — Linux Backend Build Readiness *(IN PROGRESS)*

**Goal:** Get `provider` + all test executables compiling and passing on Linux.

### Tasks

| # | Task | Details |
|---|------|---------|
| 1.1 | **Compile SQLAPI++ for Linux** | Source at `assets/SQLAPI_installers/sqlapi-5.3.5.tar.gz` — contains g++ prebuilt binaries (4.8–12) + `Makefile.gnu`. Extract and build/verify. |
| 1.2 | **Create Linux Conan profile** | `conan profile detect` → likely `x86_64-linux-gcc`; ensure gtest/easyloggingpp resolve. |
| 1.3 | **Adjust `sqlapi-config.cmake`** | Currently Windows-path-specific (points at `windows/vs2022/...`). Add Linux branch → `linux/sqlapi-5.3.5/...`. |
| 1.4 | **Verify `global-settings.cmake` Linux branches** | `UNIX AND NOT APPLE` branches exist (gRPC linux path, SQLAPI linux include). Test them. |
| 1.5 | **Fix platform-specific code** | Check: `_TSA` macro (narrow/wide), path separators, any `windows.h` usage, gRPC/protobuf find_package on Linux. |
| 1.6 | **Port all 4 test projects** | `BackendTestProject`, `grpc_proto_tests`, provider `connection/integration/unit_tests`. SQLite in-memory tests should be portable. |
| 1.7 | **Verify 154 tests pass on Linux** | Same pass criteria as Windows. |

### Current Windows-only dependencies to resolve

- Compiler: MSVC 17 2022 → **g++ / clang** on Linux
- Conan profile: `x86_64-windows-msvc194` → **`x86_64-linux-gcc`**
- SQLAPI++: Windows `.lib` → **Linux `.a`/`.so` from tar.gz**
- gRPC/protobuf: Windows prebuilt → **Linux build via find_package**

---

## STEP 2 — CI/CD *(AFTER Step 1 works)*

### CI choice

| Option | Verdict |
|--------|---------|
| **GitHub Actions** (recommended) | Hosted Linux/Windows/Mac runners; no server; YAML; best learning curve for a new CI user |
| Self-hosted Jenkins | Only if user explicitly wants to learn Jenkins + has a Linux box to maintain |

### Pipeline stages (recommended)

```text
1. Checkout
2. Conan install (cached)
3. CMake configure
4. Build provider
5. Build all test projects
6. Run unit tests (fast, no DB)
7. Run integration tests
8. Publish test reports (JUnit XML from gtest)
```

### Branch strategy

| Trigger | Scope |
|---------|-------|
| Per-PR | Build + unit tests only (fast) |
| Nightly (cron) | Full suite + integration (+ optional PostgreSQL) |
| `main` push | Full suite |

---

## Key Rule

**Do NOT set up CI before the backend builds on Linux.** CI automates what already works; the Linux build is the real engineering work.
