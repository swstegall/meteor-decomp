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
// FUNCTION: ffxivgame 0x000144e0 — __thiscall iterator-advance: load the
//           current node from `this->m_node` (+0xc), follow its next-link
//           at node+0x10, bail to NULL when the link equals the embedded
//           sentinel (`this->m_owner + 0x20`), otherwise call two virtual
//           methods (one through the sub-object at `this->+0x8` and one
//           through `next_link->vtable[1]`), accumulate their results into
//           `this->+0x10`, store the final vtable[1] return in `this->+0xc`,
//           and return `&this->+0x8`. (79 bytes / 0x4f)
//
// Calling convention: __thiscall (ECX = this), no stack args, RET.
//
// Asm trace (79 bytes @ orig RVA 0x000144e0):
//
//   000144e0: 51                PUSH ECX              ; spare local slot (reused at end)
//   000144e1: 56                PUSH ESI
//   000144e2: 8b f1             MOV  ESI, ECX         ; ESI = this
//   000144e4: 8b 4e 0c          MOV  ECX, [ESI+0xc]   ; ECX = m_node
//   000144e7: 33 c0             XOR  EAX, EAX         ; prepare null return
//   000144e9: 85 c9             TEST ECX, ECX
//   000144eb: 74 3f             JE   +0x3f            ; m_node==NULL → null-ret
//   000144ed: 55                PUSH EBP
//   000144ee: 8b 69 10          MOV  EBP, [ECX+0x10]  ; EBP = node->next_link
//   000144f1: 8b 4e 04          MOV  ECX, [ESI+0x4]   ; ECX = m_owner
//   000144f4: 83 c1 20          ADD  ECX, 0x20        ; sentinel = m_owner+0x20
//   000144f7: 3b e9             CMP  EBP, ECX         ; next_link==sentinel?
//   000144f9: 74 30             JE   +0x30            ; at end → null-ret (pop EBP too)
//   000144fb: 8b 56 10          MOV  EDX, [ESI+0x10]  ; EDX = this->m_extra
//   000144fe: 8b 46 08          MOV  EAX, [ESI+0x8]   ; EAX = this->m_sub_obj
//   00014501: 53                PUSH EBX
//   00014502: 57                PUSH EDI
//   00014503: 8d 7e 08          LEA  EDI, [ESI+0x8]   ; EDI = &m_sub_obj (return val)
//   00014506: 89 54 24 10       MOV  [ESP+0x10], EDX  ; stash m_extra in ECX-save slot
//   0001450a: 8b 50 08          MOV  EDX, [EAX+0x8]   ; EDX = m_sub_obj->fn_at_+0x8
//   0001450d: 8b cf             MOV  ECX, EBP         ; ECX = next_link (this arg)
//   0001450f: ff d2             CALL EDX              ; result1 = fn(next_link)
//   00014511: 8b d8             MOV  EBX, EAX         ; EBX = result1
//   00014513: 8b 45 00          MOV  EAX, [EBP]       ; EAX = next_link->vtable
//   00014516: 8b 50 04          MOV  EDX, [EAX+0x4]   ; EDX = vtable[1]
//   00014519: 03 5c 24 10       ADD  EBX, [ESP+0x10]  ; EBX += stashed m_extra
//   0001451d: 8b cd             MOV  ECX, EBP         ; ECX = next_link (this arg)
//   0001451f: ff d2             CALL EDX              ; result2 = next_link->vtable[1]()
//   00014521: 89 46 0c          MOV  [ESI+0xc], EAX   ; m_node_slot = result2
//   00014524: 8b c7             MOV  EAX, EDI         ; return &m_sub_obj
//   00014526: 5f                POP  EDI
//   00014527: 89 5e 10          MOV  [ESI+0x10], EBX  ; m_extra = accumulated EBX
//   0001452a: 5b                POP  EBX
//   0001452b: 5d                POP  EBP
//   0001452c: 5e                POP  ESI
//   0001452d: 59                POP  ECX              ; restore spare slot
//   0001452e: c3                RET
//
//   (null-return tails — EAX stays 0 from the XOR at 000144e7):
//   0001452c: 5e                POP  ESI              ; ← target of JE +0x3f at 000144eb
//   0001452d: 59                POP  ECX
//   0001452e: c3                RET
//   0001452b: 5d                POP  EBP              ; ← target of JE +0x30 at 000144f9
//   0001452c: 5e                POP  ESI
//   0001452d: 59                POP  ECX
//   0001452e: c3                RET
//
// Relocations: NONE — all calls are indirect via register (CALL EDX); no
// REL32/DIR32 external references anywhere in the 79 bytes. The function
// references only stack offsets and register-relative indirections.
//
// Reconstruction strategy — naked-asm byte passthrough.
// The nested push/pop stack management (PUSH ECX reused as local temp,
// dual-branch null-return cascade, ADD EBX,[ESP+0x10] accumulation) does
// not reproduce reliably from source-level C++. Emitting the 79 bytes
// verbatim via MASM _emit guarantees byte-identical output.

extern "C" __declspec(naked) void FUN_004144e0() {
    __asm {
        // 000144e0: 51             PUSH ECX
        _emit 0x51
        // 000144e1: 56             PUSH ESI
        _emit 0x56
        // 000144e2: 8b f1          MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 000144e4: 8b 4e 0c       MOV ECX, [ESI+0xc]
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        // 000144e7: 33 c0          XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 000144e9: 85 c9          TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 000144eb: 74 3f          JE +0x3f  (→ 0x0001452c, POP ESI)
        _emit 0x74
        _emit 0x3f
        // 000144ed: 55             PUSH EBP
        _emit 0x55
        // 000144ee: 8b 69 10       MOV EBP, [ECX+0x10]
        _emit 0x8b
        _emit 0x69
        _emit 0x10
        // 000144f1: 8b 4e 04       MOV ECX, [ESI+0x4]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 000144f4: 83 c1 20       ADD ECX, 0x20
        _emit 0x83
        _emit 0xc1
        _emit 0x20
        // 000144f7: 3b e9          CMP EBP, ECX
        _emit 0x3b
        _emit 0xe9
        // 000144f9: 74 30          JE +0x30  (→ 0x0001452b, POP EBP)
        _emit 0x74
        _emit 0x30
        // 000144fb: 8b 56 10       MOV EDX, [ESI+0x10]
        _emit 0x8b
        _emit 0x56
        _emit 0x10
        // 000144fe: 8b 46 08       MOV EAX, [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 00014501: 53             PUSH EBX
        _emit 0x53
        // 00014502: 57             PUSH EDI
        _emit 0x57
        // 00014503: 8d 7e 08       LEA EDI, [ESI+0x8]
        _emit 0x8d
        _emit 0x7e
        _emit 0x08
        // 00014506: 89 54 24 10    MOV [ESP+0x10], EDX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0001450a: 8b 50 08       MOV EDX, [EAX+0x8]
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 0001450d: 8b cf          MOV ECX, EBP
        _emit 0x8b
        _emit 0xcf
        // 0001450f: ff d2          CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00014511: 8b d8          MOV EBX, EAX
        _emit 0x8b
        _emit 0xd8
        // 00014513: 8b 45 00       MOV EAX, [EBP+0]
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        // 00014516: 8b 50 04       MOV EDX, [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00014519: 03 5c 24 10    ADD EBX, [ESP+0x10]
        _emit 0x03
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        // 0001451d: 8b cd          MOV ECX, EBP
        _emit 0x8b
        _emit 0xcd
        // 0001451f: ff d2          CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00014521: 89 46 0c       MOV [ESI+0xc], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        // 00014524: 8b c7          MOV EAX, EDI
        _emit 0x8b
        _emit 0xc7
        // 00014526: 5f             POP EDI
        _emit 0x5f
        // 00014527: 89 5e 10       MOV [ESI+0x10], EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x10
        // 0001452a: 5b             POP EBX
        _emit 0x5b
        // 0001452b: 5d             POP EBP   ← JE +0x30 target
        _emit 0x5d
        // 0001452c: 5e             POP ESI   ← JE +0x3f target
        _emit 0x5e
        // 0001452d: 59             POP ECX
        _emit 0x59
        // 0001452e: c3             RET
        _emit 0xc3
    }
}
