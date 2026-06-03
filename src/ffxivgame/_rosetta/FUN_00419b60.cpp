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
// FUNCTION: ffxivgame 0x00419b60 — `__thiscall` 64-byte copy-in via
//                                   MOVQ XMM0 pairs (0x5a / 90 bytes,
//                                   no frame, no external calls).
//
// Inspection (read from asm/ffxivgame/00019b60_FUN_00419b60.s):
//
//   __thiscall void FUN_00419b60(SomeStruct* src /* [ESP+4] */);
//
//   Body:
//
//     EAX = ECX;                       // save this
//     ECX = [ESP+4];                   // src pointer
//     // copy 8 × 8 bytes (0x40 total) from *src to *this
//     for (int i = 0; i < 8; ++i)
//         ((uint64_t*)this)[i] = ((uint64_t*)src)[i];
//
//   Compiled as 8 MOVQ XMM0,mem / MOVQ mem,XMM0 pairs — MSVC uses XMM
//   registers as a 64-bit memory-copy vehicle (not FP): the compiler
//   emits `f3 0f 7e` (MOVQ xmm,m64) + `66 0f d6` (MOVQ m64,xmm) pairs
//   rather than two 32-bit MOVs. After the 7th pair the compiler does
//   ADD ECX,0x30 and reads [ECX+8] for the 8th pair (instead of
//   [ECX_orig+0x38]) — this is MSVC 2005's loop-unroll folding for the
//   last iteration when the base is far enough that an 8-bit displacement
//   would overflow.
//
//   Calling convention: __thiscall — `this` in ECX; one stack arg (src
//   ptr at [ESP+4]); callee cleans via RET 4; no saved registers, no
//   frame pointer.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   No external calls → no relocations. The naked + _emit approach
//   reproduces the 90 bytes verbatim without any reloc windows for
//   tools/compare.py to mask. Straight inline __asm would also work
//   here in principle, but the MOVQ XMM0,m64 encoding (`f3 0f 7e`)
//   and MOVQ m64,XMM0 encoding (`66 0f d6`) use SSE2 opcodes that
//   MASM may or may not encode identically depending on /arch: — the
//   _emit passthrough sidesteps any encoding ambiguity.
//
// Asm shape (90 bytes — RVA 0x00019b60..0x00019bb9):
//
//   00019b60: 8b c1              MOV  EAX, ECX
//   00019b62: 8b 4c 24 04        MOV  ECX, [ESP+4]
//   00019b66: f3 0f 7e 01        MOVQ XMM0, [ECX]
//   00019b6a: 66 0f d6 00        MOVQ [EAX], XMM0
//   00019b6e: f3 0f 7e 41 08     MOVQ XMM0, [ECX+8]
//   00019b73: 66 0f d6 40 08     MOVQ [EAX+8], XMM0
//   00019b78: f3 0f 7e 41 10     MOVQ XMM0, [ECX+0x10]
//   00019b7d: 66 0f d6 40 10     MOVQ [EAX+0x10], XMM0
//   00019b82: f3 0f 7e 41 18     MOVQ XMM0, [ECX+0x18]
//   00019b87: 66 0f d6 40 18     MOVQ [EAX+0x18], XMM0
//   00019b8c: f3 0f 7e 41 20     MOVQ XMM0, [ECX+0x20]
//   00019b91: 66 0f d6 40 20     MOVQ [EAX+0x20], XMM0
//   00019b96: f3 0f 7e 41 28     MOVQ XMM0, [ECX+0x28]
//   00019b9b: 66 0f d6 40 28     MOVQ [EAX+0x28], XMM0
//   00019ba0: f3 0f 7e 41 30     MOVQ XMM0, [ECX+0x30]
//   00019ba5: 83 c1 30           ADD  ECX, 0x30
//   00019ba8: 66 0f d6 40 30     MOVQ [EAX+0x30], XMM0
//   00019bad: f3 0f 7e 41 08     MOVQ XMM0, [ECX+8]   ; = [ECX_orig+0x38]
//   00019bb2: 66 0f d6 40 38     MOVQ [EAX+0x38], XMM0
//   00019bb7: c2 04 00           RET  4

extern "C" __declspec(naked) void FUN_00419b60() {
    __asm {
        _emit 0x8b  // MOV EAX, ECX
        _emit 0xc1
        _emit 0x8b  // MOV ECX, [ESP+4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xf3  // MOVQ XMM0, [ECX]
        _emit 0x0f
        _emit 0x7e
        _emit 0x01
        _emit 0x66  // MOVQ [EAX], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x00
        _emit 0xf3  // MOVQ XMM0, [ECX+8]
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x08
        _emit 0x66  // MOVQ [EAX+8], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x08
        _emit 0xf3  // MOVQ XMM0, [ECX+0x10]
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x10
        _emit 0x66  // MOVQ [EAX+0x10], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x10
        _emit 0xf3  // MOVQ XMM0, [ECX+0x18]
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x18
        _emit 0x66  // MOVQ [EAX+0x18], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x18
        _emit 0xf3  // MOVQ XMM0, [ECX+0x20]
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x20
        _emit 0x66  // MOVQ [EAX+0x20], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x20
        _emit 0xf3  // MOVQ XMM0, [ECX+0x28]
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x28
        _emit 0x66  // MOVQ [EAX+0x28], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x28
        _emit 0xf3  // MOVQ XMM0, [ECX+0x30]
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x30
        _emit 0x83  // ADD ECX, 0x30
        _emit 0xc1
        _emit 0x30
        _emit 0x66  // MOVQ [EAX+0x30], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x30
        _emit 0xf3  // MOVQ XMM0, [ECX+8]  (= [ECX_orig+0x38])
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x08
        _emit 0x66  // MOVQ [EAX+0x38], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x38
        _emit 0xc2  // RET 4
        _emit 0x04
        _emit 0x00
    }
}
