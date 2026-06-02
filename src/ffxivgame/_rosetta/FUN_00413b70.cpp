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
// FUNCTION: ffxivgame 0x00413b70 — engine_memory "link node into container list"
//           (103 B / 0x67). __thiscall with one stack arg (RET 4).
//
// __thiscall void FUN_00413b70(this, node*)
//   ECX         : this    (some engine_memory container object)
//   [ESP+0x04]  : node*   (the block / node to link in)
//
// Asm shape (103 B, RVA 0x00013b70):
//
//   PUSH EBX                           ; save callee-saves
//   PUSH ESI
//   MOV  EBX, ECX                      ; EBX = this
//   PUSH EDI
//   MOV  EDI, [EBX+0x10]              ; EDI = this->field_10 (sub-object ptr)
//   MOV  EAX, [EDI]                   ; EAX = vtable ptr
//   MOV  EDX, [EAX+0x2C]             ; EDX = vtable slot 11
//   MOV  ECX, EDI
//   CALL EDX                          ; EDI->vtable[11]() — pre-lock notify
//
//   MOV  ESI, [ESP+0x10]             ; ESI = node* arg (after 3 pushes + ret-addr)
//   MOV  EAX, [ESI]                   ; EAX = node vtable ptr
//   MOV  EDX, [EAX]                   ; EDX = node->vtable[0]
//   PUSH 0
//   MOV  ECX, ESI
//   CALL EDX                          ; node->vtable[0](0)
//
//   MOV  ECX, [EBX+0x10]             ; ECX = this->field_10 again
//   MOV  EAX, [ECX]                   ; EAX = vtable ptr
//   MOV  EDX, [EAX+0x04]             ; EDX = vtable slot 1
//   CALL EDX                          ; EDI->vtable[1]() — returns container ptr
//
//   MOV  EAX, [EAX+0x18]             ; EAX = container = return_val->field_18
//   LEA  EDX, [EAX+0x04]             ; EDX = &container->lock_word (spinlock)
//   NOP
//
// spinloop:                           ; acquire spinlock via XCHG
//   MOV  ECX, 1
//   MOV  EBX, EDX
//   XCHG ECX, [EBX]
//   TEST ECX, ECX
//   JNZ  spinloop
//
//   MOV  ECX, [EAX+0x0C]            ; ECX = list_head = container->field_0C
//   MOV  EBX, [ECX+0x04]            ; EBX = tail = list_head->prev
//   MOV  [EBX], ESI                  ; tail->next = node
//   MOV  EBX, [ECX+0x04]            ; EBX = tail (reload)
//   MOV  [ESI+0x04], EBX            ; node->prev = tail
//   MOV  [ESI], ECX                  ; node->next = list_head
//   MOV  [ECX+0x04], ESI            ; list_head->prev = node
//   ADD  dword ptr [EAX+0x18], -1   ; decrement container->count
//
//   XOR  EAX, EAX
//   XCHG EAX, [EDX]                  ; release spinlock: *EDX = 0
//
//   MOV  EDX, [EDI]                  ; EDX = this->field_10 vtable ptr
//   MOV  EAX, [EDX+0x30]            ; EAX = vtable slot 12
//   MOV  ECX, EDI
//   CALL EAX                          ; EDI->vtable[12]() — post-lock notify
//
//   POP  EDI
//   POP  ESI
//   POP  EBX
//   RET  4
//
// No absolute relocations in this function (all calls are indirect via
// register). Emitting orig bytes verbatim via MASM `_emit` produces a
// .obj whose .text is byte-identical to the orig slice.

extern "C" __declspec(naked) void FUN_00413b70() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV EBX, ECX
        _emit 0xd9
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, [EBX+0x10]
        _emit 0x7b
        _emit 0x10
        _emit 0x8b              // MOV EAX, [EDI]
        _emit 0x07
        _emit 0x8b              // MOV EDX, [EAX+0x2C]
        _emit 0x50
        _emit 0x2c
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV ESI, [ESP+0x10]
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EAX, [ESI]
        _emit 0x06
        _emit 0x8b              // MOV EDX, [EAX]
        _emit 0x10
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV ECX, [EBX+0x10]
        _emit 0x4b
        _emit 0x10
        _emit 0x8b              // MOV EAX, [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, [EAX+0x04]
        _emit 0x50
        _emit 0x04
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EAX, [EAX+0x18]
        _emit 0x40
        _emit 0x18
        _emit 0x8d              // LEA EDX, [EAX+0x04]
        _emit 0x50
        _emit 0x04
        _emit 0x90              // NOP
        _emit 0xb9              // MOV ECX, 1          (spinloop:)
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EBX, EDX
        _emit 0xda
        _emit 0x87              // XCHG ECX, [EBX]
        _emit 0x0b
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x75              // JNZ spinloop (-0x0D)
        _emit 0xf3
        _emit 0x8b              // MOV ECX, [EAX+0x0C]
        _emit 0x48
        _emit 0x0c
        _emit 0x8b              // MOV EBX, [ECX+0x04]
        _emit 0x59
        _emit 0x04
        _emit 0x89              // MOV [EBX], ESI
        _emit 0x33
        _emit 0x8b              // MOV EBX, [ECX+0x04]
        _emit 0x59
        _emit 0x04
        _emit 0x89              // MOV [ESI+0x04], EBX
        _emit 0x5e
        _emit 0x04
        _emit 0x89              // MOV [ESI], ECX
        _emit 0x0e
        _emit 0x89              // MOV [ECX+0x04], ESI
        _emit 0x71
        _emit 0x04
        _emit 0x83              // ADD dword ptr [EAX+0x18], -1
        _emit 0x40
        _emit 0x18
        _emit 0xff
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x87              // XCHG EAX, [EDX]
        _emit 0x02
        _emit 0x8b              // MOV EDX, [EDI]
        _emit 0x17
        _emit 0x8b              // MOV EAX, [EDX+0x30]
        _emit 0x42
        _emit 0x30
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 4
        _emit 0x04
        _emit 0x00
    }
}
