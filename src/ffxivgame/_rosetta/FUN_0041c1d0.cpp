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
// FUNCTION: ffxivgame 0x0001c1d0 — __cdecl 1-byte-arg wrapper; loads global
//           object pointer into ECX and forwards to FUN_004236e0 (20 B / 0x14)
//
// Asm (20 bytes @ orig RVA 0x0001c1d0):
//   0f b6 44 24 04        MOVZX EAX, byte ptr [ESP+0x4]   ; zero-extend byte arg
//   8b 0d 7c 98 32 01     MOV   ECX, dword ptr [0x0132987c]; load global this-ptr
//   50                    PUSH  EAX                        ; push 2nd arg (byte)
//   6a 0f                 PUSH  0xf                        ; push 1st arg (15)
//   e8 fd 74 00 00        CALL  FUN_004236e0               ; rel32 reloc
//   c3                    RET
//
// Calling convention: __cdecl (bare RET; one unsigned char argument at [ESP+4]).
// The callee FUN_004236e0 is invoked via __thiscall — its `this` pointer comes
// from the global DWORD at 0x0132987c, first arg is the constant 0xf (15),
// second arg is the byte parameter zero-extended to a DWORD.
//
// Both the DIR32 reloc covering [0x0132987c] and the REL32 reloc for the CALL
// are masked by compare.py's relocation-window diffing.
//
// Sibling: FUN_0041c300.cpp uses the identical shape with constant 0xb9.

extern "C" int g_obj_0132987c;    // global object-pointer slot at VA 0x0132987c
extern "C" void FUN_004236e0();

extern "C" __declspec(naked) void FUN_0041c1d0() {
    __asm {
        movzx eax, byte ptr [esp + 4]
        mov   ecx, dword ptr [g_obj_0132987c]
        push  eax
        push  0xf
        call  FUN_004236e0
        ret
    }
}
