#!/usr/bin/env bash
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Phase 2 stub: invoke the pinned VS 2005 SP1 cl.exe under Wine.
#
# Once wired:
#   tools/cl-wine.sh /c file.cpp /Fo out.obj
# is a transparent drop-in for cl.exe.
#
# NOT WIRED UP. This is the placeholder; populate once setup.sh
# --with-msvc passes locally and a chosen cl.exe is installed under
# $MSVC_TOOLCHAIN_DIR. See docs/compiler-detection.md.

set -euo pipefail

if [[ -z "${MSVC_TOOLCHAIN_DIR:-}" ]]; then
    echo "error: \$MSVC_TOOLCHAIN_DIR not set" >&2
    echo "       see docs/compiler-detection.md" >&2
    exit 1
fi
if ! command -v wine >/dev/null && ! command -v wine64 >/dev/null; then
    echo "error: wine not on PATH" >&2
    exit 1
fi

WINE="${WINE:-$(command -v wine64 || command -v wine)}"
exec "$WINE" "$MSVC_TOOLCHAIN_DIR/cl.exe" "$@"
