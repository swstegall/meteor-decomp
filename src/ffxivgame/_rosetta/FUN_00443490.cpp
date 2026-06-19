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
// FUNCTION: ffxivgame 0x00043490 (VA 0x00443490) — scalar deleting destructor with vtable reset (36B)
//
// __thiscall, one stack arg (unsigned int freeMem). Resets the vftable
// pointer at [this+0] to 0x00f67124 (the absolute VA of this class's vtable),
// then calls the real destructor body FUN_00442840 (thiscall, ECX=this,
// no additional stack args). If bit 0 of the first argument is set the heap
// object is freed via operator_delete. Returns `this` in EAX; the single
// stack argument is callee-cleaned (RET 4).
//
// The asm listing omits the three-byte `add esp, 4` between the operator-
// delete CALL and the no_delete label, but the JZ offset of 9 (= PUSH(1)
// + CALL(5) + ADD ESP,4(3)) confirms it must be present.
//
// Byte pattern:
//   56 8b f1 c7 06 24 71 f6 00
//   e8 RR RR RR RR
//   f6 44 24 08 01 74 09
//   56 e8 RR RR RR RR 83 c4 04
//   8b c6 5e c2 04 00

extern "C" int real_dtor();
extern "C" int operator_delete();

extern "C" __declspec(naked) void FUN_00443490() {
    __asm {
        push esi
        mov esi, ecx
        mov dword ptr [esi], 0xf67124
        call real_dtor
        test byte ptr [esp+8], 1
        jz no_delete
        push esi
        call operator_delete
        add esp, 4
    no_delete:
        mov eax, esi
        pop esi
        ret 4
    }
}
