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
// FUNCTION: ffxivgame 0x00047bc0 — UTF-8 "head substring up to char boundary"
//                                  helper (__thiscall, 145 bytes / 0x91,
//                                  ret 8)
//
// Calling convention: __thiscall (ECX = this). Two stack args, cleaned on
// return (`ret 8`):
//   arg0 [ESP+0x14] — pointer to the return string object (NRVO slot)
//   arg1 [ESP+0x18] — requested byte position `pos`
// Returns the return-string pointer in EAX (the NRVO slot).
//
// Object layout inferred:
//   [this + 0x00]  char *buf  — backing UTF-8 byte buffer
//   [this + 0x08]  size_t len — buffer length in bytes
//
// Behaviour (recovered from asm @ 0x00047bc0):
//   - Clamp `pos` into [0, len-1]: if pos == (size_t)-1 or pos >= len,
//     pos = len - 1.
//   - If the clamped end is <= 0, return an empty string (assigned from
//     the static empty-string at 0x1266b10 via FUN_00447200).
//   - Otherwise walk backwards over UTF-8 continuation bytes (top bits
//     == 0x80) from buf[pos-1] to find the start of the multibyte
//     sequence, then call FUN_00445850 (sequence byte-length) to decide
//     whether the boundary lands exactly on `pos`; construct the head
//     substring of the resulting length via FUN_00447260.
//
// CALL targets (all REL32; compare.py masks the rel32 windows):
//   +0x31  CALL FUN_00447200  — assign-empty-string (this, src=0x1266b10)
//   +0x69  CALL FUN_00445850  — UTF-8 sequence length at ptr
//   +0x83  CALL FUN_00447260  — substring constructor (this, buf, count)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The C++ source for this UTF-8 boundary walk would not naturally
//   reproduce MSVC 2005's exact register allocation / branch layout
//   (the backward continuation-byte loop reuses ESI for both the index
//   and the running pointer, and the NRVO slot is re-derived from the
//   stack rather than carried). The rosetta naked-asm path emits the
//   original 145 bytes verbatim; compare.py masks the three rel32 call
//   windows and the absolute-address push, reporting GREEN.

extern "C" __declspec(naked) void FUN_00447bc0() {
    __asm {
        // 00047bc0:  51                 PUSH ECX
        _emit 0x51
        // 00047bc1:  53                 PUSH EBX
        _emit 0x53
        // 00047bc2:  56                 PUSH ESI
        _emit 0x56
        // 00047bc3:  57                 PUSH EDI
        _emit 0x57
        // 00047bc4:  8b 7c 24 18        MOV EDI,[ESP+0x18]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        // 00047bc8:  83 ff ff           CMP EDI,-1
        _emit 0x83
        _emit 0xff
        _emit 0xff
        // 00047bcb:  8b d9              MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 00047bcd:  c7 44 24 0c 00 00 00 00   MOV [ESP+0xc],0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00047bd5:  74 05              JZ 0x00047bdc
        _emit 0x74
        _emit 0x05
        // 00047bd7:  3b 7b 08           CMP EDI,[EBX+0x8]
        _emit 0x3b
        _emit 0x7b
        _emit 0x08
        // 00047bda:  72 06              JC 0x00047be2
        _emit 0x72
        _emit 0x06
        // 00047bdc:  8b 7b 08           MOV EDI,[EBX+0x8]
        _emit 0x8b
        _emit 0x7b
        _emit 0x08
        // 00047bdf:  83 ef 01           SUB EDI,0x1
        _emit 0x83
        _emit 0xef
        _emit 0x01
        // 00047be2:  85 ff              TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 00047be4:  77 19              JA 0x00047bff
        _emit 0x77
        _emit 0x19
        // 00047be6:  8b 74 24 14        MOV ESI,[ESP+0x14]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 00047bea:  68 10 6b 26 01     PUSH 0x1266b10
        _emit 0x68
        _emit 0x10
        _emit 0x6b
        _emit 0x26
        _emit 0x01
        // 00047bef:  8b ce              MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00047bf1:  e8 0a f6 ff ff     CALL 0x00447200
        _emit 0xe8
        _emit 0x0a
        _emit 0xf6
        _emit 0xff
        _emit 0xff
        // 00047bf6:  5f                 POP EDI
        _emit 0x5f
        // 00047bf7:  8b c6              MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 00047bf9:  5e                 POP ESI
        _emit 0x5e
        // 00047bfa:  5b                 POP EBX
        _emit 0x5b
        // 00047bfb:  59                 POP ECX
        _emit 0x59
        // 00047bfc:  c2 08 00           RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // 00047bff:  8b 03              MOV EAX,[EBX]
        _emit 0x8b
        _emit 0x03
        // 00047c01:  8d 77 ff           LEA ESI,[EDI-0x1]
        _emit 0x8d
        _emit 0x77
        _emit 0xff
        // 00047c04:  8a 0c 30           MOV CL,[EAX+ESI]
        _emit 0x8a
        _emit 0x0c
        _emit 0x30
        // 00047c07:  03 c6              ADD EAX,ESI
        _emit 0x03
        _emit 0xc6
        // 00047c09:  80 e1 c0           AND CL,0xc0
        _emit 0x80
        _emit 0xe1
        _emit 0xc0
        // 00047c0c:  80 f9 80           CMP CL,0x80
        _emit 0x80
        _emit 0xf9
        _emit 0x80
        // 00047c0f:  75 15              JNZ 0x00047c26
        _emit 0x75
        _emit 0x15
        // 00047c11:  85 f6              TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 00047c13:  76 d1              JBE 0x00047be6
        _emit 0x76
        _emit 0xd1
        // 00047c15:  8a 50 ff           MOV DL,[EAX-0x1]
        _emit 0x8a
        _emit 0x50
        _emit 0xff
        // 00047c18:  83 e8 01           SUB EAX,0x1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // 00047c1b:  80 e2 c0           AND DL,0xc0
        _emit 0x80
        _emit 0xe2
        _emit 0xc0
        // 00047c1e:  83 ee 01           SUB ESI,0x1
        _emit 0x83
        _emit 0xee
        _emit 0x01
        // 00047c21:  80 fa 80           CMP DL,0x80
        _emit 0x80
        _emit 0xfa
        _emit 0x80
        // 00047c24:  74 eb              JZ 0x00047c11
        _emit 0x74
        _emit 0xeb
        // 00047c26:  6a 00              PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00047c28:  50                 PUSH EAX
        _emit 0x50
        // 00047c29:  e8 22 dc ff ff     CALL 0x00445850
        _emit 0xe8
        _emit 0x22
        _emit 0xdc
        _emit 0xff
        _emit 0xff
        // 00047c2e:  03 c6              ADD EAX,ESI
        _emit 0x03
        _emit 0xc6
        // 00047c30:  83 c4 08           ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00047c33:  3b c7              CMP EAX,EDI
        _emit 0x3b
        _emit 0xc7
        // 00047c35:  75 02              JNZ 0x00047c39
        _emit 0x75
        _emit 0x02
        // 00047c37:  8b f7              MOV ESI,EDI
        _emit 0x8b
        _emit 0xf7
        // 00047c39:  8b 03              MOV EAX,[EBX]
        _emit 0x8b
        _emit 0x03
        // 00047c3b:  56                 PUSH ESI
        _emit 0x56
        // 00047c3c:  8b 74 24 18        MOV ESI,[ESP+0x18]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x18
        // 00047c40:  50                 PUSH EAX
        _emit 0x50
        // 00047c41:  8b ce              MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00047c43:  e8 18 f6 ff ff     CALL 0x00447260
        _emit 0xe8
        _emit 0x18
        _emit 0xf6
        _emit 0xff
        _emit 0xff
        // 00047c48:  5f                 POP EDI
        _emit 0x5f
        // 00047c49:  8b c6              MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 00047c4b:  5e                 POP ESI
        _emit 0x5e
        // 00047c4c:  5b                 POP EBX
        _emit 0x5b
        // 00047c4d:  59                 POP ECX
        _emit 0x59
        // 00047c4e:  c2 08 00           RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
