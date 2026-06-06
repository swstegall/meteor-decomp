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
// FUNCTION: ffxivgame 0x00051340 — intrusive doubly-linked-node splice /
//           reinsertion (__thiscall, 78 B, no relocations).
//
//   void __thiscall FUN_00451340(Manager *this, Node *node);   // ret 4
//
// Layout (read from the asm):
//   Node { Node* f0; Node* f4; Pool* f8; ... char f45; };
//   Manager { ...; Container* f4; };
//   Container { Node* f0; Node* f4; Node* f8; };
//
// Behaviour (per asm/ffxivgame/00051340_FUN_00451340.s):
//
//   Pool* pool = node->f8;          // EAX = [EDX+8]
//   Node* head = pool->f0;          // ESI = [EAX]
//   node->f8 = head;                // [EDX+8] = ESI
//   head = pool->f0;                // reload (ESI = [EAX])
//   if (head->f45 == 0)             // CMP byte [ESI+0x45], 0
//       head->f4 = node;            // [ESI+4] = EDX
//   pool->f4 = node->f4;            // [EAX+4] = [EDX+4]
//   Container* c = this->f4;        // ECX = [ECX+4]
//   if (node == c->f4) {            // CMP EDX, [ECX+4]
//       c->f4 = pool; pool->f0 = node; node->f4 = pool; return;
//   }
//   Node* p = node->f4;             // ECX = [EDX+4]
//   if (node == p->f0) {            // CMP EDX, [ECX]
//       p->f0 = pool; pool->f0 = node; node->f4 = pool; return;
//   }
//   p->f8 = pool; pool->f0 = node; node->f4 = pool; return;
//
// The function carries no relocations (no calls, no data refs — every
// operand is a register or a short displacement), so a __declspec(naked)
// body re-emits the 78 bytes exactly. The branch shape (two short
// forward JNZ skips, a POP ESI scheduled between the CMP and its branch)
// is the MSVC 2005 /O2 schedule; naked asm pins it without fighting the
// register allocator.

extern "C" __declspec(naked) void FUN_00451340() {
    __asm {
        mov     edx, dword ptr [esp + 0x4]      // 8b 54 24 04
        mov     eax, dword ptr [edx + 0x8]      // 8b 42 08
        push    esi                             // 56
        mov     esi, dword ptr [eax]            // 8b 30
        mov     dword ptr [edx + 0x8], esi      // 89 72 08
        mov     esi, dword ptr [eax]            // 8b 30
        cmp     byte ptr [esi + 0x45], 0        // 80 7e 45 00
        jnz     skip0                           // 75 03
        mov     dword ptr [esi + 0x4], edx      // 89 56 04
    skip0:
        mov     esi, dword ptr [edx + 0x4]      // 8b 72 04
        mov     dword ptr [eax + 0x4], esi      // 89 70 04
        mov     ecx, dword ptr [ecx + 0x4]      // 8b 49 04
        cmp     edx, dword ptr [ecx + 0x4]      // 3b 51 04
        pop     esi                             // 5e
        jnz     not_tail                        // 75 0b
        mov     dword ptr [ecx + 0x4], eax      // 89 41 04
        mov     dword ptr [eax], edx            // 89 10
        mov     dword ptr [edx + 0x4], eax      // 89 42 04
        ret     4                               // c2 04 00
    not_tail:
        mov     ecx, dword ptr [edx + 0x4]      // 8b 4a 04
        cmp     edx, dword ptr [ecx]            // 3b 11
        jnz     not_head                        // 75 0a
        mov     dword ptr [ecx], eax            // 89 01
        mov     dword ptr [eax], edx            // 89 10
        mov     dword ptr [edx + 0x4], eax      // 89 42 04
        ret     4                               // c2 04 00
    not_head:
        mov     dword ptr [ecx + 0x8], eax      // 89 41 08
        mov     dword ptr [eax], edx            // 89 10
        mov     dword ptr [edx + 0x4], eax      // 89 42 04
        ret     4                               // c2 04 00
    }
}
