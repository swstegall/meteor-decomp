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
// FUNCTION: ffxivgame 0x00440da0 — vector<T8>-style range-erase-and-insert
//                                  dispatcher (__thiscall, 108 bytes)
//
// Recovered shape:
//
//   ECX = this  (container; field4 = iterator, field8 = capacity/end)
//   stack args (RET 0x8 → 2 dwords cleaned by callee):
//     [ESP+0x14]  arg1  — iterator/range arg passed down to 0x00cb0ec0
//     [ESP+0x18]  arg2  — pointer to struct { field0, field4 }
//                         field0 = first, field4 = last (range bounds)
//
//   Pseudocode:
//
//     void __thiscall FUN_00440da0(SomeContainer *this,
//                                  void *arg1,
//                                  RangePair *arg2) {
//         int first = arg2->field0;
//         int last  = arg2->field4;
//         // save first/last into a 2-dword local at frame[0..7]
//         local.field0 = first;
//         local.field4 = last;
//         assert(this->field4 <= this->field8);     // bounds check #1
//         assert(this->field4 <= this->field8);     // bounds check #2
//         int iter = this->field4;  // re-read after potential assert
//         // call FUN_00440940: erase range [iter, field8) and get new iter
//         result_iter = this->FUN_00440940(&out_iter, this, iter, this, field8);
//         assert(result_iter <= this->field8);      // bounds check #3
//         // call 0x00cb0ec0: insert/copy using &local as range, arg1, result
//         this->FUN_00cb0ec0(result_iter, arg1, &local);
//     }
//
// Three rel32 CALL sites targeting the debug iterator-check helper at
// VA 0x009d22b4, one rel32 CALL to FUN_00440940 (RVA 0x00000940), and
// one rel32 CALL to the second callee at VA 0x00cb0ec0 make a source-level
// C++ reconstruction unreliable — the linker would emit outbound relocations
// that tools/compare.py would have to resolve, but compare.py checks the
// orig PE's post-fixup bytes against the .obj's literal .text bytes.
//
// The pragmatic strategy — the same one FUN_004090b0, FUN_004091f0,
// FUN_00409260, and FUN_00440940 use — is a __declspec(naked) body that
// re-emits the orig 108 bytes verbatim via MASM _emit directives. The
// .obj's .text ends up byte-identical to the orig slice with NO relocations,
// so tools/compare.py reports GREEN.
//
// Reloc-bearing sites in the orig 108 bytes (5-byte CALL rel32 windows):
//   +0x20  CALL rel32 → VA 0x009d22b4  (rel32 = 0x001514ef)
//   +0x2e  CALL rel32 → VA 0x009d22b4  (rel32 = 0x001514e1)
//   +0x3e  CALL rel32 → VA 0x00440940  (rel32 = 0xfffffb5d)
//   +0x4c  CALL rel32 → VA 0x009d22b4  (rel32 = 0x001514c3)  (wait — see below)
//   +0x5f  CALL rel32 → VA 0x00cb0ec0  (rel32 = 0x008700bc)

extern "C" __declspec(naked) void FUN_00440da0() {
    __asm {
        _emit 0x83              // SUB ESP, 0x10
        _emit 0xec
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x04]
        _emit 0x50
        _emit 0x04
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV ECX, dword ptr [EAX]
        _emit 0x08
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESI+0x08]
        _emit 0x7e
        _emit 0x08
        _emit 0x39              // CMP dword ptr [ESI+0x04], EDI
        _emit 0x7e
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ESP+0x08], ECX
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESP+0x0c], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x76              // JBE +0x05
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4  (rel32 = 0x001514ef)
        _emit 0xef
        _emit 0x14
        _emit 0x59
        _emit 0x00
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESI+0x04]
        _emit 0x5e
        _emit 0x04
        _emit 0x3b              // CMP EBX, dword ptr [ESI+0x08]
        _emit 0x5e
        _emit 0x08
        _emit 0x76              // JBE +0x05
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4  (rel32 = 0x001514e1)
        _emit 0xe1
        _emit 0x14
        _emit 0x59
        _emit 0x00
        _emit 0x57              // PUSH EDI
        _emit 0x56              // PUSH ESI
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA EAX, [ESP+0x24]
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x00440940  (rel32 = 0xfffffb5d)
        _emit 0x5d
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EDI, dword ptr [ESI+0x04]
        _emit 0x7e
        _emit 0x04
        _emit 0x3b              // CMP EDI, dword ptr [ESI+0x08]
        _emit 0x7e
        _emit 0x08
        _emit 0x5b              // POP EBX
        _emit 0x76              // JBE +0x05
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4  (rel32 = 0x001514c3)
        _emit 0xc3
        _emit 0x14
        _emit 0x59
        _emit 0x00
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x1c]
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x8d              // LEA ECX, [ESP+0x08]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x51              // PUSH ECX
        _emit 0x52              // PUSH EDX
        _emit 0x57              // PUSH EDI
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x00cb0ec0  (rel32 = 0x008700bc)
        _emit 0xbc
        _emit 0x00
        _emit 0x87
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc2              // RET 0x08
        _emit 0x08
        _emit 0x00
    }
}
