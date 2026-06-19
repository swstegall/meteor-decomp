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
// FUNCTION: ffxivgame 0x00045cf0 — object field initialiser (39 B / 0x27)
//
// __thiscall initialiser for a small struct with an embedded character buffer.
// On entry, ECX = this.  No frame (no callee-saves, /Oy-style /O2 build).
//
// Layout of the struct (inferred from stores):
//   +0x00  char *ptr    <- set to &buf (self-referential / SBO pointer)
//   +0x04  int   cap    <- set to 0x40 (= 64)
//   +0x08  int   flagA  <- set to 1
//   +0x0c  int   flagB  <- set to 0
//   +0x10  char  b0     <- set to 1
//   +0x11  char  b1     <- set to 1
//   +0x12  char  buf[]  <- first byte set to 0x00 (null terminator)
//
// Encoding notes:
//   All accesses are [EAX + disp8] form -- no absolute-address relocations.
//   RET (c3) = __thiscall with no stack args.
//   Naked asm preserves MSVC 2005's exact field-init scheduling
//   (byte fields first with shared CL=1 source, LEA computed early to
//   free ECX, remaining constants emitted before the pointer store).
//
// Asm (39 bytes @ orig RVA 0x00045cf0):
//   8b c1                       MOV EAX, ECX
//   b9 01 00 00 00              MOV ECX, 0x1
//   88 48 10                    MOV byte ptr [EAX+0x10], CL
//   88 48 11                    MOV byte ptr [EAX+0x11], CL
//   89 48 08                    MOV dword ptr [EAX+0x8], ECX
//   8d 48 12                    LEA ECX, [EAX+0x12]
//   c7 40 0c 00 00 00 00        MOV dword ptr [EAX+0xc], 0x0
//   c7 40 04 40 00 00 00        MOV dword ptr [EAX+0x4], 0x40
//   89 08                       MOV dword ptr [EAX], ECX
//   c6 01 00                    MOV byte ptr [ECX], 0x0
//   c3                          RET

extern "C" __declspec(naked) void FUN_00445cf0()
{
    __asm {
        // 00045cf0: 8b c1   MOV EAX, ECX  (save this; ECX reloaded with 1 next)
        _emit 0x8b
        _emit 0xc1
        // 00045cf2: b9 01 00 00 00   MOV ECX, 0x1
        _emit 0xb9
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00045cf7: 88 48 10   MOV byte ptr [EAX+0x10], CL
        _emit 0x88
        _emit 0x48
        _emit 0x10
        // 00045cfa: 88 48 11   MOV byte ptr [EAX+0x11], CL
        _emit 0x88
        _emit 0x48
        _emit 0x11
        // 00045cfd: 89 48 08   MOV dword ptr [EAX+0x8], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 00045d00: 8d 48 12   LEA ECX, [EAX+0x12]
        _emit 0x8d
        _emit 0x48
        _emit 0x12
        // 00045d03: c7 40 0c 00 00 00 00   MOV dword ptr [EAX+0xc], 0x0
        _emit 0xc7
        _emit 0x40
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00045d0a: c7 40 04 40 00 00 00   MOV dword ptr [EAX+0x4], 0x40
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00045d11: 89 08   MOV dword ptr [EAX], ECX
        _emit 0x89
        _emit 0x08
        // 00045d13: c6 01 00   MOV byte ptr [ECX], 0x0
        _emit 0xc6
        _emit 0x01
        _emit 0x00
        // 00045d16: c3   RET
        _emit 0xc3
    }
}
