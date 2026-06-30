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
// FUNCTION: ffxivgame 0x0005dd00 — OpenSSL `X509_PUBKEY_get0_param` (63 bytes)
//
// __cdecl int FUN_0045dd00(void **ppkalg, void **pk, int *ppklen, void **pa,
//                           int **pub)
//
// Extracts optional fields from an X509_PUBKEY structure:
//   ppkalg  ← *(*(pub))         [pub->algor->algorithm — double-deref at +0]
//   pk      ← *(*(pub+4) + 8)   [pub->public_key->data  — field at offset 8]
//   ppklen  ← *(*(pub+4))       [pub->public_key->length — first field at +0]
//   pa      ← *(pub)            [pub->algor — single-deref at +0]
//
// Each output pointer is optional; if NULL, the corresponding write is skipped.
// Always returns 1.
//
// Calling convention: __cdecl (5 DWORD stack args; plain RET, no immediate)
// Stack frame: frameless — all args accessed via ESP+N (no push EBP / mov EBP,ESP)
// No external references; no relocations in the 63-byte body.
//
// EAX (pub) is pre-loaded BEFORE the first conditional branch on ppkalg —
// MSVC hoisted the load because pub is consumed by multiple branches.
//
// Asm (63 bytes):
//   8b 4c 24 04     MOV ECX, [ESP+0x04]   ; ppkalg
//   85 c9           TEST ECX, ECX
//   8b 44 24 14     MOV EAX, [ESP+0x14]   ; pub (hoisted)
//   74 06           JZ  +6  (→ skip_alg)
//   8b 10           MOV EDX, [EAX]         ; pub->algor
//   8b 12           MOV EDX, [EDX]         ; algor->algorithm
//   89 11           MOV [ECX], EDX         ; *ppkalg = algor->algorithm
// skip_alg:
//   8b 4c 24 08     MOV ECX, [ESP+0x08]   ; pk
//   85 c9           TEST ECX, ECX
//   74 13           JZ  +0x13 (→ skip_pk)
//   8b 50 04        MOV EDX, [EAX+0x4]    ; pub->public_key
//   8b 52 08        MOV EDX, [EDX+0x8]    ; public_key->data
//   89 11           MOV [ECX], EDX         ; *pk = public_key->data
//   8b 48 04        MOV ECX, [EAX+0x4]    ; pub->public_key (again)
//   8b 11           MOV EDX, [ECX]         ; public_key->length
//   8b 4c 24 0c     MOV ECX, [ESP+0x0c]   ; ppklen
//   89 11           MOV [ECX], EDX         ; *ppklen = public_key->length
// skip_pk:
//   8b 4c 24 10     MOV ECX, [ESP+0x10]   ; pa
//   85 c9           TEST ECX, ECX
//   74 04           JZ  +4  (→ skip_pa)
//   8b 10           MOV EDX, [EAX]         ; pub->algor
//   89 11           MOV [ECX], EDX         ; *pa = pub->algor
// skip_pa:
//   b8 01 00 00 00  MOV EAX, 1
//   c3              RET

extern "C" __declspec(naked) void FUN_0045dd00() {
    __asm {
        mov     ecx, dword ptr [esp + 0x4]
        test    ecx, ecx
        mov     eax, dword ptr [esp + 0x14]
        jz      skip_alg
        mov     edx, dword ptr [eax]
        mov     edx, dword ptr [edx]
        mov     dword ptr [ecx], edx
    skip_alg:
        mov     ecx, dword ptr [esp + 0x8]
        test    ecx, ecx
        jz      skip_pk
        mov     edx, dword ptr [eax + 0x4]
        mov     edx, dword ptr [edx + 0x8]
        mov     dword ptr [ecx], edx
        mov     ecx, dword ptr [eax + 0x4]
        mov     edx, dword ptr [ecx]
        mov     ecx, dword ptr [esp + 0xc]
        mov     dword ptr [ecx], edx
    skip_pk:
        mov     ecx, dword ptr [esp + 0x10]
        test    ecx, ecx
        jz      skip_pa
        mov     edx, dword ptr [eax]
        mov     dword ptr [ecx], edx
    skip_pa:
        mov     eax, 1
        ret
    }
}
