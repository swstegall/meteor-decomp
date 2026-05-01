#!/usr/bin/env bash
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Phase 0 + Phase 1 environment setup. Idempotent.
#
# What this installs / verifies:
#   - Python 3.10+ (for tools/*.py)
#   - JDK 17+ (for Ghidra)
#   - Ghidra 11.x at $GHIDRA_HOME
#   - Wine + a VS 2005 SP1 cl.exe (Phase 2 — not yet wired in)
#   - objdiff (Phase 2)
#
# Phase 0 minimum: Python 3.10. Everything else is staged behind
# `--with-ghidra` / `--with-msvc` / `--with-objdiff` flags so you can
# bring up parts of the pipeline incrementally.

set -euo pipefail

WITH_GHIDRA=0
WITH_MSVC=0
WITH_OBJDIFF=0

while (( $# )); do
    case "$1" in
        --with-ghidra)  WITH_GHIDRA=1 ;;
        --with-msvc)    WITH_MSVC=1 ;;
        --with-objdiff) WITH_OBJDIFF=1 ;;
        --all)          WITH_GHIDRA=1; WITH_MSVC=1; WITH_OBJDIFF=1 ;;
        -h|--help)
            grep '^# ' "$0" | sed 's/^# //'
            exit 0
            ;;
        *) echo "unknown flag: $1" >&2; exit 1 ;;
    esac
    shift
done

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo ">>> meteor-decomp setup"

# --- Phase 0: Python ----------------------------------------------------
if ! command -v python3 >/dev/null; then
    echo "error: python3 not on PATH" >&2
    exit 1
fi
python3_version=$(python3 -c 'import sys; print(f"{sys.version_info.major}.{sys.version_info.minor}")')
echo "  python3: $python3_version"

# --- Phase 1: Ghidra ----------------------------------------------------
if (( WITH_GHIDRA )); then
    if [[ -z "${GHIDRA_HOME:-}" || ! -d "$GHIDRA_HOME" ]]; then
        echo "  ghidra: GHIDRA_HOME not set or invalid"
        echo "          install via: brew install --cask ghidra"
        echo "          then: export GHIDRA_HOME=/Applications/ghidra/<version>/"
        exit 1
    fi
    echo "  ghidra: $GHIDRA_HOME"
    if ! command -v java >/dev/null; then
        echo "error: java not on PATH (Ghidra needs JDK 17+)" >&2
        exit 1
    fi
    java_version=$(java -version 2>&1 | head -1)
    echo "  java:   $java_version"
fi

# --- Phase 2: MSVC / Wine ----------------------------------------------
if (( WITH_MSVC )); then
    if ! command -v wine >/dev/null && ! command -v wine64 >/dev/null; then
        echo "  wine: not found — install via: brew install --cask wine-stable"
        exit 1
    fi
    if [[ -z "${MSVC_TOOLCHAIN_DIR:-}" || ! -d "$MSVC_TOOLCHAIN_DIR" ]]; then
        echo "  msvc: MSVC_TOOLCHAIN_DIR not set or invalid"
        echo "        Expected layout:"
        echo "          \$MSVC_TOOLCHAIN_DIR/cl.exe"
        echo "          \$MSVC_TOOLCHAIN_DIR/link.exe"
        echo "          \$MSVC_TOOLCHAIN_DIR/include/"
        echo "          \$MSVC_TOOLCHAIN_DIR/lib/"
        echo "        Procure VS 2005 SP1 from your MSDN / archive copy."
        exit 1
    fi
    echo "  msvc:   $MSVC_TOOLCHAIN_DIR"
fi

# --- Phase 2: objdiff ---------------------------------------------------
if (( WITH_OBJDIFF )); then
    if ! command -v objdiff-cli >/dev/null && ! command -v objdiff >/dev/null; then
        echo "  objdiff: not found"
        echo "           install via: cargo install objdiff-cli"
        exit 1
    fi
    echo "  objdiff: $(command -v objdiff-cli || command -v objdiff)"
fi

echo
echo ">>> setup ok"
echo "    next:  ./tools/symlink_orig.sh && python3 tools/extract_pe.py"
