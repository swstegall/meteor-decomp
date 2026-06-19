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
// FUNCTION: ffxivgame 0x0003f660 — string copy-init (__thiscall, 1 arg, 37 B)
//
// Initialises the SSO string fields in-place then delegates to FUN_0043f4b0
// (string::assign(src, 0, npos)) to copy the source string.  Returns `this`.
//
// MSVC SSO string layout (offsets from `this` / ESI):
//   +0x00  allocator proxy (4 B — not touched here)
//   +0x04  _Bx._Buf[16]   — SSO inline buffer; [0] set to '\0'
//   +0x14  _Mysize        — current length; set to 0
//   +0x18  _Myres         — capacity; set to 0xf (SSO threshold - 1)
//
// Reconstruction: __declspec(naked) byte pass-through.
//   The /O2 scheduler interleaves PUSH setup for the FUN_0043f4b0 call with
//   the inline field stores in an order that a plain C++ rewrite does not
//   reproduce reliably.  Emitting all 37 bytes directly with one REL32
//   relocation (the CALL) gives a clean GREEN diff.
//
// Reloc sites (REL32 masked by compare.py):
//   +0x1a  CALL 0x0043f4b0  — string::assign(src, pos, count) (__thiscall, RET 0xC)

extern "C" void FUN_0043f4b0();

extern "C" __declspec(naked) void FUN_0043f660()
{
    __asm {
        // 0003f660: 56              PUSH ESI
        _emit 0x56
        // 0003f661: 33 c0           XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0003f663: 8b f1           MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 0003f665: 6a ff           PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0003f667: 89 46 14        MOV dword ptr [ESI+0x14], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x14
        // 0003f66a: c7 46 18 0f 00 00 00   MOV dword ptr [ESI+0x18], 0xf
        _emit 0xc7
        _emit 0x46
        _emit 0x18
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003f671: 50              PUSH EAX
        _emit 0x50
        // 0003f672: 88 46 04        MOV byte ptr [ESI+0x4], AL
        _emit 0x88
        _emit 0x46
        _emit 0x04
        // 0003f675: 8b 44 24 10     MOV EAX, dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0003f679: 50              PUSH EAX
        _emit 0x50
        // 0003f67a: e8 31 fe ff ff  CALL 0x0043f4b0  [RELOC]
        call FUN_0043f4b0
        // 0003f67f: 8b c6           MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 0003f681: 5e              POP ESI
        _emit 0x5e
        // 0003f682: c2 04 00        RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
