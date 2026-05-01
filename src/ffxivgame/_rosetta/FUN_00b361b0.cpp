// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// FUNCTION: ffxivgame 0x009d0c40 — superseded; see new top.txt
// FUNCTION: ffxivgame 0x007361b0 — Rosetta Stone candidate (Phase 2)
//
// Picked by tools/find_rosetta.py. Score 80, 86 bytes, 31 integer ops,
// no calls, no FP, no SEH. See build/rosetta/ffxivgame.top.txt for the
// disassembly.
//
// Behavior (read from the asm):
//   Args: arg0 = dest pointer, arg1 = count, arg2 = src pointer.
//   Walks `count` 32-byte slots in `dest`. For each non-null slot,
//   copies 32 bytes from `src` into the slot. Advances `dest` by 32 each
//   iteration but does NOT advance `src` — the same 32-byte template is
//   replicated `count` times.
//
//   The `if (dest != null)` check is per-iteration but `dest` never
//   becomes null mid-loop, so it's effectively a hoisted-no-op the
//   compiler chose not to eliminate. Source likely had it inside the
//   loop body (defensive coding) and /O2 didn't lift it out — the
//   ordering of TEST EAX, JZ skip is what locks the codegen.
//
//   The unrolled copy splits the 32-byte block into [dest+0..0xC] (via
//   EAX) and [dest+0x10..0x1C] (via EDX = EAX + 0x10). MSVC chose two
//   pointers because the dword-encoded displacements fit in a signed
//   byte (saves 3 bytes per write vs. one pointer + larger displacements).
//
// Calling convention: __cdecl (caller-cleans, args on stack).
// Stack frame: -8 bytes (PUSH ESI; PUSH EDI inside the no-skip arm).
//
// PHASE 2 NOTE: this draft is the contributor's starting point, not a
// matched function yet. Once VS 2005 SP1 cl.exe is set up under Wine
// (see docs/msvc-setup.md), `make rosetta` will compile this and diff
// the .obj against the binary slice at RVA 0x007361b0. Iterate the
// source until objdiff reports zero delta.

#include <stddef.h>
#include <string.h>

// Iteration history (most recent first; pre-MSVC-2005-RTM, /O2):
//
// #3 [current — 77 B vs orig 86 B; not green] — Block16-struct array
//    pointers `low = dest, high = dest + 1`. Hoped MSVC would keep
//    them as separate live values because they're array elements;
//    instead the optimizer notices `high - low == 16` is invariant
//    and consolidates back to one pointer with offsets 0..0x1c. The
//    JBE branch type is correct (unsigned count ✓), but my output is
//    9 bytes shorter than the original because I lost the second
//    PUSH EDI / POP EDI pair and the LEA EDX, [EAX+0x10] / ADD EDX,
//    0x20 instructions. Need either (a) inline asm or (b) a source
//    pattern that genuinely needs both pointers live (e.g. they
//    advance by different amounts at some point, or are handed off
//    to function calls).
//
// #2 [83 B] — `unsigned int count` + manual `d_low` + `d_high` char*
//    locals. Fixed the JLE → JBE branch type. MSVC merged the two
//    pointers; same root cause as #3.
//
// #1 [85 B] — initial port; `int count` (signed) + single pointer.
//    Compiled with JLE (signed), wrong branch type vs binary's JBE.
//
// All three iterations compile cleanly and produce valid COFF i386
// objects under VS 2005 RTM via Wine — confirms the toolchain. The
// remaining matching work is a decomp-iteration problem, not a
// toolchain problem.
struct Block16 { int v[4]; };

extern "C" void rosetta_FUN_00b361b0(Block16 *dest, unsigned int count, const Block16 *src) {
    Block16 *low = dest;
    Block16 *high = dest + 1;     // dest + 16 bytes
    while (count > 0) {
        if (low != NULL) {
            *low = src[0];
            *high = src[1];
        }
        low += 2;                  // += 32 bytes
        high += 2;                 // += 32 bytes
        count--;
    }
}

// vim: ts=4 sts=4 sw=4 et
