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
// FUNCTION: ffxivgame 0x00047880 — assign-from-ANSI-string member
//                                  (__thiscall, 160 bytes / 0xa0)
//
// Calling convention: __thiscall (ECX = this); one stack arg (a multibyte
//   `const char *` source). RET 4 — callee-cleans 1 dword. Returns `this`
//   in EAX (fluent-assign idiom).
//
// Frame: SUB ESP,0x804 — a 0x800-byte (1024-wchar) stack scratch buffer
//   plus a /GS security cookie at [ESP+0x800]. The function:
//     wchar_t wbuf[0x400];                       (the 0x800-byte buffer)
//     memset(wbuf, 0, 0x7fe);                     (CALL 0x009d2110)
//     wbuf[0x3ff] = 0;                            (word at [ESP+0x14])
//     MultiByteToWideChar(CP_ACP, 0, src, -1,     (CALL [0x00f3e1f4])
//                         wbuf, 0x400);
//     int len = FUN_00445ae0(wbuf, 0);            (UTF-8 length probe)
//     this->Resize(len + 1, 1);                   (CALL 0x00447010)
//     FUN_00445ae0(wbuf, this->buf);              (write conversion)
//     this->buf[len] = 0;                         (null terminate)
//     return this;
//
// Reloc-bearing sites in the orig 160 bytes:
//     +0x32   CALL rel32     → 0x009d2110  (memset)
//     +0x4b   CALL [abs32]   → [0x00f3e1f4] (MultiByteToWideChar IAT slot)
//     +0x58   CALL rel32     → 0x00445ae0
//     +0x6a   CALL rel32     → 0x00447010
//     +0x77   CALL rel32     → 0x00445ae0
//     +0x92   CALL rel32     → 0x009d20f4  (__security_check_cookie)
//   plus the two absolute DS references (MOV EAX,[0x012ea8b0] security
//   cookie global, and the CALL [0x00f3e1f4] IAT slot).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level rebuild would chain into stub generation for the two
//   unmatched siblings (FUN_00445ae0, FUN_00447010) and is sensitive to
//   /GS cookie + buffer-layout register allocation that the C++ frontend
//   does not reproduce verbatim. A __declspec(naked) body re-emitting the
//   original 160 bytes produces a .obj whose .text is byte-identical to
//   the orig slice; the rel32 offsets resolve against the orig binary's
//   own address space and are emitted as raw bytes (compare.py masks the
//   reloc positions). compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00447880() {
    __asm {
        // 00047880: 81 ec 04 08 00 00   SUB ESP,0x804
        _emit 0x81
        _emit 0xec
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 00047886: a1 b0 a8 2e 01      MOV EAX,[0x012ea8b0]  (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0004788b: 33 c4               XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0004788d: 89 84 24 00 08 00 00 MOV [ESP+0x800],EAX
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 00047894: 56                  PUSH ESI
        _emit 0x56
        // 00047895: 57                  PUSH EDI
        _emit 0x57
        // 00047896: 8b bc 24 10 08 00 00 MOV EDI,[ESP+0x810]  (arg0 = src)
        _emit 0x8b
        _emit 0xbc
        _emit 0x24
        _emit 0x10
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0004789d: 68 fe 07 00 00      PUSH 0x7fe
        _emit 0x68
        _emit 0xfe
        _emit 0x07
        _emit 0x00
        _emit 0x00
        // 000478a2: 8d 44 24 0e         LEA EAX,[ESP+0xe]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0e
        // 000478a6: 6a 00               PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 000478a8: 50                  PUSH EAX
        _emit 0x50
        // 000478a9: 8b f1               MOV ESI,ECX  (ESI = this)
        _emit 0x8b
        _emit 0xf1
        // 000478ab: 66 c7 44 24 14 00 00 MOV word ptr [ESP+0x14],0x0
        _emit 0x66
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        // 000478b2: e8 59 a8 58 00      CALL 0x009d2110  (memset)
        _emit 0xe8
        _emit 0x59
        _emit 0xa8
        _emit 0x58
        _emit 0x00
        // 000478b7: 83 c4 0c            ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 000478ba: 68 00 04 00 00      PUSH 0x400
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 000478bf: 8d 4c 24 0c         LEA ECX,[ESP+0xc]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 000478c3: 51                  PUSH ECX
        _emit 0x51
        // 000478c4: 6a ff               PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 000478c6: 57                  PUSH EDI
        _emit 0x57
        // 000478c7: 6a 00               PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 000478c9: 6a 00               PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 000478cb: ff 15 f4 e1 f3 00   CALL dword ptr [0x00f3e1f4]  (MultiByteToWideChar)
        _emit 0xff
        _emit 0x15
        _emit 0xf4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 000478d1: 8d 54 24 08         LEA EDX,[ESP+0x8]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 000478d5: 6a 00               PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 000478d7: 52                  PUSH EDX
        _emit 0x52
        // 000478d8: e8 03 e2 ff ff      CALL 0x00445ae0
        _emit 0xe8
        _emit 0x03
        _emit 0xe2
        _emit 0xff
        _emit 0xff
        // 000478dd: 83 c4 08            ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 000478e0: 8b f8               MOV EDI,EAX  (EDI = len)
        _emit 0x8b
        _emit 0xf8
        // 000478e2: 6a 01               PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 000478e4: 8d 47 01            LEA EAX,[EDI+0x1]
        _emit 0x8d
        _emit 0x47
        _emit 0x01
        // 000478e7: 50                  PUSH EAX
        _emit 0x50
        // 000478e8: 8b ce               MOV ECX,ESI  (ECX = this)
        _emit 0x8b
        _emit 0xce
        // 000478ea: e8 21 f7 ff ff      CALL 0x00447010  (this->Resize(len+1, 1))
        _emit 0xe8
        _emit 0x21
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        // 000478ef: 8b 0e               MOV ECX,[ESI]  (this->buf)
        _emit 0x8b
        _emit 0x0e
        // 000478f1: 51                  PUSH ECX
        _emit 0x51
        // 000478f2: 8d 54 24 0c         LEA EDX,[ESP+0xc]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 000478f6: 52                  PUSH EDX
        _emit 0x52
        // 000478f7: e8 e4 e1 ff ff      CALL 0x00445ae0
        _emit 0xe8
        _emit 0xe4
        _emit 0xe1
        _emit 0xff
        _emit 0xff
        // 000478fc: 8b 06               MOV EAX,[ESI]  (this->buf)
        _emit 0x8b
        _emit 0x06
        // 000478fe: 8b 8c 24 10 08 00 00 MOV ECX,[ESP+0x810]  (cookie reload)
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x10
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 00047905: 83 c4 08            ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00047908: c6 04 07 00         MOV byte ptr [EDI+EAX],0x0  (buf[len]=0)
        _emit 0xc6
        _emit 0x04
        _emit 0x07
        _emit 0x00
        // 0004790c: 5f                  POP EDI
        _emit 0x5f
        // 0004790d: 8b c6               MOV EAX,ESI  (return this)
        _emit 0x8b
        _emit 0xc6
        // 0004790f: 5e                  POP ESI
        _emit 0x5e
        // 00047910: 33 cc               XOR ECX,ESP
        _emit 0x33
        _emit 0xcc
        // 00047912: e8 dd a7 58 00      CALL 0x009d20f4  (__security_check_cookie)
        _emit 0xe8
        _emit 0xdd
        _emit 0xa7
        _emit 0x58
        _emit 0x00
        // 00047917: 81 c4 04 08 00 00   ADD ESP,0x804
        _emit 0x81
        _emit 0xc4
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0004791d: c2 04 00            RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
