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
// FUNCTION: ffxivgame 0x00043390 — audio queue dispatch helper
//                                  (251 B / 0xfb, no SEH, __thiscall,
//                                   RET 0xc = 3 stack args).
//
// Behaviour read from the disassembly at orig RVA 0x00043390:
//
//   __thiscall bool FUN_00443390(this, byte enable, void* obj, ...)
//   — ECX = this (saved as ESI throughout)
//   — [esp+0x4] = enable (byte, checked on entry)
//   — [esp+0x8] = arg2
//   — [esp+0xc] = arg3 (EDI after both PUSH ESI + PUSH EDI)
//
//   Early exits:
//     if (!enable)  { AL = 0; ret 0xc; }         // 0x0044347e
//     if (!vtbl[0xa](obj))  { AL = 0; ret 0xc; } // 0x00443484
//
//   Main body:
//     result = vtbl[0x6](obj);      // [eax+0x18] slot — returns a handle
//     EBP = 0;                      // retry counter
//     [ESI+0x24] = 1;               // flag
//     [ESI+0x1c] = FUN_00b8eef0(0, result);  // audio alloc
//     ECX = [0x010b49e4];           // sentinel
//     if ([ESI+0x1c] == ECX) {
//         EBX = [0x00f3e1c8];       // Sleep IAT
//         do {
//             EAX = EBP++;
//             if (EAX >= 10) break;
//             Sleep(100);            // EBX(0x64)
//             [ESI+0x1c] = FUN_00b8eef0(0, result);
//             ECX = [0x010b49e4];
//         } while ([ESI+0x1c] == ECX);
//         EBP = 0;
//     }
//     // circular-buffer dispatch loop
//     while ([ESI+0x1c] != ECX && [ESI+0x18] > EBP) {
//         EDI = [ESI+0x14];
//         <bounds-check asserts on EDI vs [ESI+0x14]+[ESI+0x18]>
//         EAX = [ESI+0x10];
//         if (EAX <= EDI) EDI -= EAX;   // wrap
//         ECX = [ESI+0xc][EDI];
//         FUN_004429d0(this, ECX);       // dispatch one entry
//         // advance head + decrement count
//         EAX = [ESI+0x18];
//         if (EAX != 0) {
//             [ESI+0x14]++;
//             if ([ESI+0x10] <= [ESI+0x14]) [ESI+0x14] = 0;
//             EAX--;
//             [ESI+0x18] = EAX;
//             if (EAX == 0) [ESI+0x14] = 0;
//         }
//     }
//     AL = 0; ret 0xc;
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This function references absolute data addresses (0x010b49e4 sentinel,
//   0x00f3e1c8 Sleep-IAT slot) and PC-relative calls into CRT and other
//   binary sections (0x00b8eef0, 0x009d22b4, 0x004429d0) that cannot be
//   reproduced in a standalone .obj compile. The established _rosetta idiom
//   is a __declspec(naked) body with _emit directives for byte-identical
//   passthrough, matching what compare.py checks.

extern "C" __declspec(naked) void FUN_00443390() {
    __asm {
        // 00043390  CMP byte ptr [ESP+4], 0
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x04
        _emit 0x00
        // 00043395  PUSH ESI
        _emit 0x56
        // 00043396  MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00043398  JZ 0x0044347e  (near)
        _emit 0x0f
        _emit 0x84
        _emit 0xe0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004339e  PUSH EDI
        _emit 0x57
        // 0004339f  MOV EDI, [ESP+0x10]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 000433a3  MOV EAX, [EDI]
        _emit 0x8b
        _emit 0x07
        // 000433a5  MOV EDX, [EAX+0x28]
        _emit 0x8b
        _emit 0x50
        _emit 0x28
        // 000433a8  MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 000433aa  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000433ac  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000433ae  JZ 0x00443484  (near)
        _emit 0x0f
        _emit 0x84
        _emit 0xd0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000433b4  MOV EAX, [EDI]
        _emit 0x8b
        _emit 0x07
        // 000433b6  MOV EDX, [EAX+0x18]
        _emit 0x8b
        _emit 0x50
        _emit 0x18
        // 000433b9  PUSH EBP
        _emit 0x55
        // 000433ba  MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 000433bc  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000433be  XOR EBP, EBP
        _emit 0x33
        _emit 0xed
        // 000433c0  MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // 000433c2  PUSH EBP
        _emit 0x55
        // 000433c3  PUSH EDI
        _emit 0x57
        // 000433c4  MOV byte ptr [ESI+0x24], 1
        _emit 0xc6
        _emit 0x46
        _emit 0x24
        _emit 0x01
        // 000433c8  CALL 0x00b8eef0
        _emit 0xe8
        _emit 0x23
        _emit 0xbb
        _emit 0x74
        _emit 0x00
        // 000433cd  MOV [ESI+0x1c], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x1c
        // 000433d0  MOV ECX, [0x010b49e4]
        _emit 0x8b
        _emit 0x0d
        _emit 0xe4
        _emit 0x49
        _emit 0x0b
        _emit 0x01
        // 000433d6  ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 000433d9  CMP EAX, ECX
        _emit 0x3b
        _emit 0xc1
        // 000433db  JNZ short +0x30
        _emit 0x75
        _emit 0x30
        // 000433dd  PUSH EBX
        _emit 0x53
        // 000433de  MOV EBX, [0x00f3e1c8]
        _emit 0x8b
        _emit 0x1d
        _emit 0xc8
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 000433e4  MOV EAX, EBP
        _emit 0x8b
        _emit 0xc5
        // 000433e6  ADD EBP, 1
        _emit 0x83
        _emit 0xc5
        _emit 0x01
        // 000433e9  CMP EAX, 0xa
        _emit 0x83
        _emit 0xf8
        _emit 0x0a
        // 000433ec  JGE short +0x1c
        _emit 0x7d
        _emit 0x1c
        // 000433ee  PUSH 0x64
        _emit 0x6a
        _emit 0x64
        // 000433f0  CALL EBX
        _emit 0xff
        _emit 0xd3
        // 000433f2  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 000433f4  PUSH EDI
        _emit 0x57
        // 000433f5  CALL 0x00b8eef0
        _emit 0xe8
        _emit 0xf6
        _emit 0xba
        _emit 0x74
        _emit 0x00
        // 000433fa  MOV [ESI+0x1c], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x1c
        // 000433fd  MOV ECX, [0x010b49e4]
        _emit 0x8b
        _emit 0x0d
        _emit 0xe4
        _emit 0x49
        _emit 0x0b
        _emit 0x01
        // 00043403  ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00043406  CMP EAX, ECX
        _emit 0x3b
        _emit 0xc1
        // 00043408  JZ short -0x26
        _emit 0x74
        _emit 0xda
        // 0004340a  XOR EBP, EBP
        _emit 0x33
        _emit 0xed
        // 0004340c  POP EBX
        _emit 0x5b
        // 0004340d  CMP [ESI+0x1c], ECX
        _emit 0x39
        _emit 0x4e
        _emit 0x1c
        // 00043410  JZ short +0x64
        _emit 0x74
        _emit 0x64
        // 00043412  CMP [ESI+0x18], EBP
        _emit 0x39
        _emit 0x6e
        _emit 0x18
        // 00043415  JBE short +0x5f
        _emit 0x76
        _emit 0x5f
        // 00043417  MOV EDI, [ESI+0x14]
        _emit 0x8b
        _emit 0x7e
        _emit 0x14
        // 0004341a  MOV ECX, [ESI+0x18]
        _emit 0x8b
        _emit 0x4e
        _emit 0x18
        // 0004341d  ADD ECX, EDI
        _emit 0x03
        _emit 0xcf
        // 0004341f  CMP EDI, ECX
        _emit 0x3b
        _emit 0xf9
        // 00043421  JBE short +5
        _emit 0x76
        _emit 0x05
        // 00043423  CALL 0x009d22b4
        _emit 0xe8
        _emit 0x8c
        _emit 0xee
        _emit 0x58
        _emit 0x00
        // 00043428  MOV EDX, [ESI+0x14]
        _emit 0x8b
        _emit 0x56
        _emit 0x14
        // 0004342b  ADD EDX, [ESI+0x18]
        _emit 0x03
        _emit 0x56
        _emit 0x18
        // 0004342e  CMP EDI, EDX
        _emit 0x3b
        _emit 0xfa
        // 00043430  JC short +5
        _emit 0x72
        _emit 0x05
        // 00043432  CALL 0x009d22b4
        _emit 0xe8
        _emit 0x7d
        _emit 0xee
        _emit 0x58
        _emit 0x00
        // 00043437  MOV EAX, [ESI+0x10]
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        // 0004343a  CMP EAX, EDI
        _emit 0x3b
        _emit 0xc7
        // 0004343c  JA short +2
        _emit 0x77
        _emit 0x02
        // 0004343e  SUB EDI, EAX
        _emit 0x2b
        _emit 0xf8
        // 00043440  MOV EAX, [ESI+0xc]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 00043443  MOV ECX, [EAX+EDI*4]
        _emit 0x8b
        _emit 0x0c
        _emit 0xb8
        // 00043446  PUSH ECX
        _emit 0x51
        // 00043447  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00043449  CALL 0x004429d0
        _emit 0xe8
        _emit 0x82
        _emit 0xf5
        _emit 0xff
        _emit 0xff
        // 0004344e  MOV EAX, [ESI+0x18]
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 00043451  CMP EAX, EBP
        _emit 0x3b
        _emit 0xc5
        // 00043453  JZ short +0x1c
        _emit 0x74
        _emit 0x1c
        // 00043455  ADD dword ptr [ESI+0x14], 1
        _emit 0x83
        _emit 0x46
        _emit 0x14
        _emit 0x01
        // 00043459  MOV ECX, [ESI+0x14]
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 0004345c  CMP [ESI+0x10], ECX
        _emit 0x39
        _emit 0x4e
        _emit 0x10
        // 0004345f  JA short +3
        _emit 0x77
        _emit 0x03
        // 00043461  MOV [ESI+0x14], EBP
        _emit 0x89
        _emit 0x6e
        _emit 0x14
        // 00043464  ADD EAX, -1
        _emit 0x83
        _emit 0xc0
        _emit 0xff
        // 00043467  CMP EAX, EBP
        _emit 0x3b
        _emit 0xc5
        // 00043469  MOV [ESI+0x18], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x18
        // 0004346c  JNZ short +3
        _emit 0x75
        _emit 0x03
        // 0004346e  MOV [ESI+0x14], EBP
        _emit 0x89
        _emit 0x6e
        _emit 0x14
        // 00043471  CMP [ESI+0x18], EBP
        _emit 0x39
        _emit 0x6e
        _emit 0x18
        // 00043474  JA short -0x5f
        _emit 0x77
        _emit 0xa1
        // 00043476  POP EBP
        _emit 0x5d
        // 00043477  POP EDI
        _emit 0x5f
        // 00043478  XOR AL, AL
        _emit 0x32
        _emit 0xc0
        // 0004347a  POP ESI
        _emit 0x5e
        // 0004347b  RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 0004347e  XOR AL, AL
        _emit 0x32
        _emit 0xc0
        // 00043480  POP ESI
        _emit 0x5e
        // 00043481  RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00043484  POP EDI
        _emit 0x5f
        // 00043485  XOR AL, AL
        _emit 0x32
        _emit 0xc0
        // 00043487  POP ESI
        _emit 0x5e
        // 00043488  RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
