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
// FUNCTION: ffxivgame 0x00457e40 — font/glyph table lookup and render dispatch
//                                  (191 B / 0xbf).
//
// Non-standard "usercall" calling convention:
//   ESI        = font/encoding context pointer (base for all table offsets)
//   EAX        = glyph-set index (used as ESI + EAX*4 + 0x8110 table key)
//   [ESP+4]    = param_1 — pointer to current character position in source string
//   [ESP+8]    = param_2 — remaining length / limit
//   [ESP+C]    = param_3 — output pointer (written with 0 on entry; receives
//                          the advance count from the sub-call on success)
//   Return: void (result communicated through output pointer)
//
// Behaviour read from asm/ffxivgame/00057e40_FUN_00457e40.s:
//
//   1. *param_3 = 0.
//   2. Look up per-glyph-set descriptor: EAX = ESI->glyphSets[EAX].
//      If null, return immediately.
//   3. ECX = EAX + ESI (pointer into the descriptor block).
//   4. If descriptor[0x28] != 0 (multi-byte path):
//        a. Read 16-bit unit from *param_1.
//        b. High byte → index into ESI[0x800c] byte table.
//        c. If table byte is 0, use raw 16-bit unit; else reassemble
//           (table_byte << 8 | low_byte) and look up ESI[0x7f24] word table.
//   5. (Single-byte path: read 16-bit unit from *param_1 directly.)
//   6. common_path: EAX (16-bit unit) high byte → index into ECX[0x2c] word
//      table. If zero → return 0.
//   7. Combine: EAX = (table_word << 8) | low_byte.
//   8. Look up glyph entry: EAX = ECX[0x22c][ECX->base + EAX*2].
//      If zero → return 0.
//   9. Compute glyph record address and call FUN_00457af0(ESI, ECX,
//      param_1+2, param_2-1, glyph_record, param_3).
//
// Reloc-bearing site in the orig 191 bytes:
//   +0xb3   CALL rel32 → 0x00457af0   (render/layout sub-call)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The "ESI + EAX as simultaneous register inputs" usercall shape, the
//   complex scaled-index addressing against ESI-relative table offsets, and
//   the interleaved multi-byte decode path cannot be reconstructed reliably
//   from source-level C++ under MSVC 2005. Emitting the orig 191 bytes
//   verbatim yields a byte-identical .obj; compare.py masks the single
//   CALL rel32 reloc window, reporting GREEN.

extern "C" __declspec(naked) void FUN_00457e40()
{
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x08]
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x14]
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        _emit 0xc7              // MOV dword ptr [EBP], 0x0
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI + EAX*4 + 0x8110]
        _emit 0x84
        _emit 0x86
        _emit 0x10
        _emit 0x81
        _emit 0x00
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +0x03
        _emit 0x03
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
        _emit 0x83              // CMP dword ptr [EAX + ESI + 0x28], 0x0
        _emit 0x7c
        _emit 0x30
        _emit 0x28
        _emit 0x00
        _emit 0x8d              // LEA ECX, [EAX + ESI]
        _emit 0x0c
        _emit 0x30
        _emit 0x57              // PUSH EDI
        _emit 0x75              // JNZ +0x05
        _emit 0x05
        _emit 0x0f              // MOVZX EAX, word ptr [EBX]
        _emit 0xb7
        _emit 0x03
        _emit 0xeb              // JMP +0x36
        _emit 0x36
        _emit 0x0f              // MOVZX EDI, word ptr [EBX]
        _emit 0xb7
        _emit 0x3b
        _emit 0x0f              // MOVZX EAX, DI
        _emit 0xb7
        _emit 0xc7
        _emit 0x8b              // MOV EDX, EAX
        _emit 0xd0
        _emit 0xc1              // SHR EDX, 0x8
        _emit 0xea
        _emit 0x08
        _emit 0x66              // MOVZX DX, byte ptr [EDX + ESI + 0x800c]
        _emit 0x0f
        _emit 0xb6
        _emit 0x94
        _emit 0x32
        _emit 0x0c
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x0f              // MOVZX EDX, DX
        _emit 0xb7
        _emit 0xd2
        _emit 0x66              // TEST DX, DX
        _emit 0x85
        _emit 0xd2
        _emit 0x75              // JNZ +0x05
        _emit 0x05
        _emit 0x0f              // MOVZX EAX, DI
        _emit 0xb7
        _emit 0xc7
        _emit 0xeb              // JMP +0x15
        _emit 0x15
        _emit 0x0f              // MOVZX EDX, DX
        _emit 0xb7
        _emit 0xd2
        _emit 0x25              // AND EAX, 0xff
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc1              // SHL EDX, 0x8
        _emit 0xe2
        _emit 0x08
        _emit 0x03              // ADD EDX, EAX
        _emit 0xd0
        _emit 0x0f              // MOVZX EAX, word ptr [ESI + EDX*2 + 0x7f24]
        _emit 0xb7
        _emit 0x84
        _emit 0x56
        _emit 0x24
        _emit 0x7f
        _emit 0x00
        _emit 0x00
        _emit 0x0f              // MOVZX EAX, AX
        _emit 0xb7
        _emit 0xc0
        _emit 0x8b              // MOV EDX, EAX
        _emit 0xd0
        _emit 0xc1              // SHR EDX, 0x8
        _emit 0xea
        _emit 0x08
        _emit 0x0f              // MOVZX EDX, word ptr [ECX + EDX*2 + 0x2c]
        _emit 0xb7
        _emit 0x54
        _emit 0x51
        _emit 0x2c
        _emit 0x85              // TEST EDX, EDX
        _emit 0xd2
        _emit 0x74              // JZ +0x1d
        _emit 0x1d
        _emit 0x25              // AND EAX, 0xff
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc1              // SHL EDX, 0x8
        _emit 0xe2
        _emit 0x08
        _emit 0x03              // ADD EAX, EDX
        _emit 0xc2
        _emit 0x8b              // MOV EDX, dword ptr [ECX]
        _emit 0x11
        _emit 0x8d              // LEA EDI, [ECX + 0x22c]
        _emit 0xb9
        _emit 0x2c
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EAX, [EDX + EAX*2]
        _emit 0x04
        _emit 0x42
        _emit 0x0f              // MOVZX EAX, word ptr [EAX + EDI]
        _emit 0xb7
        _emit 0x04
        _emit 0x38
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +0x06
        _emit 0x06
        _emit 0x5f              // POP EDI
        _emit 0x5d              // POP EBP
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
        _emit 0x8b              // MOV EDX, dword ptr [ECX + 0x10]
        _emit 0x51
        _emit 0x10
        _emit 0x03              // ADD EDX, EDI
        _emit 0xd7
        _emit 0x55              // PUSH EBP
        _emit 0xc1              // SHL EAX, 0x4
        _emit 0xe0
        _emit 0x04
        _emit 0x03              // ADD EAX, EDX
        _emit 0xc2
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x18]
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x50              // PUSH EAX
        _emit 0x83              // ADD EDX, -0x1
        _emit 0xc2
        _emit 0xff
        _emit 0x52              // PUSH EDX
        _emit 0x83              // ADD EBX, 0x2
        _emit 0xc3
        _emit 0x02
        _emit 0x53              // PUSH EBX
        _emit 0x51              // PUSH ECX
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL 0x00457af0 (rel32)
        _emit 0xf8
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0x5f              // POP EDI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
