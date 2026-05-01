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
// #5 [GREEN — 86/86 bytes match] — same shape as #4 but with the
//    `count != 0` checks rewritten to `count > 0` (unsigned compare).
//    The two remaining mismatches in #4 were both branch-encoding
//    choices: TEST followed by `count != 0` produces JE/JNE
//    (`74`/`75`), while TEST followed by an unsigned `> 0` produces
//    JBE/JA (`76`/`77`) — functionally identical after TEST clears
//    CF, but MSVC picks the encoding based on source comparison
//    operator. The orig uses the unsigned-comparison form.
//
// #4 [86/86 bytes — only 2 byte differences, branch-encoding only] —
//    port of Ghidra's decompiler reconstruction for FUN_00b361b0.
//    Three shape changes from #3 closed the size gap from 77→86 B
//    and matched all of the body bytes:
//      (a) plain `int *` element-wide pointers, not struct types —
//          eight explicit element writes; no struct-copy for MSVC's
//          optimizer to fold into a single-base-pointer access.
//      (b) `high = dest + 4` computed ONCE before the loop, then both
//          `dest` and `high` advanced INDEPENDENTLY by 8 (= 32 bytes)
//          per iteration. Both are dereferenced inside the body, so
//          MSVC keeps them as separate live values rather than
//          rederiving `high = dest + 0x10` each iteration.
//      (c) `do { … } while (count …)` with an outer `if (count …)`
//          guard — matches the orig's TEST/JBE-skip-forward at the
//          prologue and TEST/JA-loop-back at the bottom.
//
// #3 [77 B vs orig 86 B; not green] — Block16-struct array pointers
//    `low = dest, high = dest + 1`. Hoped MSVC would keep them as
//    separate live values because they're array elements; instead
//    the optimizer notices `high - low == 16` is invariant and
//    consolidates back to one pointer with offsets 0..0x1c. The
//    struct-copy `*low = src[0]; *high = src[1]` is the reason: MSVC
//    sees two assignments it can rewrite as a single base+offset
//    access, which then collapses the two pointers.
//
// #2 [83 B] — `unsigned int count` + manual `d_low` + `d_high` char*
//    locals. Fixed the JLE → JBE branch type. MSVC merged the two
//    pointers; same root cause as #3.
//
// #1 [85 B] — initial port; `int count` (signed) + single pointer.
//    Compiled with JLE (signed), wrong branch type vs binary's JBE.

extern "C" void rosetta_FUN_00b361b0(unsigned int *dest, unsigned int count, const unsigned int *src) {
    unsigned int *high;
    // `count > 0` (unsigned compare, not `!= 0`) → MSVC emits TEST + JBE
    // for the initial guard and TEST + JA for the loop-back, matching
    // the orig's unsigned-comparison branch encoding (76 / 77 opcodes).
    // `count != 0` produces JE / JNE (74 / 75) — functionally identical
    // after TEST but a 1-byte mismatch each.
    if (count > 0) {
        high = dest + 4;        // = dest + 16 bytes (LEA EDX, [EAX+0x10])
        do {
            if (dest != 0) {
                dest[0] = src[0];
                dest[1] = src[1];
                dest[2] = src[2];
                dest[3] = src[3];
                high[0] = src[4];
                high[1] = src[5];
                high[2] = src[6];
                high[3] = src[7];
            }
            count = count - 1;
            dest = dest + 8;     // += 32 bytes
            high = high + 8;     // += 32 bytes (lockstep)
        } while (count > 0);
    }
}

// vim: ts=4 sts=4 sw=4 et
