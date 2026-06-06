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
// FUNCTION: ffxivgame 0x4b350 — packed-id validity probe (__cdecl, 78 B).
//
// __cdecl bool FUN_0044b350(unsigned int id);
//
// Fast-out: a global "always-valid" override byte at 0x01266b64; when set,
// every id is reported valid (return 1) without further work.
//
// Otherwise the 32-bit `id` is split: the high word selects a descriptor via
// FUN_0044b2a0(id >> 16). On a null descriptor → invalid (0). The low word is
// range-checked against the descriptor's count field at +0xa; if it is out of
// range → invalid (0). Then a manager singleton at 0x0132cb8c is consulted —
// `mgr->vtbl->method8(descriptor->base[+0x4] + lowWord)` — and the returned
// slot's first dword is tested for non-null; non-null → valid (1).
//
// Pseudo-source (logical structure):
//
//   bool FUN_0044b350(unsigned int id) {
//       if (g_alwaysValid) return true;
//       Desc* d = FUN_0044b2a0(id >> 16);
//       if (!d) return false;
//       unsigned int low = id & 0xffff;
//       if (low >= d->count) return false;
//       Mgr* m = *(Mgr**)0x0132cb8c;
//       void** slot = m->vtbl->method8(d->base + low);
//       return slot[0] != 0;
//   }
//
// Naked asm: two DIR32 globals (override byte, manager ptr), one REL32 call
// (FUN_0044b2a0), and an indirect `call edx` through the manager's vtable.
// The interleaved register schedule (mov esi,[ecx] mid-sequence, edx reused
// as both the zero-extended low word and the call target) is MSVC 2005's
// exact /O2 choice; compare.py reloc-masks the 4-byte DIR32/REL32 windows.
//
// Reloc-bearing sites:
//   +0x02  CMP byte ptr [data_01266b64], 0   (dir32, override flag)
//   +0x17  CALL FUN_0044b2a0                  (rel32)
//   +0x30  MOV ECX, [data_0132cb8c]           (dir32, manager singleton)

extern "C" {
    // .text — RVA 0x0044b2a0. Descriptor lookup, __cdecl(highWord) -> Desc*.
    int FUN_0044b2a0();
    // .data — global "always valid" override byte.
    extern char data_01266b64;
    // .data — manager singleton pointer.
    extern int data_0132cb8c;
}

extern "C" __declspec(naked) char FUN_0044b350() {
    __asm {
        cmp     byte ptr [data_01266b64], 0
        jz      probe
        mov     al, 1
        ret
    probe:
        push    esi
        mov     esi, dword ptr [esp + 8]
        mov     eax, esi
        shr     eax, 0x10
        push    eax
        call    FUN_0044b2a0
        add     esp, 4
        test    eax, eax
        jz      fail
        movzx   ecx, word ptr [eax + 0xa]
        movzx   edx, si
        cmp     edx, ecx
        jae     fail
        mov     ecx, dword ptr [data_0132cb8c]
        mov     eax, dword ptr [eax + 4]
        mov     esi, dword ptr [ecx]
        add     eax, edx
        mov     edx, dword ptr [esi + 8]
        push    eax
        call    edx
        cmp     dword ptr [eax], 0
        jz      fail
        mov     al, 1
        pop     esi
        ret
    fail:
        xor     al, al
        pop     esi
        ret
    }
}
