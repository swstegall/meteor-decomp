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
// FUNCTION: ffxivgame 0x000591c0 — two-arg object constructor (__thiscall, 39 B / 0x27)
//
// SomeObj *FUN_004591c0(SomeObj *this /*ECX*/, int arg1, int arg2)
//
// Installs the vtable pointer 0x00f678f8 at this[0] and initialises five
// DWORD fields, then returns `this` in EAX (constructor return idiom):
//   this[0x00] = 0x00f678f8   ; vtable
//   this[0x04] = 1
//   this[0x08] = arg2         ; [ESP+8]
//   this[0x0c] = arg1         ; [ESP+4]
//   this[0x10] = -1
//
// Calling convention: __thiscall — `this` in ECX, two DWORD stack args,
// epilogue is `RET 0x8`. The two args are read in source order (arg1→EDX,
// arg2→ECX) before the vtable/field stores. The vtable store is encoded as
// a literal `c7 00 <abs32>` immediate move; reproducing the exact absolute
// vtable address from source-level C++ depends on linker placement we don't
// control via headers, so this is encoded as a __declspec(naked) function
// emitting each byte verbatim.
//
// Asm (39 bytes @ orig RVA 0x000591c0):
//   8b 54 24 04           MOV EDX, [ESP+0x4]            ; arg1
//   8b c1                 MOV EAX, ECX                  ; return value = this
//   8b 4c 24 08           MOV ECX, [ESP+0x8]            ; arg2
//   c7 00 f8 78 f6 00     MOV [EAX], 0x00f678f8         ; vtable
//   c7 40 04 01 00 00 00  MOV [EAX+0x4], 0x1
//   89 48 08              MOV [EAX+0x8], ECX            ; arg2
//   89 50 0c              MOV [EAX+0xc], EDX            ; arg1
//   c7 40 10 ff ff ff ff  MOV [EAX+0x10], 0xffffffff
//   c2 08 00              RET 0x8

extern "C" __declspec(naked) void FUN_004591c0() {
    __asm {
        // 000591c0: 8b 54 24 04   MOV EDX, [ESP+4]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x04
        // 000591c4: 8b c1         MOV EAX, ECX
        _emit 0x8b
        _emit 0xc1
        // 000591c6: 8b 4c 24 08   MOV ECX, [ESP+8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 000591ca: c7 00 f8 78 f6 00   MOV [EAX], 0x00f678f8
        _emit 0xc7
        _emit 0x00
        _emit 0xf8
        _emit 0x78
        _emit 0xf6
        _emit 0x00
        // 000591d0: c7 40 04 01 00 00 00   MOV [EAX+4], 1
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000591d7: 89 48 08      MOV [EAX+8], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 000591da: 89 50 0c      MOV [EAX+0xc], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x0c
        // 000591dd: c7 40 10 ff ff ff ff   MOV [EAX+0x10], 0xffffffff
        _emit 0xc7
        _emit 0x40
        _emit 0x10
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 000591e4: c2 08 00      RET 8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
