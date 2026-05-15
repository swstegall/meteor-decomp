#!/usr/bin/env bash
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Stage D — first link attempt. Drives link.exe under cl-wine.sh against
# the full _passthrough/ obj inventory of one binary.
#
# Usage:  tools/link_pe.sh <binary>             # e.g. ffxivlogin
#         tools/link_pe.sh <binary> <entry-rva> # override entry RVA
#
# This is a DIAGNOSTIC tool — the first run is expected to spit pages
# of unresolved-external errors. The point is to capture the gap list
# so we can iterate on infrastructure (entry alias, IAT handling,
# section directives, etc.). See docs/recompilable-strategy.md.

set -euo pipefail

bin="${1:?usage: tools/link_pe.sh <binary> [entry-va-hex]}"
out_dir="build/link"
mkdir -p "$out_dir"

obj_dir="build/obj/_passthrough/${bin}"
if [[ ! -d "$obj_dir" ]]; then
    echo "error: $obj_dir missing — run 'make compile-passthrough BINARY=$bin.exe' first" >&2
    exit 1
fi

# Entry symbol selection:
#   - If `_section_text_blob*.cpp` was emitted (Phase 2.7 blob path),
#     use `_text_blob` as the entry symbol — first chunk always
#     defines that symbol regardless of single vs. multi-chunk output.
#     Actual entry RVA gets fixed up post-link via postlink_patch.py.
#   - Else fall back to FUN_<va> derived from PE layout's entry_rva.
if ls "src/${bin}/_passthrough/"_section_text_blob*.cpp >/dev/null 2>&1; then
    entry_sym="_text_blob"
elif [[ -n "${2:-}" ]]; then
    entry_va="$2"
    entry_sym="FUN_${entry_va}"
else
    entry_va=$(python3 -c "
import json
pe = json.loads(open('build/pe-layout/${bin}.json').read())
print(f'{int(pe[\"image_base\"], 16) + int(pe[\"entry_rva\"], 16):08x}')
")
    entry_sym="FUN_${entry_va}"
fi

# Subsystem from PE.
subsys=$(python3 -c "
import json
pe = json.loads(open('build/pe-layout/${bin}.json').read())
print(pe['subsystem'])
")
case "$subsys" in
    2) subsys_str="WINDOWS,4.0" ;;
    3) subsys_str="CONSOLE,4.0" ;;
    *) subsys_str="WINDOWS,4.0" ;;
esac

# Build the objlist (POSIX paths; cl-wine.sh translates internally).
objlist="$out_dir/${bin}.objlist"
ls "${obj_dir}"/*.obj > "$objlist"
n_obj=$(wc -l < "$objlist" | tr -d ' ')

# Linker flag list — passed verbatim to cl-wine.sh --link.
#
# Note: /MERGE doesn't accept wildcards. We don't need it for `$X<rva>`
# subsections — link.exe's grouped-section convention auto-concatenates
# all `<sec>$<key>` siblings into the merged `<sec>` output, sorted
# lexicographically by key. Our `$X<rva>` keys are already padded so
# they sort numerically.
flags=(
    /NOLOGO
    /BASE:0x00400000
    /SUBSYSTEM:"${subsys_str}"
    /MACHINE:X86
    "/ENTRY:${entry_sym}"
    /OPT:NOREF
    /OPT:NOICF
    /INCREMENTAL:NO
    /FIXED
    /NODEFAULTLIB
    "/OUT:${out_dir}/${bin}.exe"
    "@${objlist}"
)

echo ">>> link[${bin}] ${n_obj} objs, entry=${entry_sym}, subsystem=${subsys_str}"
exec tools/cl-wine.sh --link "${flags[@]}"
