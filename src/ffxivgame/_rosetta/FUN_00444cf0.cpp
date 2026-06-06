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
// FUNCTION: ffxivgame 0x00044cf0 — uninitialized_copy of a 0x54-stride range
//                                  (__cdecl, 73 B / 0x49)
//
// __cdecl T* FUN_00444cf0(T *first, T *last, T *dest)
//
//   const int N = (last - first) / 0x54;          // element count
//   T *result_end = dest + N * 0x54;               // EDI — returned
//   for (T *src = first; src != last; src += 0x54) // copy-construct loop
//       FUN_00447450(/*this=*/ dest + (src - first), src);
//   return result_end;
//
// Each element is 0x54 (84) bytes. The (last - first) byte span is divided
// by 0x54 via the signed magic constant 0x30c30c31 (`imul`, `sar 4`, plus
// the sign-bit fixup) to recover the element count; that count is multiplied
// back by 0x54 and added to `dest` to form the returned end pointer (EDI).
// The walk copy-constructs each destination element from its corresponding
// source via the __thiscall helper FUN_00447450 (ECX = destination element,
// source pushed as the single stack arg).
//
// Calling convention: __cdecl, 3 stack args, caller-cleans, returns EAX.
// Stack frame: PUSH EBX / EBP / ESI bracket the body, with PUSH EDI nested
// after the count computation — typical MSVC 2005 lazy callee-save spill.
//
// Reloc-bearing site: the CALL rel32 at off 0x36 (→ FUN_00447450). The .obj
// emits a zeroed rel32 there; tools/compare.py masks the 4-byte reloc.
//
// Reconstruction strategy: __declspec(naked) byte-for-byte passthrough,
// following the established sibling idiom (FUN_004086a0, FUN_00403d10).

extern "C" void FUN_00447450();

extern "C" __declspec(naked) void FUN_00444cf0() {
    __asm {
        push    ebx
        mov     ebx, dword ptr [esp+0xc]
        push    ebp
        push    esi
        mov     esi, dword ptr [esp+0x10]
        mov     ecx, ebx
        sub     ecx, esi
        mov     eax, 0x30c30c31
        imul    ecx
        mov     ebp, dword ptr [esp+0x18]
        sar     edx, 4
        mov     eax, edx
        shr     eax, 0x1f
        push    edi
        add     eax, edx
        mov     edi, eax
        imul    edi, edi, 0x54
        add     edi, ebp
        cmp     esi, ebx
        jz      done
        sub     ebp, esi
    loop_top:
        push    esi
        lea     ecx, [esi + ebp]
        call    FUN_00447450
        add     esi, 0x54
        cmp     esi, ebx
        jnz     loop_top
    done:
        mov     eax, edi
        pop     edi
        pop     esi
        pop     ebp
        pop     ebx
        ret
    }
}
