#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Phase 2 stub: per-function and project-wide matching diff via objdiff.

Per-function:
    tools/compare.py FUNC=Blowfish::Init
    -> looks up Blowfish::Init in config/ffxivgame.yaml
    -> finds the rva range in orig/ffxivgame.exe
    -> finds the matching .obj in build/obj/.../Blowfish__Init.obj
    -> shells out to objdiff-cli
    -> prints OK / PARTIAL / MISMATCH

Project-wide:
    tools/compare.py --all
    -> walks every status=matched row in config/ffxivgame.yaml
    -> writes build/reports/diff.csv (fn, rva, size, status, delta_bytes)

NOT WIRED UP YET — Phase 2 deliverable (after the first matching
function exists). Documented here so the shape of the wrapper is
clear for the contributor who builds it.
"""

from __future__ import annotations

import sys


def main() -> int:
    print("compare.py: Phase 2 stub — see PLAN.md §6 Phase 2", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
