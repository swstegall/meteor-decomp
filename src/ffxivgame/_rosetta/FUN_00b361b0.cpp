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

extern "C" void rosetta_FUN_00b361b0(void *dest, int count, const void *src) {
    char *d = static_cast<char *>(dest);
    while (count > 0) {
        if (d != nullptr) {
            // Unrolled 32-byte memcpy from `src` to `d`.
            // The original asm splits this into two writes via EAX (d)
            // and EDX (d+0x10) but a straight loop is byte-equivalent
            // under /O2 — verify with objdiff.
            const int *s = static_cast<const int *>(src);
            int *o = reinterpret_cast<int *>(d);
            o[0] = s[0];
            o[1] = s[1];
            o[2] = s[2];
            o[3] = s[3];
            o[4] = s[4];
            o[5] = s[5];
            o[6] = s[6];
            o[7] = s[7];
        }
        d += 32;
        count--;
    }
}

// vim: ts=4 sts=4 sw=4 et
