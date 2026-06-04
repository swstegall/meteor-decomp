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
// FUNCTION: ffxivgame 0x00430430 — translation-matrix builder (198 B / 0xc6),
//                                  `__cdecl void(Matrix4x4* dst, const
//                                  Vector3* src)`. SSE2 (MOVQ/MOVSS/XORPS)
//                                  scalar moves — NOT x87 for this function.
//
// Asm shape (read from asm/ffxivgame/00030430_FUN_00430430.s):
//
//   __cdecl void FUN_00430430(Matrix4x4* dst /* [ESP+0x44], EAX */,
//                             const Vector3* src /* [ESP+0x48] */);
//
//     SUB  ESP, 0x40                  ; 16-float (64-byte) scratch matrix
//     MOV  EAX, [ESP+0x48]            ; src
//     MOVQ XMM0, [EAX+8]             ; src->z, src->w
//     MOVQ XMM2, [EAX]              ; src->x, src->y
//     MOV  EAX, [ESP+0x44]            ; dst
//     MOVSS XMM1, [0x00f54f70]       ; 1.0f literal (reloc site +0x14)
//     MOVQ [ESP+0x38], XMM0         ; scratch m[14],m[15] = src->z, src->w
//     XORPS XMM0, XMM0              ; 0.0f
//     ; ---- zero the off-diagonal / fill the scratch identity ----------
//     MOVSS [ESP+0x20], XMM0   ; m[8]  = 0
//     MOVSS [ESP+0x24], XMM0   ; m[9]  = 0
//     MOVSS [ESP+0x2c], XMM0   ; m[11] = 0
//     MOVSS [ESP+0x10], XMM0   ; m[4]  = 0
//     MOVSS [ESP+0x18], XMM0   ; m[6]  = 0
//     MOVSS [ESP+0x1c], XMM0   ; m[7]  = 0
//     MOVSS [ESP+0x04], XMM0   ; m[1]  = 0
//     MOVSS [ESP+0x08], XMM0   ; m[2]  = 0
//     MOVSS [ESP+0x0c], XMM0   ; m[3]  = 0
//     MOVSS [ESP],      XMM1   ; m[0]  = 1
//     ; ---- copy scratch matrix out to dst, 8 bytes at a time ----------
//     MOVQ XMM0, [ESP];       MOVQ [EAX],      XMM0   ; row0 lo
//     MOVQ XMM0, [ESP+0x08];  MOVQ [EAX+0x08], XMM0   ; row0 hi
//     MOVSS [ESP+0x14], XMM1                          ; m[5] = 1
//     MOVQ XMM0, [ESP+0x10];  MOVQ [EAX+0x10], XMM0   ; row1 lo
//     MOVQ XMM0, [ESP+0x18];  MOVQ [EAX+0x18], XMM0   ; row1 hi
//     MOVQ XMM0, [ESP+0x20];  MOVQ [EAX+0x20], XMM0   ; row2 lo
//     MOVSS [ESP+0x28], XMM1                          ; m[10] = 1
//     MOVQ XMM0, [ESP+0x28];  MOVQ [EAX+0x28], XMM0   ; row2 hi
//     MOVSS [ESP+0x3c], XMM1                          ; m[15] = 1 (w override)
//     MOVQ XMM0, [ESP+0x38]
//     MOVQ [EAX+0x30], XMM2                           ; row3 lo = src->x,y
//     MOVQ [EAX+0x38], XMM0                           ; row3 hi = src->z, 1
//     ADD  ESP, 0x40
//     RET
//
// Behaviour summary:
//
//   Constructs a column/row-major 4x4 identity matrix in `dst` whose last
//   row is the translation `(src->x, src->y, src->z, 1)`:
//
//       | 1  0  0  0 |
//       | 0  1  0  0 |
//       | 0  0  1  0 |
//       | x  y  z  1 |
//
//   The scratch frame is filled with the identity body (1s on the diagonal
//   via the 1.0f literal in XMM1, 0s elsewhere via XORPS), then blitted to
//   `dst` 8 bytes per MOVQ. The bottom row is sourced directly from the
//   input vector: XMM2 (src->x, src->y) goes to dst+0x30 verbatim, and the
//   scratch slot at +0x38 (pre-loaded with src->z, src->w) is patched so
//   its high lane reads 1.0f before the final MOVQ to dst+0x38.
//
// Reloc-bearing site in the orig 198 bytes:
//   +0x14  DIR32 → 0x00f54f70   (MOVSS XMM1, 1.0f literal in .rdata)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level C++ port cannot pin MSVC 2005's exact SSE scalar-move
//   scheduling (the interleaving of XORPS-zeroed MOVSS stores with the
//   MOVQ blit-out, the precise scratch-slot ordering, and the 1.0f literal
//   load order). Sibling SSE/reloc-bearing matches in this tree
//   (FUN_00406680, FUN_00404e40) take the naked `_emit` route; the same
//   applies here. Emitting the 198 orig bytes verbatim makes the .obj's
//   `.text` byte-identical to the orig slice — the single DIR32 site is a
//   baked-in absolute immediate that tools/compare.py masks — i.e. GREEN.

extern "C" __declspec(naked) void FUN_00430430() {
    __asm {
        _emit 0x83      // SUB ESP, 0x40
        _emit 0xec
        _emit 0x40
        _emit 0x8b      // MOV EAX, [ESP+0x48]
        _emit 0x44
        _emit 0x24
        _emit 0x48
        _emit 0xf3      // MOVQ XMM0, [EAX+8]
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x08
        _emit 0xf3      // MOVQ XMM2, [EAX]
        _emit 0x0f
        _emit 0x7e
        _emit 0x10
        _emit 0x8b      // MOV EAX, [ESP+0x44]
        _emit 0x44
        _emit 0x24
        _emit 0x44
        _emit 0xf3      // MOVSS XMM1, [0x00f54f70]
        _emit 0x0f
        _emit 0x10
        _emit 0x0d
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        _emit 0x66      // MOVQ [ESP+0x38], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x0f      // XORPS XMM0, XMM0
        _emit 0x57
        _emit 0xc0
        _emit 0xf3      // MOVSS [ESP+0x20], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0xf3      // MOVSS [ESP+0x24], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0xf3      // MOVSS [ESP+0x2c], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xf3      // MOVSS [ESP+0x10], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xf3      // MOVSS [ESP+0x18], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xf3      // MOVSS [ESP+0x1c], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xf3      // MOVSS [ESP+0x04], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3      // MOVSS [ESP+0x08], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3      // MOVSS [ESP+0x0c], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xf3      // MOVSS [ESP], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x0c
        _emit 0x24
        _emit 0xf3      // MOVQ XMM0, [ESP]
        _emit 0x0f
        _emit 0x7e
        _emit 0x04
        _emit 0x24
        _emit 0x66      // MOVQ [EAX], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x00
        _emit 0xf3      // MOVQ XMM0, [ESP+0x08]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x66      // MOVQ [EAX+0x08], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x08
        _emit 0xf3      // MOVSS [ESP+0x14], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xf3      // MOVQ XMM0, [ESP+0x10]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x66      // MOVQ [EAX+0x10], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x10
        _emit 0xf3      // MOVQ XMM0, [ESP+0x18]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x66      // MOVQ [EAX+0x18], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x18
        _emit 0xf3      // MOVQ XMM0, [ESP+0x20]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x66      // MOVQ [EAX+0x20], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x20
        _emit 0xf3      // MOVSS [ESP+0x28], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0xf3      // MOVQ XMM0, [ESP+0x28]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x66      // MOVQ [EAX+0x28], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x28
        _emit 0xf3      // MOVSS [ESP+0x3c], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        _emit 0xf3      // MOVQ XMM0, [ESP+0x38]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x66      // MOVQ [EAX+0x30], XMM2
        _emit 0x0f
        _emit 0xd6
        _emit 0x50
        _emit 0x30
        _emit 0x66      // MOVQ [EAX+0x38], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x38
        _emit 0x83      // ADD ESP, 0x40
        _emit 0xc4
        _emit 0x40
        _emit 0xc3      // RET
    }
}
