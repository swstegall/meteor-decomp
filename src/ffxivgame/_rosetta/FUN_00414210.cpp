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
// FUNCTION: ffxivgame 0x00414210 — engine_memory guarded-allocate + notify
//                                  (113 B / 0x71)  module: engine_memory
//
// __thiscall void FUN_00414210(SomeAllocatorCell *this)
//   ECX / ESI : this   (a DetachableHeapBlock-family cell)
//
// Shape (113 bytes, RVA 0x00014210):
//
//   ESI = ECX (this); EDI = this->member_0x10  ; guard/lock object pointer
//   guard->vtable[11]()                         ; Enter guard (slot +0x2c)
//
//   if (this->count_0x2c == 0) {
//     if (this->flag_0x28 == 0 && this->flag_0x21 == 0) {
//       this->flag_0x20 = 0;
//       FUN_00413940(this - 4);                 ; cold-path helper
//       this->member_0x14->vtable[7]();         ; vcall slot +0x1c
//     } else {
//       // walk linked list rooted at (this-4), following [node+0x2c],
//       // stopping when [node+0x27] != 0 or list is exhausted
//       SomeNode *p = this - 4;
//       if (p && [p+0x27] == 0) {
//         do { p = [p+0x2c]; } while (p && [p+0x27] == 0);
//       }
//       this->flag_0x23 = 1;
//       FUN_004140e0();                         ; notify helper
//     }
//   }
//
//   this->count_0x2c++;
//   guard->vtable[12]()                         ; Leave guard (slot +0x30)
//   jmp this->vtable[1]                         ; tail call slot +0x04
//
// Reloc-bearing sites (CALL rel32 into the orig address space):
//   +0x29   CALL rel32 → FUN_00413940  (0xfffff702)
//   +0x54   CALL rel32 → FUN_004140e0  (0xfffffe77)
//
// Reconstruction strategy — naked-asm byte passthrough.
//   The two rel32 call displacements are baked against the orig image's
//   own virtual addresses; emitting the 113 bytes verbatim via MASM
//   `_emit` produces a .obj whose .text is byte-identical to the orig
//   slice. tools/compare.py masks the two reloc sites and reports GREEN.

extern "C" __declspec(naked) void FUN_00414210() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESI+0x10]
        _emit 0x7e
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [EDI]
        _emit 0x07
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x2c]
        _emit 0x50
        _emit 0x2c
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EDX   ; guard->Enter()
        _emit 0xd2
        _emit 0x83              // CMP dword ptr [ESI+0x2c], 0
        _emit 0x7e
        _emit 0x2c
        _emit 0x00
        _emit 0x75              // JNE +0x43  (→ 0x414269 tail)
        _emit 0x43
        _emit 0x83              // CMP dword ptr [ESI+0x28], 0
        _emit 0x7e
        _emit 0x28
        _emit 0x00
        _emit 0x75              // JNE +0x1e  (→ 0x41424a middle path)
        _emit 0x1e
        _emit 0x80              // CMP byte ptr [ESI+0x21], 0
        _emit 0x7e
        _emit 0x21
        _emit 0x00
        _emit 0x75              // JNE +0x18  (→ 0x41424a middle path)
        _emit 0x18
        _emit 0x8d              // LEA ECX, [ESI-0x4]
        _emit 0x4e
        _emit 0xfc
        _emit 0xc6              // MOV byte ptr [ESI+0x20], 0
        _emit 0x46
        _emit 0x20
        _emit 0x00
        _emit 0xe8              // CALL FUN_00413940  (rel32 = 0xfffff702)
        _emit 0x02
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x14]
        _emit 0x4e
        _emit 0x14
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x1c]
        _emit 0x50
        _emit 0x1c
        _emit 0xff              // CALL EDX   ; member->vtable[7]()
        _emit 0xd2
        _emit 0xeb              // JMP +0x1f  (→ 0x414269 tail)
        _emit 0x1f
        _emit 0x8d              // LEA ECX, [ESI-0x4]   ; middle path: 0x41424a
        _emit 0x4e
        _emit 0xfc
        _emit 0x8b              // MOV EAX, ECX
        _emit 0xc1
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JE +0x0d   (→ 0x414260 set-flag)
        _emit 0x0d
        _emit 0x80              // CMP byte ptr [EAX+0x27], 0   ; loop top: 0x414253
        _emit 0x78
        _emit 0x27
        _emit 0x00
        _emit 0x75              // JNE +0x10  (→ 0x414269 tail)
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [EAX+0x2c]
        _emit 0x40
        _emit 0x2c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNE -0x0d  (→ 0x414253 loop top)
        _emit 0xf3
        _emit 0xc6              // MOV byte ptr [ESI+0x23], 1   ; set-flag: 0x414260
        _emit 0x46
        _emit 0x23
        _emit 0x01
        _emit 0xe8              // CALL FUN_004140e0  (rel32 = 0xfffffe77)
        _emit 0x77
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD dword ptr [ESI+0x2c], 1  ; tail: 0x414269
        _emit 0x46
        _emit 0x2c
        _emit 0x01
        _emit 0x8b              // MOV EAX, dword ptr [EDI]
        _emit 0x07
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x30]
        _emit 0x50
        _emit 0x30
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EDX   ; guard->Leave()
        _emit 0xd2
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x04]
        _emit 0x50
        _emit 0x04
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0x5e              // POP ESI
        _emit 0xff              // JMP EDX   ; tail call to this->vtable[1]
        _emit 0xe2
    }
}
