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
// FUNCTION: ffxivgame 0x00011df0 — vtable[1] indirect call through chain + addend return
//
// __thiscall, no explicit args. Caches `this` in EAX early (freeing ECX for the
// nested __thiscall), dereferences *(*(this+0x2c)+0x28) into ECX, calls vtable[1]
// on that inner object, then adds *(this+0x28) to the return value.
//
// The PUSH ESI is deferred until after the vtable pointer is loaded — MSVC uses
// EAX as a no-cost temp for `this` (clobbered by the call anyway) and only saves
// ESI once it's actually needed as a post-call addend register.
//
// Asm: 8b c1 8b 48 2c 8b 49 28 8b 11 56 8b 70 28 8b 42 04 ff d0 03 c6 5e c3

#if defined(__clang__) || defined(__GNUC__)
extern "C" int FUN_00411df0() { return 0; }
#else
extern "C" __declspec(naked) int FUN_00411df0() {
    __asm {
        mov eax, ecx
        mov ecx, [eax + 0x2c]
        mov ecx, [ecx + 0x28]
        mov edx, [ecx]
        push esi
        mov esi, [eax + 0x28]
        mov eax, [edx + 0x4]
        call eax
        add eax, esi
        pop esi
        ret
    }
}
#endif
