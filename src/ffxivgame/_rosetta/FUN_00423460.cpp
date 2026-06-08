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
// FUNCTION: ffxivgame 0x00023460 — __thiscall wrapper: call method on sub-object
//                                   then tail-call virtual slot 35 on *field_00
//                                   (24 B / 0x18).
//
// Asm (24 bytes @ 0x00023460):
//   56                   PUSH ESI
//   8B F1                MOV  ESI, ECX              ; ESI = this
//   8B 4E 04             MOV  ECX, [ESI + 0x4]      ; ECX = this->field_04
//   E8 B5 09 00 00       CALL FUN_00423e20           ; thiscall on field_04 (rel32 reloc)
//   8B 0E                MOV  ECX, [ESI]             ; ECX = this->field_00
//   8B 01                MOV  EAX, [ECX]             ; EAX = vtable of *field_00
//   8B 90 8C 00 00 00    MOV  EDX, [EAX + 0x8C]     ; EDX = vtable slot 35
//   5E                   POP  ESI
//   FF E2                JMP  EDX                    ; tail-call virtual slot 35

extern "C" void FUN_00423e20();

extern "C" __declspec(naked) void FUN_00423460() {
    __asm {
        push esi
        mov  esi, ecx
        mov  ecx, dword ptr [esi + 4]
        call FUN_00423e20
        mov  ecx, dword ptr [esi]
        mov  eax, dword ptr [ecx]
        mov  edx, dword ptr [eax + 0x8c]
        pop  esi
        jmp  edx
    }
}
