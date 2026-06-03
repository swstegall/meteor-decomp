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
// FUNCTION: ffxivgame 0x00019410 — FUN_00419410 (__cdecl, 26 B)
//
// Stores the low byte of its single argument to a global byte variable,
// then conditionally forwards the argument to FUN_0041a930 if a global
// pointer is non-null.
//
// Asm (26 bytes @ RVA 0x00019410):
//   8b 0d ?? ?? ?? ??   MOV  ECX, dword ptr [g_ptr_1328db4]  ; load ptr (DIR32)
//   85 c9               TEST ECX, ECX                         ; null check
//   8b 44 24 04         MOV  EAX, dword ptr [ESP + 4]         ; load arg
//   a2 ?? ?? ?? ??      MOV  byte ptr [g_byte_1328fa8], AL    ; store byte (DIR32)
//   74 06               JE   skip                              ; skip if ptr null
//   50                  PUSH EAX                               ; forward arg
//   e8 ?? ?? ?? ??      CALL FUN_0041a930                      ; (REL32 reloc)
// skip:
//   c3                  RET
//
// Calling convention: __cdecl (plain RET; caller cleans stack).
// Stack frame: none (no prologue / epilogue).

extern "C" {

int  g_ptr_1328db4;
char g_byte_1328fa8;

void FUN_0041a930();

__declspec(naked) void FUN_00419410()
{
    __asm {
        mov  ecx, dword ptr [g_ptr_1328db4]
        test ecx, ecx
        mov  eax, dword ptr [esp + 4]
        mov  byte ptr [g_byte_1328fa8], al
        je   skip
        push eax
        call FUN_0041a930
    skip:
        ret
    }
}

}  // extern "C"
