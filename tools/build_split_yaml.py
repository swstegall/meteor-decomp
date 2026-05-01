#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Phase 1: turn the Ghidra dumps in `config/<binary>.symbols.json` and
`config/strings.json` and `config/rtti.json` into the per-function work
pool at `config/<binary>.yaml`.

For every function, decide:
  - module guess (from RTTI class name + __FILE__ string heuristics)
  - matching vs functional vs middleware-* tier
  - initial status (always `unmatched` on first build)

STUB. Phase 1 deliverable. The shape of one row in the output:

    - rva: 0x004a1230
      end: 0x004a12a0
      size: 0x70
      module: net/blowfish
      symbol: Blowfish::Init
      type: matching                  # matching | functional | middleware-crt | middleware-miles | middleware-dx9
      status: unmatched               # unmatched | wip | matched | functional
      owner: null
      seed_source: rtti+__FUNCTION__
      notes: null

The middleware tiers automatically downgrade to status=middleware-*
(skipped from the contributor work pool) when:

  - rva range falls inside the MSSMIXER section → middleware-miles
  - calls into a CRT entry point (e.g. __security_init_cookie) →
    middleware-crt
  - imports d3d9!IDirect3D* → middleware-dx9

Detection heuristics live in this script; the YAML is regenerable.
"""

from __future__ import annotations

import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent


def main() -> int:
    print(
        "build_split_yaml.py: Phase 1 stub — not yet implemented",
        file=sys.stderr,
    )
    print("  see PLAN.md §6 Phase 1 for the full deliverable spec.", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
