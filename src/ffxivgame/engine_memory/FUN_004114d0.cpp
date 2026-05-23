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
// FUNCTION: ffxivgame 0x004114d0 — count nodes in intrusive linked list
//           __thiscall, no stack args, returns int (node count)
//           (27 bytes / 0x1b)
//
// Asm (27 bytes @ orig RVA 0x000114d0):
//   8b 51 04    MOV EDX,[ECX+4]       ; EDX = this->m_inner
//   8b 4a 4c    MOV ECX,[EDX+0x4c]   ; ECX = m_inner->m_head (first node)
//   83 c2 44    ADD EDX,0x44          ; EDX = &m_inner->m_sentinel
//   33 c0       XOR EAX,EAX           ; count = 0
//   3b ca       CMP ECX,EDX           ; head == sentinel?
//   74 0b       JZ  +0xb (ret)        ; if empty, skip
//   90          NOP                   ; alignment
//   8b 49 08    MOV ECX,[ECX+8]       ; ECX = node->m_next
//   83 c0 01    ADD EAX,1             ; count++
//   3b ca       CMP ECX,EDX           ; node == sentinel?
//   75 f6       JNZ -0xa (loop)
//   c3          RET
//
// Walks a circular intrusive linked list rooted at this->m_inner->m_head
// (offset 0x4c). The sentinel is embedded in m_inner at offset 0x44.
// List nodes link forward via field +0x8 (m_next). Returns the number
// of elements in the list (0 if empty).

extern "C" __declspec(naked) void FUN_004114d0() {
    __asm {
        mov edx, dword ptr [ecx + 0x4]
        mov ecx, dword ptr [edx + 0x4c]
        add edx, 0x44
        xor eax, eax
        cmp ecx, edx
        jz  done
        nop
    loop_body:
        mov ecx, dword ptr [ecx + 0x8]
        add eax, 1
        cmp ecx, edx
        jnz loop_body
    done:
        ret
    }
}
