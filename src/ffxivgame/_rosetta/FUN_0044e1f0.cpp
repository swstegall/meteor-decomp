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
// FUNCTION: ffxivgame 0x0004e1f0 — backward-iterate range and call per-element
//                                  function (__cdecl, 73 B / 0x49)
//
// __cdecl void* FUN_0044e1f0(void* first, void* last, void* dest_end)
//
//   Element stride: 0x54 (84) bytes.
//
//   Computes result = dest_end - (last - first) / 84 * 84, then (if the
//   range is non-empty) iterates backward from last-84 down to first,
//   calling FUN_00447450(__thiscall: this = dest_end+(src-last), arg = src)
//   for each element.  Returns result.
//
// Orig codegen (73 bytes at RVA 0x0004e1f0):
//
//   53                     push  ebx
//   55                     push  ebp
//   56                     push  esi
//   8b 74 24 14            mov   esi, [esp+0x14]        ; esi = last
//   8b 6c 24 18            mov   ebp, [esp+0x18]        ; ebp = dest_end
//   57                     push  edi
//   8b 7c 24 14            mov   edi, [esp+0x14]        ; edi = first (after push edi, sp-4)
//   8b ce                  mov   ecx, esi
//   2b cf                  sub   ecx, edi               ; ecx = last - first (bytes)
//   b8 31 0c c3 30         mov   eax, 0x30c30c31        ; magic for /84
//   f7 e9                  imul  ecx
//   c1 fa 04               sar   edx, 4
//   8b c2                  mov   eax, edx
//   c1 e8 1f               shr   eax, 0x1f
//   03 c2                  add   eax, edx               ; eax = (last-first)/84
//   6b c0 54               imul  eax, eax, 0x54         ; eax = count*84
//   8b dd                  mov   ebx, ebp
//   2b d8                  sub   ebx, eax               ; ebx = dest_end - count*84 (result)
//   3b fe                  cmp   edi, esi
//   74 12                  jz    end_loop
//   2b ee                  sub   ebp, esi               ; ebp = dest_end - last (const offset)
//   83 ee 54               sub   esi, 0x54              ; step back one element
// loop:
//   56                     push  esi                    ; stack arg = src element
//   8d 0c 2e               lea   ecx, [esi+ebp]         ; this = src + (dest_end-last)
//   e8 22 92 ff ff         call  FUN_00447450
//   3b f7                  cmp   esi, edi
//   75 f0                  jnz   loop
// end_loop:
//   5f                     pop   edi
//   5e                     pop   esi
//   5d                     pop   ebp
//   8b c3                  mov   eax, ebx
//   5b                     pop   ebx
//   c3                     ret
//
// Calling convention: __cdecl (3 stack args, caller-cleans).
// Reloc-bearing sites:
//   off 0x39  IMAGE_REL_I386_REL32  → FUN_00447450 (CALL rel32)

extern "C" void FUN_00447450();

extern "C" __declspec(naked) void FUN_0044e1f0() {
    __asm {
        push    ebx
        push    ebp
        push    esi
        mov     esi, dword ptr [esp+0x14]
        mov     ebp, dword ptr [esp+0x18]
        push    edi
        mov     edi, dword ptr [esp+0x14]
        mov     ecx, esi
        sub     ecx, edi
        mov     eax, 0x30c30c31
        imul    ecx
        sar     edx, 4
        mov     eax, edx
        shr     eax, 0x1f
        add     eax, edx
        imul    eax, eax, 0x54
        mov     ebx, ebp
        sub     ebx, eax
        cmp     edi, esi
        jz      end_loop
        sub     ebp, esi
    loop_top:
        sub     esi, 0x54
        push    esi
        lea     ecx, [esi+ebp]
        call    FUN_00447450
        cmp     esi, edi
        jnz     loop_top
    end_loop:
        pop     edi
        pop     esi
        pop     ebp
        mov     eax, ebx
        pop     ebx
        ret
    }
}
