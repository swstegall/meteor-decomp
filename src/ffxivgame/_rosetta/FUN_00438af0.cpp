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
// FUNCTION: ffxivgame 0x00038af0 — build a 0x1c-byte command/event object on
//                                  the stack and dispatch it (__thiscall, 78 B,
//                                  ret 0x18 → six 4-byte stack args).
//
// __thiscall void FUN_00438af0(Outer *this, int a0, int a1, int a2,
//                              int a3, int a4, int a5);
//
// Lays a temporary object out on the stack frame:
//
//   struct Temp {
//       void *vtable;   // +0x00  0xf649b8 (.rdata vtable literal)
//       int   a0;       // +0x04
//       int   a1;       // +0x08
//       int   a2;       // +0x0c
//       int   a3;       // +0x10
//       int   a4;       // +0x14
//       int   a5;       // +0x18
//   };
//
// The six incoming args are interleaved into the slots, the vtable literal is
// written into +0x00 (reusing the slot the PUSH'd argument vacated via the
// LEA ECX, [ESP+4] address-of trick), then the dispatcher at FUN_00435d60 is
// invoked with ECX = &Temp and one pushed argument = this->field_4. This is
// the same "construct-temp-then-thiscall-dispatch" shape as sibling
// FUN_004344b0 (which stores the adjacent vtable 0xf649b0).
//
// Reloc-bearing sites (4-byte windows, wildcarded by compare.py):
//   +0x37  MOV [ESP+4], offset data_00f649b8   (dir32, .rdata vtable)
//   +0x43  CALL FUN_00435d60                   (rel32, dispatcher)
//
// Naked asm: the interleaved arg copies, the PUSH/LEA address-of trick that
// reuses the +0x00 slot, and the precise instruction schedule are MSVC 2005
// /O2 choices that source-level C++ won't reproduce byte-for-byte.

extern "C" {
    // .text — RVA 0x00035d60. __thiscall dispatch(this=&Temp, arg).
    int FUN_00435d60();
    // .rdata — vtable literal stored at Temp+0x00.
    extern int data_00f649b8;
}

extern "C" __declspec(naked) void FUN_00438af0() {
    __asm {
        sub     esp, 0x1c
        mov     eax, dword ptr [esp + 0x20]
        mov     edx, dword ptr [esp + 0x24]
        mov     dword ptr [esp + 0x4], eax
        mov     eax, dword ptr [esp + 0x28]
        mov     dword ptr [esp + 0xc], eax
        mov     eax, dword ptr [esp + 0x30]
        mov     dword ptr [esp + 0x8], edx
        mov     edx, dword ptr [esp + 0x2c]
        mov     dword ptr [esp + 0x14], eax
        mov     eax, dword ptr [ecx + 0x4]
        mov     dword ptr [esp + 0x10], edx
        mov     edx, dword ptr [esp + 0x34]
        push    eax
        lea     ecx, [esp + 0x4]
        mov     dword ptr [esp + 0x4], offset data_00f649b8
        mov     dword ptr [esp + 0x1c], edx
        call    FUN_00435d60
        add     esp, 0x1c
        ret     0x18
    }
}
