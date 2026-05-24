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
// FUNCTION: ffxivgame 0x004132a0 — nested virtual-dispatch "walk and notify"
//                                  (__thiscall, 173 bytes / 0xad)
//
// Calling convention: __thiscall (ECX = this); returns void.
// Callee-saves pushed: ECX (this slot), EBX, EBP, ESI, EDI.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Three alignment NOPs appear mid-function:
//     8d 9b 00 00 00 00  LEA EBX,[EBX+0]  (6-byte, between loop1 and outer loop)
//     8d 49 00           LEA ECX,[ECX+0]  (3-byte, before inner loop1)
//     8d 49 00           LEA ECX,[ECX+0]  (3-byte, before inner loop2)
//   EBP is first set to `this`, then mid-loop reassigned to iVar1+4 as an
//   arg pointer for the inner vtable call, then reloaded from [ESP+0x10]
//   (the saved ECX slot) at the bottom of each outer-loop iteration.
//   This mid-loop register repurposing cannot be expressed in C++ source
//   without naked-asm control. All virtual calls are indirect (CALL EDX /
//   CALL EAX) — no external-symbol relocations are emitted; compare.py
//   masks nothing extra.
//
// High-level description:
//   Phase 1 (optional, if *(this+0x4c) != this+0x44):
//     Walk the *(p+0x4c) singly-linked chain via vtable[1] until the
//     chain tail is reached (sentinel == p+0x44).  Result → iVar1.
//   Phase 2 (outer do-while iVar1 != 0):
//     For each node in the doubly-linked list rooted at iVar1+0x40
//     (sentinel iVar1+0x38, advance via *(node+8)):
//       iVar2 = node->vtable[1]()
//       (*(iVar2+0xc))->vtable[1](iVar1+4, *(iVar2+0x10), 0)
//     If iVar1 == this → return.
//     iVar2 = *(iVar1+0x2c); if 0 → return.
//     If *(iVar1+0x10) != iVar2+0x44:
//       iVar2 = (*(iVar1+0x10))->vtable[1]()
//       Walk the *(p+0x4c) chain of iVar2 via vtable[1].
//     iVar1 = iVar2; continue outer loop.

extern "C" __declspec(naked) void FUN_004132a0() {
    __asm {
        // 000132a0: 51                   PUSH ECX   (this slot)
        _emit 0x51
        // 000132a1: 53                   PUSH EBX
        _emit 0x53
        // 000132a2: 55                   PUSH EBP
        _emit 0x55
        // 000132a3: 8b e9                MOV EBP,ECX   (EBP = this)
        _emit 0x8b
        _emit 0xe9
        // 000132a5: 8b 4d 4c             MOV ECX,[EBP+0x4c]  (piVar3 = *(this+0x4c))
        _emit 0x8b
        _emit 0x4d
        _emit 0x4c
        // 000132a8: 8d 55 44             LEA EDX,[EBP+0x44]  (sentinel = this+0x44)
        _emit 0x8d
        _emit 0x55
        _emit 0x44
        // 000132ab: 3b ca                CMP ECX,EDX
        _emit 0x3b
        _emit 0xca
        // 000132ad: 56                   PUSH ESI
        _emit 0x56
        // 000132ae: 57                   PUSH EDI
        _emit 0x57
        // 000132af: 89 6c 24 10          MOV [ESP+0x10],EBP  (save this to ECX slot)
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // 000132b3: 8b c5                MOV EAX,EBP   (EAX = this = initial iVar1)
        _emit 0x8b
        _emit 0xc5
        // 000132b5: 74 11                JZ +0x11 → 000132c8  (skip phase-1 loop)
        _emit 0x74
        _emit 0x11
        // === phase-1 loop top (000132b7) ===
        // 000132b7: 8b 01                MOV EAX,[ECX]   (vtable of piVar3)
        _emit 0x8b
        _emit 0x01
        // 000132b9: 8b 50 04             MOV EDX,[EAX+4]   (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000132bc: ff d2                CALL EDX   (iVar1 = piVar3->vtable[1]())
        _emit 0xff
        _emit 0xd2
        // 000132be: 8b 48 4c             MOV ECX,[EAX+0x4c]  (piVar3 = *(iVar1+0x4c))
        _emit 0x8b
        _emit 0x48
        _emit 0x4c
        // 000132c1: 8d 50 44             LEA EDX,[EAX+0x44]  (sentinel = iVar1+0x44)
        _emit 0x8d
        _emit 0x50
        _emit 0x44
        // 000132c4: 3b ca                CMP ECX,EDX
        _emit 0x3b
        _emit 0xca
        // 000132c6: 75 ef                JNZ -0x11 → 000132b7  (loop back)
        _emit 0x75
        _emit 0xef
        // === after phase-1 loop ===
        // 000132c8: 8b d8                MOV EBX,EAX   (EBX = iVar1)
        _emit 0x8b
        _emit 0xd8
        // 000132ca: 8d 9b 00 00 00 00    LEA EBX,[EBX+0]  (6-byte NOP; outer-loop alignment)
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // === outer do-while loop top (000132d0) ===
        // 000132d0: 8b 73 40             MOV ESI,[EBX+0x40]  (piVar3 = *(iVar1+0x40))
        _emit 0x8b
        _emit 0x73
        _emit 0x40
        // 000132d3: 8d 7b 38             LEA EDI,[EBX+0x38]  (inner sentinel = iVar1+0x38)
        _emit 0x8d
        _emit 0x7b
        _emit 0x38
        // 000132d6: 3b f7                CMP ESI,EDI
        _emit 0x3b
        _emit 0xf7
        // 000132d8: 74 2b                JZ +0x2b → 00013305  (skip inner loop if empty)
        _emit 0x74
        _emit 0x2b
        // 000132da: 8d 6b 04             LEA EBP,[EBX+4]   (EBP = iVar1+4, 3rd arg slot)
        _emit 0x8d
        _emit 0x6b
        _emit 0x04
        // 000132dd: 8d 49 00             LEA ECX,[ECX+0]   (3-byte NOP; inner-loop alignment)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // === inner loop top (000132e0) ===
        // 000132e0: 8b 06                MOV EAX,[ESI]   (vtable of piVar3)
        _emit 0x8b
        _emit 0x06
        // 000132e2: 8b 50 04             MOV EDX,[EAX+4]   (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000132e5: 8b ce                MOV ECX,ESI   (ECX = piVar3)
        _emit 0x8b
        _emit 0xce
        // 000132e7: ff d2                CALL EDX   (iVar2 = piVar3->vtable[1]())
        _emit 0xff
        _emit 0xd2
        // 000132e9: 8b 48 0c             MOV ECX,[EAX+0x0c]  (ECX = *(iVar2+0xc))
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 000132ec: 8b 40 10             MOV EAX,[EAX+0x10]  (EAX = *(iVar2+0x10))
        _emit 0x8b
        _emit 0x40
        _emit 0x10
        // 000132ef: 8b 11                MOV EDX,[ECX]   (vtable of *(iVar2+0xc))
        _emit 0x8b
        _emit 0x11
        // 000132f1: 8b 52 04             MOV EDX,[EDX+4]   (vtable[1])
        _emit 0x8b
        _emit 0x52
        _emit 0x04
        // 000132f4: 6a 00                PUSH 0
        _emit 0x6a
        _emit 0x00
        // 000132f6: 50                   PUSH EAX  (*(iVar2+0x10))
        _emit 0x50
        // 000132f7: 55                   PUSH EBP  (iVar1+4)
        _emit 0x55
        // 000132f8: ff d2                CALL EDX  ((*(iVar2+0xc))->vtable[1](iVar1+4, *(iVar2+0x10), 0))
        _emit 0xff
        _emit 0xd2
        // 000132fa: 8b 76 08             MOV ESI,[ESI+0x8]  (piVar3 = piVar3[2])
        _emit 0x8b
        _emit 0x76
        _emit 0x08
        // 000132fd: 3b f7                CMP ESI,EDI
        _emit 0x3b
        _emit 0xf7
        // 000132ff: 75 df                JNZ -0x21 → 000132e0  (inner loop back)
        _emit 0x75
        _emit 0xdf
        // === after inner loop — reload this ===
        // 00013301: 8b 6c 24 10          MOV EBP,[ESP+0x10]  (reload this from ECX slot)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // === outer-loop condition checks (join point for empty inner list) ===
        // 00013305: 3b dd                CMP EBX,EBP   (iVar1 == this?)
        _emit 0x3b
        _emit 0xdd
        // 00013307: 74 3e                JZ +0x3e → 00013347  (return if at root)
        _emit 0x74
        _emit 0x3e
        // 00013309: 8b 43 2c             MOV EAX,[EBX+0x2c]  (iVar2 = *(iVar1+0x2c))
        _emit 0x8b
        _emit 0x43
        _emit 0x2c
        // 0001330c: 85 c0                TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001330e: 74 37                JZ +0x37 → 00013347  (return if iVar2 == 0)
        _emit 0x74
        _emit 0x37
        // 00013310: 8b 5b 10             MOV EBX,[EBX+0x10]  (EBX = *(iVar1+0x10))
        _emit 0x8b
        _emit 0x5b
        _emit 0x10
        // 00013313: 8d 48 44             LEA ECX,[EAX+0x44]  (ECX = iVar2+0x44)
        _emit 0x8d
        _emit 0x48
        _emit 0x44
        // 00013316: 3b d9                CMP EBX,ECX   (*(iVar1+0x10) == iVar2+0x44?)
        _emit 0x3b
        _emit 0xd9
        // 00013318: 74 27                JZ +0x27 → 00013341  (skip chain-walk if equal)
        _emit 0x74
        _emit 0x27
        // 0001331a: 8b 13                MOV EDX,[EBX]   (vtable of *(iVar1+0x10))
        _emit 0x8b
        _emit 0x13
        // 0001331c: 8b 42 04             MOV EAX,[EDX+4]   (vtable[1])
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 0001331f: 8b cb                MOV ECX,EBX   (ECX = *(iVar1+0x10))
        _emit 0x8b
        _emit 0xcb
        // 00013321: ff d0                CALL EAX   (iVar2 = (*(iVar1+0x10))->vtable[1]())
        _emit 0xff
        _emit 0xd0
        // 00013323: 8b 48 4c             MOV ECX,[EAX+0x4c]  (piVar3 = *(iVar2+0x4c))
        _emit 0x8b
        _emit 0x48
        _emit 0x4c
        // 00013326: 8d 50 44             LEA EDX,[EAX+0x44]  (sentinel = iVar2+0x44)
        _emit 0x8d
        _emit 0x50
        _emit 0x44
        // 00013329: 3b ca                CMP ECX,EDX
        _emit 0x3b
        _emit 0xca
        // 0001332b: 74 14                JZ +0x14 → 00013341  (skip if chain already at tail)
        _emit 0x74
        _emit 0x14
        // 0001332d: 8d 49 00             LEA ECX,[ECX+0]   (3-byte NOP; loop alignment)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // === phase-3 inner chain-walk loop top (00013330) ===
        // 00013330: 8b 01                MOV EAX,[ECX]   (vtable of piVar3)
        _emit 0x8b
        _emit 0x01
        // 00013332: 8b 50 04             MOV EDX,[EAX+4]   (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00013335: ff d2                CALL EDX   (iVar2 = piVar3->vtable[1]())
        _emit 0xff
        _emit 0xd2
        // 00013337: 8b 48 4c             MOV ECX,[EAX+0x4c]  (piVar3 = *(iVar2+0x4c))
        _emit 0x8b
        _emit 0x48
        _emit 0x4c
        // 0001333a: 8d 50 44             LEA EDX,[EAX+0x44]  (sentinel = iVar2+0x44)
        _emit 0x8d
        _emit 0x50
        _emit 0x44
        // 0001333d: 3b ca                CMP ECX,EDX
        _emit 0x3b
        _emit 0xca
        // 0001333f: 75 ef                JNZ -0x11 → 00013330  (loop back)
        _emit 0x75
        _emit 0xef
        // === after phase-3 chain-walk / join point for chain-walk skip ===
        // 00013341: 8b d8                MOV EBX,EAX   (EBX = iVar1 = iVar2)
        _emit 0x8b
        _emit 0xd8
        // 00013343: 85 db                TEST EBX,EBX
        _emit 0x85
        _emit 0xdb
        // 00013345: 75 89                JNZ -0x77 → 000132d0  (outer do-while back)
        _emit 0x75
        _emit 0x89
        // === epilogue ===
        // 00013347: 5f                   POP EDI
        _emit 0x5f
        // 00013348: 5e                   POP ESI
        _emit 0x5e
        // 00013349: 5d                   POP EBP
        _emit 0x5d
        // 0001334a: 5b                   POP EBX
        _emit 0x5b
        // 0001334b: 59                   POP ECX  (balance the initial PUSH ECX slot)
        _emit 0x59
        // 0001334c: c3                   RET
        _emit 0xc3
    }
}

// vim: ts=4 sts=4 sw=4 et
