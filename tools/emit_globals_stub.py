#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Emit `src/<binary>/_passthrough/_globals_stub.cpp` for every binary
(or one passed as argument). Defines `_fltused` (and any other
bridging-symbols-the-CRT-would-provide) so link.exe resolves
references emitted by cl.exe-compiled swap sources.
"""

import argparse
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"
ALL = ("ffxivboot", "ffxivconfig", "ffxivgame", "ffxivlogin", "ffxivupdater")

BODY = """\
// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Bridging symbols the CRT would normally provide. cl.exe emits a
// reference to `__fltused` whenever it sees an FP operation (FLD /
// FSTP / FADD / etc.) in a translation unit; without /DEFAULTLIB the
// linker has nowhere to find it. Define it here so the link resolves.
// The symbol's actual value isn't used at runtime — we don't run any
// cl.exe-generated startup code.
//
// Section name `.data$Z_globals` ensures this lands AFTER our
// `.data$X<rva>` byte-blob in the merged .data section (Z > X
// lexicographically). Without this, _fltused sits in the default
// `.data` (no key), which sorts BEFORE `.data$X<...>` (`\\0` < `$`),
// shifting the byte-blob and breaking byte-identity vs orig.
#pragma section(".data$Z_globals", read, write)
extern "C" __declspec(allocate(".data$Z_globals")) int _fltused = 0x9875;
"""


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("binary", nargs="?")
    args = ap.parse_args()
    targets = [args.binary] if args.binary else ALL
    for b in targets:
        out_dir = SRC / b / "_passthrough"
        if not out_dir.exists():
            continue
        out = out_dir / "_globals_stub.cpp"
        out.write_text(BODY)
        print(f"  wrote {out.relative_to(REPO_ROOT)}")


if __name__ == "__main__":
    sys.exit(main())
