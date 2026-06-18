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
// FUNCTION: ffxivgame 0x00433d20 — allocate a 20-byte typed entry from a
//           slot-pool, fill it with a vtable + 16 bytes copied from arg,
//           then push it (or NULL on failure) onto this->member_0xC.
//           __thiscall, 1 stack argument, 95 bytes.
//
// Stack layout (image-base 0x00400000):
//   ECX at entry    : this (saved as ESI)
//   [ESP+4] at entry: pointer to 16-byte source struct (arg1)
//
// The global at DAT_01328d90 holds a pointer to a slot-pool header:
//   *(byte*)header      = byte index into the slot table
//   *(dword*)(header+4) = base pointer of the slot table
// Pool slot address = base + index * 28.
//
// FUN_00417ab0(__thiscall, pool_slot, size=0x14) allocates 20 bytes from
// the slot; on success the new object is stamped with vtable DAT_00f64938
// and 16 bytes from arg1 are copied in two 8-byte chunks via MOVQ/XMM0.
// FUN_0043c2d0 on this->member_0xC is called with the new object (or NULL
// if allocation failed) to register the entry in the container.

extern "C" {
    void FUN_00417ab0();
    void FUN_0043c2d0();
    extern int DAT_01328d90;
    extern int DAT_00f64938;
}

extern "C" __declspec(naked) void FUN_00433d20() {
    __asm {
        push esi
        mov  esi, ecx
        mov  ecx, dword ptr [DAT_01328d90]
        movzx eax, byte ptr [ecx]
        lea  edx, [eax*8]
        sub  edx, eax
        mov  eax, dword ptr [ecx + 4]
        lea  ecx, [eax + edx*4]
        push 0x14
        call FUN_00417ab0
        test eax, eax
        jz   null_path
        mov  ecx, dword ptr [esp + 8]
        mov  dword ptr [eax], offset DAT_00f64938
        _emit 0xf3              // MOVQ XMM0, qword ptr [ECX]
        _emit 0x0f
        _emit 0x7e
        _emit 0x01
        _emit 0x66              // MOVQ qword ptr [EAX+4], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x04
        _emit 0xf3              // MOVQ XMM0, qword ptr [ECX+8]
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x08
        _emit 0x66              // MOVQ qword ptr [EAX+0xC], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x0c
        mov  ecx, dword ptr [esi + 0xc]
        push eax
        call FUN_0043c2d0
        pop  esi
        ret  4
    null_path:
        mov  ecx, dword ptr [esi + 0xc]
        xor  eax, eax
        push eax
        call FUN_0043c2d0
        pop  esi
        ret  4
    }
}
