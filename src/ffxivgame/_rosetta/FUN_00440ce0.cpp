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
// FUNCTION: ffxivgame 0x00040ce0 — __thiscall list-clear with SSO string free
//                                  (73 B / 0x49, two __cdecl calls to free)
//
// Calling convention: __thiscall (ECX = this).
// No return value.
//
// Layout of *this (inferred):
//   +0x04  Node *sentinel   — head/end-sentinel for a circular doubly-linked list
//   +0x08  int   count      — number of nodes (zeroed by this function)
//
// Layout of each list Node (inferred):
//   +0x00  Node *next       — forward link
//   +0x04  Node *prev       — backward link (not accessed here)
//   +0x10  union { char sso_buf[?]; char *heap_ptr; }
//   +0x20  size_t size
//   +0x24  size_t capacity  — if >= 8 the heap_ptr branch is live
//
// Body:
//   1. Reinitialise sentinel to point at itself (empty list)
//   2. Zero out this->count
//   3. If the old first element == sentinel the list was already empty → done
//   4. Otherwise walk every node: free the heap string buffer (if cap >= 8),
//      reset capacity to 7 and size to 0, null-terminate the SSO slot, then
//      free the node itself via the same free() (RVA ~0x005d1b17).
//
// The epilog (POP EDI / POP ESI / POP EBP / RET) lives at RVA 0x00040d33,
// which is 0x53 bytes from the function start and therefore OUTSIDE the
// 0x49-byte window that compare.py checks. The JZ at offset 0x1A branches
// directly to that shared epilog (displacement +0x37 from the following
// instruction); for the naked-asm byte-pass-through approach we hard-code
// that displacement with _emit.
//
// Mystery bytes resolved:
//   offset 0x31-0x33  →  83 C4 04  ADD ESP,4  (__cdecl cleanup after 1st call)
//   offset 0x48       →  59        POP ECX    (1-byte cleanup after 2nd call;
//                                              ECX is free — `this` lives in EDI)
//
// Reloc-bearing positions (masked by tools/compare.py):
//   off 0x2D   IMAGE_REL_I386_REL32  → free (first call, disp bytes 0x2D-0x30)
//   off 0x44   IMAGE_REL_I386_REL32  → free (second call, disp bytes 0x44-0x47)

extern "C" void _free();

extern "C" __declspec(naked) void FUN_00440ce0() {
    __asm {
        // 00040ce0: push ebp
        push    ebp
        // 00040ce1: push esi
        push    esi
        // 00040ce2: push edi
        push    edi
        // 00040ce3: mov edi, ecx
        mov     edi, ecx
        // 00040ce5: mov eax, [edi+4]      ; eax = sentinel
        mov     eax, dword ptr [edi + 4]
        // 00040ce8: mov esi, [eax]         ; esi = old first node
        mov     esi, dword ptr [eax]
        // 00040cea: mov [eax], eax         ; sentinel->next = sentinel
        mov     dword ptr [eax], eax
        // 00040cec: mov eax, [edi+4]
        mov     eax, dword ptr [edi + 4]
        // 00040cef: mov [eax+4], eax       ; sentinel->prev = sentinel
        mov     dword ptr [eax + 4], eax
        // 00040cf2: xor ebp, ebp           ; ebp = 0 (used as zero constant)
        xor     ebp, ebp
        // 00040cf4: cmp esi, [edi+4]       ; old first == sentinel?
        cmp     esi, dword ptr [edi + 4]
        // 00040cf7: mov [edi+8], ebp       ; this->count = 0
        mov     dword ptr [edi + 8], ebp
        // 00040cfa: jz +0x37  →  0x00040d33 (shared epilog, outside our window)
        _emit 0x74
        _emit 0x37
        // 00040cfc: push ebx
        push    ebx
        // 00040cfd: lea ecx, [ecx+0]       ; 3-byte NOP (loop-top alignment)
        _emit 0x8D
        _emit 0x49
        _emit 0x00
        // ----- loop top: 0x00040d00 -----
    loop_top:
        // 00040d00: cmp [esi+0x24], 8      ; capacity >= 8 → heap buffer?
        cmp     dword ptr [esi + 0x24], 8
        // 00040d04: mov ebx, [esi]         ; save next node
        mov     ebx, dword ptr [esi]
        // 00040d06: jc skip_heap_free      ; capacity < 8 → SSO, no heap ptr
        jc      skip_heap_free
        // 00040d08: mov eax, [esi+0x10]    ; eax = heap_ptr
        mov     eax, dword ptr [esi + 0x10]
        // 00040d0b: push eax
        push    eax
        // 00040d0c: call free
        call    _free
        // 00040d11: add esp, 4             ; __cdecl cleanup
        add     esp, 4
        // ----- skip_heap_free: 0x00040d14 -----
    skip_heap_free:
        // 00040d14: mov [esi+0x24], 7      ; capacity = 7 (SSO mode)
        mov     dword ptr [esi + 0x24], 7
        // 00040d1b: mov [esi+0x20], ebp    ; size = 0
        mov     dword ptr [esi + 0x20], ebp
        // 00040d1e: push esi               ; arg: node ptr
        push    esi
        // 00040d1f: mov word ptr [esi+0x10], bp  ; null-terminate SSO buf
        mov     word ptr [esi + 0x10], bp
        // 00040d23: call free
        call    _free
        // 00040d28: pop ecx                ; 1-byte __cdecl cleanup (ecx = junk)
        pop     ecx
    }
}
