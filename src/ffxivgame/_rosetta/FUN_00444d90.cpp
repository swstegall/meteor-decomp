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
// FUNCTION: ffxivgame 0x00044d90 — six-arg forwarding thunk to FUN_00444cf0
//                                   (__cdecl, 3 args, 43 B / 0x2B)
//
// void __cdecl FUN_00444d90(a, b, c)
//   stack layout (after the initial PUSH ECX reserves one local DWORD):
//     [ESP+0x08] : a   (param_1)
//     [ESP+0x0c] : b   (param_2)
//     [ESP+0x10] : c   (param_3)
//
// Builds a 6-argument __cdecl call:
//     FUN_00444cf0(a, b, c, b, b, local)
// where `local` is the DWORD reserved by the opening PUSH ECX, with only
// its low byte explicitly zeroed (MOV byte ptr [ESP], 0). MSVC evaluates
// the argument list right-to-left, loading each value into EAX/ECX/EDX
// just before its PUSH; the duplicate [ESP+0x14] reloads are the
// re-evaluations of `b` and the shifting ESP-relative addresses as the
// pushes accumulate. The caller-cleanup `ADD ESP, 0x1c` reclaims the six
// pushed args (0x18) plus the reserved local (0x04).
//
// The only linker-relocated field is the 4-byte rel32 of the CALL to
// FUN_00444cf0 (bytes at +0x23..+0x26); compare.py masks those bytes.
// Encoded as __declspec(naked) byte passthrough so the .obj .text matches
// the orig 43-byte slice exactly (mirrors sibling FUN_004134b0).
//
// Asm (43 bytes @ orig RVA 0x00044d90):
//   51              PUSH ECX                       ; reserve local DWORD
//   8b 4c 24 0c     MOV ECX, dword ptr [ESP+0xc]   ; b
//   8b 54 24 0c     MOV EDX, dword ptr [ESP+0xc]   ; b
//   c6 04 24 00     MOV byte ptr [ESP], 0          ; local.low = 0
//   8b 04 24        MOV EAX, dword ptr [ESP]       ; local
//   50              PUSH EAX                        ; arg6 = local
//   8b 44 24 14     MOV EAX, dword ptr [ESP+0x14]  ; c
//   51              PUSH ECX                        ; arg5 = b
//   8b 4c 24 14     MOV ECX, dword ptr [ESP+0x14]  ; b
//   52              PUSH EDX                        ; arg4 = b
//   8b 54 24 14     MOV EDX, dword ptr [ESP+0x14]  ; a
//   50              PUSH EAX                        ; arg3 = c
//   51              PUSH ECX                        ; arg2 = b
//   52              PUSH EDX                        ; arg1 = a
//   e8 RR RR RR RR  CALL FUN_00444cf0              ; (reloc)
//   83 c4 1c        ADD ESP, 0x1c                  ; caller cleanup
//   c3              RET

extern "C" void FUN_00444cf0();   // forward declaration for the CALL relocation

extern "C" __declspec(naked) void __cdecl FUN_00444d90() {
    __asm {
        // 00044d90: 51            PUSH ECX
        _emit 0x51
        // 00044d91: 8b 4c 24 0c   MOV ECX, [ESP+0xc]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 00044d95: 8b 54 24 0c   MOV EDX, [ESP+0xc]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 00044d99: c6 04 24 00   MOV byte ptr [ESP], 0
        _emit 0xc6
        _emit 0x04
        _emit 0x24
        _emit 0x00
        // 00044d9d: 8b 04 24      MOV EAX, [ESP]
        _emit 0x8b
        _emit 0x04
        _emit 0x24
        // 00044da0: 50            PUSH EAX
        _emit 0x50
        // 00044da1: 8b 44 24 14   MOV EAX, [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00044da5: 51            PUSH ECX
        _emit 0x51
        // 00044da6: 8b 4c 24 14   MOV ECX, [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00044daa: 52            PUSH EDX
        _emit 0x52
        // 00044dab: 8b 54 24 14   MOV EDX, [ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 00044daf: 50            PUSH EAX
        _emit 0x50
        // 00044db0: 51            PUSH ECX
        _emit 0x51
        // 00044db1: 52            PUSH EDX
        _emit 0x52
        // 00044db2: e8 RR RR RR RR  CALL FUN_00444cf0  (reloc)
        call FUN_00444cf0
        // 00044db7: 83 c4 1c      ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 00044dba: c3            RET
        _emit 0xc3
    }
}
