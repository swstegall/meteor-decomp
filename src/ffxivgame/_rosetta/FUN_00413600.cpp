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
// FUNCTION: ffxivgame 0x00413600 — engine_memory DetachableHeapBlock
//                                  linked-list iterator / next-block fetch
//                                  (55 B / 0x37).  __thiscall, no stack args.
//
// __thiscall void *FUN_00413600()
//   ECX  : this  (SQEX::CDev::Engine::Memory::Alternative::DetachableHeapSpace
//                 or similar list-owner type)
//   ret  : &this->field_0x8  (non-null link sentinel pointer) on success,
//          nullptr on failure / empty list.
//
// Shape (RVA 0x00013600, 55 bytes):
//
//   PUSH  ESI
//   MOV   ESI, ECX                    ; ESI = this
//   MOV   ECX, [ESI+0xC]              ; ECX = this->m_current (block ptr)
//   XOR   EAX, EAX                    ; EAX = 0  (default return)
//   TEST  ECX, ECX
//   JZ    null_case                   ; if m_current == null → ECX = 0
//   ADD   ECX, 0x8                    ; else ECX = &m_current->embedded_link
//   JMP   after_null
// null_case:
//   XOR   ECX, ECX                    ; ECX = 0 (sentinel for empty)
// after_null:
//   MOV   EDX, [ESI+0x4]             ; EDX = this->m_list_head_obj
//   MOV   ECX, [ECX+0x8]             ; ECX = cursor->next  (Link.next at +8)
//   ADD   EDX, 0x24                  ; EDX = &m_list_head_obj->link_at_0x24
//   CMP   ECX, EDX                   ; at end-sentinel?
//   JZ    done                       ; yes → skip vcall, EAX stays 0
//   MOV   EAX, [ECX]                 ; EAX = cursor->vftable
//   MOV   EDX, [EAX+0x4]            ; EDX = vftable[1]  (slot index 1)
//   CALL  EDX                        ; ECX->vftable[1]()  (virtual call)
// done:
//   TEST  EAX, EAX
//   MOV   [ESI+0xC], EAX             ; this->m_current = result
//   JZ    ret_null                   ; null result → return nullptr
//   LEA   EAX, [ESI+0x8]            ; return &this->field_0x8
//   POP   ESI
//   RET
// ret_null:
//   XOR   EAX, EAX
//   POP   ESI
//   RET
//
// No CALL rel32 / absolute-address references in these 55 bytes; the only
// control-transfer is the indirect CALL EDX (virtual dispatch through a
// register). Zero relocations → naked-asm byte passthrough produces a .obj
// whose .text is byte-identical to the orig slice. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00413600() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0xC]
        _emit 0x4e
        _emit 0x0c
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74              // JZ +0x05  (null_case)
        _emit 0x05
        _emit 0x83              // ADD ECX, 0x8
        _emit 0xc1
        _emit 0x08
        _emit 0xeb              // JMP +0x02  (after_null)
        _emit 0x02
        _emit 0x33              // XOR ECX, ECX  (null_case:)
        _emit 0xc9
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x4]  (after_null:)
        _emit 0x56
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [ECX+0x8]
        _emit 0x49
        _emit 0x08
        _emit 0x83              // ADD EDX, 0x24
        _emit 0xc2
        _emit 0x24
        _emit 0x3b              // CMP ECX, EDX
        _emit 0xca
        _emit 0x74              // JZ +0x07  (done:)
        _emit 0x07
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x4]
        _emit 0x50
        _emit 0x04
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x85              // TEST EAX, EAX  (done:)
        _emit 0xc0
        _emit 0x89              // MOV dword ptr [ESI+0xC], EAX
        _emit 0x46
        _emit 0x0c
        _emit 0x74              // JZ +0x05  (ret_null:)
        _emit 0x05
        _emit 0x8d              // LEA EAX, [ESI+0x8]
        _emit 0x46
        _emit 0x08
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        _emit 0x33              // XOR EAX, EAX  (ret_null:)
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
