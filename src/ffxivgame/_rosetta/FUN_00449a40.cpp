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
// FUNCTION: ffxivgame 0x00449a40 — __cdecl bounds-checked DWORD block copy
//                                  (54 bytes / 0x36)
//
// Signature (inferred):
//   void * __cdecl FUN_00449a40(void *dst, unsigned cap, void *src, unsigned cnt)
//
// Stack layout (ESP-relative after CALL):
//   [ESP+0x04]  void     *dst   (param_1: destination buffer pointer)
//   [ESP+0x08]  unsigned  cap   (param_2: destination capacity in DWORDs)
//   [ESP+0x0c]  void     *src   (param_3: source pointer)
//   [ESP+0x10]  unsigned  cnt   (param_4: number of DWORDs to copy)
//
// Logic:
//   if (cap < cnt)  { _invalid_parameter_noinfo(); return NULL; }
//   if (cnt == 0)   { return dst; }
//   // forward DWORD copy via the dst-src base-relative trick:
//   //   ESI = dst - src  (constant throughout loop)
//   //   ECX = src        (advances +4 each iteration)
//   //   [ESI + ECX*1]    always resolves to current dst position
//   return dst;          // EAX = original dst (loaded before the loop)
//
// Calling convention: __cdecl (caller cleans; plain RET).
// Frame: PUSH ESI + PUSH EDI only on the cnt>0 path (lazy callee-save).
//
// Notes:
//   - The bounds-check failure path calls _invalid_parameter_noinfo at
//     VA 0x9d22b4 (MSVC 2005 CRT iterator-debug hook; same target used by
//     sibling FUN_004061a0 etc.).  The REL32 is masked by tools/compare.py.
//   - `MOV [ESI + ECX*1], EDI` (bytes 89 3C 0E) requires the explicit *1
//     scale so MASM places ESI as base and ECX as the scaled index, not
//     the reverse (which would give SIB 0x31 instead of 0x0E).
//   - The loop uses `SUB EDX, 1` (83 EA 01), not `DEC EDX` (4A).

extern "C" void _invalid_parameter_noinfo(void);

extern "C" __declspec(naked) void FUN_00449a40() {
    __asm {
        mov     edx, dword ptr [esp + 0x10]
        cmp     dword ptr [esp + 0x8], edx
        mov     ecx, dword ptr [esp + 0xc]
        jnc     ok1
        call    _invalid_parameter_noinfo
        xor     eax, eax
        ret
    ok1:
        test    edx, edx
        mov     eax, dword ptr [esp + 0x4]
        jbe     done
        push    esi
        mov     esi, eax
        push    edi
        sub     esi, ecx
    loop_top:
        mov     edi, dword ptr [ecx]
        mov     dword ptr [esi + ecx*1], edi
        sub     edx, 1
        add     ecx, 4
        test    edx, edx
        ja      loop_top
        pop     edi
        pop     esi
    done:
        ret
    }
}
