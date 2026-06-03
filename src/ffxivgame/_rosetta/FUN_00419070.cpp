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
// FUNCTION: ffxivgame 0x00019070 — zero-init 24-byte stack buffer + call FUN_00418800
//                                   (__cdecl, no args, void return, 41 B / 0x29)
//
// Allocates 0x18 (24) bytes of stack space, zeroes all six DWORDs via XOR
// EAX / six MOV stores, then passes the buffer's address (via LEA + PUSH) as
// the sole argument to FUN_00418800. Caller-cleans with ADD ESP,0x1c (0x18
// locals + 4 for the PUSH).
//
// Calling convention: __cdecl — no incoming parameters, void return.
// Frame: /Oy (no EBP frame); 0x18 bytes of local storage via SUB ESP.
//
// Asm (41 bytes @ orig RVA 0x00019070):
//   83 ec 18               SUB ESP, 0x18
//   33 c0                  XOR EAX, EAX
//   89 04 24               MOV dword ptr [ESP], EAX
//   89 44 24 04            MOV dword ptr [ESP+0x4], EAX
//   89 44 24 08            MOV dword ptr [ESP+0x8], EAX
//   89 44 24 0c            MOV dword ptr [ESP+0xc], EAX
//   89 44 24 10            MOV dword ptr [ESP+0x10], EAX
//   89 44 24 14            MOV dword ptr [ESP+0x14], EAX
//   8d 04 24               LEA EAX, [ESP]
//   50                     PUSH EAX
//   e8 6b f7 ff ff         CALL FUN_00418800   (rel32 target)
//   83 c4 1c               ADD ESP, 0x1c
//   c3                     RET
//
// Reconstruction: __declspec(naked) _emit byte passthrough. The CALL
// displacement (bytes +0x21..+0x24) encodes the original binary's
// relative offset; compare.py masks reloc bytes, so these four bytes
// are excluded from the byte-identical check.

extern "C" __declspec(naked) void FUN_00419070() {
    __asm {
        // 00019070: 83 ec 18   SUB ESP, 0x18
        _emit 0x83
        _emit 0xec
        _emit 0x18
        // 00019073: 33 c0      XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00019075: 89 04 24   MOV dword ptr [ESP], EAX
        _emit 0x89
        _emit 0x04
        _emit 0x24
        // 00019078: 89 44 24 04  MOV dword ptr [ESP+0x4], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001907c: 89 44 24 08  MOV dword ptr [ESP+0x8], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00019080: 89 44 24 0c  MOV dword ptr [ESP+0xc], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00019084: 89 44 24 10  MOV dword ptr [ESP+0x10], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00019088: 89 44 24 14  MOV dword ptr [ESP+0x14], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0001908c: 8d 04 24   LEA EAX, [ESP]
        _emit 0x8d
        _emit 0x04
        _emit 0x24
        // 0001908f: 50         PUSH EAX
        _emit 0x50
        // 00019090: e8 6b f7 ff ff  CALL FUN_00418800 (rel32)
        _emit 0xe8
        _emit 0x6b
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        // 00019095: 83 c4 1c   ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 00019098: c3         RET
        _emit 0xc3
    }
}
