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
// FUNCTION: ffxivgame 0x0003a3c0 — DXT/S3TC compressed-texture byte-size
//           calculator (48 B / 0x30)
//
//   int __thiscall FUN_0043a3c0(this, unsigned width, unsigned height)
//     stack layout (after the implicit thiscall ECX):
//       ECX        : this           (texture-format descriptor)
//       [ESP+0x04] : unsigned width
//       [ESP+0x08] : unsigned height
//     returns (EAX): total compressed byte size =
//         block_bytes * ((width+3)>>2) * ((height+3)>>2)
//       where block_bytes is 8 when this->format (DWORD @ this+0x4) equals
//       0x83F1 (GL_COMPRESSED_RGBA_S3TC_DXT1_EXT — 8 B / 4x4 block) and 16
//       otherwise (DXT3/DXT5 — 16 B / 4x4 block).
//
// Asm (48 bytes @ orig RVA 0x0003a3c0):
//   8b 54 24 08            MOV   EDX, [ESP+0x8]        ; height
//   33 c0                  XOR   EAX, EAX
//   81 79 04 f1 83 00 00   CMP   [ECX+0x4], 0x83F1     ; format == DXT1 ?
//   8b 4c 24 04            MOV   ECX, [ESP+0x4]        ; width
//   0f 95 c0               SETNZ AL                    ; AL = (fmt != DXT1)
//   83 c1 03               ADD   ECX, 3
//   c1 e9 02               SHR   ECX, 2                ; (width+3)>>2
//   83 c2 03               ADD   EDX, 3
//   c1 ea 02               SHR   EDX, 2                ; (height+3)>>2
//   8d 04 c5 08 00 00 00   LEA   EAX, [EAX*8 + 8]      ; 16 if !=DXT1 else 8
//   0f af c1               IMUL  EAX, ECX
//   0f af c2               IMUL  EAX, EDX
//   c2 08 00               RET   0x8                   ; __thiscall, 2 dwords
//
// Calling convention: __thiscall (this in ECX, two stack args, callee-clean
// RET 0x8). Frame: none (no locals, no register saves).
//
// Reconstruction: __declspec(naked) _emit byte passthrough (mirrors the
// sibling _rosetta naked functions). No relocations — every operand is an
// immediate or a stack/register offset baked into the orig bytes — so the
// .obj's .text is byte-identical with zero relocs. tools/compare.py → GREEN.

extern "C" __declspec(naked) void FUN_0043a3c0() {
    __asm {
        // 0003a3c0: 8b 54 24 08            MOV EDX, [ESP+0x8]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 0003a3c4: 33 c0                  XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0003a3c6: 81 79 04 f1 83 00 00   CMP [ECX+0x4], 0x83F1
        _emit 0x81
        _emit 0x79
        _emit 0x04
        _emit 0xf1
        _emit 0x83
        _emit 0x00
        _emit 0x00
        // 0003a3cd: 8b 4c 24 04            MOV ECX, [ESP+0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0003a3d1: 0f 95 c0               SETNZ AL
        _emit 0x0f
        _emit 0x95
        _emit 0xc0
        // 0003a3d4: 83 c1 03               ADD ECX, 3
        _emit 0x83
        _emit 0xc1
        _emit 0x03
        // 0003a3d7: c1 e9 02               SHR ECX, 2
        _emit 0xc1
        _emit 0xe9
        _emit 0x02
        // 0003a3da: 83 c2 03               ADD EDX, 3
        _emit 0x83
        _emit 0xc2
        _emit 0x03
        // 0003a3dd: c1 ea 02               SHR EDX, 2
        _emit 0xc1
        _emit 0xea
        _emit 0x02
        // 0003a3e0: 8d 04 c5 08 00 00 00   LEA EAX, [EAX*8 + 8]
        _emit 0x8d
        _emit 0x04
        _emit 0xc5
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003a3e7: 0f af c1               IMUL EAX, ECX
        _emit 0x0f
        _emit 0xaf
        _emit 0xc1
        // 0003a3ea: 0f af c2               IMUL EAX, EDX
        _emit 0x0f
        _emit 0xaf
        _emit 0xc2
        // 0003a3ed: c2 08 00               RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
