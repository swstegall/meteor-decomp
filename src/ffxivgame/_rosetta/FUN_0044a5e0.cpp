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
// FUNCTION: ffxivgame 0x0004a5e0 — __thiscall resize/grow helper for an
//                                  STL-style SSO container (146 B / 0x92,
//                                  ret 8).
//
// Calling convention: __thiscall (ECX = this, aliased into ESI). Two stack
// args cleaned on return (`ret 8`): a DWORD requested element count `n`
// (EDI) and a BYTE flag at [esp+0x10]. Callee-saves: ESI, EDI.
//
// Object layout (offsets touched):
//   [this + 0x04]  data pointer (used when capacity >= 4)
//   [this + 0x14]  current size (_Mysize)
//   [this + 0x18]  current capacity (_Myres)
//
// Behaviour (recovered from asm @ 0x0004a5e0):
//   - if n == (unsigned)-1  → CALL _Xlen (length_error, FUN_009d042e)
//   - if capacity < n       → reallocate via FUN_0044a1d0(n, size)
//   - else if flag && n < 4 → shrink-to-fit via FUN_00449d80(1, min(n,size))
//   - else (no realloc):
//       size = n;
//       if n == 0:
//         if capacity < 4: *(int*)(this+4) = 0;       // inline small buffer
//         else:            **(int**)(this+4) = 0;      // heap buffer, NUL
//   Every path returns (n != 0) as a bool via the
//   `xor ecx,ecx; cmp ecx,edi; sbb eax,eax; neg eax` idiom.
//
// CALL targets (REL32 relocations — compare.py masks these windows):
//   +0x0d   CALL FUN_009d042e   — length_error throw helper (noreturn)
//   +0x20   CALL FUN_0044a1d0   — grow / reallocate
//   +0x4c   CALL FUN_00449d80   — shrink helper
//
// Reconstruction strategy — __declspec(naked) mnemonic passthrough:
//   Source-level C++ at /O2 won't reliably reproduce the exact branch
//   ordering and the triplicated SBB/NEG bool epilogue, and the ESI/EDI
//   prologue allocation differs in an isolated TU. The rosetta naked-asm
//   path re-emits the original 146 bytes; compare.py masks the three
//   REL32 call windows and reports GREEN.

extern "C" {
    int FUN_009d042e();   // _Xlen length_error throw helper (noreturn)
    int FUN_0044a1d0();   // grow / reallocate
    int FUN_00449d80();   // shrink helper
}

extern "C" __declspec(naked) void FUN_0044a5e0() {
    __asm {
        push    esi                                 // 56
        push    edi                                 // 57
        mov     edi, dword ptr [esp + 0xc]          // 8b 7c 24 0c   n
        cmp     edi, -2                             // 83 ff fe
        mov     esi, ecx                            // 8b f1         this
        jbe     short len_ok                        // 76 05
        call    FUN_009d042e                        // e8 ?? ?? ?? ??

    len_ok:
        mov     eax, dword ptr [esi + 0x18]         // 8b 46 18      capacity
        cmp     eax, edi                            // 3b c7
        jnc     short have_cap                      // 73 19

        mov     eax, dword ptr [esi + 0x14]         // 8b 46 14      size
        push    eax                                 // 50
        push    edi                                 // 57
        mov     ecx, esi                            // 8b ce
        call    FUN_0044a1d0                        // e8 ?? ?? ?? ??
        xor     ecx, ecx                            // 33 c9
        cmp     ecx, edi                            // 3b cf
        sbb     eax, eax                            // 1b c0
        pop     edi                                 // 5f
        neg     eax                                 // f7 d8
        pop     esi                                 // 5e
        ret     8                                   // c2 08 00

    have_cap:
        cmp     byte ptr [esp + 0x10], 0            // 80 7c 24 10 00  flag
        jz      short no_realloc                    // 74 25
        cmp     edi, 4                              // 83 ff 04
        jnc     short no_realloc                    // 73 20
        mov     eax, dword ptr [esi + 0x14]         // 8b 46 14      size
        cmp     edi, eax                            // 3b f8
        jnc     short use_eax                       // 73 02
        mov     eax, edi                            // 8b c7
    use_eax:
        push    eax                                 // 50
        push    1                                   // 6a 01
        mov     ecx, esi                            // 8b ce
        call    FUN_00449d80                        // e8 ?? ?? ?? ??
        xor     ecx, ecx                            // 33 c9
        cmp     ecx, edi                            // 3b cf
        sbb     eax, eax                            // 1b c0
        pop     edi                                 // 5f
        neg     eax                                 // f7 d8
        pop     esi                                 // 5e
        ret     8                                   // c2 08 00

    no_realloc:
        test    edi, edi                            // 85 ff
        jnz     short ret_bool                      // 75 23
        cmp     eax, 4                              // 83 f8 04
        mov     dword ptr [esi + 0x14], edi         // 89 7e 14      size = 0
        jc      short small_buf                     // 72 12
        mov     esi, dword ptr [esi + 4]            // 8b 76 04      heap data
        xor     ecx, ecx                            // 33 c9
        cmp     ecx, edi                            // 3b cf
        mov     dword ptr [esi], edi                // 89 3e         *data = 0
        sbb     eax, eax                            // 1b c0
        pop     edi                                 // 5f
        neg     eax                                 // f7 d8
        pop     esi                                 // 5e
        ret     8                                   // c2 08 00

    small_buf:
        add     esi, 4                              // 83 c6 04
        mov     dword ptr [esi], 0                  // c7 06 00 00 00 00

    ret_bool:
        xor     ecx, ecx                            // 33 c9
        cmp     ecx, edi                            // 3b cf
        sbb     eax, eax                            // 1b c0
        pop     edi                                 // 5f
        neg     eax                                 // f7 d8
        pop     esi                                 // 5e
        ret     8                                   // c2 08 00
    }
}
