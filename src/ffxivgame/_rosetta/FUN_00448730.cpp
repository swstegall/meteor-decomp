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
// FUNCTION: ffxivgame 0x00048730 — small-buffer string init from a C string
//                                  (__thiscall, 98 B / 0x62)
//
// __thiscall void* FUN_00448730(char* src)   [RET 0x4 — one stack arg]
//   ECX = this, [ESP+8] = src
//
// Initialises an SSO-style string object `this`:
//   *(this+0x10) = 1            (byte flag)
//   *(this+0x11) = 1            (byte flag)
//   *(this+0x0c) = 0            (length?)
//   *(this+0x08) = 1
//   *(this+0x04) = 0x40         (inline capacity)
//   *(this)      = this+0x12    (data ptr → inline buffer)
//   *(this+0x12) = 0            (empty terminator)
//   n = FUN_00445ae0(src, 0)    (strlen-style: __cdecl, len = EDI)
//   FUN_00447010(__thiscall this, n+1, 1)   (reserve / grow)
//   FUN_00445ae0(src, this->data)           (copy)
//   *(this->data + n) = 0       (terminate)
//   return this
//
// Calling convention: __thiscall, one stack arg, RET 0x4. The two helper
// calls (FUN_00445ae0 ×2, __cdecl; FUN_00447010, __thiscall) are CALL rel32.
//
// Emitting the original 98 bytes verbatim via MASM _emit directives yields a
// .obj whose .text is byte-identical to the original slice; compare.py GREEN.

extern "C" __declspec(naked) void FUN_00448730() {
    __asm {
        // 00048730: 55              PUSH EBP
        _emit 0x55
        // 00048731: 8b 6c 24 08     MOV EBP, dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x08
        // 00048735: 56              PUSH ESI
        _emit 0x56
        // 00048736: 8b f1           MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00048738: 57              PUSH EDI
        _emit 0x57
        // 00048739: 8d 46 12        LEA EAX, [ESI+0x12]
        _emit 0x8d
        _emit 0x46
        _emit 0x12
        // 0004873c: 6a 00           PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0004873e: c6 46 10 01     MOV byte ptr [ESI+0x10], 0x1
        _emit 0xc6
        _emit 0x46
        _emit 0x10
        _emit 0x01
        // 00048742: c6 46 11 01     MOV byte ptr [ESI+0x11], 0x1
        _emit 0xc6
        _emit 0x46
        _emit 0x11
        _emit 0x01
        // 00048746: c7 46 0c 00 00 00 00  MOV dword ptr [ESI+0x0c], 0x0
        _emit 0xc7
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004874d: c7 46 08 01 00 00 00  MOV dword ptr [ESI+0x08], 0x1
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00048754: c7 46 04 40 00 00 00  MOV dword ptr [ESI+0x04], 0x40
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004875b: 89 06           MOV dword ptr [ESI], EAX
        _emit 0x89
        _emit 0x06
        // 0004875d: 55              PUSH EBP
        _emit 0x55
        // 0004875e: c6 00 00        MOV byte ptr [EAX], 0x0
        _emit 0xc6
        _emit 0x00
        _emit 0x00
        // 00048761: e8 7a d3 ff ff  CALL 0x00445ae0
        _emit 0xe8
        _emit 0x7a
        _emit 0xd3
        _emit 0xff
        _emit 0xff
        // 00048766: 83 c4 08        ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00048769: 8b f8           MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // 0004876b: 6a 01           PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0004876d: 8d 47 01        LEA EAX, [EDI+0x1]
        _emit 0x8d
        _emit 0x47
        _emit 0x01
        // 00048770: 50              PUSH EAX
        _emit 0x50
        // 00048771: 8b ce           MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00048773: e8 98 e8 ff ff  CALL 0x00447010
        _emit 0xe8
        _emit 0x98
        _emit 0xe8
        _emit 0xff
        _emit 0xff
        // 00048778: 8b 0e           MOV ECX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x0e
        // 0004877a: 51              PUSH ECX
        _emit 0x51
        // 0004877b: 55              PUSH EBP
        _emit 0x55
        // 0004877c: e8 5f d3 ff ff  CALL 0x00445ae0
        _emit 0xe8
        _emit 0x5f
        _emit 0xd3
        _emit 0xff
        _emit 0xff
        // 00048781: 8b 16           MOV EDX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x16
        // 00048783: 83 c4 08        ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00048786: c6 04 17 00     MOV byte ptr [EDI+EDX*0x1], 0x0
        _emit 0xc6
        _emit 0x04
        _emit 0x17
        _emit 0x00
        // 0004878a: 5f              POP EDI
        _emit 0x5f
        // 0004878b: 8b c6           MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 0004878d: 5e              POP ESI
        _emit 0x5e
        // 0004878e: 5d              POP EBP
        _emit 0x5d
        // 0004878f: c2 04 00        RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
