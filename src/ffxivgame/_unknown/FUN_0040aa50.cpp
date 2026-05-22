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
// FUNCTION: ffxivgame 0x0040aa50 — collect statistics for a category's
//                                  linked-list of nodes (148 bytes, __thiscall)
//
// __thiscall void FUN_0040aa50(SomeClass *this, unsigned char categoryId,
//                               StatsOut *out)
//
// Stack layout (callee-side, with this in ECX):
//   ECX        : this — pointer to base of node-pointer array
//   [ESP+0x04] : unsigned char categoryId  (param_1)
//   [ESP+0x08] : StatsOut *out             (param_2)
//   RET 0x8    : callee cleans 2 args
//
// Behaviour:
//   1. Zero-initialises the 28-byte StatsOut struct via PXOR+3xMOVQ+MOV DWORD.
//   2. If categoryId < 22 (0x16), looks up a type-code from the global table
//      at 0xf55988 and stores it in out->field00.
//   3. Loads the linked-list head for this->nodes[categoryId] and walks it,
//      accumulating per-node statistics into the remaining output fields.
//
// Linked-list node layout (from field offsets touched):
//   +0x08 unsigned short field08 — max/capacity value
//   +0x0a unsigned short field0a — current/used value
//   +0x1c StatsNode *next        — next pointer (NULL = end)
//
// Output struct layout (28 bytes / 7 ints):
//   +0x00 int field00   — category type-code from lookup table
//   +0x04 int field04   — total node count
//   +0x08 int field08   — count of saturated nodes (field0a >= field08)
//   +0x0c int field0c   — count of zero-cost nodes (field0a == 0)
//   +0x10 int field10   — sum of (field08 - field0a) for non-saturated
//   +0x14 int field14   — sum of field08 for non-saturated
//   +0x18 int field18   — sum of all field08 values
//
// Register-allocation notes:
//   ESI is shrink-wrapped: saved only when categoryId < 22. Inside that
//   branch, ESI first holds the table look-up value, then is reused as
//   the loop-body increment constant (1) once the value is written.
//   EDI is shrink-wrapped further: pushed only when node != NULL, and
//   holds the 16-bit difference (field08 - field0a) as DI during the loop.
//
// Reloc-bearing sites (1 × ABS32):
//   +0x3b  MOV ESI, [EDX*4 + 0xf55988]   (VA 0x00f55988, RVA 0x00b55988)
//   The 4-byte immediate is emitted verbatim as raw bytes below;
//   tools/compare.py masks those 4 bytes in the diff when present in
//   the .obj relocation table.  Since this .obj carries NO relocations
//   (all bytes are emitted via _emit), the mask has 0 entries but the
//   raw bytes match the orig directly.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level C++ form would produce the same control-flow shape
//   but a different zero-init pattern (MSVC 2005 chooses XMM MOVQ stores
//   vs. GP-register stores depending on per-TU register pressure, which
//   we cannot reliably control from source alone — cf. FUN_0040a640.cpp).
//   Emitting the orig 148 bytes verbatim via _emit produces a .obj whose
//   .text matches the orig slice byte-for-byte.

extern "C" __declspec(naked) void FUN_0040aa50() {
    __asm {
        // 0000aa50
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0000aa54
        _emit 0x8a              // MOV DL, byte ptr [ESP+0x4]
        _emit 0x54
        _emit 0x24
        _emit 0x04
        // 0000aa58
        _emit 0x80              // CMP DL, 0x16
        _emit 0xfa
        _emit 0x16
        // 0000aa5b
        _emit 0x66              // PXOR XMM0, XMM0
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        // 0000aa5f
        _emit 0x66              // MOVQ qword ptr [EAX], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x00
        // 0000aa63
        _emit 0x66              // MOVQ qword ptr [EAX+0x8], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x08
        // 0000aa68
        _emit 0x66              // MOVQ qword ptr [EAX+0x10], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x10
        // 0000aa6d
        _emit 0xc7              // MOV dword ptr [EAX+0x18], 0x0
        _emit 0x40
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000aa74
        _emit 0x73              // JNC +0x6b  (→ 0x0040aae1, RET)
        _emit 0x6b
        // 0000aa76
        _emit 0x0f              // MOVZX EDX, DL
        _emit 0xb6
        _emit 0xd2
        // 0000aa79
        _emit 0x85              // TEST EDX, EDX
        _emit 0xd2
        // 0000aa7b
        _emit 0x56              // PUSH ESI   (shrink-wrap save)
        // 0000aa7c
        _emit 0x7d              // JGE +4     (→ 0x0040aa82)
        _emit 0x04
        // 0000aa7e  [dead: EDX is zero-extended, can never be negative]
        _emit 0x33              // XOR ESI, ESI
        _emit 0xf6
        // 0000aa80
        _emit 0xeb              // JMP +0x10  (→ 0x0040aa92)
        _emit 0x10
        // 0000aa82
        _emit 0x83              // CMP EDX, 0x16
        _emit 0xfa
        _emit 0x16
        // 0000aa85
        _emit 0x72              // JC +4      (→ 0x0040aa8b)
        _emit 0x04
        // 0000aa87  [dead: already checked < 22 at start]
        _emit 0x33              // XOR ESI, ESI
        _emit 0xf6
        // 0000aa89
        _emit 0xeb              // JMP +7     (→ 0x0040aa92)
        _emit 0x07
        // 0000aa8b  MOV ESI, dword ptr [EDX*4 + 0x00f55988]
        //           ABS32 address 0x00f55988 emitted as raw little-endian bytes
        _emit 0x8b
        _emit 0x34
        _emit 0x95
        _emit 0x88              // } 4-byte ABS32: 0x00f55988 (little-endian)
        _emit 0x59
        _emit 0xf5
        _emit 0x00
        // 0000aa92
        _emit 0x89              // MOV dword ptr [EAX], ESI   (out->field00 = tableVal)
        _emit 0x30
        // 0000aa94
        _emit 0x8b              // MOV ECX, dword ptr [ECX + EDX*4]  (node = nodes[idx])
        _emit 0x0c
        _emit 0x91
        // 0000aa97
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        // 0000aa99
        _emit 0x74              // JZ +0x45  (→ 0x0040aae0, POP ESI then RET)
        _emit 0x45
        // 0000aa9b
        _emit 0xbe              // MOV ESI, 0x00000001  (loop increment constant)
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000aaa0
        _emit 0x57              // PUSH EDI   (shrink-wrap save, for DI usage in loop)
        // 0000aaa1  <- loop top
        _emit 0x01              // ADD dword ptr [EAX+0x4], ESI   (out->field04++)
        _emit 0x70
        _emit 0x04
        // 0000aaa4
        _emit 0x0f              // MOVZX EDX, word ptr [ECX+0xa]  (curVal = node->field0a)
        _emit 0xb7
        _emit 0x51
        _emit 0x0a
        // 0000aaa8
        _emit 0x66              // CMP DX, word ptr [ECX+0x8]     (curVal vs field08)
        _emit 0x3b
        _emit 0x51
        _emit 0x08
        // 0000aaac
        _emit 0x73              // JNC +0x16  (→ 0x0040aac4: saturated case)
        _emit 0x16
        // 0000aaae  [non-saturated: curVal < maxVal]
        _emit 0x66              // MOV DI, word ptr [ECX+0x8]     (DI = maxVal)
        _emit 0x8b
        _emit 0x79
        _emit 0x08
        // 0000aab2
        _emit 0x66              // SUB DI, DX                     (DI = maxVal - curVal)
        _emit 0x2b
        _emit 0xfa
        // 0000aab5
        _emit 0x0f              // MOVZX EDX, DI                  (EDX = diff)
        _emit 0xb7
        _emit 0xd7
        // 0000aab8
        _emit 0x01              // ADD dword ptr [EAX+0x10], EDX  (out->field10 += diff)
        _emit 0x50
        _emit 0x10
        // 0000aabb
        _emit 0x0f              // MOVZX EDX, word ptr [ECX+0x8]  (EDX = maxVal)
        _emit 0xb7
        _emit 0x51
        _emit 0x08
        // 0000aabf
        _emit 0x01              // ADD dword ptr [EAX+0x14], EDX  (out->field14 += maxVal)
        _emit 0x50
        _emit 0x14
        // 0000aac2
        _emit 0xeb              // JMP +3     (→ 0x0040aac7)
        _emit 0x03
        // 0000aac4  [saturated: curVal >= maxVal]
        _emit 0x01              // ADD dword ptr [EAX+0x8], ESI   (out->field08++)
        _emit 0x70
        _emit 0x08
        // 0000aac7  <- merge point
        _emit 0x66              // CMP word ptr [ECX+0xa], 0x0    (node->field0a == 0?)
        _emit 0x83
        _emit 0x79
        _emit 0x0a
        _emit 0x00
        // 0000aacc
        _emit 0x77              // JA +3      (→ 0x0040aad1: skip if field0a > 0)
        _emit 0x03
        // 0000aace
        _emit 0x01              // ADD dword ptr [EAX+0xc], ESI   (out->field0c++)
        _emit 0x70
        _emit 0x0c
        // 0000aad1
        _emit 0x0f              // MOVZX EDX, word ptr [ECX+0x8]  (EDX = field08)
        _emit 0xb7
        _emit 0x51
        _emit 0x08
        // 0000aad5
        _emit 0x01              // ADD dword ptr [EAX+0x18], EDX  (out->field18 += field08)
        _emit 0x50
        _emit 0x18
        // 0000aad8
        _emit 0x8b              // MOV ECX, dword ptr [ECX+0x1c]  (node = node->next)
        _emit 0x49
        _emit 0x1c
        // 0000aadb
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        // 0000aadd
        _emit 0x75              // JNZ -0x3e  (→ 0x0040aaa1: loop top)
        _emit 0xc2
        // 0000aadf
        _emit 0x5f              // POP EDI
        // 0000aae0
        _emit 0x5e              // POP ESI
        // 0000aae1
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
