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
// FUNCTION: ffxivgame 0x004c4a0 — vtable slot-0x4 call, sum result with arg
// __thiscall member: loads the vtable from `this`, calls the method at
// slot 0x4 (ECX = this passed through, no args), then adds the single
// stack argument to the returned value and returns. `ret 4` cleans the
// one stack arg.
//
// Asm (14 bytes):
//   8b 01           mov eax, [ecx]        ; vtable
//   8b 50 04        mov edx, [eax + 4]    ; slot 0x4
//   ff d2           call edx
//   03 44 24 04     add eax, [esp + 4]    ; += arg
//   c2 04 00        ret 4

extern "C" __declspec(naked) void vtable_call_add_arg() {
    __asm {
        mov eax, [ecx]
        mov edx, [eax + 4]
        call edx
        add eax, [esp + 4]
        ret 4
    }
}
