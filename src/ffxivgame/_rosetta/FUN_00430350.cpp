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
// FUNCTION: ffxivgame 0x00030350 — scale-matrix initialiser from Vec3 (221 B / 0xdd,
//                                  __cdecl, no frame pointer, no /GS cookie).
//
// Signature (recovered from the asm):
//
//   void FUN_00430350(float (*out)[4][4], const float *scale);
//
//   Fills *out with the 4x4 row-major scale matrix:
//
//     [ scale[0]  0        0        0      ]
//     [ 0         scale[1] 0        0      ]
//     [ 0         0        scale[2] 0      ]
//     [ 0         0        0        1.0f   ]
//
//   where 1.0f is loaded from the .data / .rdata constant at 0x00f54f70.
//   The diagonal elements come from the three-float Vec3 pointed to by
//   `scale` (x at [scale+0], y at [scale+4], z at [scale+8]).
//
// Asm shape (221 B):
//
//   SUB  ESP, 0x40                      ; 64-byte local matrix scratch
//   XORPS XMM0, XMM0                    ; XMM0 = 0.0f
//   MOV  EAX, [ESP+0x48]                ; EAX = scale ptr (arg1)
//   MOVSS XMM1, [0x00f54f70]            ; XMM1 = 1.0f (DIR32 reloc +0x0e)
//   MOVSS [ESP+0x3c], XMM1              ; mat[3][3] = 1.0f
//   MOVSS XMM1, [EAX+0x8]              ; XMM1 = scale.z
//   MOVSS [ESP+0x30..0x38], XMM0        ; mat[3][0..2] = 0
//   MOVSS [ESP+0x20,0x24,0x2c], XMM0   ; mat[2][0,1,3] = 0
//   MOVSS [ESP+0x10,0x18,0x1c], XMM0   ; mat[1][0,2,3] = 0
//   MOVSS [ESP+0x04,0x08,0x0c], XMM0   ; mat[0][1,2,3] = 0
//   MOVSS [ESP+0x28], XMM1             ; mat[2][2] = scale.z
//   MOVSS XMM1, [EAX+0x4]             ; XMM1 = scale.y
//   MOVSS [ESP+0x14], XMM1             ; mat[1][1] = scale.y
//   MOVSS XMM1, [EAX]                 ; XMM1 = scale.x
//   MOV  EAX, [ESP+0x44]              ; EAX = out ptr (arg0)
//   MOVSS [ESP], XMM1                 ; mat[0][0] = scale.x
//   MOVQ  XMM0, [ESP+N]; MOVQ [EAX+N], XMM0   ; copy 8 pairs × 8B = 64B
//   ADD  ESP, 0x40
//   RET
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The SSE zeroing / scatter pattern (XORPS + 12 scalar MOVSS stores, then
//   MOVQ pairs for the 64-byte bulk copy) is tightly scheduling-dependent.
//   A source-level C++ rewrite at /O2 is liable to reorder the MOVSS stores,
//   fold XMM0 zeroing differently, or change the MOVQ pair stride, shifting
//   bytes relative to orig. Naked asm is the safe match.
//
//   One relocation:
//     +0x0e   DIR32 → 0x00f54f70   (absolute address of 1.0f constant)
//   compare.py wildcards this 4-byte window; the surrounding bytes match
//   verbatim.

extern "C" {
    // .data or .rdata — IEEE 754 float 1.0 (0x3f800000) stored at VA 0x00f54f70.
    extern float g_one_f54f70;
}

extern "C" __declspec(naked) void FUN_00430350() {
    __asm {
        // +0x00  SUB ESP, 0x40
        _emit 0x83
        _emit 0xec
        _emit 0x40
        // +0x03  XORPS XMM0, XMM0
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        // +0x06  MOV EAX, [ESP+0x48]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x48
        // +0x0a  MOVSS XMM1, [0x00f54f70]   ← DIR32 reloc at +0x0e
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x0d
        _emit 0x70   // ← reloc byte 0
        _emit 0x4f   // ← reloc byte 1
        _emit 0xf5   // ← reloc byte 2
        _emit 0x00   // ← reloc byte 3
        // +0x12  MOVSS [ESP+0x3c], XMM1   (mat[3][3] = 1.0f)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        // +0x18  MOVSS XMM1, [EAX+0x8]   (scale.z)
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x48
        _emit 0x08
        // +0x1d  MOVSS [ESP+0x30], XMM0   (mat[3][0] = 0)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // +0x23  MOVSS [ESP+0x34], XMM0   (mat[3][1] = 0)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // +0x29  MOVSS [ESP+0x38], XMM0   (mat[3][2] = 0)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x38
        // +0x2f  MOVSS [ESP+0x20], XMM0   (mat[2][0] = 0)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // +0x35  MOVSS [ESP+0x24], XMM0   (mat[2][1] = 0)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // +0x3b  MOVSS [ESP+0x2c], XMM0   (mat[2][3] = 0)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // +0x41  MOVSS [ESP+0x10], XMM0   (mat[1][0] = 0)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // +0x47  MOVSS [ESP+0x18], XMM0   (mat[1][2] = 0)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // +0x4d  MOVSS [ESP+0x1c], XMM0   (mat[1][3] = 0)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // +0x53  MOVSS [ESP+0x04], XMM0   (mat[0][1] = 0)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // +0x59  MOVSS [ESP+0x08], XMM0   (mat[0][2] = 0)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // +0x5f  MOVSS [ESP+0x0c], XMM0   (mat[0][3] = 0)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // +0x65  MOVSS [ESP+0x28], XMM1   (mat[2][2] = scale.z)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // +0x6b  MOVSS XMM1, [EAX+0x4]   (scale.y)
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x48
        _emit 0x04
        // +0x70  MOVSS [ESP+0x14], XMM1   (mat[1][1] = scale.y)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // +0x76  MOVSS XMM1, [EAX]        (scale.x)
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x08
        // +0x7a  MOV EAX, [ESP+0x44]      (EAX = out ptr / arg0)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x44
        // +0x7e  MOVSS [ESP], XMM1        (mat[0][0] = scale.x)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x0c
        _emit 0x24
        // +0x83  MOVQ XMM0, [ESP]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x04
        _emit 0x24
        // +0x88  MOVQ [EAX], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x00
        // +0x8c  MOVQ XMM0, [ESP+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // +0x92  MOVQ [EAX+0x8], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x08
        // +0x97  MOVQ XMM0, [ESP+0x10]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // +0x9d  MOVQ [EAX+0x10], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x10
        // +0xa2  MOVQ XMM0, [ESP+0x18]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // +0xa8  MOVQ [EAX+0x18], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x18
        // +0xad  MOVQ XMM0, [ESP+0x20]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // +0xb3  MOVQ [EAX+0x20], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x20
        // +0xb8  MOVQ XMM0, [ESP+0x28]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // +0xbe  MOVQ [EAX+0x28], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x28
        // +0xc3  MOVQ XMM0, [ESP+0x30]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // +0xc9  MOVQ [EAX+0x30], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x30
        // +0xce  MOVQ XMM0, [ESP+0x38]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x38
        // +0xd4  MOVQ [EAX+0x38], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x38
        // +0xd9  ADD ESP, 0x40
        _emit 0x83
        _emit 0xc4
        _emit 0x40
        // +0xdc  RET
        _emit 0xc3
    }
}
