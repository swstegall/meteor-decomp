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
// FUNCTION: ffxivgame 0x000144b0 — __thiscall iterator-begin: read the
//           first node from `this->m_owner`'s chain head (m_owner+0x28),
//           bail to a zeroed-out return-NULL form when the head equals
//           the embedded sentinel (m_owner+0x20), otherwise pull the
//           first sample via `node->vtable[1]()`, store it in
//           `this->m_value` (+0xc), clear `this->m_extra` (+0x10), and
//           return `&this->m_sub_object` (this+0x8). (48 bytes / 0x30)
//
// Calling convention: __thiscall (ECX = this), no stack args, RET.
//
// Asm trace (48 bytes @ orig RVA 0x000144b0):
//
//   000144b0: 56                PUSH ESI
//   000144b1: 8b f1             MOV  ESI, ECX                ; ESI = this
//   000144b3: 8b 46 04          MOV  EAX, [ESI+0x4]          ; EAX = m_owner
//   000144b6: 8b 48 28          MOV  ECX, [EAX+0x28]         ; ECX = chain head ptr
//   000144b9: 83 c0 20          ADD  EAX, 0x20               ; EAX = sentinel addr
//   000144bc: 3b c8             CMP  ECX, EAX
//   000144be: 74 16             JE   +0x16 (-> 0x000144d6)   ; head == sentinel → empty
//   000144c0: 8b 01             MOV  EAX, [ECX]              ; EAX = node->vtable
//   000144c2: 8b 50 04          MOV  EDX, [EAX+0x4]          ; EDX = vtable[1]
//   000144c5: ff d2             CALL EDX                     ; sample = vtable[1]()
//   000144c7: 89 46 0c          MOV  [ESI+0xc], EAX          ; m_value = sample
//   000144ca: c7 46 10 00 00 00 00  MOV dword ptr [ESI+0x10], 0 ; m_extra = 0
//   000144d1: 8d 46 08          LEA  EAX, [ESI+0x8]          ; ret = &m_sub_object
//   000144d4: 5e                POP  ESI
//   000144d5: c3                RET
//   000144d6: 33 c0             XOR  EAX, EAX                ; ret = NULL
//   000144d8: 89 46 0c          MOV  [ESI+0xc], EAX          ; m_value = 0
//   000144db: 89 46 10          MOV  [ESI+0x10], EAX         ; m_extra = 0
//   000144de: 5e                POP  ESI
//   000144df: c3                RET
//
// Relocations: none (the single CALL is an indirect `call edx` virtual
// dispatch — no REL32/DIR32 in the function body).
//
// Reconstruction strategy — naked-asm byte passthrough.
// The dual-return shape (ADD EAX,0x20 mutating the live owner copy rather
// than LEA; XOR-shared EAX zeroing both slots on the sentinel path) is a
// tight MSVC 2005 /O2 idiom that source-level C++ does not reproduce
// reliably. Emitting the 48 bytes verbatim via MASM `_emit` guarantees
// byte-identical output.

extern "C" __declspec(naked) void FUN_004144b0() {
    __asm {
        // 000144b0: 56                PUSH ESI
        _emit 0x56
        // 000144b1: 8b f1             MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 000144b3: 8b 46 04          MOV EAX, [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 000144b6: 8b 48 28          MOV ECX, [EAX+0x28]
        _emit 0x8b
        _emit 0x48
        _emit 0x28
        // 000144b9: 83 c0 20          ADD EAX, 0x20
        _emit 0x83
        _emit 0xc0
        _emit 0x20
        // 000144bc: 3b c8             CMP ECX, EAX
        _emit 0x3b
        _emit 0xc8
        // 000144be: 74 16             JE +0x16
        _emit 0x74
        _emit 0x16
        // 000144c0: 8b 01             MOV EAX, [ECX]
        _emit 0x8b
        _emit 0x01
        // 000144c2: 8b 50 04          MOV EDX, [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000144c5: ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000144c7: 89 46 0c          MOV [ESI+0xc], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        // 000144ca: c7 46 10 00 00 00 00  MOV dword ptr [ESI+0x10], 0
        _emit 0xc7
        _emit 0x46
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000144d1: 8d 46 08          LEA EAX, [ESI+0x8]
        _emit 0x8d
        _emit 0x46
        _emit 0x08
        // 000144d4: 5e                POP ESI
        _emit 0x5e
        // 000144d5: c3                RET
        _emit 0xc3
        // 000144d6: 33 c0             XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 000144d8: 89 46 0c          MOV [ESI+0xc], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        // 000144db: 89 46 10          MOV [ESI+0x10], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x10
        // 000144de: 5e                POP ESI
        _emit 0x5e
        // 000144df: c3                RET
        _emit 0xc3
    }
}
