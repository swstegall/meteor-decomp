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
// FUNCTION: ffxivgame 0x00011d10 — __thiscall LeaveCriticalSection wrapper
//
// Thin member function that calls LeaveCriticalSection on the CRITICAL_SECTION
// embedded at offset 0x5c in the host object.
//
//   void __thiscall FUN_00411d10(SomeClass* this) {
//       LeaveCriticalSection((CRITICAL_SECTION*)((char*)this + 0x5c));
//   }
//
// ECX is the `this` pointer on entry (__thiscall). MSVC 2005 /O2 reuses ECX
// directly (ADD ECX, 0x5c) since `this` is not needed after the call.
// LeaveCriticalSection is __stdcall and cleans its one argument; no cdecl
// cleanup needed. Returns with plain RET (no stack args beyond `this` in ECX).
//
// Sibling at RVA 0x00011d00 is the matching EnterCriticalSection wrapper
// (CALL dword ptr [0x00f3e16c]) for the same critical section slot.
//
// IAT: LeaveCriticalSection (kernel32) @ [0x00f3e168]

extern "C" __declspec(naked) void FUN_00411d10() {
    __asm {
        add  ecx, 0x5c
        push ecx
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        ret
    }
}
