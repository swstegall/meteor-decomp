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
// FUNCTION: ffxivgame 0x0045ba00 — _EVP_add_cipher wrapper (70 B / 0x46)
//
// __cdecl int _EVP_add_cipher(const void *c)
//   One stack arg, caller-cleans, returns int in EAX, plain RET epilogue.
//
// Body (read from the 70-byte .text slice at RVA 0x0005ba00):
//
//   ESI = c (arg)
//   EAX = *ESI  (c->nid — first dword in the EVP_CIPHER struct)
//   r = FUN_00465770(FUN_00464ab0(nid, 2, c), 2, c)
//   if (r == 0) return 0;
//   FUN_00464990(nid);          // called for side-effects; return value discarded
//   nid = *ESI;                 // reload (FUN_00464990 may have modified nothing)
//   r = FUN_00465770(FUN_00464b50(nid, 2, c), 2, c)
//   return r;
//
// Stack interleave pattern (both arms):
//   CALL fn_outer(nid, 2, c)   ; push c, 2, nid in that order
//   ADD ESP, 4                 ; pop only the nid (1 arg)
//   PUSH EAX                   ; push fn_outer's result
//   CALL FUN_00465770          ; call fn_inner(outer_result, 2, c)
//   ADD ESP, 0xC               ; pop remaining 3 args (result, 2, c)
//
// MSVC 2005 register allocation:
//   ESI = c throughout (saved/restored across the function).
//   ECX / EDX are ephemeral scratch for the second arm's nid reload.
//
// Reloc-bearing positions (5-byte CALL rel32 each; masked by compare.py):
//   off 0x0b  →  FUN_00464ab0
//   off 0x14  →  FUN_00465770
//   off 0x25  →  FUN_00464990
//   off 0x33  →  FUN_00464b50
//   off 0x3c  →  FUN_00465770

extern "C" void FUN_00464ab0();
extern "C" void FUN_00465770();
extern "C" void FUN_00464990();
extern "C" void FUN_00464b50();

extern "C" __declspec(naked) void FUN_0045ba00() {
    __asm {
        push    esi
        mov     esi, dword ptr [esp+0x8]
        mov     eax, dword ptr [esi]
        push    esi
        push    0x2
        push    eax
        call    FUN_00464ab0
        add     esp, 0x4
        push    eax
        call    FUN_00465770
        add     esp, 0xc
        test    eax, eax
        jnz     cont
        pop     esi
        ret
    cont:
        mov     ecx, dword ptr [esi]
        push    ecx
        call    FUN_00464990
        mov     edx, dword ptr [esi]
        add     esp, 0x4
        push    esi
        push    0x2
        push    edx
        call    FUN_00464b50
        add     esp, 0x4
        push    eax
        call    FUN_00465770
        add     esp, 0xc
        pop     esi
        ret
    }
}
