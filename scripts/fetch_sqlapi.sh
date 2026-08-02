#!/usr/bin/env bash
# fetch_sqlapi.sh — Restore + build the SQLAPI++ Linux source (gitignored).
#
# SQLAPI++ is intentionally NOT kept in git. On Linux the source package is
# extracted to:
#   source/cpp/backend/source/3party/SQLAPI/linux/sqlapi-5.3.5/
# then compiled with its bundled Makefile.gnu (produces libsqlapi.a).
#
# Sources, in order of preference:
#   1. Local tarball : assets/SQLAPI_installers/sqlapi-5.3.5.tar.gz
#   2. Manual download: place the tarball in assets/SQLAPI_installers/ and re-run
#
# Usage:  bash scripts/fetch_sqlapi.sh          # extract + build
#         bash scripts/fetch_sqlapi.sh --extract-only
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TARGET_DIR="$REPO_ROOT/source/cpp/backend/source/3party/SQLAPI/linux/sqlapi-5.3.5"
INSTALLERS_DIR="$REPO_ROOT/assets/SQLAPI_installers"
TARBALL="$INSTALLERS_DIR/sqlapi-5.3.5.tar.gz"
EXTRACT_ONLY=0
[[ "${1:-}" == "--extract-only" ]] && EXTRACT_ONLY=1

echo "SQLAPI++ restore for Linux"
echo "Target: $TARGET_DIR"

if [ -f "$TARGET_DIR/include/sqlapi.h" ]; then
    echo "SQLAPI++ already present — nothing to do."
    exit 0
fi

if [ ! -f "$TARBALL" ]; then
    echo "ERROR: $TARBALL not found." >&2
    echo "Download sqlapi-5.3.5.tar.gz from https://www.sqlapi.com/ , place it in" >&2
    echo "assets/SQLAPI_installers/ and re-run this script." >&2
    exit 1
fi

mkdir -p "$(dirname "$TARGET_DIR")"
echo "Extracting $TARBALL ..."
tar -xzf "$TARBALL" -C "$(dirname "$TARGET_DIR")"
if [ ! -d "$TARGET_DIR" ]; then
    # Some tarballs unpack to a versioned top dir; normalize it.
    UNPACKED="$(find "$(dirname "$TARGET_DIR")" -maxdepth 1 -type d -name 'sqlapi-*' | head -n1)"
    [ -n "$UNPACKED" ] && [ "$UNPACKED" != "$TARGET_DIR" ] && mv "$UNPACKED" "$TARGET_DIR"
fi

if [ "$EXTRACT_ONLY" -eq 1 ]; then
    echo "Extracted to $TARGET_DIR (build skipped)."
    exit 0
fi

if command -v make >/dev/null 2>&1 && [ -f "$TARGET_DIR/Makefile.gnu" ]; then
    echo "Building SQLAPI++ (make -f Makefile.gnu) ..."
    ( cd "$TARGET_DIR" && make -f Makefile.gnu )
    if [ -f "$TARGET_DIR/lib/libsqlapi.a" ]; then
        echo "OK: libsqlapi.a built."
    else
        echo "WARNING: build finished but lib/libsqlapi.a not found." >&2
    fi
else
    echo "make not available (or no Makefile.gnu). Build manually:"
    echo "  cd $TARGET_DIR && make -f Makefile.gnu"
fi
