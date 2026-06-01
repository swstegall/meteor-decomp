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
// FUNCTION: ffxivgame 0x0000e0d0 — __thiscall conditional virtual dispatch (51 B / 0x33)
//
// Calling convention: __thiscall (ECX = this; 3 DWORD args on stack; callee
// cleans 0xC bytes via `ret 0xc`).
//
// Frame:
//   PUSH ESI (callee-save; ESI = this throughout)
//   no ESP adjustment
//
// Behavior:
//   1. Load inner = *this (the inner interface pointer at this+0x00).
//   2. Call inner->vtable[6]() (no args; vtable offset 0x18). If result != 0,
//      return immediately.
//   3. Otherwise, call inner->vtable[5](arg1, arg2, arg3) (vtable offset 0x14)
//      and pass the returned pointer as ECX to FUN_0040edd0.
//
// Register scheduling (MSVC 2005 interleave):
//   Load arg3 into EAX, load arg2 into EDX, reload ECX from [ESI],
//   PUSH EAX (arg3), then load arg1 from [ESP+0xC] (shifted by that push),
//   PUSH EDX (arg2), reload vtable from ECX, PUSH EAX (arg1),
//   load vtable[5] into EAX, CALL EAX.
//   This exact interleaving is pinned by the naked asm.

extern "C" void FUN_0040edd0();

extern "C" __declspec(naked) void FUN_0040e0d0()
{
    __asm {
        push    esi
        mov     esi, ecx
        mov     ecx, dword ptr [esi]
        mov     eax, dword ptr [ecx]
        mov     edx, dword ptr [eax + 0x18]
        call    edx
        test    eax, eax
        jnz     done
        mov     eax, dword ptr [esp + 0x10]
        mov     edx, dword ptr [esp + 0xc]
        mov     ecx, dword ptr [esi]
        push    eax
        mov     eax, dword ptr [esp + 0xc]
        push    edx
        mov     edx, dword ptr [ecx]
        push    eax
        mov     eax, dword ptr [edx + 0x14]
        call    eax
        mov     ecx, eax
        call    FUN_0040edd0
    done:
        pop     esi
        ret     0xc
    }
}
