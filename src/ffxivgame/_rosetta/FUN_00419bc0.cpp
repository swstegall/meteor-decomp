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
// FUNCTION: ffxivgame 0x00019bc0 — ref-counted pointer setter (33 B)
//
// __thiscall; one stack arg (the new pointer value); callee pops via RET 4.
// ECX = this (the owning object); [ESP+4] on entry = newVal.
//
// Stores a new pointer into this->field0.  If the currently stored pointer
// (old) differs from newVal AND is non-null, calls old->vtable[0](1) — a
// Release / dec-ref pattern commonly seen in COM-style 1.x engine objects —
// before overwriting the slot.
//
// Disassembly (33 bytes @ orig RVA 0x00019bc0):
//
//   56              PUSH ESI
//   8b f1           MOV ESI, ECX              ; ESI = this
//   8b 0e           MOV ECX, [ESI]            ; ECX = this->field0 (old)
//   57              PUSH EDI
//   8b 7c 24 0c     MOV EDI, [ESP+0xC]        ; EDI = newVal (stack arg)
//   3b f9           CMP EDI, ECX              ; new == old?
//   74 0c           JE  skip                  ; yes → skip Release
//   85 c9           TEST ECX, ECX             ; old == null?
//   74 08           JE  skip                  ; yes → skip Release
//   8b 01           MOV EAX, [ECX]            ; EAX = old->vtable
//   8b 10           MOV EDX, [EAX]            ; EDX = vtable[0]
//   6a 01           PUSH 1                    ; arg = 1
//   ff d2           CALL EDX                  ; old->vtable[0](1)
// skip:
//   89 3e           MOV [ESI], EDI            ; this->field0 = newVal
//   5f              POP EDI
//   5e              POP ESI
//   c2 04 00        RET 4
//
// Reconstruction: naked asm; the indirect CALL EDX is an intra-register
// dispatch with no relocations, so the naked form gives a stable
// byte-identical match without chasing MSVC register-scheduler behaviour.

extern "C" __declspec(naked) void FUN_00419bc0() {
    __asm {
        push    esi
        mov     esi, ecx
        mov     ecx, dword ptr [esi]
        push    edi
        mov     edi, dword ptr [esp + 0xc]
        cmp     edi, ecx
        je      skip
        test    ecx, ecx
        je      skip
        mov     eax, dword ptr [ecx]
        mov     edx, dword ptr [eax]
        push    1
        call    edx
    skip:
        mov     dword ptr [esi], edi
        pop     edi
        pop     esi
        ret     4
    }
}
