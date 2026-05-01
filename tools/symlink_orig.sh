#!/usr/bin/env bash
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Populate orig/ with symlinks to the five shipped FFXIV 1.23b binaries
# from the workspace's ffxiv-install-environment install. Idempotent.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ORIG_DIR="${REPO_ROOT}/orig"

# Default install location used by xiv1point0-apple-silicon-installer.
DEFAULT_INSTALL="${REPO_ROOT}/../ffxiv-install-environment/target/prefix/drive_c/Program Files (x86)/SquareEnix/FINAL FANTASY XIV"

INSTALL_DIR="${1:-$DEFAULT_INSTALL}"

if [[ ! -d "$INSTALL_DIR" ]]; then
    echo "error: install dir not found: $INSTALL_DIR" >&2
    echo "       pass an alternative as the first argument:" >&2
    echo "       $0 /path/to/FINAL FANTASY XIV" >&2
    exit 1
fi

mkdir -p "$ORIG_DIR"

# We DO NOT symlink ffxivgame.patched.exe — meteor-decomp targets the
# unmodified binary. The patched .exe is a workspace runtime artefact.
EXES=(
    ffxivgame.exe
    ffxivboot.exe
    ffxivconfig.exe
    ffxivlogin.exe
    ffxivupdater.exe
)

linked=0
missing=0
for exe in "${EXES[@]}"; do
    src="$INSTALL_DIR/$exe"
    dst="$ORIG_DIR/$exe"
    if [[ ! -f "$src" ]]; then
        echo "warning: missing $src — skipping" >&2
        missing=$((missing + 1))
        continue
    fi
    # Always replace; the install may have been re-extracted.
    rm -f "$dst"
    ln -s "$src" "$dst"
    linked=$((linked + 1))
    echo "  linked $dst -> $src"
done

echo
echo "linked $linked binaries, $missing missing"
if (( missing > 0 )); then
    exit 2
fi
