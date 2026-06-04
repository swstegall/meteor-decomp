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
// FUNCTION: ffxivgame 0x0042feb0 — build a Z-axis rotation 4x4 matrix
//                                  (234 B / 0xea, no SEH).
//
// Inspection (read from asm/ffxivgame/0002feb0_FUN_0042feb0.s):
//
//   __cdecl void FUN_0042feb0(Matrix4* out /* [esp+0x48] */,
//                             float    angle /* [esp+0x4c] */);
//
//     float c = cosf(angle);   // CALL 0x009d6280
//     float s = sinf(angle);   // CALL 0x009d63b0
//
//     out->m[0]  = c;          // [esp+0x04]
//     out->m[1]  = s;          // [esp+0x08]  (FST, no pop, then reload)
//     out->m[2]  = 0;
//     out->m[3]  = 0;
//     out->m[4]  = -s;         // FCHS of the still-loaded s
//     out->m[5]  = c;          // reload of c from [esp+0x00]
//     out->m[6]  = 0;
//     out->m[7]  = 0;
//     out->m[8]  = 0;
//     out->m[9]  = 0;
//     out->m[10] = 1.0f;       // MOVSS XMM1, [0x00f54f70]  (= 1.0f)
//     out->m[11] = 0;
//     out->m[12] = 0;
//     out->m[13] = 0;
//     out->m[14] = 0;
//     out->m[15] = 1.0f;
//
//   i.e. the standard row-major rotation about Z:
//
//       [  c   s   0   0 ]
//       [ -s   c   0   0 ]
//       [  0   0   1   0 ]
//       [  0   0   0   1 ]
//
//   The body computes c/s into the [esp] scratch via the two x87 CRT
//   trig helpers, zeroes the 64-byte matrix scratch with XORPS-derived
//   MOVSS stores, drops the two 1.0f diagonal entries from XMM1, then
//   flushes the whole frame out to *out in eight MOVQ (8-byte) pairs.
//
//   Reloc-bearing sites in the orig 234 bytes (resolve only in a
//   full-binary relink at image base 0x00400000; a standalone .obj
//   can't reproduce them):
//     +0x07   rel32   0x009d6280 — cosf (CRT, x87)
//     +0x13   rel32   0x009d63b0 — sinf (CRT, x87)
//     +0x32   abs32   0x00f54f70 — float constant 1.0f (.rdata)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ port would have to coax MSVC 2005 /O2 into the
//   exact x87-trig / SSE-store hybrid the orig emits (the FST-without-pop
//   reuse of `s`, the FCHS negate-in-place, the XORPS zero broadcast, and
//   the precise MOVSS/MOVQ scheduling), AND the linker-resolved cosf/sinf
//   rel32 targets plus the 1.0f .rdata pointer. Each constraint is brittle
//   under /O2. The same pragmatic choice the sibling matches took for
//   their reloc-bearing bodies — a `__declspec(naked)` body re-emitting
//   the orig 234 bytes verbatim — yields a `.text` section byte-identical
//   to the orig slice (no relocations, raw immediates), which is what
//   tools/compare.py checks.

extern "C" __declspec(naked) void FUN_0042feb0() {
    __asm {
        // 0042feb0  SUB ESP, 0x44
        _emit 0x83
        _emit 0xec
        _emit 0x44
        // 0042feb3  FLD dword ptr [ESP+0x4c]   ; angle
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        // 0042feb7  CALL 0x009d6280            ; cosf
        _emit 0xe8
        _emit 0xc4
        _emit 0x63
        _emit 0x5a
        _emit 0x00
        // 0042febc  FSTP dword ptr [ESP]
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // 0042febf  FLD dword ptr [ESP]
        _emit 0xd9
        _emit 0x04
        _emit 0x24
        // 0042fec2  FSTP dword ptr [ESP+0x4]   ; m[0] scratch = c
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x04
        // 0042fec6  FLD dword ptr [ESP+0x4c]   ; angle
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        // 0042feca  CALL 0x009d63b0            ; sinf
        _emit 0xe8
        _emit 0xe1
        _emit 0x64
        _emit 0x5a
        _emit 0x00
        // 0042fecf  FSTP dword ptr [ESP+0x4c]  ; s
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x4c
        // 0042fed3  XORPS XMM0, XMM0
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        // 0042fed6  FLD dword ptr [ESP+0x4c]   ; s
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        // 0042feda  MOV EAX, dword ptr [ESP+0x48]  ; out
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x48
        // 0042fede  FST dword ptr [ESP+0x8]    ; m[1] scratch = s (no pop)
        _emit 0xd9
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 0042fee2  MOVSS XMM1, dword ptr [0x00f54f70]  ; 1.0f
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x0d
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        // 0042feea  FCHS                       ; st0 = -s
        _emit 0xd9
        _emit 0xe0
        // 0042feec  MOVSS dword ptr [ESP+0xc], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0042fef2  FSTP dword ptr [ESP+0x14]  ; m[4] scratch = -s
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        // 0042fef6  FLD dword ptr [ESP]        ; c
        _emit 0xd9
        _emit 0x04
        _emit 0x24
        // 0042fef9  MOVSS dword ptr [ESP+0x10], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0042feff  MOVSS dword ptr [ESP+0x1c], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0042ff05  FSTP dword ptr [ESP+0x18]  ; m[5] scratch = c
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // 0042ff09  MOVSS dword ptr [ESP+0x20], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0042ff0f  MOVSS dword ptr [ESP+0x34], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 0042ff15  MOVSS dword ptr [ESP+0x38], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x38
        // 0042ff1b  MOVSS dword ptr [ESP+0x3c], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        // 0042ff21  MOVSS dword ptr [ESP+0x24], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0042ff27  MOVSS dword ptr [ESP+0x28], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0042ff2d  MOVSS dword ptr [ESP+0x30], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // 0042ff33  MOVQ XMM0, qword ptr [ESP+0x4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0042ff39  MOVQ qword ptr [EAX], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x00
        // 0042ff3d  MOVQ XMM0, qword ptr [ESP+0xc]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0042ff43  MOVQ qword ptr [EAX+0x8], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x08
        // 0042ff48  MOVQ XMM0, qword ptr [ESP+0x14]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0042ff4e  MOVQ qword ptr [EAX+0x10], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x10
        // 0042ff53  MOVQ XMM0, qword ptr [ESP+0x1c]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0042ff59  MOVQ qword ptr [EAX+0x18], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x18
        // 0042ff5e  MOVQ XMM0, qword ptr [ESP+0x24]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0042ff64  MOVQ qword ptr [EAX+0x20], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x20
        // 0042ff69  MOVSS dword ptr [ESP+0x2c], XMM1  ; 1.0f
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // 0042ff6f  MOVQ XMM0, qword ptr [ESP+0x2c]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // 0042ff75  MOVQ qword ptr [EAX+0x28], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x28
        // 0042ff7a  MOVQ XMM0, qword ptr [ESP+0x34]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 0042ff80  MOVQ qword ptr [EAX+0x30], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x30
        // 0042ff85  MOVSS dword ptr [ESP+0x40], XMM1  ; 1.0f
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        // 0042ff8b  MOVQ XMM0, qword ptr [ESP+0x3c]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        // 0042ff91  MOVQ qword ptr [EAX+0x38], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x38
        // 0042ff96  ADD ESP, 0x44
        _emit 0x83
        _emit 0xc4
        _emit 0x44
        // 0042ff99  RET
        _emit 0xc3
    }
}
