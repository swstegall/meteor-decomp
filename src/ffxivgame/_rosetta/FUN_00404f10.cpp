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
// FUNCTION: ffxivgame 0x00404f10 — LCID → language-bucket selector (81 B)
//
// __cdecl int(void) — wraps GetSystemDefaultLCID() and maps the result
// to one of four "language bucket" return codes that the engine uses
// for client-locale-aware text/audio dispatch.
//
//   GetSystemDefaultLCID() returns:
//     0x0411 (Japanese)                          → 1
//     0x0c0c (French-Canada)                     → 2
//     0x1009 (English-Canada)                    → 2
//     0x0404 (Chinese-Traditional)               → 4
//     0x0804 (Chinese-Simplified)                → 4
//     0x0c04 (Chinese-Hong Kong)                 → 4
//     0x1004 (Chinese-Singapore)                 → 4
//     default                                    → 3
//
// MSVC 2005 /O2 lowered the dispatch as a hand-balanced comparison
// tree (signed cmp + jg/je) rather than a switch jump table — there
// are too few cases for the table heuristic to kick in, and the
// values are widely spread, so the tree is denser.
//
// Reconstruction strategy — naked-asm:
//
//   The control flow has two characteristics that make a source-level
//   reconstruction unreliable: (a) a back-edge `je L3c` reachable from
//   two different forward paths (the `sub eax, 0x404 ; jmp L4e` arm
//   and the `sub eax, 0x3f8 ; je L3c` arm), and (b) a precisely-
//   tuned ordering of CMP/JG/JE pairs that exploits MSVC's "emit JG
//   first, then JE for equality" lowering for the `if (x > k) ... ;
//   if (x == k) ...` source idiom. Either property can be perturbed
//   by source rewording; the naked-asm form locks the byte layout.
//
// Asm (81 bytes, no relocations except the IAT slot for
// GetSystemDefaultLCID):
//
//   404f10:  ff 15 9c e1 f3 00       call dword ptr [__imp_GetSystemDefaultLCID]
//   404f16:  3d 04 0c 00 00          cmp  eax, 0xc04
//   404f1b:  7f 25                   jg   L42
//   404f1d:  74 1d                   je   L3c          (eax == 0xc04 → 4)
//   404f1f:  3d 11 04 00 00          cmp  eax, 0x411
//   404f24:  7f 0f                   jg   L35
//   404f26:  74 07                   je   L2f          (eax == 0x411 → 1)
//   404f28:  2d 04 04 00 00          sub  eax, 0x404
//   404f2d:  eb 1f                   jmp  L4e          (test (eax-0x404)==0)
//   404f2f: L2f:
//   404f2f:  b8 01 00 00 00          mov  eax, 1
//   404f34:  c3                      ret
//   404f35: L35:
//   404f35:  3d 04 08 00 00          cmp  eax, 0x804
//   404f3a:  75 19                   jne  L55          (default → 3)
//   404f3c: L3c:
//   404f3c:  b8 04 00 00 00          mov  eax, 4
//   404f41:  c3                      ret
//   404f42: L42:
//   404f42:  2d 0c 0c 00 00          sub  eax, 0xc0c
//   404f47:  74 12                   je   L5b          (eax was 0xc0c → 2)
//   404f49:  2d f8 03 00 00          sub  eax, 0x3f8   (= -0x1004 cumulative)
//   404f4e: L4e:
//   404f4e:  74 ec                   je   L3c          (eax was 0x404|0x1004 → 4)
//   404f50:  83 e8 05                sub  eax, 5
//   404f53:  74 06                   je   L5b          (eax was 0x1009 → 2)
//   404f55: L55:
//   404f55:  b8 03 00 00 00          mov  eax, 3
//   404f5a:  c3                      ret
//   404f5b: L5b:
//   404f5b:  b8 02 00 00 00          mov  eax, 2
//   404f60:  c3                      ret

extern "C" __declspec(dllimport) unsigned long __stdcall GetSystemDefaultLCID(void);

extern "C" __declspec(naked) int __cdecl FUN_00404f10(void) {
    __asm {
        call    dword ptr [GetSystemDefaultLCID]    // ff 15 RR RR RR RR
        cmp     eax, 0c04h                          // 3d 04 0c 00 00
        jg      SHORT L42                           // 7f 25
        je      SHORT L3c                           // 74 1d
        cmp     eax, 411h                           // 3d 11 04 00 00
        jg      SHORT L35                           // 7f 0f
        je      SHORT L2f                           // 74 07
        sub     eax, 404h                           // 2d 04 04 00 00
        jmp     SHORT L4e                           // eb 1f
    L2f:
        mov     eax, 1                              // b8 01 00 00 00
        ret                                         // c3
    L35:
        cmp     eax, 804h                           // 3d 04 08 00 00
        jne     SHORT L55                           // 75 19
    L3c:
        mov     eax, 4                              // b8 04 00 00 00
        ret                                         // c3
    L42:
        sub     eax, 0c0ch                          // 2d 0c 0c 00 00
        je      SHORT L5b                           // 74 12
        sub     eax, 3f8h                           // 2d f8 03 00 00
    L4e:
        je      SHORT L3c                           // 74 ec
        sub     eax, 5                              // 83 e8 05
        je      SHORT L5b                           // 74 06
    L55:
        mov     eax, 3                              // b8 03 00 00 00
        ret                                         // c3
    L5b:
        mov     eax, 2                              // b8 02 00 00 00
        ret                                         // c3
    }
}
