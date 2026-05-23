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
// FUNCTION: ffxivgame 0x00411550 — try-get-first-node virtual dispatch
//           __thiscall, no stack args, returns void* (41 bytes / 0x29)
//
// ECX = this (an object with m_inner at +0x4, a result field at +0xc,
//             and a sub-object at +0x8).
// m_inner->field_0x54 is the head of an intrusive list.
// m_inner->field_0x4c is the list sentinel (embedded node).
//
// Behaviour:
//   1. Load m_inner = this->field_0x4 into EDX.
//   2. Load head = m_inner->field_0x54 into ECX.
//   3. Compute sentinel = &m_inner->field_0x4c (EDX += 0x4c).
//   4. EAX = 0.
//   5. If head != sentinel: call head->vtable[1]() (virtual dispatch
//      through the first dword of the node, slot 1 at vtable+4).
//      EAX = return value of that call.
//   6. Store EAX into this->field_0xc.
//   7. If EAX != 0: return &this->field_0x8.
//   8. Else: return NULL.
//
// Reconstruction: naked-asm — two separate RET paths and register
// scheduling around the virtual dispatch cannot be reproduced verbatim
// from source-level C++.
//
// Asm (41 bytes @ orig RVA 0x00011550):
//   56              PUSH ESI
//   8b f1           MOV ESI,ECX
//   8b 56 04        MOV EDX,dword ptr [ESI+0x4]
//   8b 4a 54        MOV ECX,dword ptr [EDX+0x54]
//   83 c2 4c        ADD EDX,0x4c
//   33 c0           XOR EAX,EAX
//   3b ca           CMP ECX,EDX
//   74 07           JZ +7
//   8b 01           MOV EAX,dword ptr [ECX]
//   8b 50 04        MOV EDX,dword ptr [EAX+0x4]
//   ff d2           CALL EDX
//   85 c0           TEST EAX,EAX
//   89 46 0c        MOV dword ptr [ESI+0xc],EAX
//   74 05           JZ +5
//   8d 46 08        LEA EAX,[ESI+0x8]
//   5e              POP ESI
//   c3              RET
//   33 c0           XOR EAX,EAX
//   5e              POP ESI
//   c3              RET

extern "C" __declspec(naked) void FUN_00411550()
{
    __asm {
        push esi
        mov  esi, ecx
        mov  edx, dword ptr [esi + 0x4]
        mov  ecx, dword ptr [edx + 0x54]
        add  edx, 0x4c
        xor  eax, eax
        cmp  ecx, edx
        jz   done
        mov  eax, dword ptr [ecx]
        mov  edx, dword ptr [eax + 0x4]
        call edx
    done:
        test eax, eax
        mov  dword ptr [esi + 0xc], eax
        jz   ret_null
        lea  eax, [esi + 0x8]
        pop  esi
        ret
    ret_null:
        xor  eax, eax
        pop  esi
        ret
    }
}
