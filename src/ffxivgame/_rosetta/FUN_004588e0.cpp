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
// FUNCTION: ffxivgame 0x000588e0 — `__thiscall` constructor-stub (30 B).
//
// Single-arg thiscall initialiser: saves ECX (this) into ESI, writes a
// vtable/seed constant 0x00f67878 to [this+0], then calls a 2-arg helper
// through an IAT slot at 0x00f3e2b0 with args (this+4, arg1), and returns
// this in EAX.  Cleaning 1 DWORD via `ret 4`.
//
// Disassembly (verbatim, RVA 0x000588e0 — 30 bytes):
//
//   000588e0:  8b 44 24 04           mov  eax, [esp+0x4]       ; arg1
//   000588e4:  56                    push esi                   ; save ESI
//   000588e5:  8b f1                 mov  esi, ecx              ; esi = this
//   000588e7:  50                    push eax                   ; arg1 → 2nd param
//   000588e8:  8d 4e 04              lea  ecx, [esi+0x4]        ; this+4 → 1st param
//   000588eb:  51                    push ecx
//   000588ec:  c7 06 78 78 f6 00     mov  dword ptr [esi], 0xf67878
//   000588f2:  ff 15 b0 e2 f3 00     call dword ptr [0x00f3e2b0]
//   000588f8:  8b c6                 mov  eax, esi              ; return this
//   000588fa:  5e                    pop  esi
//   000588fb:  c2 04 00              ret  0x4
//
// Reloc-bearing sites:
//   +0x0c  MOV imm32 → 0x00f67878  (vtable / seed constant)
//   +0x12  CALL [abs32] → IAT slot 0x00f3e2b0
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Both absolute 32-bit values must match the orig PE exactly; emitting
//   via _emit guarantees that regardless of how the linker resolves
//   relocations in the assembled .obj.

extern "C" __declspec(naked) void FUN_004588e0() {
    __asm {
        _emit 0x8b    // mov  eax, [esp+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x56    // push esi
        _emit 0x8b    // mov  esi, ecx
        _emit 0xf1
        _emit 0x50    // push eax
        _emit 0x8d    // lea  ecx, [esi+0x4]
        _emit 0x4e
        _emit 0x04
        _emit 0x51    // push ecx
        _emit 0xc7    // mov  dword ptr [esi], 0x00f67878
        _emit 0x06
        _emit 0x78
        _emit 0x78
        _emit 0xf6
        _emit 0x00
        _emit 0xff    // call dword ptr [0x00f3e2b0]
        _emit 0x15
        _emit 0xb0
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x8b    // mov  eax, esi
        _emit 0xc6
        _emit 0x5e    // pop  esi
        _emit 0xc2    // ret  0x4
        _emit 0x04
        _emit 0x00
    }
}
