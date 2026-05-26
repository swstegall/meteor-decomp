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
// FUNCTION: ffxivgame 0x004135d0 — __thiscall: intrusive-list peek-front
//                                  with vtable[1] dispatch on head node
//                                  (41 bytes)
//
// Asm (41 bytes @ orig RVA 0x000135d0):
//   56              PUSH ESI
//   8b f1           MOV ESI, ECX              ; this
//   8b 56 04        MOV EDX, [ESI + 0x4]      ; container = this->m_container
//   8b 4a 2c        MOV ECX, [EDX + 0x2c]     ; node = container->head
//   83 c2 24        ADD EDX, 0x24             ; sentinel = &container->sentinel
//   33 c0           XOR EAX, EAX              ; result = 0
//   3b ca           CMP ECX, EDX
//   74 07           JZ  +7 (skip vcall)       ; if (node == sentinel) goto skip
//   8b 01           MOV EAX, [ECX]            ; vtbl = node->vtbl
//   8b 50 04        MOV EDX, [EAX + 0x4]      ; fp = vtbl[1]
//   ff d2           CALL EDX                  ; result = (node->*vtbl[1])()
//   85 c0           TEST EAX, EAX             ; skip:
//   89 46 0c        MOV [ESI + 0xc], EAX      ; this->m_result = result
//   74 05           JZ  +5
//   8d 46 08        LEA EAX, [ESI + 0x8]      ; return &this->m_outparam
//   5e              POP ESI
//   c3              RET
//   33 c0           XOR EAX, EAX              ; (result was 0) return 0
//   5e              POP ESI
//   c3              RET
//
// Looks like an iterator/visitor pattern: read the head of an intrusive
// linked list off a sub-container at this->m_container; if non-empty,
// dispatch through vtable slot 1 of the head node; cache the result on
// `this` and either return a pointer to a member or NULL.
//
// Written as a `__declspec(naked)` __thiscall member because the
// frame manipulation (single PUSH/POP ESI, no SUB ESP, no `ret N`) is
// awkward to coax out of MSVC 2005 from C++ — and naked-asm is the
// canonical engine_memory idiom for this shape (see FUN_00412230.cpp,
// FUN_00412530.cpp).

extern "C" __declspec(naked) void FUN_004135d0() {
    __asm {
        push esi
        mov esi, ecx
        mov edx, [esi + 0x4]
        mov ecx, [edx + 0x2c]
        add edx, 0x24
        xor eax, eax
        cmp ecx, edx
        jz skip
        mov eax, [ecx]
        mov edx, [eax + 0x4]
        call edx
skip:
        test eax, eax
        mov [esi + 0xc], eax
        jz zero_ret
        lea eax, [esi + 0x8]
        pop esi
        ret
zero_ret:
        xor eax, eax
        pop esi
        ret
    }
}
