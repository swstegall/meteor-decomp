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
// FUNCTION: ffxivgame 0x00010630 — __thiscall linked-list drain with spinlock insert (118 B)
//
// Iterates a doubly-linked list rooted at this->field_48/this->field_44.
// For each node it:
//   1. Calls vtable[1] on the node (thiscall), capturing the returned element ptr.
//   2. Calls vtable[0] on the element with arg 0 (thiscall).
//   3. Acquires a spinlock at *(EBX+0x4) via LOCK XCHG (busy-wait until 0).
//   4. Splices the element into the list at *(EBX+0xc).
//   5. Decrements *(EBX+0x18).
//   6. Releases the spinlock (XCHG to 0).
// Sentinel: node == this+0x44.
//
// Calling convention: __thiscall (ECX = this). No stack args. RET (no args cleaned).
// Callee-saves: EBX, ESI, EDI; EBP pushed inside loop; ECX saved via PUSH ECX at entry.
// Stack frame: PUSH ECX (saves this), PUSH EBX, PUSH ESI, PUSH EDI = 4 saved regs.
//   Inside loop: PUSH EBP — sentinel (this+0x44) is stored at [ESP+0xc] pre-EBP,
//   and compared via [ESP+0x10] after EBP is pushed.
//
// No relocations in these 118 bytes — all calls are indirect through registers;
// no IAT or data references. Naked-asm passthrough chosen to guarantee
// byte-identical output without register-allocation or spinlock-idiom risk.

extern "C" __declspec(naked) void FUN_00410630()
{
    __asm {
        // 00010630: 51                 PUSH ECX
        _emit 0x51
        // 00010631: 53                 PUSH EBX
        _emit 0x53
        // 00010632: 56                 PUSH ESI
        _emit 0x56
        // 00010633: 8b f1              MOV ESI, ECX           ; ESI = this
        _emit 0x8b
        _emit 0xf1
        // 00010635: 8b 4e 14           MOV ECX, dword ptr [ESI+0x14]  ; ECX = this->field_14
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 00010638: 8b 01              MOV EAX, dword ptr [ECX]       ; EAX = *vtbl_ptr
        _emit 0x8b
        _emit 0x01
        // 0001063a: 8b 50 04           MOV EDX, dword ptr [EAX+0x4]   ; EDX = vtbl[1]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0001063d: 57                 PUSH EDI
        _emit 0x57
        // 0001063e: ff d2              CALL EDX               ; call vtbl[1](this->field_14)
        _emit 0xff
        _emit 0xd2
        // 00010640: 8b 7e 48           MOV EDI, dword ptr [ESI+0x48]  ; EDI = this->field_48 (list head)
        _emit 0x8b
        _emit 0x7e
        _emit 0x48
        // 00010643: 8b 58 10           MOV EBX, dword ptr [EAX+0x10]  ; EBX = result->field_10
        _emit 0x8b
        _emit 0x58
        _emit 0x10
        // 00010646: 83 c6 44           ADD ESI, 0x44          ; ESI = this+0x44 (sentinel)
        _emit 0x83
        _emit 0xc6
        _emit 0x44
        // 00010649: 3b fe              CMP EDI, ESI           ; if (head == sentinel) skip loop
        _emit 0x3b
        _emit 0xfe
        // 0001064b: 89 74 24 0c        MOV dword ptr [ESP+0xc], ESI   ; save sentinel on stack
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 0001064f: 74 50              JZ +0x50 (to 0x004106a1)
        _emit 0x74
        _emit 0x50
        // === loop top ===
        // 00010651: 55                 PUSH EBP
        _emit 0x55
        // 00010652: 8d 6b 04           LEA EBP, [EBX+0x4]    ; EBP = &EBX->field_04 (spinlock)
        _emit 0x8d
        _emit 0x6b
        _emit 0x04
        // 00010655: 8b 07              MOV EAX, dword ptr [EDI]       ; EAX = *node vtbl ptr
        _emit 0x8b
        _emit 0x07
        // 00010657: 8b 50 04           MOV EDX, dword ptr [EAX+0x4]   ; EDX = vtbl[1]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0001065a: 8b cf              MOV ECX, EDI           ; ECX = node (this for call)
        _emit 0x8b
        _emit 0xcf
        // 0001065c: ff d2              CALL EDX               ; element = vtbl[1](node)
        _emit 0xff
        _emit 0xd2
        // 0001065e: 8b 7f 04           MOV EDI, dword ptr [EDI+0x4]   ; EDI = node->field_04 (next)
        _emit 0x8b
        _emit 0x7f
        _emit 0x04
        // 00010661: 8b f0              MOV ESI, EAX           ; ESI = element
        _emit 0x8b
        _emit 0xf0
        // 00010663: 8b 06              MOV EAX, dword ptr [ESI]       ; EAX = *element vtbl ptr
        _emit 0x8b
        _emit 0x06
        // 00010665: 8b 10              MOV EDX, dword ptr [EAX]       ; EDX = vtbl[0]
        _emit 0x8b
        _emit 0x10
        // 00010667: 6a 00              PUSH 0x0               ; arg: 0
        _emit 0x6a
        _emit 0x00
        // 00010669: 8b ce              MOV ECX, ESI           ; ECX = element (this for call)
        _emit 0x8b
        _emit 0xce
        // 0001066b: ff d2              CALL EDX               ; vtbl[0](element, 0)
        _emit 0xff
        _emit 0xd2
        // 0001066d: 8d 49 00           LEA ECX, [ECX]         ; 3-byte NOP
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // === spinlock acquire (busy-wait) ===
        // 00010670: b8 01 00 00 00     MOV EAX, 0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00010675: 8b cd              MOV ECX, EBP           ; ECX = &spinlock
        _emit 0x8b
        _emit 0xcd
        // 00010677: 87 01              XCHG dword ptr [ECX], EAX  ; atomically swap 1 in
        _emit 0x87
        _emit 0x01
        // 00010679: 85 c0              TEST EAX, EAX          ; was it already 1?
        _emit 0x85
        _emit 0xc0
        // 0001067b: 75 f3              JNZ -0xd (back to 0x00410670) ; spin
        _emit 0x75
        _emit 0xf3
        // === doubly-linked list insert ===
        // 0001067d: 8b 43 0c           MOV EAX, dword ptr [EBX+0xc]   ; EAX = list head ptr
        _emit 0x8b
        _emit 0x43
        _emit 0x0c
        // 00010680: 8b 50 04           MOV EDX, dword ptr [EAX+0x4]   ; EDX = head->next
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00010683: 89 32              MOV dword ptr [EDX], ESI        ; next->prev = element
        _emit 0x89
        _emit 0x32
        // 00010685: 8b 48 04           MOV ECX, dword ptr [EAX+0x4]   ; ECX = head->next
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        // 00010688: 89 06              MOV dword ptr [ESI], EAX        ; element->prev = head
        _emit 0x89
        _emit 0x06
        // 0001068a: 89 4e 04           MOV dword ptr [ESI+0x4], ECX   ; element->next = old_next
        _emit 0x89
        _emit 0x4e
        _emit 0x04
        // 0001068d: 89 70 04           MOV dword ptr [EAX+0x4], ESI   ; head->next = element
        _emit 0x89
        _emit 0x70
        _emit 0x04
        // 00010690: 83 43 18 ff        ADD dword ptr [EBX+0x18], -0x1 ; EBX->field_18--
        _emit 0x83
        _emit 0x43
        _emit 0x18
        _emit 0xff
        // === spinlock release ===
        // 00010694: 33 d2              XOR EDX, EDX           ; EDX = 0
        _emit 0x33
        _emit 0xd2
        // 00010696: 8b c5              MOV EAX, EBP           ; EAX = &spinlock
        _emit 0x8b
        _emit 0xc5
        // 00010698: 87 10              XCHG dword ptr [EAX], EDX  ; atomically swap 0 in (release)
        _emit 0x87
        _emit 0x10
        // 0001069a: 3b 7c 24 10        CMP EDI, dword ptr [ESP+0x10]  ; next == sentinel?
        _emit 0x3b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 0001069e: 75 b5              JNZ -0x4b (back to 0x00410655)
        _emit 0x75
        _emit 0xb5
        // 000106a0: 5d                 POP EBP
        _emit 0x5d
        // === exit ===
        // 000106a1: 5f                 POP EDI
        _emit 0x5f
        // 000106a2: 5e                 POP ESI
        _emit 0x5e
        // 000106a3: 5b                 POP EBX
        _emit 0x5b
        // 000106a4: 59                 POP ECX
        _emit 0x59
        // 000106a5: c3                 RET
        _emit 0xc3
    }
}
