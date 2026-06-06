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
// FUNCTION: ffxivgame 0x00413140 — engine_memory allocate-and-link node
//   (136 B / 0x88).  __thiscall with 3 stack args (RET 0xc).
//
// Likely belongs to SQEX::CDev::Engine::Memory::Alternative (cf. types note
// 0x00013640.md for the DetachableHeapSpace / Link subobject layout).
//
// Shape:
//   1. ECX = this (ESI).  Lock: this->vtable[11]() (vtable slot 0x2c/4).
//   2. Dispatch allocation: this->field_4->vtable[3](arg1, arg2, arg3)
//      with ECX = this->field_4.  Result → EDI.
//   3. If EDI == 0 (alloc failed) → jump to epilogue (step 8).
//   4. FUN_004109a0(ECX = this->field_10) → EAX (find a free node slot).
//   5. If EAX != 0: FUN_00412a80(ECX=EAX, ESI=this, EDI=alloc, EBX=0, EBX=0)
//         → EAX (wrapped node).  EBX = EAX.
//      Else EAX = 0, EBX = 0.
//   6. If EAX != 0: EAX += 8 (point to Link subobject inside node).
//      Else        : EAX  = 0.
//   7. Doubly-linked list insert before this->field_0x24 sentinel
//      (Link::next at +4, Link::prev at +8):
//        EDX = sentinel->prev;
//        EDX->next = EAX;  EAX->prev = EDX;
//        EAX->next = sentinel;  sentinel->prev = EAX;
//   8. Unlock: this->vtable[12]() (vtable slot 0x30/4).
//   9. If EBX != 0: return EBX+4.  Else return 0.
//
// Calling convention: __thiscall (ECX = this), 3 stack args, RET 0xc.
// Callee-saves: EBX, ESI, EDI.
//
// Reloc-bearing sites (CALL rel32, baked into orig image address space):
//   +0x30  CALL rel32 → FUN_004109a0  (e8 2b d8 ff ff)
//   +0x3f  CALL rel32 → FUN_00412a80  (e8 fc f8 ff ff)

extern "C" __declspec(naked) void FUN_00413140() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x8b              // MOV EDX, dword ptr [EAX + 0x2c]
        _emit 0x50
        _emit 0x2c
        _emit 0x57              // PUSH EDI
        _emit 0xff              // CALL EDX  (this->vtable[11]())
        _emit 0xd2
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 0x18]   ; arg3
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x4]    ; this->field_4
        _emit 0x4e
        _emit 0x04
        _emit 0x8b              // MOV EAX, dword ptr [ECX]          ; vtable
        _emit 0x01
        _emit 0x8b              // MOV EAX, dword ptr [EAX + 0xc]    ; vtable[3]
        _emit 0x40
        _emit 0x0c
        _emit 0x52              // PUSH EDX                           ; arg3
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 0x18]   ; arg2
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x52              // PUSH EDX                           ; arg2
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 0x18]   ; arg1
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x52              // PUSH EDX                           ; arg1
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0xff              // CALL EAX  (this->field_4->vtable[3](arg1,arg2,arg3))
        _emit 0xd0
        _emit 0x8b              // MOV EDI, EAX
        _emit 0xf8
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x74              // JZ +0x3d  (alloc failed → epilogue)
        _emit 0x3d
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x10]   ; this->field_10
        _emit 0x4e
        _emit 0x10
        _emit 0xe8              // CALL FUN_004109a0 (rel32 = 0xffffd82b)
        _emit 0x2b
        _emit 0xd8
        _emit 0xff
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x0d  (no slot → skip FUN_00412a80)
        _emit 0x0d
        _emit 0x53              // PUSH EBX                           ; 0
        _emit 0x53              // PUSH EBX                           ; 0
        _emit 0x57              // PUSH EDI                           ; alloc result
        _emit 0x56              // PUSH ESI                           ; this
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0xe8              // CALL FUN_00412a80 (rel32 = 0xfffff8fc)
        _emit 0xfc
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        _emit 0xeb              // JMP +0x02
        _emit 0x02
        _emit 0x33              // XOR EAX, EAX                       ; null path
        _emit 0xc0
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x8b              // MOV EBX, EAX
        _emit 0xd8
        _emit 0x74              // JZ +0x05
        _emit 0x05
        _emit 0x83              // ADD EAX, 0x8
        _emit 0xc0
        _emit 0x08
        _emit 0xeb              // JMP +0x02
        _emit 0x02
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x24]   ; sentinel
        _emit 0x4e
        _emit 0x24
        _emit 0x8b              // MOV EDX, dword ptr [ECX + 0x8]    ; sentinel->prev
        _emit 0x51
        _emit 0x08
        _emit 0x89              // MOV dword ptr [EDX + 0x4], EAX    ; prev->next = node
        _emit 0x42
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [ECX + 0x8]    ; reload sentinel->prev
        _emit 0x51
        _emit 0x08
        _emit 0x89              // MOV dword ptr [EAX + 0x8], EDX    ; node->prev = old_prev
        _emit 0x50
        _emit 0x08
        _emit 0x89              // MOV dword ptr [EAX + 0x4], ECX    ; node->next = sentinel
        _emit 0x48
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ECX + 0x8], EAX    ; sentinel->prev = node
        _emit 0x41
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ESI]          ; this->vtable
        _emit 0x06
        _emit 0x8b              // MOV EDX, dword ptr [EAX + 0x30]   ; vtable[12]
        _emit 0x50
        _emit 0x30
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EDX  (this->vtable[12]())
        _emit 0xd2
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x74              // JZ +0x09
        _emit 0x09
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x8d              // LEA EAX, [EBX + 0x4]
        _emit 0x43
        _emit 0x04
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x0c
        _emit 0x0c
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x0c
        _emit 0x0c
        _emit 0x00
    }
}
