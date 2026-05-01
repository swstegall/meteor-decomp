#!/usr/bin/env bash
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Wine wrapper around VS 2005 SP1 cl.exe / link.exe.
#
# Usage:
#   cl-wine.sh <args>...      # passes through to cl.exe
#   cl-wine.sh --link <args>  # passes through to link.exe instead
#
# Required env (or set in ~/.config/meteor-decomp.env):
#   MSVC_TOOLCHAIN_DIR   layout described in docs/msvc-setup.md
#   WINEPREFIX           default: ~/.wine-msvc2005
#
# See docs/msvc-setup.md for the procurement guide.

set -euo pipefail

# Source per-user override file if present.
if [[ -r "$HOME/.config/meteor-decomp.env" ]]; then
    # shellcheck disable=SC1091
    source "$HOME/.config/meteor-decomp.env"
fi

if [[ -z "${MSVC_TOOLCHAIN_DIR:-}" ]]; then
    echo "error: \$MSVC_TOOLCHAIN_DIR not set" >&2
    echo "       set in ~/.config/meteor-decomp.env or your shell rc:" >&2
    echo "         export MSVC_TOOLCHAIN_DIR=\"\$HOME/sdk/msvc-2005-sp1\"" >&2
    echo "       see docs/msvc-setup.md for the layout." >&2
    exit 1
fi

if [[ ! -x "$MSVC_TOOLCHAIN_DIR/VC/bin/cl.exe" ]]; then
    echo "error: cl.exe not found at \$MSVC_TOOLCHAIN_DIR/VC/bin/cl.exe" >&2
    echo "       (\$MSVC_TOOLCHAIN_DIR=$MSVC_TOOLCHAIN_DIR)" >&2
    echo "       see docs/msvc-setup.md §3 for the expected layout." >&2
    exit 1
fi

# Wine binary discovery — prefer wine64 fallback to wine.
WINE="${WINE:-$(command -v wine64 2>/dev/null || command -v wine 2>/dev/null || true)}"
if [[ -z "$WINE" ]]; then
    echo "error: wine not on PATH (brew install --cask wine-stable)" >&2
    exit 1
fi

# Per-project Wine prefix (32-bit — cl.exe is i386).
export WINEPREFIX="${WINEPREFIX:-$HOME/.wine-msvc2005}"
export WINEARCH=win32
export WINEDEBUG="${WINEDEBUG:--all}"   # silence the noisy default fixme:'s

# Translate POSIX paths in MSVC_TOOLCHAIN_DIR → Wine drive letters via
# the prefix's dosdevices. We use a `Z:` drive that maps to '/' which
# Wine sets up automatically; that's enough to reach absolute POSIX
# paths under MSVC_TOOLCHAIN_DIR.
mvtc_winpath() {
    # /foo/bar -> Z:\foo\bar
    local p="$1"
    printf 'Z:%s\n' "${p//\//\\}"
}

VC_BIN="$MSVC_TOOLCHAIN_DIR/VC/bin"
INCLUDE_PARTS=(
    "$(mvtc_winpath "$MSVC_TOOLCHAIN_DIR/VC/include")"
)
LIB_PARTS=(
    "$(mvtc_winpath "$MSVC_TOOLCHAIN_DIR/VC/lib")"
)
if [[ -d "$MSVC_TOOLCHAIN_DIR/PSDK/Include" ]]; then
    INCLUDE_PARTS+=("$(mvtc_winpath "$MSVC_TOOLCHAIN_DIR/PSDK/Include")")
fi
if [[ -d "$MSVC_TOOLCHAIN_DIR/PSDK/Lib" ]]; then
    LIB_PARTS+=("$(mvtc_winpath "$MSVC_TOOLCHAIN_DIR/PSDK/Lib")")
fi

# Join with `;` (Windows path separator).
IFS=';' INCLUDE="${INCLUDE_PARTS[*]}"
IFS=';' LIB="${LIB_PARTS[*]}"
unset IFS
export WINE_INCLUDE="$INCLUDE"
export WINE_LIB="$LIB"

# Pass INCLUDE / LIB via WINE's environment-passthrough.
# wine inherits the parent process env vars; setting these here is
# sufficient.
export INCLUDE
export LIB

# `--link` switches us from cl.exe to link.exe.
if [[ "${1:-}" == "--link" ]]; then
    shift
    exec "$WINE" "$VC_BIN/link.exe" "$@"
fi
exec "$WINE" "$VC_BIN/cl.exe" /nologo "$@"
