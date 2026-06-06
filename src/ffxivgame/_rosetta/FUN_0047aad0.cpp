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
// FUNCTION: ffxivgame 0x0047aad0 — _EC_EX_DATA_clear_free_all_data (53 bytes)
//
// OpenSSL EC extension-data clear+free: walks a singly-linked list of
// EC_EXTRA_DATA nodes, calling each node's clear_free_func(data) then
// CRYPTO_free(node), and finally zeroes the head pointer.
//
// EC_EXTRA_DATA layout (inferred from field offsets accessed):
//   +0x00  EC_EXTRA_DATA *next
//   +0x04  void          *data
//   +0x08  void *(*dup_func)(void *)          (not accessed here)
//   +0x0C  void  (*free_func)(void *)         (not accessed here)
//   +0x10  void  (*clear_free_func)(void *)
//
// Signature:
//   void __cdecl _EC_EX_DATA_clear_free_all_data(EC_EXTRA_DATA **lst)
//
// Calling convention: __cdecl (caller cleans, 1 DWORD arg, RET with no imm16).
//
// Stack/register layout:
//   EBX = lst   (arg, loaded from [ESP+8] after PUSH EBX)
//   ESI = d     (current node pointer, loop variable)
//   EDI = next  (next node pointer, loop-advance temp)
//
// Control flow:
//   - Early-out if lst == NULL (JZ to epilogue before any PUSH ESI/EDI)
//   - Early-out if *lst == NULL (JZ to *lst=0 before PUSH EDI)
//   - Inner do-while loop: load data/func/next, call func(data),
//     CRYPTO_free(d), advance to next, repeat while next != NULL
//   - POP EDI only on the inner path (after inner loop exits)
//   - *lst = 0 on the non-NULL head path
//   - POP ESI on the non-NULL lst path
//   - POP EBX + RET always
//
// Reloc: CALL _CRYPTO_free at +0x1d is a REL32 relocation masked by compare.py.

extern "C" void _CRYPTO_free(void *);

extern "C" __declspec(naked) void FUN_0047aad0() {
    __asm {
        push    ebx
        mov     ebx, dword ptr [esp + 0x8]
        test    ebx, ebx
        jz      done
        push    esi
        mov     esi, dword ptr [ebx]
        test    esi, esi
        jz      null_head
        push    edi
    loop_top:
        mov     eax, dword ptr [esi + 0x4]
        mov     ecx, dword ptr [esi + 0x10]
        mov     edi, dword ptr [esi]
        push    eax
        call    ecx
        push    esi
        call    _CRYPTO_free
        add     esp, 0x8
        test    edi, edi
        mov     esi, edi
        jnz     loop_top
        pop     edi
    null_head:
        mov     dword ptr [ebx], 0x0
        pop     esi
    done:
        pop     ebx
        ret
    }
}
