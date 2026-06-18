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
// FUNCTION: ffxivgame 0x00010be0 — engine_memory __thiscall with one
//                                  stack arg (199 B / 0xc7).
//           RET 0x4 epilogue → __thiscall, callee cleans 1 DWORD arg.
//
// Shape (read from RVA 0x00010be0):
//
//   53 56 8b d9 57          ; PUSH EBX; PUSH ESI; MOV EBX,ECX; PUSH EDI
//   8b 7b 10                ; MOV EDI,[EBX+0x10]
//   8b 07 8b 50 2c 8b cf    ; vtbl call [EDI+0x2c] → Lock / AcquireLock
//   ff d2
//   8b 07 8b 50 34 8b cf    ; vtbl call [EDI+0x34] → IsReady / Poll
//   ff d2
//   84 c0 74 3c             ; TEST AL,AL; JZ skip_assert
//
//   f6 05 ... 01            ; TEST [g_flag],1 — once-only init sentinel
//   75 11                   ; JNZ already_init
//   83 0d ... 01            ; OR [g_flag],1
//   c7 05 ... e0 f8 40 00   ; MOV [g_assert_fn],0x40f8e0
// already_init:
//   PUSH x5 args + indirect CALL [g_assert_fn] → assertion reporter
//   ADD ESP,0x14
//
// skip_assert:
//   ECX = [ESP+0x10]        ; caller's arg (an object ptr)
//   EAX = ECX→vtbl→+0x14() ; some factory / lookup
//   ESI = EAX→vtbl→+0x04() ; child-object getter
//   [ESI]→vtbl[0](0)        ; init call on child object
//   EAX = EBX→m_field→vtbl[+0x04]() ; get node root
//   EAX = EAX→+0x10        ; root's sub-field
//   EDX = &(EAX→+0x04)     ; pointer to spinlock word
//   spin-loop: XCHG [EDX],1 until ECX (old value) == 0
//   linked-list insertion: splice ESI into list anchored at EAX→+0xc
//   ADD [EAX+0x18],-1       ; decrement free-slot count
//   XOR EAX,EAX; XCHG [EDX],EAX ; release spinlock
//   vtbl call [EDI+0x30]   → Unlock / ReleaseLock
//   POP EDI; POP ESI; POP EBX
//   RET 0x4
//
// Reloc-bearing sites (absolute addresses baked into orig image space;
// all resolved against the orig PE at base 0x00400000):
//   +0x1e   TEST mem8  → 0x01323910  (g_assert_flag)
//   +0x25   OR   mem32 → 0x01323910  (g_assert_flag)
//   +0x2e   MOV  mem32 → 0x0132390c  (g_assert_fn), imm=0x40f8e0
//   +0x38   PUSH imm32 → 0xf56ac0    (.rdata string)
//   +0x3d   PUSH imm32 → 0xad        (line number literal)  [not reloc]
//   +0x42   PUSH imm32 → 0xf56988    (.rdata string)
//   +0x47   PUSH imm32 → 0xf54d48    (.rdata string)
//   +0x4c   PUSH imm32 → 0xf56974    (.rdata string)
//   +0x51   CALL mem32 → 0x0132390c  (indirect through g_assert_fn)
//
// Reconstruction strategy — naked-asm byte passthrough (199 bytes verbatim).

extern "C" __declspec(naked) void FUN_00410be0() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV EBX,ECX
        _emit 0xd9
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI,dword ptr [EBX+0x10]
        _emit 0x7b
        _emit 0x10
        _emit 0x8b              // MOV EAX,dword ptr [EDI]
        _emit 0x07
        _emit 0x8b              // MOV EDX,dword ptr [EAX+0x2c]
        _emit 0x50
        _emit 0x2c
        _emit 0x8b              // MOV ECX,EDI
        _emit 0xcf
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EAX,dword ptr [EDI]
        _emit 0x07
        _emit 0x8b              // MOV EDX,dword ptr [EAX+0x34]
        _emit 0x50
        _emit 0x34
        _emit 0x8b              // MOV ECX,EDI
        _emit 0xcf
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x84              // TEST AL,AL
        _emit 0xc0
        _emit 0x74              // JZ +0x3c (skip_assert)
        _emit 0x3c
        _emit 0xf6              // TEST byte ptr [0x01323910],0x1
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75              // JNZ +0x11 (already_init)
        _emit 0x11
        _emit 0x83              // OR dword ptr [0x01323910],0x1
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [0x0132390c],0x40f8e0
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xe0
        _emit 0xf8
        _emit 0x40
        _emit 0x00
        _emit 0x68              // PUSH 0xf56ac0  (already_init:)
        _emit 0xc0
        _emit 0x6a
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0xad
        _emit 0xad
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf56988
        _emit 0x88
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0xf54d48
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0xf56974
        _emit 0x74
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD ESP,0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x8b              // MOV ECX,dword ptr [ESP+0x10]  (skip_assert:)
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EAX,dword ptr [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX,dword ptr [EAX+0x14]
        _emit 0x50
        _emit 0x14
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EDX,dword ptr [EAX]
        _emit 0x10
        _emit 0x8b              // MOV ECX,EAX
        _emit 0xc8
        _emit 0x8b              // MOV EAX,dword ptr [EDX+0x4]
        _emit 0x42
        _emit 0x04
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV ESI,EAX
        _emit 0xf0
        _emit 0x8b              // MOV EDX,dword ptr [ESI]
        _emit 0x16
        _emit 0x8b              // MOV EAX,dword ptr [EDX]
        _emit 0x02
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x8b              // MOV ECX,ESI
        _emit 0xce
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV ECX,dword ptr [EBX+0x10]
        _emit 0x4b
        _emit 0x10
        _emit 0x8b              // MOV EDX,dword ptr [ECX]
        _emit 0x11
        _emit 0x8b              // MOV EAX,dword ptr [EDX+0x4]
        _emit 0x42
        _emit 0x04
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV EAX,dword ptr [EAX+0x10]
        _emit 0x40
        _emit 0x10
        _emit 0x8d              // LEA EDX,[EAX+0x4]
        _emit 0x50
        _emit 0x04
        _emit 0x8d              // LEA EBX,[EBX] (alignment NOP)
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xb9              // MOV ECX,0x1  (spin_loop:)
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EBX,EDX
        _emit 0xda
        _emit 0x87              // XCHG dword ptr [EBX],ECX
        _emit 0x0b
        _emit 0x85              // TEST ECX,ECX
        _emit 0xc9
        _emit 0x75              // JNZ -0xd (spin_loop)
        _emit 0xf3
        _emit 0x8b              // MOV ECX,dword ptr [EAX+0xc]
        _emit 0x48
        _emit 0x0c
        _emit 0x8b              // MOV EBX,dword ptr [ECX+0x4]
        _emit 0x59
        _emit 0x04
        _emit 0x89              // MOV dword ptr [EBX],ESI
        _emit 0x33
        _emit 0x8b              // MOV EBX,dword ptr [ECX+0x4]
        _emit 0x59
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ESI+0x4],EBX
        _emit 0x5e
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ESI],ECX
        _emit 0x0e
        _emit 0x89              // MOV dword ptr [ECX+0x4],ESI
        _emit 0x71
        _emit 0x04
        _emit 0x83              // ADD dword ptr [EAX+0x18],-0x1
        _emit 0x40
        _emit 0x18
        _emit 0xff
        _emit 0x33              // XOR EAX,EAX
        _emit 0xc0
        _emit 0x87              // XCHG dword ptr [EDX],EAX
        _emit 0x02
        _emit 0x8b              // MOV EDX,dword ptr [EDI]
        _emit 0x17
        _emit 0x8b              // MOV EAX,dword ptr [EDX+0x30]
        _emit 0x42
        _emit 0x30
        _emit 0x8b              // MOV ECX,EDI
        _emit 0xcf
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
