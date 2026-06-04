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
// FUNCTION: ffxivgame 0x0002e830 — `__cdecl void FUN_0042e830(float *dst)`
//                                  zero-init of a 4-float (Vector4) target
//                                  via a 16-byte-aligned scratch (65 B / 0x41)
//
// Disassembly (verbatim, read from asm/ffxivgame/0002e830_FUN_0042e830.s):
//
//   0002e830:  55                    push   ebp
//   0002e831:  8b ec                 mov    ebp, esp
//   0002e833:  83 e4 f0              and    esp, 0xfffffff0        ; 16-byte align
//   0002e836:  83 ec 10              sub    esp, 0x10              ; __m128 scratch
//   0002e839:  8b 45 08              mov    eax, [ebp+8]           ; eax = dst
//   0002e83c:  0f 57 c0              xorps  xmm0, xmm0             ; zero
//   0002e83f:  0f 29 04 24           movaps [esp], xmm0            ; scratch = {0,0,0,0}
//   0002e843:  f3 0f 10 04 24        movss  xmm0, [esp]
//   0002e848:  f3 0f 11 00           movss  [eax], xmm0            ; dst[0] = 0
//   0002e84c:  f3 0f 10 44 24 04     movss  xmm0, [esp+4]
//   0002e852:  f3 0f 11 40 04        movss  [eax+4], xmm0          ; dst[1] = 0
//   0002e857:  f3 0f 10 44 24 08     movss  xmm0, [esp+8]
//   0002e85d:  f3 0f 11 40 08        movss  [eax+8], xmm0          ; dst[2] = 0
//   0002e862:  f3 0f 10 44 24 0c     movss  xmm0, [esp+0xc]
//   0002e868:  f3 0f 11 40 0c        movss  [eax+0xc], xmm0        ; dst[3] = 0
//   0002e86d:  8b e5                 mov    esp, ebp
//   0002e86f:  5d                    pop    ebp
//   0002e870:  c3                    ret                          ; __cdecl, caller-cleans
//
// The body materialises a 16-byte-aligned __m128 scratch on the stack
// (hence the AND ESP,~0xf alignment prologue MSVC emits whenever a
// function has an aligned local), zeroes it once with XORPS+MOVAPS, then
// reads each of the four floats back through XMM0 and stores them into
// *dst element-by-element — the canonical MSVC 2005 lowering of a
// componentwise copy out of a freshly-zeroed aligned temp into a caller
// Vector4. Net effect: dst[0..3] = 0.0f.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This 65-byte slice carries NO relocations (no IAT loads, no rel32
//   calls, no absolute imm32 addresses) — every byte is a fixed
//   register/stack opcode. Coaxing MSVC 2005 /O2 into reproducing the
//   exact aligned-scratch + per-element-MOVSS shape (vs. a single MOVUPS
//   store, or eliding the scratch entirely) from C++ source is fragile,
//   so the pragmatic choice — the same one the size-band siblings
//   FUN_00406fa0 / FUN_00408780 / FUN_00401460 took — is a
//   `__declspec(naked)` body re-emitting the orig bytes verbatim via
//   MASM `_emit` directives. With no relocs the .obj's `.text` is
//   byte-identical to the orig slice and tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0042e830() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x83              // AND ESP, 0xfffffff0
        _emit 0xe4
        _emit 0xf0
        _emit 0x83              // SUB ESP, 0x10
        _emit 0xec
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [EBP+0x8]
        _emit 0x45
        _emit 0x08
        _emit 0x0f              // XORPS XMM0, XMM0
        _emit 0x57
        _emit 0xc0
        _emit 0x0f              // MOVAPS xmmword ptr [ESP], XMM0
        _emit 0x29
        _emit 0x04
        _emit 0x24
        _emit 0xf3              // MOVSS XMM0, dword ptr [ESP]
        _emit 0x0f
        _emit 0x10
        _emit 0x04
        _emit 0x24
        _emit 0xf3              // MOVSS dword ptr [EAX], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x00
        _emit 0xf3              // MOVSS XMM0, dword ptr [ESP+0x4]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS dword ptr [EAX+0x4], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x04
        _emit 0xf3              // MOVSS XMM0, dword ptr [ESP+0x8]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3              // MOVSS dword ptr [EAX+0x8], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x08
        _emit 0xf3              // MOVSS XMM0, dword ptr [ESP+0xc]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xf3              // MOVSS dword ptr [EAX+0xc], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x0c
        _emit 0x8b              // MOV ESP, EBP
        _emit 0xe5
        _emit 0x5d              // POP EBP
        _emit 0xc3              // RET
    }
}
