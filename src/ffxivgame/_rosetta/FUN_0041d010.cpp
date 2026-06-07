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
// FUNCTION: ffxivgame 0x0041d010 — __cdecl two-call dispatcher that looks up
//                                  two indices in a global pointer table and
//                                  invokes FUN_004236e0 for each (57 bytes).
//
// Source shape (inferred):
//
//   void FUN_0041d010(int arg1, int arg2) {
//       void* obj = *(void**)0x0132987c;        // g_object
//       g_object->method(0xab, g_table[arg1]);  // __thiscall, callee ret 8
//       g_object->method(0xd1, g_table[arg2]);
//   }
//
// Calling convention: __cdecl (bare RET; caller responsible for args).
//   Two 32-bit args at [ESP+4] and [ESP+8].  No callee-saved registers
//   used — EAX/ECX/EDX only.
//
// The inner call FUN_004236e0 is __thiscall with two DWORD stack args
// (ECX = this = *[0x0132987c]) and callee-cleans 8 bytes via ret 8.
// After the first call, ESP is fully restored so [ESP+8] still holds
// the original arg2.
//
// Push order for each inner call (right-to-left, rightmost first):
//   PUSH table_value   → rightmost / second stack arg
//   PUSH imm32         → leftmost / first stack arg
//   MOV ECX, [global]  → this pointer
//
// Absolute addresses encoded as literals in inline asm (no relocation):
//   0xf5971c   — base of global DWORD pointer table
//   0x0132987c — pointer-to-object global
// The two REL32 CALLs to FUN_004236e0 are masked by tools/compare.py.

extern "C" void FUN_004236e0(void);

extern "C" __declspec(naked) void FUN_0041d010(void)
{
    __asm {
        mov     eax, dword ptr [esp + 0x4]
        mov     ecx, dword ptr [eax*4 + 0xf5971c]
        push    ecx
        // MOV ECX, dword ptr [0x0132987c]  — 8b 0d 7c 98 32 01
        // Inline asm mis-encodes [constant] as imm32; emit raw bytes.
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        push    0xab
        call    FUN_004236e0
        mov     edx, dword ptr [esp + 0x8]
        mov     eax, dword ptr [edx*4 + 0xf5971c]
        // MOV ECX, dword ptr [0x0132987c]  — 8b 0d 7c 98 32 01
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        push    eax
        push    0xd1
        call    FUN_004236e0
        ret
    }
}
