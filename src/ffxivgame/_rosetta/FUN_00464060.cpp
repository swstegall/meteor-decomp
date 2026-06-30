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
// FUNCTION: ffxivgame 0x00064060 — `sk_set` bounds-checked array store
//                                   helper (34 B, __cdecl).
//
// Part of the embedded OpenSSL STACK API (cf. sk_num @0x00064030,
// sk_value @0x00064040).  Takes a pointer to an int-count/ptr-data
// aggregate, a signed index, and a value; bounds-checks the pointer
// and index, stores the value at data[i], then returns the value.
// Returns 0 / NULL on any validation failure.
//
// Aggregate layout:
//   struct SkList { int count; void **data; };
//
// C equivalent:
//   void *sk_set(SkList *st, int i, void *value) {
//       if (!st)             return NULL;
//       if (i < 0)           return NULL;
//       if (i >= st->count)  return NULL;
//       st->data[i] = value;
//       return value;
//   }
//
// Asm shape (34 bytes, no relocations, RVA 0x00064060):
//
//   00064060:  8b 44 24 04     mov  eax, [esp+0x4]          ; st
//   00064064:  85 c0           test eax, eax
//   00064066:  74 17           jz   return_null              ; null check
//   00064068:  8b 4c 24 08     mov  ecx, [esp+0x8]          ; i
//   0006406c:  85 c9           test ecx, ecx
//   0006406e:  7c 0f           jl   return_null              ; negative check
//   00064070:  3b 08           cmp  ecx, [eax]              ; i vs st->count
//   00064072:  7d 0b           jge  return_null              ; bounds check
//   00064074:  8b 50 04        mov  edx, [eax+0x4]          ; st->data
//   00064077:  8b 44 24 0c     mov  eax, [esp+0xc]          ; value
//   0006407b:  89 04 8a        mov  [edx+ecx*4], eax        ; st->data[i] = value
//   0006407e:  c3              ret                           ; return value (in eax)
//   0006407f:  33 c0           xor  eax, eax                ; return_null:
//   00064081:  c3              ret
//
// Reconstruction strategy — naked asm with mnemonics:
//
//   No relocations; the mnemonic form reproduces the orig bytes exactly.
//   The three forward-jump targets all fit in a signed byte so MSVC's
//   inline assembler emits the short (Jcc +disp8) encoding.

extern "C" __declspec(naked) void * __cdecl FUN_00464060(void *, int, void *) {
    __asm {
        mov     eax, dword ptr [esp+0x4]        // 8b 44 24 04  (st)
        test    eax, eax                        // 85 c0
        jz      return_null                     // 74 17
        mov     ecx, dword ptr [esp+0x8]        // 8b 4c 24 08  (i)
        test    ecx, ecx                        // 85 c9
        jl      return_null                     // 7c 0f
        cmp     ecx, dword ptr [eax]            // 3b 08        (i vs st->count)
        jge     return_null                     // 7d 0b
        mov     edx, dword ptr [eax+0x4]        // 8b 50 04     (st->data)
        mov     eax, dword ptr [esp+0xc]        // 8b 44 24 0c  (value)
        mov     dword ptr [edx+ecx*4], eax      // 89 04 8a     (st->data[i] = value)
        ret                                     // c3
    return_null:
        xor     eax, eax                        // 33 c0
        ret                                     // c3
    }
}
