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
// FUNCTION: ffxivgame 0x00012600 — __thiscall: conditional vtable-indirect call,
//           store result, return this+8 or NULL. (41 bytes)
//
// Asm (41 bytes @ orig RVA 0x00012600):
//   56              PUSH ESI
//   8b f1           MOV ESI, ECX               ; cache `this` (thiscall)
//   8b 56 04        MOV EDX, [ESI+4]            ; EDX = this->field4 (sub-object ptr)
//   8b 4a 28        MOV ECX, [EDX+0x28]         ; ECX = sub->current (iterator ptr)
//   83 c2 20        ADD EDX, 0x20               ; EDX = &sub->field20 (end sentinel addr)
//   33 c0           XOR EAX, EAX                ; iVar2 = 0 (default)
//   3b ca           CMP ECX, EDX                ; current == end?
//   74 07           JE  +0x7 (→412619)           ; skip call if at end
//   8b 01           MOV EAX, [ECX]              ; EAX = current->vtbl
//   8b 50 04        MOV EDX, [EAX+4]            ; EDX = vtbl[1]
//   ff d2           CALL EDX                    ; (*vtbl[1])() — thiscall, ECX=current
//   85 c0           TEST EAX, EAX               ; (here or after jump)
//   89 46 0c        MOV [ESI+0xc], EAX          ; this->field_c = iVar2
//   74 05           JE  +0x5 (→412625)           ; if zero, return 0
//   8d 46 08        LEA EAX, [ESI+8]            ; EAX = this + 8
//   5e              POP ESI
//   c3              RET
//   33 c0           XOR EAX, EAX                ; return 0
//   5e              POP ESI
//   c3              RET
//
// __thiscall member: ECX = this, no stack args, returns int*.
// Looks up a sub-object via this->field4, checks if a stored iterator
// (field4+0x28) equals a sentinel address (field4+0x20). If not at end,
// dispatches through the object's vtable slot 1. Stores the result to
// this->field_c and returns this+8 on success, NULL on zero result.

extern "C" __declspec(naked) void FUN_00412600() {
    __asm {
        push esi
        mov  esi, ecx
        mov  edx, dword ptr [esi + 4]
        mov  ecx, dword ptr [edx + 0x28]
        add  edx, 0x20
        xor  eax, eax
        cmp  ecx, edx
        je   skip_call
        mov  eax, dword ptr [ecx]
        mov  edx, dword ptr [eax + 4]
        call edx
skip_call:
        test eax, eax
        mov  dword ptr [esi + 0xc], eax
        je   ret_null
        lea  eax, [esi + 8]
        pop  esi
        ret
ret_null:
        xor  eax, eax
        pop  esi
        ret
    }
}
