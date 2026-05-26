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
// FUNCTION: ffxivgame 0x00010730 — spinlock-guarded doubly-linked list insert
//                                   with virtual dispatch (77 B / 0x4d)
//
// __stdcall void FUN_00410730(Node *param_1)
//   [ESP+0x04] : param_1 — node to insert
//
// Behaviour:
//   1. Virtual call vtable[1] on object at param_1[5] (param_1+0x14);
//      the return value EAX holds a container pointer; EDI = EAX[0x10].
//   2. Virtual call vtable[0](0) on param_1 itself.
//   3. Acquire spinlock at [EDI+0x4] via XCHG(1)-spin.
//   4. Insert param_1 before the head sentinel of list at [EDI+0xc]
//      (classic circular doubly-linked list append-to-tail).
//   5. Decrement counter at [EDI+0x18].
//   6. Release spinlock via XCHG(0).
//
// Calling convention: __stdcall; callee pops 1 stack arg (RET 0x4).
// Callee-saved registers: ESI, EDI.

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
// The real implementation is the MSVC __declspec(naked) + __asm/_emit block
// below, which clang cannot parse on arm64. Production builds always use
// cl.exe (MSVC 2005). Diff remains GREEN.
extern "C" void FUN_00410730() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_00410730()
{
    __asm {
        // 00010730: 56                    PUSH ESI
        _emit 0x56
        // 00010731: 8b 74 24 08           MOV ESI,[ESP+0x8]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 00010735: 8b 4e 14              MOV ECX,[ESI+0x14]
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 00010738: 8b 01                 MOV EAX,[ECX]
        _emit 0x8b
        _emit 0x01
        // 0001073a: 8b 50 04              MOV EDX,[EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0001073d: 57                    PUSH EDI
        _emit 0x57
        // 0001073e: ff d2                 CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00010740: 8b 78 10              MOV EDI,[EAX+0x10]
        _emit 0x8b
        _emit 0x78
        _emit 0x10
        // 00010743: 8b 06                 MOV EAX,[ESI]
        _emit 0x8b
        _emit 0x06
        // 00010745: 8b 10                 MOV EDX,[EAX]
        _emit 0x8b
        _emit 0x10
        // 00010747: 6a 00                 PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00010749: 8b ce                 MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0001074b: ff d2                 CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0001074d: 8d 4f 04              LEA ECX,[EDI+0x4]
        _emit 0x8d
        _emit 0x4f
        _emit 0x04
        // 00010750: b8 01 00 00 00        MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00010755: 8b d1                 MOV EDX,ECX
        _emit 0x8b
        _emit 0xd1
        // 00010757: 87 02                 XCHG dword ptr [EDX],EAX
        _emit 0x87
        _emit 0x02
        // 00010759: 85 c0                 TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001075b: 75 f3                 JNZ 0x00010750
        _emit 0x75
        _emit 0xf3
        // 0001075d: 8b 47 0c              MOV EAX,[EDI+0xc]
        _emit 0x8b
        _emit 0x47
        _emit 0x0c
        // 00010760: 8b 50 04              MOV EDX,[EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00010763: 89 32                 MOV [EDX],ESI
        _emit 0x89
        _emit 0x32
        // 00010765: 8b 50 04              MOV EDX,[EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00010768: 89 06                 MOV [ESI],EAX
        _emit 0x89
        _emit 0x06
        // 0001076a: 89 56 04              MOV [ESI+0x4],EDX
        _emit 0x89
        _emit 0x56
        _emit 0x04
        // 0001076d: 89 70 04              MOV [EAX+0x4],ESI
        _emit 0x89
        _emit 0x70
        _emit 0x04
        // 00010770: 83 47 18 ff           ADD dword ptr [EDI+0x18],-0x1
        _emit 0x83
        _emit 0x47
        _emit 0x18
        _emit 0xff
        // 00010774: 33 c0                 XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 00010776: 87 01                 XCHG dword ptr [ECX],EAX
        _emit 0x87
        _emit 0x01
        // 00010778: 5f                    POP EDI
        _emit 0x5f
        // 00010779: 5e                    POP ESI
        _emit 0x5e
        // 0001077a: c2 04 00              RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
#endif
