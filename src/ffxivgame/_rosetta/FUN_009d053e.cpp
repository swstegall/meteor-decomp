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
// FUNCTION: ffxivgame 0x005d053e — std::_Lockit::~_Lockit (18 B / 0x12)
//
// Destructor for the STL mutex guard std::_Lockit. Reads this->_Kind from
// [ECX], indexes into the global table of CRITICAL_SECTION objects at VA
// 0x01363c40 (stride 0x18 = 24 bytes == sizeof(CRITICAL_SECTION) on Win32),
// and calls the LeaveCriticalSection IAT thunk at RVA 0x005d17a3.
//
// Calling convention: __thiscall (ECX = this, no explicit stack args).
// Frame: none (/Oy omits frame pointer). Callee-saves: none. Return: void.
//
// Asm (18 bytes @ orig RVA 0x005d053e):
//   8b 01           MOV  EAX, [ECX]         ; this->_Kind (first field, int)
//   6b c0 18        IMUL EAX, EAX, 0x18     ; _Kind * 24 (stride = sizeof CRITICAL_SECTION)
//   05 [4 reloc]    ADD  EAX, g_lockit_cs   ; &g_lockit_cs[_Kind]
//   50              PUSH EAX                ; arg: pointer to critical section
//   e8 [4 reloc]    CALL FUN_009d17a3       ; LeaveCriticalSection IAT thunk (__cdecl, 1 arg)
//   59              POP  ECX               ; cdecl cleanup (1 arg; ECX used as scratch)
//   c3              RET
//
// Both 4-byte relocation sites (ADD EAX, imm32 and CALL rel32) are masked
// by compare.py and do not affect the diff outcome.
//
// Sibling: the matching constructor (symbol _Lockit, RVA 0x005d051d) calls
// FUN_009d1798 (EnterCriticalSection thunk) with the same index expression;
// this destructor calls FUN_009d17a3 (LeaveCriticalSection thunk).

// LeaveCriticalSection IAT thunk at RVA 0x005d17a3 (__cdecl, 1 arg).
extern "C" void __cdecl FUN_009d17a3(void *cs);

// Global CRITICAL_SECTION table (4 entries, stride 0x18 each).
// Base VA in the loaded image: 0x01363c40.
extern "C" char g_lockit_cs[];

extern "C" __declspec(naked) void FUN_009d053e() {
    __asm {
        mov  eax, dword ptr [ecx]
        imul eax, eax, 0x18
        add  eax, offset g_lockit_cs
        push eax
        call FUN_009d17a3
        pop  ecx
        ret
    }
}
