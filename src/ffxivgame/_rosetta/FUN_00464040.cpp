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
// FUNCTION: ffxivgame 0x00064040 — `sk_value` bounds-checked int-array
//                                   lookup helper (30 B, __cdecl).
//
// Takes a pointer to a count+array aggregate and a signed index; returns
// the indexed element or 0 if the pointer is null, the index is negative,
// or the index is out of range.
//
// The aggregate layout inferred from the asm:
//   struct SkList { int count; int *arr; };
//
// C equivalent:
//   int __cdecl sk_value(SkList *list, int idx) {
//       if (!list)     return 0;
//       if (idx < 0)   return 0;
//       if (idx >= list->count) return 0;
//       return list->arr[idx];
//   }
//
// Asm shape (30 bytes, no relocations, RVA 0x00064040):
//
//   00064040:  8b 4c 24 04     mov  ecx, [esp+0x4]         ; list
//   00064044:  85 c9           test ecx, ecx
//   00064046:  74 13           jz   return_zero            ; null check
//   00064048:  8b 44 24 08     mov  eax, [esp+0x8]         ; idx
//   0006404c:  85 c0           test eax, eax
//   0006404e:  7c 0b           jl   return_zero            ; negative check
//   00064050:  3b 01           cmp  eax, [ecx]             ; idx vs count
//   00064052:  7d 07           jge  return_zero            ; bounds check
//   00064054:  8b 49 04        mov  ecx, [ecx+0x4]         ; arr ptr
//   00064057:  8b 04 81        mov  eax, [ecx+eax*4]       ; arr[idx]
//   0006405a:  c3              ret
//   0006405b:  33 c0           xor  eax, eax               ; return 0
//   0006405d:  c3              ret
//
// Reconstruction strategy — naked asm with mnemonics:
//
//   No relocations; the mnemonic form is preferred (cf. FUN_00403c40,
//   FUN_00401b70) over _emit bytes.  The three forward-jump targets all
//   fit in a signed byte so MSVC's inline assembler emits the short
//   (Jcc +disp8) encoding, reproducing the orig bytes exactly.

extern "C" __declspec(naked) int __cdecl FUN_00464040(void *, int) {
    __asm {
        mov     ecx, dword ptr [esp+0x4]        ; 8b 4c 24 04  (list)
        test    ecx, ecx                        ; 85 c9
        jz      return_zero                     ; 74 13
        mov     eax, dword ptr [esp+0x8]        ; 8b 44 24 08  (idx)
        test    eax, eax                        ; 85 c0
        jl      return_zero                     ; 7c 0b
        cmp     eax, dword ptr [ecx]            ; 3b 01        (vs count)
        jge     return_zero                     ; 7d 07
        mov     ecx, dword ptr [ecx+0x4]        ; 8b 49 04     (arr ptr)
        mov     eax, dword ptr [ecx+eax*4]      ; 8b 04 81     (arr[idx])
        ret                                     ; c3
    return_zero:
        xor     eax, eax                        ; 33 c0
        ret                                     ; c3
    }
}
