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
// FUNCTION: ffxivgame 0x00012b50 — __thiscall vtable-slot-2 dispatcher through
//           this->member_0x14: load sub-object from [ECX+0x14], then tail-call
//           sub-object->vtable[2] (byte offset 0x08).
//           (10 bytes)
//
// Asm (10 bytes @ orig RVA 0x00012b50):
//   8B 49 14    MOV  ECX, [ECX+0x14]  ; ecx = this->member_0x14 (new 'this')
//   8B 01       MOV  EAX, [ECX]        ; eax = vtable ptr of member
//   8B 50 08    MOV  EDX, [EAX+0x08]  ; edx = vtable slot 2 (byte offset 8)
//   FF E2       JMP  EDX               ; tail-call with ecx = sub-object's this

extern "C" __declspec(naked) void FUN_00412b50()
{
    __asm {
        mov ecx, [ecx+14h]
        mov eax, [ecx]
        mov edx, [eax+8]
        jmp edx
    }
}
