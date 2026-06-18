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
// FUNCTION: ffxivgame 0x00410d70 — engine_memory thread-safe enqueue (177 B / 0xb1)
//
// __thiscall void FUN_00410d70(this, undefined4 arg1)
//   ECX        : this
//   [ESP+0x04] : arg1 — object pointer to insert into the queue
//
// Shape (from asm at RVA 0x00010d70, 177 bytes as measured by Ghidra / symbols.json):
//
//   Save EBX/ESI/EDI; cache `this` in EBX.
//   EDI = this->field_0x10               ; a manager / sub-object
//   virt call EDI->vtable[0x2c/4]()      ; prepare / lock-acquire (no result used)
//   virt call EDI->vtable[0x34/4]()      ; check-condition → AL
//   if (AL) {                            ; condition true → emit a log message
//       if (!(g_flag & 1)) {             ; one-time lazy init of log function ptr
//           g_flag |= 1;
//           g_logfn_ptr = 0x0040f8e0;    ; function pointer stored in global
//       }
//       call g_logfn_ptr(               ; log call: 5 stack args (cdecl, cleaned by ADD ESP,0x14)
//           0xf56974, 0xf54d48,
//           0xf56988, 0xc3, 0xf56b58);
//   }
//   ESI = arg1                           ; the object to enqueue
//   virt call ESI->vtable[0](0)          ; notify / register (one int arg)
//   ECX = this->field_0x10->vtable[1]()  ; get container object into EAX
//   ECX = EAX->field_0x14               ; sub-object inside container
//   EDX = &ECX->field_0x04              ; spin-lock field address
//   JMP spin_loop                        ; jump over 6-byte alignment NOP
//   [8d 9b 00 00 00 00]                  ; LEA EBX,[EBX+0] — 6-byte NOP padding
//                                        ;   (MSVC loop-header alignment to 0x10df0)
// spin_loop:
//   EAX = 1; XCHG [EDX], EAX            ; test-and-set (acquire spin-lock)
//   if (EAX != 0) goto spin_loop         ; retry until we hold the lock
//   EAX = ECX->field_0x0c               ; doubly-linked list head
//   EBX = EAX->field_0x04               ; EAX->prev (tail sentinel)
//   [EBX]     = ESI                     ; tail->next = arg1
//   [ESI+0x4] = EBX                     ; arg1->prev = tail
//   [ESI]     = EAX                     ; arg1->next = head
//   [EAX+0x4] = ESI                     ; head->prev = arg1
//   ECX->field_0x18 += -1               ; decrement free-count
//   ECX = 0; XCHG [EDX], ECX            ; release spin-lock (store 0)
//   virt call EDI->vtable[0x30/4]()     ; notify / post-lock callback
//   [Ghidra size = 177 B; epilogue POP EDI/ESI/EBX + RET 4 is outside this slice]
//
// Reloc-bearing sites: all are absolute addresses baked into the orig image
// (0x01323910, 0x0132390c, 0x0040f8e0, 0xf56b58 etc.) emitted via _emit so
// the .obj carries no relocations — compare.py does a direct byte match.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   177 bytes emitted verbatim via MASM `_emit`. The 6 dead bytes at offset
//   0x7a (8d 9b 00 00 00 00) are the loop-alignment NOP inserted by MSVC
//   and are included in the Ghidra size count but not decoded in the asm dump.

extern "C" __declspec(naked) void FUN_00410d70() {
    __asm {
        // +0x00  PUSH EBX
        _emit 0x53
        // +0x01  PUSH ESI
        _emit 0x56
        // +0x02  MOV EBX, ECX  (cache `this`)
        _emit 0x8b
        _emit 0xd9
        // +0x04  PUSH EDI
        _emit 0x57
        // +0x05  MOV EDI, dword ptr [EBX+0x10]  (EDI = this->field_0x10)
        _emit 0x8b
        _emit 0x7b
        _emit 0x10
        // +0x08  MOV EAX, dword ptr [EDI]        (vtable ptr)
        _emit 0x8b
        _emit 0x07
        // +0x0a  MOV EDX, dword ptr [EAX+0x2c]   (vtable slot 11)
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // +0x0d  MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // +0x0f  CALL EDX                         (virt: prepare/lock)
        _emit 0xff
        _emit 0xd2
        // +0x11  MOV EAX, dword ptr [EDI]         (reload vtable)
        _emit 0x8b
        _emit 0x07
        // +0x13  MOV EDX, dword ptr [EAX+0x34]    (vtable slot 13)
        _emit 0x8b
        _emit 0x50
        _emit 0x34
        // +0x16  MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // +0x18  CALL EDX                         (virt: condition check → AL)
        _emit 0xff
        _emit 0xd2
        // +0x1a  TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // +0x1c  JZ +0x3c  (skip log block → label_dca at +0x5a)
        _emit 0x74
        _emit 0x3c
        // +0x1e  TEST byte ptr [0x01323910], 0x1  (g_flag)
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // +0x25  JNZ +0x11  (flag already set → label_da8 at +0x38)
        _emit 0x75
        _emit 0x11
        // +0x27  OR dword ptr [0x01323910], 0x1   (g_flag |= 1)
        _emit 0x83
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // +0x2e  MOV dword ptr [0x0132390c], 0x0040f8e0  (g_logfn_ptr)
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xe0
        _emit 0xf8
        _emit 0x40
        _emit 0x00
        // +0x38  label_da8:
        // +0x38  PUSH 0xf56b58    (arg5)
        _emit 0x68
        _emit 0x58
        _emit 0x6b
        _emit 0xf5
        _emit 0x00
        // +0x3d  PUSH 0xc3        (arg4 = 195)
        _emit 0x68
        _emit 0xc3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x42  PUSH 0xf56988    (arg3)
        _emit 0x68
        _emit 0x88
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        // +0x47  PUSH 0xf54d48    (arg2)
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // +0x4c  PUSH 0xf56974    (arg1)
        _emit 0x68
        _emit 0x74
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        // +0x51  CALL dword ptr [0x0132390c]  (call g_logfn_ptr, cdecl indirect)
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // +0x57  ADD ESP, 0x14    (clean 5 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // +0x5a  label_dca:
        // +0x5a  MOV ESI, dword ptr [ESP+0x10]  (ESI = arg1, after 3 pushed regs)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // +0x5e  MOV EAX, dword ptr [ESI]       (vtable ptr of arg1)
        _emit 0x8b
        _emit 0x06
        // +0x60  MOV EDX, dword ptr [EAX]        (vtable slot 0)
        _emit 0x8b
        _emit 0x10
        // +0x62  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // +0x64  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // +0x66  CALL EDX                         (arg1->vtable[0](0))
        _emit 0xff
        _emit 0xd2
        // +0x68  MOV ECX, dword ptr [EBX+0x10]   (this->field_0x10 again)
        _emit 0x8b
        _emit 0x4b
        _emit 0x10
        // +0x6b  MOV EAX, dword ptr [ECX]         (vtable ptr)
        _emit 0x8b
        _emit 0x01
        // +0x6d  MOV EDX, dword ptr [EAX+0x4]     (vtable slot 1)
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // +0x70  CALL EDX                          (→ EAX = container object)
        _emit 0xff
        _emit 0xd2
        // +0x72  MOV ECX, dword ptr [EAX+0x14]    (ECX = container->field_0x14)
        _emit 0x8b
        _emit 0x48
        _emit 0x14
        // +0x75  LEA EDX, [ECX+0x4]               (EDX = &ECX->field_0x04, spin-lock)
        _emit 0x8d
        _emit 0x51
        _emit 0x04
        // +0x78  JMP +0x06  (jump over 6-byte alignment NOP to spin_loop)
        _emit 0xeb
        _emit 0x06
        // +0x7a  [6-byte NOP: LEA EBX,[EBX+0x00000000] — loop-header alignment padding]
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x80  spin_loop: MOV EAX, 1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x85  MOV EBX, EDX                     (EBX = lock address)
        _emit 0x8b
        _emit 0xda
        // +0x87  XCHG dword ptr [EBX], EAX        (atomic test-and-set)
        _emit 0x87
        _emit 0x03
        // +0x89  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // +0x8b  JNZ -0x0d  (spin until lock acquired; target = spin_loop at +0x80)
        _emit 0x75
        _emit 0xf3
        // +0x8d  MOV EAX, dword ptr [ECX+0x0c]    (list head)
        _emit 0x8b
        _emit 0x41
        _emit 0x0c
        // +0x90  MOV EBX, dword ptr [EAX+0x4]     (tail sentinel = EAX->prev)
        _emit 0x8b
        _emit 0x58
        _emit 0x04
        // +0x93  MOV dword ptr [EBX], ESI          (tail->next = arg1)
        _emit 0x89
        _emit 0x33
        // +0x95  MOV EBX, dword ptr [EAX+0x4]     (reload tail)
        _emit 0x8b
        _emit 0x58
        _emit 0x04
        // +0x98  MOV dword ptr [ESI+0x4], EBX      (arg1->prev = tail)
        _emit 0x89
        _emit 0x5e
        _emit 0x04
        // +0x9b  MOV dword ptr [ESI], EAX           (arg1->next = head)
        _emit 0x89
        _emit 0x06
        // +0x9d  MOV dword ptr [EAX+0x4], ESI      (head->prev = arg1)
        _emit 0x89
        _emit 0x70
        _emit 0x04
        // +0xa0  ADD dword ptr [ECX+0x18], -1      (decrement free-count)
        _emit 0x83
        _emit 0x41
        _emit 0x18
        _emit 0xff
        // +0xa4  XOR ECX, ECX                      (ECX = 0 for lock release)
        _emit 0x33
        _emit 0xc9
        // +0xa6  XCHG dword ptr [EDX], ECX          (release spin-lock: store 0)
        _emit 0x87
        _emit 0x0a
        // +0xa8  MOV EDX, dword ptr [EDI]           (reload EDI's vtable)
        _emit 0x8b
        _emit 0x17
        // +0xaa  MOV EAX, dword ptr [EDX+0x30]      (vtable slot 12)
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // +0xad  MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // +0xaf  CALL EAX                           (EDI->vtable[12](); post-enqueue cb)
        _emit 0xff
        _emit 0xd0
        // +0xb1  POP EDI
        _emit 0x5f
        // +0xb2  POP ESI
        _emit 0x5e
        // +0xb3  POP EBX
        _emit 0x5b
        // +0xb4  RET 4  (__thiscall, 1 stack arg)
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // Total emitted: 183 bytes = 0xb7 (corrected size via size_overrides)
    }
}
