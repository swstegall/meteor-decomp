#!/usr/bin/env bash
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Verify the VS 2005 SP1 toolchain is reachable and sane.
# Does NOT install anything — procurement is manual; see docs/msvc-setup.md.

set -euo pipefail

# Source per-user override file if present.
if [[ -r "$HOME/.config/meteor-decomp.env" ]]; then
    # shellcheck disable=SC1091
    source "$HOME/.config/meteor-decomp.env"
fi

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

fail=0

step() { printf '>>> %s\n' "$*"; }
ok()   { printf '    OK: %s\n' "$*"; }
err()  { printf '    FAIL: %s\n' "$*" >&2; fail=$((fail + 1)); }

step "Wine"
# Prefer the explicit $WINE env (set by ~/.config/meteor-decomp.env when
# vstudio2005-workspace/install.sh ran). Fall back to PATH lookup.
WINE_BIN=""
if [[ -n "${WINE:-}" && -x "$WINE" ]]; then
    WINE_BIN="$WINE"
elif command -v wine64 >/dev/null 2>&1; then
    WINE_BIN="$(command -v wine64)"
elif command -v wine >/dev/null 2>&1; then
    WINE_BIN="$(command -v wine)"
fi
if [[ -n "$WINE_BIN" ]]; then
    ok "wine: $WINE_BIN ($("$WINE_BIN" --version 2>&1 | head -1))"
else
    err "wine not found via \$WINE or PATH; either run vstudio2005-workspace/install.sh (sets \$WINE) or brew install --cask wine-stable"
fi

step "MSVC_TOOLCHAIN_DIR"
if [[ -z "${MSVC_TOOLCHAIN_DIR:-}" ]]; then
    err "\$MSVC_TOOLCHAIN_DIR is not set"
    err "see docs/msvc-setup.md §3 for the expected layout"
else
    ok "\$MSVC_TOOLCHAIN_DIR=$MSVC_TOOLCHAIN_DIR"
    for required in \
        "VC/bin/cl.exe" \
        "VC/bin/link.exe" \
        "VC/bin/c1.dll" \
        "VC/bin/c1xx.dll" \
        "VC/bin/c2.dll" \
        "VC/bin/mspdb80.dll" \
        "VC/include" \
        "VC/lib"; do
        if [[ -e "$MSVC_TOOLCHAIN_DIR/$required" ]]; then
            ok "$required"
        else
            err "missing $MSVC_TOOLCHAIN_DIR/$required"
        fi
    done
    # PSDK is optional for the Rosetta Stone match (the candidate function is
    # a leaf with no Win32 calls). It IS required for any subsequent matching
    # work that touches sockets / threads / registry. Treat as a warning.
    if [[ -d "$MSVC_TOOLCHAIN_DIR/PSDK/Include" ]]; then
        ok "PSDK/Include"
    else
        printf '    WARN: missing PSDK/Include (Platform SDK 2003 R2). Rosetta Stone OK without it; later phases will need it. See %s/sdk/PSDK_TODO.md\n' \
            "${MSVC_TOOLCHAIN_DIR%/sdk}" >&2
    fi
    if [[ -d "$MSVC_TOOLCHAIN_DIR/PSDK/Lib" ]]; then
        ok "PSDK/Lib"
    else
        printf '    WARN: missing PSDK/Lib (Platform SDK 2003 R2). Rosetta Stone OK without it.\n' >&2
    fi
fi

step "cl.exe version"
if (( fail == 0 )); then
    # Invoke cl.exe directly via wine (without cl-wine.sh's /nologo) so we
    # get the banner. Use the same WINE binary cl-wine.sh resolves to.
    ver_out=$("$WINE_BIN" "$MSVC_TOOLCHAIN_DIR/VC/bin/cl.exe" 2>&1 | grep -m1 "Microsoft.* C/C++ Optimizing Compiler" || true)
    if echo "$ver_out" | grep -q "Microsoft.* C/C++ Optimizing Compiler.*Version 14\.00"; then
        ok "$ver_out"
    else
        err "cl.exe didn't report a 'Microsoft C/C++ Optimizing Compiler Version 14.00.x' line"
        err "got: $ver_out"
    fi
fi

step "objdiff (matching diff)"
if command -v objdiff-cli >/dev/null 2>&1; then
    ok "objdiff-cli: $(objdiff-cli --version 2>&1 | head -1)"
elif command -v objdiff >/dev/null 2>&1; then
    ok "objdiff: $(objdiff --version 2>&1 | head -1)"
else
    err "objdiff not on PATH; cargo install objdiff-cli"
fi

echo
if (( fail == 0 )); then
    echo ">>> setup ok — ready for 'make rosetta'"
else
    echo ">>> $fail problem(s); see above"
    exit 1
fi
