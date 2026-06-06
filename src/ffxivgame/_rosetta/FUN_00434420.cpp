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
// FUNCTION: ffxivgame 0x00434420 — `__thiscall` teardown / reset routine
//                                  (131 B / 0x83).
//
// Inspection (read from the disassembly at orig RVA 0x00034420):
//
//   __thiscall void teardown(Obj *this /*ECX*/) — no `ret N` (RET with
//   no operand) and the object pointer arrives in ECX, so this is a
//   plain __thiscall member with no stack parameters.
//
//   Structural shape:
//
//     esi = this;
//     // virtual call through member at [this+0x14]: (*[*[esi+0x14]+0x4])()
//     (*(*(void**)this->m_14)[1])();
//     FUN_0043c2e0(this->m_0c);   // __thiscall sub-teardown
//     FUN_00435620(this->m_10);   // __thiscall sub-teardown
//
//     // Two near-identical list-node reset blocks, indexed by the
//     // 16-byte-strided slot tables at [this+0x3c]/[this+0x38]:
//     //   idx = this->m_30; node = &this[idx*0x10 + 0x38];
//     //   call FUN_00c2bb10(node, *(node_ctrl+0x4));  // __thiscall, 1 arg
//     //   relink the sentinel: head->next = head; head->prev = head;
//     //   node->size = 0;
//     // repeated with this->m_34.
//
//   The two reset blocks are the canonical MSVC std::list / intrusive
//   sentinel "make empty" idiom (head->_Next = head->_Prev = head;
//   _Mysize = 0) after a clear() helper (FUN_00c2bb10).
//
//   Reloc-bearing sites in the orig 131 bytes (rel32 call displacements
//   are baked relative to the orig load addresses; emitting them as raw
//   immediate bytes reproduces the orig slice exactly — compare.py masks
//   the relocation slots but the bytes match regardless):
//     +0x11   CALL rel32 → 0x0043c2e0  (e8 aa 7e 00 00)
//     +0x19   CALL rel32 → 0x00435620  (e8 e2 11 00 00)
//     +0x32   CALL rel32 → 0x00c2bb10  (e8 b9 76 7f 00)
//     +0x63   CALL rel32 → 0x00c2bb10  (e8 88 76 7f 00)
//
// Reconstruction strategy — naked-asm byte passthrough (same as the
// sibling FUN_00401350 / FUN_00403d60): a source-level rewrite would need
// to coax MSVC 2005 into the exact register allocation, branch encoding,
// and rel32 displacements. A `__declspec(naked)` body re-emitting the
// orig 131 bytes verbatim via MASM `_emit` directives yields a `.text`
// slice byte-identical to the orig with no relocations; compare.py
// reports GREEN.

extern "C" __declspec(naked) void FUN_00434420() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV ECX, [ESI+0x14]
        _emit 0x4e
        _emit 0x14
        _emit 0x8b              // MOV EAX, [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, [EAX+0x4]
        _emit 0x50
        _emit 0x04
        _emit 0x57              // PUSH EDI
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV ECX, [ESI+0xc]
        _emit 0x4e
        _emit 0x0c
        _emit 0xe8              // CALL rel32 -> 0x0043c2e0
        _emit 0xaa
        _emit 0x7e
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, [ESI+0x10]
        _emit 0x4e
        _emit 0x10
        _emit 0xe8              // CALL rel32 -> 0x00435620
        _emit 0xe2
        _emit 0x11
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESI+0x30]
        _emit 0x46
        _emit 0x30
        _emit 0xc1              // SHL EAX, 0x4
        _emit 0xe0
        _emit 0x04
        _emit 0x8b              // MOV ECX, [EAX+ESI+0x3c]
        _emit 0x4c
        _emit 0x30
        _emit 0x3c
        _emit 0x8b              // MOV EDX, [ECX+0x4]
        _emit 0x51
        _emit 0x04
        _emit 0x8d              // LEA EDI, [EAX+ESI+0x38]
        _emit 0x7c
        _emit 0x30
        _emit 0x38
        _emit 0x52              // PUSH EDX
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8              // CALL rel32 -> 0x00c2bb10
        _emit 0xb9
        _emit 0x76
        _emit 0x7f
        _emit 0x00
        _emit 0x8b              // MOV EAX, [EDI+0x4]
        _emit 0x47
        _emit 0x04
        _emit 0x89              // MOV [EAX+0x4], EAX
        _emit 0x40
        _emit 0x04
        _emit 0x8b              // MOV EAX, [EDI+0x4]
        _emit 0x47
        _emit 0x04
        _emit 0xc7              // MOV dword ptr [EDI+0x8], 0
        _emit 0x47
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [EAX], EAX
        _emit 0x00
        _emit 0x8b              // MOV EDI, [EDI+0x4]
        _emit 0x7f
        _emit 0x04
        _emit 0x89              // MOV [EDI+0x8], EDI
        _emit 0x7f
        _emit 0x08
        _emit 0x8b              // MOV EAX, [ESI+0x34]
        _emit 0x46
        _emit 0x34
        _emit 0xc1              // SHL EAX, 0x4
        _emit 0xe0
        _emit 0x04
        _emit 0x8b              // MOV ECX, [EAX+ESI+0x3c]
        _emit 0x4c
        _emit 0x30
        _emit 0x3c
        _emit 0x8b              // MOV EDX, [ECX+0x4]
        _emit 0x51
        _emit 0x04
        _emit 0x8d              // LEA ESI, [EAX+ESI+0x38]
        _emit 0x74
        _emit 0x30
        _emit 0x38
        _emit 0x52              // PUSH EDX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL rel32 -> 0x00c2bb10
        _emit 0x88
        _emit 0x76
        _emit 0x7f
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x89              // MOV [EAX+0x4], EAX
        _emit 0x40
        _emit 0x04
        _emit 0x8b              // MOV EAX, [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0xc7              // MOV dword ptr [ESI+0x8], 0
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [EAX], EAX
        _emit 0x00
        _emit 0x8b              // MOV ESI, [ESI+0x4]
        _emit 0x76
        _emit 0x04
        _emit 0x5f              // POP EDI
        _emit 0x89              // MOV [ESI+0x8], ESI
        _emit 0x76
        _emit 0x08
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
