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
// FUNCTION: ffxivgame 0x00414b20 — __thiscall LeaveCriticalSection on
//                                  sub-object at this+8 (11 bytes).
//
// Asm:
//   83 c1 08              ADD  ECX, 0x8           ; &this->cs (+8)
//   51                    PUSH ECX
//   ff 15 68 e1 f3 00     CALL [LeaveCriticalSection]  ; IAT @ 0x00f3e168
//   c3                    RET
//
// __thiscall: ECX = this; no stack args; void return.
// The CRITICAL_SECTION member lives at this+0x08 (matches the SystemHeapSpace
// layout documented in decomp-notes/types/ffxivgame/0x00014b50.md).
// ADD ECX,8 then PUSH ECX is MSVC's compact form of PUSH &this->cs
// (saves 1 byte vs LEA EAX,[ECX+8]; PUSH EAX).

extern "C" int g_imp_LeaveCriticalSection;  // kernel32 IAT @ 0x00f3e168

extern "C" __declspec(naked) void FUN_00414b20() {
    __asm {
        add  ecx, 8
        push ecx
        call dword ptr [g_imp_LeaveCriticalSection]
        ret
    }
}
