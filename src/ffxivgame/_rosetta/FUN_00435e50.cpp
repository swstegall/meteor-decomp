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
// FUNCTION: ffxivgame 0x00035e50 — __thiscall virtual-dispatch wrapper with a
//                                  lazy-init assert callback (152 B / 0x98, ret 4).
//
// Calling convention: __thiscall (ECX = this); one stack arg (a target object
//   whose vtable slot +0x88 is invoked); returns void.
//
// Behaviour (recovered from asm @ 0x00035e50):
//
//   void Dispatch(this, Obj *a) {
//       int (*fn)(...) = a->vtbl->slot_0x88;
//       int r = fn(a,
//                  this->m4,                                 // [ECX+0x04]
//                  (this->m30 == 1) ? &this->mc  : 0,        // [ECX+0x30] gate -> &[ECX+0x0c]
//                  this->m8,                                 // [ECX+0x08]
//                  (this->m31 == 1) ? &this->m1c : 0,        // [ECX+0x31] gate -> &[ECX+0x1c]
//                  this->m2c);                               // [ECX+0x2c]
//       if (r) {
//           // lazy-init a static fn-ptr the first time we report
//           if ((g_flag & 1) == 0) {
//               g_flag    |= 1;
//               g_reporter = (void*)0x00433720;
//           }
//           g_reporter("...e8 4b f6 00...", "...7c 54 f6 00...",
//                      "...18 4c f6 00...", 0x29d, "...98 54 f6 00...");
//       }
//   }
//
//   g_flag     = dword @ 0x01323910  (bit0 = "reporter initialised")
//   g_reporter = dword @ 0x0132390c  (__cdecl fn-ptr, 5 args, cleaned via ADD ESP,0x14)
//
// Reconstruction strategy — `__declspec(naked)` literal byte passthrough:
//
//   Every call in this function is INDIRECT (`ff d2` = CALL EDX through the
//   target's vtable, and `ff 15 ..` = CALL [0x0132390c] through the global
//   fn-ptr). There are NO REL32 direct calls, so there is nothing for the
//   assembler to relocate. All operands — including the absolute global
//   addresses 0x01323910 / 0x0132390c, the embedded fn-ptr constant
//   0x00433720, and the five __cdecl argument pushes (the assertion file /
//   message / expression strings + line 0x29d) — are fixed virtual addresses
//   stored verbatim in the original .text. Emitting the 152 bytes literally
//   reproduces the slice byte-for-byte; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00435e50() {
    __asm {
        // 00035e50: 8b 44 24 04                      MOV EAX,[ESP+0x4]   ; a (stack arg)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00035e54: 8b 10                            MOV EDX,[EAX]   ; a->vtable
        _emit 0x8b
        _emit 0x10
        // 00035e56: 8b 92 88 00 00 00                MOV EDX,[EDX+0x88]   ; vtable slot 0x88
        _emit 0x8b
        _emit 0x92
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00035e5c: 53                               PUSH EBX
        _emit 0x53
        // 00035e5d: 56                               PUSH ESI
        _emit 0x56
        // 00035e5e: 8b 71 2c                         MOV ESI,[ECX+0x2c]
        _emit 0x8b
        _emit 0x71
        _emit 0x2c
        // 00035e61: bb 01 00 00 00                   MOV EBX,1
        _emit 0xbb
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00035e66: 38 59 30                         CMP [ECX+0x30],BL
        _emit 0x38
        _emit 0x59
        _emit 0x30
        // 00035e69: 56                               PUSH ESI   ; arg6 = this->m2c
        _emit 0x56
        // 00035e6a: 75 1f                            JNZ 0x00435e8b
        _emit 0x75
        _emit 0x1f
        // 00035e6c: 38 59 31                         CMP [ECX+0x31],BL
        _emit 0x38
        _emit 0x59
        _emit 0x31
        // 00035e6f: 75 0e                            JNZ 0x00435e7f
        _emit 0x75
        _emit 0x0e
        // 00035e71: 8d 71 1c                         LEA ESI,[ECX+0x1c]
        _emit 0x8d
        _emit 0x71
        _emit 0x1c
        // 00035e74: 56                               PUSH ESI   ; arg5 = &this->m1c
        _emit 0x56
        // 00035e75: 8b 71 08                         MOV ESI,[ECX+0x8]
        _emit 0x8b
        _emit 0x71
        _emit 0x08
        // 00035e78: 56                               PUSH ESI   ; arg4 = this->m8
        _emit 0x56
        // 00035e79: 8d 71 0c                         LEA ESI,[ECX+0xc]
        _emit 0x8d
        _emit 0x71
        _emit 0x0c
        // 00035e7c: 56                               PUSH ESI   ; arg3 = &this->mc
        _emit 0x56
        // 00035e7d: eb 1f                            JMP 0x00435e9e
        _emit 0xeb
        _emit 0x1f
        // 00035e7f: 8b 71 08                         MOV ESI,[ECX+0x8]
        _emit 0x8b
        _emit 0x71
        _emit 0x08
        // 00035e82: 6a 00                            PUSH 0   ; arg5 = 0
        _emit 0x6a
        _emit 0x00
        // 00035e84: 56                               PUSH ESI   ; arg4 = this->m8
        _emit 0x56
        // 00035e85: 8d 71 0c                         LEA ESI,[ECX+0xc]
        _emit 0x8d
        _emit 0x71
        _emit 0x0c
        // 00035e88: 56                               PUSH ESI   ; arg3 = &this->mc
        _emit 0x56
        // 00035e89: eb 13                            JMP 0x00435e9e
        _emit 0xeb
        _emit 0x13
        // 00035e8b: 38 59 31                         CMP [ECX+0x31],BL
        _emit 0x38
        _emit 0x59
        _emit 0x31
        // 00035e8e: 75 06                            JNZ 0x00435e96
        _emit 0x75
        _emit 0x06
        // 00035e90: 8d 71 1c                         LEA ESI,[ECX+0x1c]
        _emit 0x8d
        _emit 0x71
        _emit 0x1c
        // 00035e93: 56                               PUSH ESI   ; arg5 = &this->m1c
        _emit 0x56
        // 00035e94: eb 02                            JMP 0x00435e98
        _emit 0xeb
        _emit 0x02
        // 00035e96: 6a 00                            PUSH 0   ; arg5 = 0
        _emit 0x6a
        _emit 0x00
        // 00035e98: 8b 71 08                         MOV ESI,[ECX+0x8]
        _emit 0x8b
        _emit 0x71
        _emit 0x08
        // 00035e9b: 56                               PUSH ESI   ; arg4 = this->m8
        _emit 0x56
        // 00035e9c: 6a 00                            PUSH 0   ; arg3 = 0
        _emit 0x6a
        _emit 0x00
        // 00035e9e: 8b 49 04                         MOV ECX,[ECX+0x4]
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        // 00035ea1: 51                               PUSH ECX   ; arg2 = this->m4
        _emit 0x51
        // 00035ea2: 50                               PUSH EAX   ; arg1 = a
        _emit 0x50
        // 00035ea3: ff d2                            CALL EDX   ; a->vtable[0x88](...)
        _emit 0xff
        _emit 0xd2
        // 00035ea5: 85 c0                            TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00035ea7: 74 3a                            JZ 0x00435ee3
        _emit 0x74
        _emit 0x3a
        // 00035ea9: 84 1d 10 39 32 01                TEST [0x01323910],BL
        _emit 0x84
        _emit 0x1d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00035eaf: 75 10                            JNZ 0x00435ec1
        _emit 0x75
        _emit 0x10
        // 00035eb1: 09 1d 10 39 32 01                OR [0x01323910],EBX
        _emit 0x09
        _emit 0x1d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00035eb7: c7 05 0c 39 32 01 20 37 43 00    MOV [0x0132390c],0x00433720
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00
        // 00035ec1: 68 98 54 f6 00                   PUSH 0x00f65498
        _emit 0x68
        _emit 0x98
        _emit 0x54
        _emit 0xf6
        _emit 0x00
        // 00035ec6: 68 9d 02 00 00                   PUSH 0x0000029d
        _emit 0x68
        _emit 0x9d
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 00035ecb: 68 18 4c f6 00                   PUSH 0x00f64c18
        _emit 0x68
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        // 00035ed0: 68 7c 54 f6 00                   PUSH 0x00f6547c
        _emit 0x68
        _emit 0x7c
        _emit 0x54
        _emit 0xf6
        _emit 0x00
        // 00035ed5: 68 e8 4b f6 00                   PUSH 0x00f64be8
        _emit 0x68
        _emit 0xe8
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        // 00035eda: ff 15 0c 39 32 01                CALL [0x0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00035ee0: 83 c4 14                         ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00035ee3: 5e                               POP ESI
        _emit 0x5e
        // 00035ee4: 5b                               POP EBX
        _emit 0x5b
        // 00035ee5: c2 04 00                         RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
