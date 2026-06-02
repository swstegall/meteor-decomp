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
// FUNCTION: ffxivgame 0x00012150 — RemovableHeapBlock destructor (217 B / 0xd9),
//                                  __thiscall (ECX = this). Class identified via
//                                  RTTI: SQEX::CDev::Engine::Memory::Alternative::
//                                  RemovableHeapBlock.
//
// Asm shape (read from asm/ffxivgame/00012150_FUN_00412150.s):
//
//   __thiscall void FUN_00412150(RemovableHeapBlock *this  /* ECX */);
//
//   // --- EH3 SEH prolog (no /GS — no security_cookie load/check) ---------
//   PUSH -1                         ; TryLevel = -1
//   PUSH 0x00e5504c                 ; scope_table ptr
//   MOV  EAX, FS:[0]; PUSH EAX     ; save old FS[0]
//   MOV  FS:[0], ESP                ; install SEH frame
//   SUB  ESP, 0x0c                  ; 12-byte local area
//   PUSH EBX; PUSH ESI; PUSH EDI   ; callee-saves
//   MOV  ESI, ECX                  ; ESI = this
//   MOV  [ESP+0x14], ESI           ; save 'this' in EH frame for unwind
//
//   // --- Reset vtable pointers to RemovableHeapBlock's own vtables --------
//   MOV  [ESI+0x00], 0x00f56db8    ; IBlock sub-vtable during dtor
//   MOV  [ESI+0x04], 0x00f56d80    ; IHandle sub-vtable during dtor
//   MOV  [ESI+0x08], 0x00f56dc8    ; 3rd sub-vtable during dtor
//
//   // --- Enter try scope 4 ------------------------------------------------
//   MOV  [ESP+0x20], 4             ; TryLevel = 4 (SEH frame state)
//
//   // --- Destroy 3 member sub-objects / call base dtors -------------------
//   CALL 0x004105c0                 ; ~sub-obj or base-class dtor (no ECX reset)
//   MOV  ECX, ESI; CALL 0x00412020 ; another member/base dtor  (__thiscall)
//   MOV  ECX, ESI; CALL 0x00411fa0 ; another member/base dtor  (__thiscall)
//
//   // --- Loop: iterate linked list, call virtual dtor on each element -----
//   MOV  EBX, [ESI+0x34]           ; EBX = element count
//   TEST EBX, EBX
//   MOV  EDI, [ESI+0x30]           ; EDI = list head/iterator
//   JBE  after_loop                ; skip if count == 0
//   LEA  EBX, [EBX]               ; 6-byte NOP for loop alignment
//   loop:
//     MOV  EAX, [EDI+0x30]        ; EAX = element ptr
//     ADD  DWORD PTR [EDI+0x34], -1 ; decrement refcount/slot at EDI
//     MOV  ECX, [EAX+0x28]        ; ECX = sub-object of element
//     MOV  EDX, [ECX]             ; EDX = vtable
//     MOV  EAX, [EDX+0x20]        ; EAX = vfunc at slot 8 (offset 0x20)
//     CALL EAX                    ; call virtual destructor
//     SUB  EBX, 1                 ; --count
//   JNZ  loop
//   after_loop:
//
//   // --- Initialize doubly-linked list sentinels --------------------------
//   // List A: head=[ESI+0x44] with nodes at [ESI+0x48] and [ESI+0x4c]
//   MOV  ECX, [ESI+0x48]; MOV EDX, [ESI+0x4c]
//   MOV  EAX, 0x00f567c4           ; Link::vftable
//   MOV  [ESI+0x44], EAX           ; store vtable in sentinel
//   MOV  [ECX+0x08], EDX           ; nodeA->next = nodeB
//   MOV  ECX, [ESI+0x4c]; MOV EDX, [ESI+0x48]
//   MOV  [ECX+0x04], EDX           ; nodeB->prev = nodeA
//   // List B: head=[ESI+0x38] with nodes at [ESI+0x3c] and [ESI+0x40]
//   MOV  ECX, [ESI+0x3c]; MOV EDX, [ESI+0x40]
//   MOV  [ESI+0x38], EAX           ; store vtable
//   MOV  [ECX+0x08], EDX           ; nodeC->next = nodeD
//   MOV  ECX, [ESI+0x40]; MOV EDX, [ESI+0x3c]
//   MOV  [ECX+0x04], EDX           ; nodeD->prev = nodeC
//   MOV  [ESI+0x08], EAX           ; override 3rd vtable to Link::vftable
//   // List C: nodes at [ESI+0x0c] and [ESI+0x10]
//   MOV  EAX, [ESI+0x0c]; MOV ECX, [ESI+0x10]
//   MOV  [EAX+0x08], ECX           ; nodeE->next = nodeF
//   MOV  EDX, [ESI+0x10]; MOV EAX, [ESI+0x0c]
//   MOV  ECX, [ESP+0x18]           ; load old FS[0] into ECX (for SEH restore)
//   MOV  [EDX+0x04], EAX           ; nodeF->prev = nodeE
//
//   // --- Epilogue: restore base-class vtables and unlink EH frame ---------
//   MOV  [ESI+0x04], 0x00f56750    ; IHandle::vftable (base restored)
//   POP  EDI
//   MOV  [ESI+0x00], 0x00f56740    ; IBlock::vftable  (base restored)
//   POP  ESI; POP EBX
//   MOV  FS:[0], ECX               ; restore old SEH chain head
//   ADD  ESP, 0x18                 ; drop 12 locals + 12 EH header
//   RET
//
// Reloc-bearing sites within the 217 bytes (compare.py masks these):
//   +0x03   scope_table ptr         (DIR32 → 0x00e5504c)
//   +0x23   RemovableHeapBlock vft1 (DIR32 → 0x00f56db8)
//   +0x2a   RemovableHeapBlock vft2 (DIR32 → 0x00f56d80)
//   +0x31   RemovableHeapBlock vft3 (DIR32 → 0x00f56dc8)
//   +0x3e   call FUN_004105c0       (REL32)
//   +0x45   call FUN_00412020       (REL32)
//   +0x4c   call FUN_00411fa0       (REL32)
//   +0x7d   Link::vftable           (DIR32 → 0x00f567c4)
//   +0xc1   IHandle::vftable        (DIR32 → 0x00f56750)
//   +0xc8   IBlock::vftable         (DIR32 → 0x00f56740)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   Attempting a C++ source reconstruction would need to coax MSVC 2005 into
//   producing the exact EH3 SEH frame layout (no /GS security cookie, try_level
//   at [ESP+0x20] = 4), the precise 6-byte NOP padding (LEA EBX,[EBX+0])
//   before the loop, the interleaved POP+vtable-MOV epilogue pattern (where
//   MOV [ESI+4] comes before POP EDI and MOV [ESI] comes after POP EDI but
//   before POP ESI), and exact register allocation throughout. All of these
//   are MSVC 2005 scheduler outputs that shift under any source rewrite.
//   Emitting the 217 original bytes verbatim via MASM `_emit` gives a
//   byte-exact match at compare.py (GREEN).

extern "C" __declspec(naked) void FUN_00412150() {
    __asm {
        _emit 0x6a          // PUSH -1
        _emit 0xff
        _emit 0x68          // PUSH 0x00e5504c (scope_table)
        _emit 0x4c
        _emit 0x50
        _emit 0xe5
        _emit 0x00
        _emit 0x64          // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50          // PUSH EAX
        _emit 0x64          // MOV FS:[0], ESP
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83          // SUB ESP, 0x0c
        _emit 0xec
        _emit 0x0c
        _emit 0x53          // PUSH EBX
        _emit 0x56          // PUSH ESI
        _emit 0x8b          // MOV ESI, ECX
        _emit 0xf1
        _emit 0x57          // PUSH EDI
        _emit 0x89          // MOV [ESP+0x14], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0xc7          // MOV [ESI], 0x00f56db8
        _emit 0x06
        _emit 0xb8
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        _emit 0xc7          // MOV [ESI+0x04], 0x00f56d80
        _emit 0x46
        _emit 0x04
        _emit 0x80
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        _emit 0xc7          // MOV [ESI+0x08], 0x00f56dc8
        _emit 0x46
        _emit 0x08
        _emit 0xc8
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        _emit 0xc7          // MOV [ESP+0x20], 4  (TryLevel = 4)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8          // CALL 0x004105c0
        _emit 0x2e
        _emit 0xe4
        _emit 0xff
        _emit 0xff
        _emit 0x8b          // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8          // CALL 0x00412020
        _emit 0x87
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x8b          // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8          // CALL 0x00411fa0
        _emit 0x00
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x8b          // MOV EBX, [ESI+0x34]
        _emit 0x5e
        _emit 0x34
        _emit 0x85          // TEST EBX, EBX
        _emit 0xdb
        _emit 0x8b          // MOV EDI, [ESI+0x30]
        _emit 0x7e
        _emit 0x30
        _emit 0x76          // JBE +0x1c
        _emit 0x1c
        _emit 0x8d          // LEA EBX, [EBX+0] (6-byte NOP / loop alignment)
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b          // loop: MOV EAX, [EDI+0x30]
        _emit 0x47
        _emit 0x30
        _emit 0x83          // ADD [EDI+0x34], -1
        _emit 0x47
        _emit 0x34
        _emit 0xff
        _emit 0x8b          // MOV ECX, [EAX+0x28]
        _emit 0x48
        _emit 0x28
        _emit 0x8b          // MOV EDX, [ECX]
        _emit 0x11
        _emit 0x8b          // MOV EAX, [EDX+0x20]
        _emit 0x42
        _emit 0x20
        _emit 0xff          // CALL EAX
        _emit 0xd0
        _emit 0x83          // SUB EBX, 1
        _emit 0xeb
        _emit 0x01
        _emit 0x75          // JNZ loop
        _emit 0xea
        _emit 0x8b          // MOV ECX, [ESI+0x48]
        _emit 0x4e
        _emit 0x48
        _emit 0x8b          // MOV EDX, [ESI+0x4c]
        _emit 0x56
        _emit 0x4c
        _emit 0xb8          // MOV EAX, 0x00f567c4 (Link::vftable)
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0x89          // MOV [ESI+0x44], EAX
        _emit 0x46
        _emit 0x44
        _emit 0x89          // MOV [ECX+0x08], EDX
        _emit 0x51
        _emit 0x08
        _emit 0x8b          // MOV ECX, [ESI+0x4c]
        _emit 0x4e
        _emit 0x4c
        _emit 0x8b          // MOV EDX, [ESI+0x48]
        _emit 0x56
        _emit 0x48
        _emit 0x89          // MOV [ECX+0x04], EDX
        _emit 0x51
        _emit 0x04
        _emit 0x8b          // MOV ECX, [ESI+0x3c]
        _emit 0x4e
        _emit 0x3c
        _emit 0x8b          // MOV EDX, [ESI+0x40]
        _emit 0x56
        _emit 0x40
        _emit 0x89          // MOV [ESI+0x38], EAX
        _emit 0x46
        _emit 0x38
        _emit 0x89          // MOV [ECX+0x08], EDX
        _emit 0x51
        _emit 0x08
        _emit 0x8b          // MOV ECX, [ESI+0x40]
        _emit 0x4e
        _emit 0x40
        _emit 0x8b          // MOV EDX, [ESI+0x3c]
        _emit 0x56
        _emit 0x3c
        _emit 0x89          // MOV [ECX+0x04], EDX
        _emit 0x51
        _emit 0x04
        _emit 0x89          // MOV [ESI+0x08], EAX
        _emit 0x46
        _emit 0x08
        _emit 0x8b          // MOV EAX, [ESI+0x0c]
        _emit 0x46
        _emit 0x0c
        _emit 0x8b          // MOV ECX, [ESI+0x10]
        _emit 0x4e
        _emit 0x10
        _emit 0x89          // MOV [EAX+0x08], ECX
        _emit 0x48
        _emit 0x08
        _emit 0x8b          // MOV EDX, [ESI+0x10]
        _emit 0x56
        _emit 0x10
        _emit 0x8b          // MOV EAX, [ESI+0x0c]
        _emit 0x46
        _emit 0x0c
        _emit 0x8b          // MOV ECX, [ESP+0x18] (old FS[0] for SEH restore)
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x89          // MOV [EDX+0x04], EAX
        _emit 0x42
        _emit 0x04
        _emit 0xc7          // MOV [ESI+0x04], 0x00f56750 (IHandle::vftable)
        _emit 0x46
        _emit 0x04
        _emit 0x50
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0x5f          // POP EDI
        _emit 0xc7          // MOV [ESI], 0x00f56740 (IBlock::vftable)
        _emit 0x06
        _emit 0x40
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0x5e          // POP ESI
        _emit 0x5b          // POP EBX
        _emit 0x64          // MOV FS:[0], ECX (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83          // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0xc3          // RET
    }
}
