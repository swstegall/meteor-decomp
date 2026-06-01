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
// FUNCTION: ffxivgame 0x00010730 — spin-lock insert into a doubly-linked list
// __stdcall void FUN_00410730(Node* node) — 77 bytes
//
// Inserts `node` into a spin-lock-protected doubly-linked list owned by
// the node's "owner" field at offset 0x14:
//
//   1. owner   = node->field_0x14
//   2. handle  = owner->vtable[1]()        (__thiscall, no args)
//   3. queue   = handle->field_0x10        (actual queue/pool struct)
//   4. node->vtable[0](0)                  (__thiscall, one int arg = 0)
//   5. spin-acquire lock at queue->field_0x4 (xchg test-and-set loop)
//   6. insert node at front of queue->field_0xc intrusive doubly-linked list
//      (field_0 = prev-link, field_4 = next-link on list nodes)
//   7. queue->field_0x18-- (decrement pending/active count)
//   8. release lock (xchg 0 back into queue->field_0x4)
//
// Both virtual calls use indirect CALL EDX (no REL32/DIR32 fixups), so
// all 77 bytes are fully deterministic — naked asm passes byte-for-byte.
//
// Byte map (offsets from function start):
//   +00  56                    push esi
//   +01  8b 74 24 08           mov esi, [esp+8]          ; node
//   +05  8b 4e 14              mov ecx, [esi+0x14]       ; owner
//   +08  8b 01                 mov eax, [ecx]            ; vtable
//   +0a  8b 50 04              mov edx, [eax+4]          ; vtable[1]
//   +0d  57                    push edi
//   +0e  ff d2                 call edx                  ; owner->vtable[1]()
//   +10  8b 78 10              mov edi, [eax+0x10]       ; queue = eax->field_0x10
//   +13  8b 06                 mov eax, [esi]            ; node vtable
//   +15  8b 10                 mov edx, [eax]            ; vtable[0]
//   +17  6a 00                 push 0
//   +19  8b ce                 mov ecx, esi
//   +1b  ff d2                 call edx                  ; node->vtable[0](0)
//   +1d  8d 4f 04              lea ecx, [edi+4]          ; &queue->lock
//   +20  b8 01 00 00 00        mov eax, 1                ; <- spin_loop
//   +25  8b d1                 mov edx, ecx
//   +27  87 02                 xchg [edx], eax
//   +29  85 c0                 test eax, eax
//   +2b  75 f3                 jnz spin_loop             ; retry if was locked
//   +2d  8b 47 0c              mov eax, [edi+0xc]        ; head sentinel
//   +30  8b 50 04              mov edx, [eax+4]          ; head->next
//   +33  89 32                 mov [edx], esi            ; head->next->prev = node
//   +35  8b 50 04              mov edx, [eax+4]          ; reload head->next
//   +38  89 06                 mov [esi], eax            ; node->prev = head
//   +3a  89 56 04              mov [esi+4], edx          ; node->next = old head->next
//   +3d  89 70 04              mov [eax+4], esi          ; head->next = node
//   +40  83 47 18 ff           add [edi+0x18], -1
//   +44  33 c0                 xor eax, eax
//   +46  87 01                 xchg [ecx], eax           ; release lock (store 0)
//   +48  5f                    pop edi
//   +49  5e                    pop esi
//   +4a  c2 04 00              ret 4

extern "C" __declspec(naked) void __stdcall FUN_00410730(void*)
{
    __asm {
        push    esi
        mov     esi, dword ptr [esp + 0x8]
        mov     ecx, dword ptr [esi + 0x14]
        mov     eax, dword ptr [ecx]
        mov     edx, dword ptr [eax + 0x4]
        push    edi
        call    edx
        mov     edi, dword ptr [eax + 0x10]
        mov     eax, dword ptr [esi]
        mov     edx, dword ptr [eax]
        push    0
        mov     ecx, esi
        call    edx
        lea     ecx, [edi + 0x4]
    spin_loop:
        mov     eax, 1
        mov     edx, ecx
        xchg    dword ptr [edx], eax
        test    eax, eax
        jnz     spin_loop
        mov     eax, dword ptr [edi + 0xc]
        mov     edx, dword ptr [eax + 0x4]
        mov     dword ptr [edx], esi
        mov     edx, dword ptr [eax + 0x4]
        mov     dword ptr [esi], eax
        mov     dword ptr [esi + 0x4], edx
        mov     dword ptr [eax + 0x4], esi
        add     dword ptr [edi + 0x18], -1
        xor     eax, eax
        xchg    dword ptr [ecx], eax
        pop     edi
        pop     esi
        ret     4
    }
}
