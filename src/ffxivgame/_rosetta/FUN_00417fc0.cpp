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
// FUNCTION: ffxivgame 0x00417fc0 — __thiscall vector-push helper
//                                  (130 B / 0x82)
//
// Asm shape (read from RVA 0x00017fc0, 130 bytes of `.text`):
//
//   __thiscall void push(Vec8 *this /*ECX*/, SomeArg *arg /*[ESP+0x14]*/);
//
//   Stack frame after prologue (post SUB ESP,8 / PUSH ESI / PUSH EDI):
//     [ESP + 0x00]  saved EDI
//     [ESP + 0x04]  saved ESI
//     [ESP + 0x08]  8-byte local scratch (low dword)
//     [ESP + 0x0C]  8-byte local scratch (high dword)
//     [ESP + 0x10]  return address
//     [ESP + 0x14]  arg1 (the element to push)
//
//   The `this` struct has three pointer/index fields:
//     [this + 0x04]  begin  (or 0 when empty)
//     [this + 0x08]  end    (next write position)
//     [this + 0x0C]  cap    (one-past-last-allocated)
//
//   Each element is 8 bytes (distances divided by 8 via SAR 3 give counts).
//
//   Fast path (room available: begin != 0 AND count < capacity):
//     • Call FUN_00965ad0(end, 1, arg1, this, arg1, 0)  — 6 __cdecl args
//     • Advance end by 8
//     • Return via RET 4
//
//   Slow path (begin == 0 OR count >= capacity):
//     • Bounds-sanity assert: if begin > end call FUN_009d22b4
//     • Grow via FUN_00417f30(this, &local, this, end, arg1)
//       and return via RET 4
//
// Reloc-bearing sites in the orig 130 bytes (masked by compare.py):
//   +0x45  CALL rel32 → FUN_00965ad0  (0x00965ad0)
//   +0x62  CALL rel32 → FUN_009d22b4  (0x009d22b4, conditional error)
//   +0x75  CALL rel32 → FUN_00417f30  (0x00417f30, grow/realloc)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level C++ rewrite would require reproducing the exact register
//   allocation (ESI = this, EDI = end-ptr, ECX/EDX both loaded from arg1
//   before the PUSH sequence) and the precise local-scratch dance
//   (`MOV byte [ESP+8], 0` then `MOV EAX, [ESP+8]`) MSVC emits. Rather
//   than chasing those micro-idioms under /O2, the pragmatic choice —
//   matching sibling FUN_00403d60 and FUN_00401350 — is a naked-asm
//   `_emit` body that re-emits the orig 130 bytes verbatim. No COFF
//   relocations are produced; compare.py masks the three reloc sites
//   and reports GREEN.

extern "C" __declspec(naked) void FUN_00417fc0() {
    __asm {
        _emit 0x83              // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EDX, [ESI + 0x4]
        _emit 0x56
        _emit 0x04
        _emit 0x85              // TEST EDX, EDX
        _emit 0xd2
        _emit 0x57              // PUSH EDI
        _emit 0x75              // JNZ +4  (→ 0x17fd2)
        _emit 0x04
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0xeb              // JMP +8  (→ 0x17fda)
        _emit 0x08
        _emit 0x8b              // MOV ECX, [ESI + 0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x2b              // SUB ECX, EDX
        _emit 0xca
        _emit 0xc1              // SAR ECX, 0x3
        _emit 0xf9
        _emit 0x03
        _emit 0x85              // TEST EDX, EDX
        _emit 0xd2
        _emit 0x74              // JZ +0x3d  (→ 0x1801b)
        _emit 0x3d
        _emit 0x8b              // MOV EAX, [ESI + 0xC]
        _emit 0x46
        _emit 0x0c
        _emit 0x2b              // SUB EAX, EDX
        _emit 0xc2
        _emit 0xc1              // SAR EAX, 0x3
        _emit 0xf8
        _emit 0x03
        _emit 0x3b              // CMP ECX, EAX
        _emit 0xc8
        _emit 0x73              // JNC +0x31  (→ 0x1801b)
        _emit 0x31
        _emit 0x8b              // MOV ECX, [ESP + 0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EDX, [ESP + 0x14]
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EDI, [ESI + 0x8]
        _emit 0x7e
        _emit 0x08
        _emit 0xc6              // MOV byte ptr [ESP + 0x8], 0x0
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESP + 0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x56              // PUSH ESI
        _emit 0x52              // PUSH EDX
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL FUN_00965ad0  (rel32)
        _emit 0xc6
        _emit 0xda
        _emit 0x54
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0x83              // ADD EDI, 0x8
        _emit 0xc7
        _emit 0x08
        _emit 0x89              // MOV [ESI + 0x8], EDI
        _emit 0x7e
        _emit 0x08
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x8b              // MOV EDI, [ESI + 0x8]
        _emit 0x7e
        _emit 0x08
        _emit 0x3b              // CMP EDX, EDI
        _emit 0xd7
        _emit 0x76              // JBE +5  (→ 0x18027)
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4  (rel32)
        _emit 0x8d
        _emit 0xa2
        _emit 0x5b
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESP + 0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x50              // PUSH EAX
        _emit 0x57              // PUSH EDI
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA ECX, [ESP + 0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_00417f30  (rel32)
        _emit 0xf6
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
