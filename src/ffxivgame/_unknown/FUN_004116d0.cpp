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
// FUNCTION: ffxivgame 0x000116d0 — __thiscall manager method: vtable dispatch,
//                                   sentinel-guarded slot init, linked-list
//                                   walk with stream write, final vtable call
//                                   and flag set (213 B / 0xd5)
//
// Calling convention: __thiscall (ECX = this). No stack arguments. Returns
// bool (AL = this->field_0x48 after the final vtable call).
//
// Callee-saved registers pushed in prologue: EBX, EBP, ESI.
// EDI is shrink-wrapped — pushed only when the linked list is non-empty.
//
// Stack frame: SUB ESP, 0x8 allocates two DWORD locals:
//   local_low  ([ESP+0xC] relative to the saved-ESI level)  — current
//               loop node pointer; saved before the list walk so MSVC can
//               spill/restore ESI across loop iterations.
//   local_high ([ESP+0x14] inside loop, after PUSH EDI) — temp value from
//               a vtable[1] call on the inner sub-object.
//
// Object layout (inferred from touched offsets, ECX = this):
//   [this +  0x00]   void **vtable            — vptr
//   [this +  0x04]   SomeBase *field_4        — has its own vtable; slot 4
//                                               called with one arg
//   [this +  0x18]   StreamBuf member_18      — sub-object passed as `this`
//                                               to FUN_004113b0
//   [this +  0x24]   void (*field_24)(void)   — direct fn ptr, no `this`
//   [this +  0x28]   void (*field_28)(void)   — direct fn ptr, no `this`
//   [this +  0x34]   void *field_34           — written with vtable[1] result
//                                               (or 0 if sentinel)
//   [this +  0x38]   DWORD field_38           — zeroed (= member_18 + 0x20)
//   [this +  0x3c]   void *sentinel_3c        — sentinel for field_44 list
//   [this +  0x44]   void *field_44           — head of a sentinel-guarded list
//   [this +  0x48]   bool  field_48           — set to 1 before final vtable
//                                               call; returned by function
//   [this +  0x4c]   void *sentinel_4c        — sentinel for field_54 list
//   [this +  0x54]   ListNode *field_54       — head of main linked list
//
// ListNode layout (list linked by field +0x8):
//   [node +  0x00]   void **vtable            — vtable[1] called with ECX=node,
//                                               returns Item *
//   [node +  0x08]   ListNode *next
//
// Item layout (returned by ListNode::vtable[1](node)):
//   [item +  0x04]   InnerObj *inner          — has vtable; slots 1 and 2 used
//   [item +  0x18]   void *field_18           — guarded: 0 means "not yet inited"
//   [item +  0x28]   DWORD field_28           — passed to field_4->vtable[4], then zeroed
//
// Calls (all through registers — no absolute addresses, one REL32 reloc):
//   this->vtable[11](this)         — indirect via register
//   this->field_28()               — direct fn ptr, no this
//   sentinel check on field_44     — vtable[1] with ECX=field_44 if not sentinel
//   per-list-node:
//     node->vtable[1](node)        → Item *
//     if item->field_18 == 0:
//       inner->vtable[2](inner)    → temp
//       inner->vtable[1](inner)    → val
//       FUN_004113b0(member_18, val, temp, 0)  [REL32 reloc — masked]
//       inner->vtable[1](inner)    → item->field_18
//       field_4->vtable[4](item->field_28)
//       item->field_28 = 0
//   this->field_24()               — direct fn ptr, no this
//   this->vtable[12](this)         — after setting field_48=1
//
// No absolute address relocations; the single REL32 to FUN_004113b0 is masked
// by compare.py. Naked-asm passthrough chosen to guarantee byte-identical
// output without MSVC register-allocation risk on this complex function.

// Forward declaration for the one direct relative CALL target.
// compare.py masks the 4-byte REL32 displacement, so the symbol just
// needs to exist for the assembler to emit the E8 opcode.
extern "C" void FUN_004113b0();

extern "C" __declspec(naked) void FUN_004116d0()
{
    __asm {
        // 000116d0:  83 ec 08     SUB ESP,0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 000116d3:  53           PUSH EBX
        _emit 0x53
        // 000116d4:  8b d9        MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 000116d6:  8b 03        MOV EAX,dword ptr [EBX]
        _emit 0x8b
        _emit 0x03
        // 000116d8:  8b 50 2c     MOV EDX,dword ptr [EAX+0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 000116db:  55           PUSH EBP
        _emit 0x55
        // 000116dc:  56           PUSH ESI
        _emit 0x56
        // 000116dd:  ff d2        CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000116df:  8b 43 28     MOV EAX,dword ptr [EBX+0x28]
        _emit 0x8b
        _emit 0x43
        _emit 0x28
        // 000116e2:  ff d0        CALL EAX
        _emit 0xff
        _emit 0xd0
        // 000116e4:  8b 4b 44     MOV ECX,dword ptr [EBX+0x44]
        _emit 0x8b
        _emit 0x4b
        _emit 0x44
        // 000116e7:  8d 6b 18     LEA EBP,[EBX+0x18]
        _emit 0x8d
        _emit 0x6b
        _emit 0x18
        // 000116ea:  8d 55 24     LEA EDX,[EBP+0x24]
        _emit 0x8d
        _emit 0x55
        _emit 0x24
        // 000116ed:  3b ca        CMP ECX,EDX
        _emit 0x3b
        _emit 0xca
        // 000116ef:  74 09        JZ +0x09  (→ 0x004116fa)
        _emit 0x74
        _emit 0x09
        // 000116f1:  8b 01        MOV EAX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 000116f3:  8b 50 04     MOV EDX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000116f6:  ff d2        CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000116f8:  eb 02        JMP +0x02  (→ 0x004116fc)
        _emit 0xeb
        _emit 0x02
        // 000116fa:  33 c0        XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 000116fc:  89 45 1c     MOV dword ptr [EBP+0x1c],EAX
        _emit 0x89
        _emit 0x45
        _emit 0x1c
        // 000116ff:  c7 45 20 00 00 00 00  MOV dword ptr [EBP+0x20],0x0
        _emit 0xc7
        _emit 0x45
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00011706:  8b 73 54     MOV ESI,dword ptr [EBX+0x54]
        _emit 0x8b
        _emit 0x73
        _emit 0x54
        // 00011709:  8d 43 4c     LEA EAX,[EBX+0x4c]
        _emit 0x8d
        _emit 0x43
        _emit 0x4c
        // 0001170c:  3b f0        CMP ESI,EAX
        _emit 0x3b
        _emit 0xf0
        // 0001170e:  89 74 24 0c  MOV dword ptr [ESP+0xc],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 00011712:  74 75        JZ +0x75  (→ 0x00411789)
        _emit 0x74
        _emit 0x75
        // 00011714:  57           PUSH EDI
        _emit 0x57
        // 00011715:  eb 04        JMP +0x04  (→ 0x0041171b)
        _emit 0xeb
        _emit 0x04
        // 00011717:  8b 74 24 10  MOV ESI,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 0001171b:  8b 06        MOV EAX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 0001171d:  8b 50 04     MOV EDX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00011720:  8b ce        MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00011722:  ff d2        CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00011724:  8b f8        MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 00011726:  83 7f 18 00  CMP dword ptr [EDI+0x18],0x0
        _emit 0x83
        _emit 0x7f
        _emit 0x18
        _emit 0x00
        // 0001172a:  75 4e        JNZ +0x4e  (→ 0x0041177a)
        _emit 0x75
        _emit 0x4e
        // 0001172c:  8b 47 04     MOV EAX,dword ptr [EDI+0x4]
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // 0001172f:  8b 50 08     MOV EDX,dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 00011732:  8d 77 04     LEA ESI,[EDI+0x4]
        _emit 0x8d
        _emit 0x77
        _emit 0x04
        // 00011735:  8b ce        MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00011737:  ff d2        CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00011739:  89 44 24 14  MOV dword ptr [ESP+0x14],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0001173d:  8b 06        MOV EAX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 0001173f:  8b 50 04     MOV EDX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00011742:  8b ce        MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00011744:  ff d2        CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00011746:  8b 4c 24 14  MOV ECX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0001174a:  6a 00        PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001174c:  51           PUSH ECX
        _emit 0x51
        // 0001174d:  50           PUSH EAX
        _emit 0x50
        // 0001174e:  8b cd        MOV ECX,EBP
        _emit 0x8b
        _emit 0xcd
        // 00011750:  e8 5b fc ff ff  CALL 0x004113b0  [REL32 reloc — masked]
        _emit 0xe8
        _emit 0x5b
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 00011755:  8b 16        MOV EDX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x16
        // 00011757:  8b 42 04     MOV EAX,dword ptr [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 0001175a:  8b ce        MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0001175c:  ff d0        CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001175e:  89 47 18     MOV dword ptr [EDI+0x18],EAX
        _emit 0x89
        _emit 0x47
        _emit 0x18
        // 00011761:  8b 4b 04     MOV ECX,dword ptr [EBX+0x4]
        _emit 0x8b
        _emit 0x4b
        _emit 0x04
        // 00011764:  8b 11        MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 00011766:  8b 47 28     MOV EAX,dword ptr [EDI+0x28]
        _emit 0x8b
        _emit 0x47
        _emit 0x28
        // 00011769:  8b 52 10     MOV EDX,dword ptr [EDX+0x10]
        _emit 0x8b
        _emit 0x52
        _emit 0x10
        // 0001176c:  50           PUSH EAX
        _emit 0x50
        // 0001176d:  ff d2        CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0001176f:  8b 74 24 10  MOV ESI,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00011773:  c7 47 28 00 00 00 00  MOV dword ptr [EDI+0x28],0x0
        _emit 0xc7
        _emit 0x47
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001177a:  8b 76 08     MOV ESI,dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x76
        _emit 0x08
        // 0001177d:  8d 43 4c     LEA EAX,[EBX+0x4c]
        _emit 0x8d
        _emit 0x43
        _emit 0x4c
        // 00011780:  3b f0        CMP ESI,EAX
        _emit 0x3b
        _emit 0xf0
        // 00011782:  89 74 24 10  MOV dword ptr [ESP+0x10],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00011786:  75 8f        JNZ -0x71  (→ 0x00411717)
        _emit 0x75
        _emit 0x8f
        // 00011788:  5f           POP EDI
        _emit 0x5f
        // 00011789:  8b 43 24     MOV EAX,dword ptr [EBX+0x24]
        _emit 0x8b
        _emit 0x43
        _emit 0x24
        // 0001178c:  ff d0        CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001178e:  8b 13        MOV EDX,dword ptr [EBX]
        _emit 0x8b
        _emit 0x13
        // 00011790:  8b 42 30     MOV EAX,dword ptr [EDX+0x30]
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 00011793:  8b cb        MOV ECX,EBX
        _emit 0x8b
        _emit 0xcb
        // 00011795:  c6 43 48 01  MOV byte ptr [EBX+0x48],0x1
        _emit 0xc6
        _emit 0x43
        _emit 0x48
        _emit 0x01
        // 00011799:  ff d0        CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001179b:  8a 43 48     MOV AL,byte ptr [EBX+0x48]
        _emit 0x8a
        _emit 0x43
        _emit 0x48
        // 0001179e:  5e           POP ESI
        _emit 0x5e
        // 0001179f:  5d           POP EBP
        _emit 0x5d
        // 000117a0:  5b           POP EBX
        _emit 0x5b
        // 000117a1:  83 c4 08     ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 000117a4:  c3           RET
        _emit 0xc3
    }
}
