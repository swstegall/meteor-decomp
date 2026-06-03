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
// FUNCTION: ffxivgame 0x0001d0c0 — __cdecl 1-arg index-to-method forwarder (26 B)
//
// Takes an integer index, looks it up in a global int array at VA 0x00f597f4
// (scaled by 4), then calls FUN_004236e0 (a __thiscall member on the object
// at global 0x0132987c) with arguments (0x8, looked_up_value).
//
// Calling convention: __cdecl (bare RET — caller owns argument cleanup).
// Stack frame: none (no prologue/epilogue — ESP-relative only).
//
// Asm (26 bytes @ 0x0001d0c0):
//   8b 44 24 04                       MOV EAX, dword ptr [ESP + 0x4]           ; load idx arg
//   8b 0c 85 f4 97 f5 00              MOV ECX, dword ptr [EAX*0x4 + 0x00f597f4] ; g_table[idx]
//   51                                PUSH ECX                                   ; push table value
//   8b 0d 7c 98 32 01                 MOV ECX, dword ptr [0x0132987c]           ; load this ptr
//   6a 08                             PUSH 0x8                                   ; push arg1 = 8
//   e8 07 66 00 00                    CALL FUN_004236e0                          ; rel32 reloc
//   c3                                RET
//
// The two DIR32 reloc slots (array base and global object ptr) and the
// REL32 reloc slot (CALL target) are masked by compare.py, so the .obj
// matches the original slice byte-for-byte.

extern "C" {

// Global int array at VA 0x00f597f4; element 0 used as the displacement
// base for the SIB [EAX*4 + offset g_table_f597f4] addressing.
int g_table_f597f4;

// Global object pointer at VA 0x0132987c; loaded into ECX as `this` before
// the call to FUN_004236e0.
int g_obj_132987c;

// Member function called with this=*[0x0132987c], args (0x8, table_val).
void FUN_004236e0();

__declspec(naked) void FUN_0041d0c0() {
    __asm {
        mov eax, dword ptr [esp + 4]
        mov ecx, dword ptr g_table_f597f4[eax*4]
        push ecx
        mov ecx, dword ptr [g_obj_132987c]
        push 8
        call FUN_004236e0
        ret
    }
}

}  // extern "C"
