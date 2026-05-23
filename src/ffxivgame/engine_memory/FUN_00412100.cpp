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
// FUNCTION: ffxivgame 0x00012100 — engine_memory vtable dispatch chain
// __thiscall, 1 stack arg (int *param_1), returns void.
//
// Disassembly (76 bytes):
//   56                      PUSH ESI
//   8b f1                   MOV ESI, ECX             ; ESI = this
//   8b 06                   MOV EAX, [ESI]           ; EAX = *this (vtable)
//   8b 50 2c                MOV EDX, [EAX+0x2c]      ; EDX = vtable slot 11 (offset 0x2c)
//   57                      PUSH EDI                 ; save EDI (deferred callee-save)
//   ff d2                   CALL EDX                 ; this->vtable[0x2c]()
//   8b 4c 24 0c             MOV ECX, [ESP+0xc]       ; ECX = param_1 (after 2 pushes)
//   8b 01                   MOV EAX, [ECX]           ; EAX = *param_1 (vtable)
//   8b 50 14                MOV EDX, [EAX+0x14]      ; EDX = param_1->vtable[5]
//   ff d2                   CALL EDX                 ; param_1->vtable[0x14]() → piVar1
//   8b 10                   MOV EDX, [EAX]           ; EDX = piVar1->vtable
//   8b c8                   MOV ECX, EAX             ; ECX = piVar1
//   8b 42 04                MOV EAX, [EDX+4]         ; EAX = piVar1->vtable[1]
//   ff d0                   CALL EAX                 ; piVar1->vtable[0x4]() → iVar2
//   8b 4e 04                MOV ECX, [ESI+4]         ; ECX = this->field[1] (offset 4)
//   8b 11                   MOV EDX, [ECX]           ; EDX = field[1]->vtable
//   8b 52 10                MOV EDX, [EDX+0x10]      ; EDX = field[1]->vtable[4]
//   8b f8                   MOV EDI, EAX             ; EDI = iVar2
//   8b 47 28                MOV EAX, [EDI+0x28]      ; EAX = *(iVar2+0x28)
//   50                      PUSH EAX                 ; push arg *(iVar2+0x28)
//   ff d2                   CALL EDX                 ; field[1]->vtable[0x10](*(iVar2+0x28))
//   57                      PUSH EDI                 ; push iVar2 (arg for FUN_004120b0)
//   8b ce                   MOV ECX, ESI             ; ECX = this (for __thiscall)
//   c7 47 28 00 00 00 00    MOV [EDI+0x28], 0        ; *(iVar2+0x28) = 0
//   e8 ?? ?? ?? ??          CALL FUN_004120b0        ; reloc: this->FUN_004120b0(iVar2)
//   8b 06                   MOV EAX, [ESI]           ; EAX = *this (vtable)
//   8b 50 30                MOV EDX, [EAX+0x30]      ; EDX = vtable slot 12 (offset 0x30)
//   8b ce                   MOV ECX, ESI             ; ECX = this
//   ff d2                   CALL EDX                 ; this->vtable[0x30]()
//   5f                      POP EDI
//   5e                      POP ESI
//   c2 04 00                RET 4                    ; __thiscall, clean 4 bytes

// Forward reference to unmatched sibling called as __thiscall with 1 stack arg.
extern "C" void FUN_004120b0(void);

extern "C" __declspec(naked) void FUN_00412100(void)
{
    __asm {
        push esi
        mov esi, ecx
        mov eax, [esi]
        mov edx, [eax+0x2c]
        push edi
        call edx
        mov ecx, [esp+0xc]
        mov eax, [ecx]
        mov edx, [eax+0x14]
        call edx
        mov edx, [eax]
        mov ecx, eax
        mov eax, [edx+4]
        call eax
        mov ecx, [esi+4]
        mov edx, [ecx]
        mov edx, [edx+0x10]
        mov edi, eax
        mov eax, [edi+0x28]
        push eax
        call edx
        push edi
        mov ecx, esi
        mov dword ptr [edi+0x28], 0
        call FUN_004120b0
        mov eax, [esi]
        mov edx, [eax+0x30]
        mov ecx, esi
        call edx
        pop edi
        pop esi
        ret 4
    }
}
