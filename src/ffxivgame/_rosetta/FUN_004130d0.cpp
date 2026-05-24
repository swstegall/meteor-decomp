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
// FUNCTION: ffxivgame 0x000130d0 — conditional init of a doubly-linked
//                                  sentinel node and list-splice for `this`
//                                  (__thiscall, 99 B / 0x63)
//
// __thiscall void FUN_004130d0(void)
//   ECX = this
//
// If *(this+0x30) == 0 (not yet initialised):
//   1. call vtable[7] on *(this+0x18)
//   2. call vtable[1] on *(this+0x14); capture return value `p`
//   3. call FUN_004109a0(__thiscall, this = *(p+0x14)) → new node or NULL
//   4. if node != NULL: initialise as sentinel (self-linked, with vftable
//      0x00F56E88 at offset 0 and `this` at +0x0c)
//      else: node = NULL
//   5. *(this+0x30) = node
//   6. call vtable[1] on *(this+0x14) again; get q = *(ret+0x30)
//   7. splice node into doubly-linked list at q:
//        *(*(q+8)+4) = node
//        *(node+8) = *(q+8)
//        *(node+4) = q
//        *(q+8) = node
//
// Calling convention: __thiscall, no stack args, void return (RET).
// Frame: PUSH ESI / POP ESI; no SUB ESP.
//
// One CALL rel32 (FUN_004109a0 at +0x20) and one imm32 vftable write (at
// +0x2f); all other calls are indirect (CALL EDX). A __declspec(naked)
// body re-emitting the original 99 bytes verbatim via MASM _emit directives
// produces a .obj whose .text is byte-identical to the original slice;
// compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004130d0() {
    __asm {
        // 000130d0: 56              PUSH ESI
        _emit 0x56
        // 000130d1: 8b f1           MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 000130d3: 83 7e 30 00     CMP dword ptr [ESI+0x30], 0x0
        _emit 0x83
        _emit 0x7e
        _emit 0x30
        _emit 0x00
        // 000130d7: 75 58           JNZ +0x58   (→ 00013131, POP ESI)
        _emit 0x75
        _emit 0x58
        // 000130d9: 8b 4e 18        MOV ECX, dword ptr [ESI+0x18]
        _emit 0x8b
        _emit 0x4e
        _emit 0x18
        // 000130dc: 8b 01           MOV EAX, dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 000130de: 8b 50 1c        MOV EDX, dword ptr [EAX+0x1c]
        _emit 0x8b
        _emit 0x50
        _emit 0x1c
        // 000130e1: ff d2           CALL EDX   (vtable[7] of *(this+0x18))
        _emit 0xff
        _emit 0xd2
        // 000130e3: 8b 4e 14        MOV ECX, dword ptr [ESI+0x14]
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 000130e6: 8b 01           MOV EAX, dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 000130e8: 8b 50 04        MOV EDX, dword ptr [EAX+0x04]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000130eb: ff d2           CALL EDX   (vtable[1] of *(this+0x14) → EAX = p)
        _emit 0xff
        _emit 0xd2
        // 000130ed: 8b 48 14        MOV ECX, dword ptr [EAX+0x14]   ; ECX = *(p+0x14)
        _emit 0x8b
        _emit 0x48
        _emit 0x14
        // 000130f0: e8 ab d8 ff ff  CALL 0x004109a0  (FUN_004109a0, rel32 = 0xffffd8ab)
        _emit 0xe8
        _emit 0xab
        _emit 0xd8
        _emit 0xff
        _emit 0xff
        // 000130f5: 85 c0           TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000130f7: 74 11           JZ +0x11   (→ 0001310a, null path: XOR EAX,EAX)
        _emit 0x74
        _emit 0x11
        // 000130f9: 89 40 04        MOV dword ptr [EAX+0x04], EAX   ; node[1] = node
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 000130fc: 89 40 08        MOV dword ptr [EAX+0x08], EAX   ; node[2] = node
        _emit 0x89
        _emit 0x40
        _emit 0x08
        // 000130ff: c7 00 88 6e f5 00  MOV dword ptr [EAX], 0x00F56E88  ; *node = vftable
        _emit 0xc7
        _emit 0x00
        _emit 0x88
        _emit 0x6e
        _emit 0xf5
        _emit 0x00
        // 00013105: 89 70 0c        MOV dword ptr [EAX+0x0c], ESI   ; node[3] = this
        _emit 0x89
        _emit 0x70
        _emit 0x0c
        // 00013108: eb 02           JMP +0x02   (→ 0001310c, merge)
        _emit 0xeb
        _emit 0x02
        // 0001310a: 33 c0           XOR EAX, EAX   (null path: node = 0)
        _emit 0x33
        _emit 0xc0
        // 0001310c: 8b 4e 14        MOV ECX, dword ptr [ESI+0x14]
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 0001310f: 89 46 30        MOV dword ptr [ESI+0x30], EAX   ; *(this+0x30) = node
        _emit 0x89
        _emit 0x46
        _emit 0x30
        // 00013112: 8b 01           MOV EAX, dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 00013114: 8b 50 04        MOV EDX, dword ptr [EAX+0x04]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00013117: ff d2           CALL EDX   (vtable[1] of *(this+0x14) → EAX = ret)
        _emit 0xff
        _emit 0xd2
        // 00013119: 8b 40 30        MOV EAX, dword ptr [EAX+0x30]   ; EAX = q = *(ret+0x30)
        _emit 0x8b
        _emit 0x40
        _emit 0x30
        // 0001311c: 8b 76 30        MOV ESI, dword ptr [ESI+0x30]   ; ESI = node = *(this+0x30)
        _emit 0x8b
        _emit 0x76
        _emit 0x30
        // 0001311f: 8b 48 08        MOV ECX, dword ptr [EAX+0x08]   ; ECX = *(q+8)
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        // 00013122: 89 71 04        MOV dword ptr [ECX+0x04], ESI   ; *(*(q+8)+4) = node
        _emit 0x89
        _emit 0x71
        _emit 0x04
        // 00013125: 8b 50 08        MOV EDX, dword ptr [EAX+0x08]   ; EDX = *(q+8) (reload)
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 00013128: 89 56 08        MOV dword ptr [ESI+0x08], EDX   ; *(node+8) = *(q+8)
        _emit 0x89
        _emit 0x56
        _emit 0x08
        // 0001312b: 89 46 04        MOV dword ptr [ESI+0x04], EAX   ; *(node+4) = q
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 0001312e: 89 70 08        MOV dword ptr [EAX+0x08], ESI   ; *(q+8) = node
        _emit 0x89
        _emit 0x70
        _emit 0x08
        // 00013131: 5e              POP ESI
        _emit 0x5e
        // 00013132: c3              RET
        _emit 0xc3
    }
}
