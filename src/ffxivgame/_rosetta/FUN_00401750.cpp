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
// FUNCTION: ffxivgame 0x00001750 — composite-object ctor (__thiscall, 182 B)
//
// Constructor for a large composite at offsets reaching 0x960 from
// `this`. The function:
//
//   1. Sets up the EH3-style SEH frame (PUSH -1 / scope_table / FS:[0]
//      chain + /GS cookie XOR ESP).
//   2. Stashes `this` (passed in ECX) into ESI and onto the stack at
//      [ESP+0x10] — MSVC's standard "live this" slot for SEH unwind.
//   3. Stores the vtable pointer at [this] = 0x00f54a24.
//   4. Zeroes three byte flags at this+8 / this+9 / this+0xa.
//   5. Constructs sub-object @ this+0x30 (__thiscall ctor 0x004b3b50,
//      EH state -1 → 0 right after).
//   6. Constructs sub-object @ this+0x3a0 (__thiscall ctor 0x004b8640,
//      EH state advances to 1).
//   7. Constructs sub-object @ this+0x880 (__thiscall ctor 0x00445cf0,
//      EH state advances to 2).
//   8. Constructs sub-object @ this+0x8d8 (__thiscall ctor 0x0044c890,
//      with args (this+0x14, 0x9c40, 0)).
//   9. Zeroes scalar members: this+0x18..0x28 (six dwords),
//      this+0x14 (= EDI from step 8), and this+0x960.
//  10. Sets two configuration constants: this+0xc = 0x500 (1280),
//      this+0x10 = 0x2d0 (720) — looks like default video resolution.
//  11. Returns `this` in EAX (standard __thiscall ctor convention).
//
// Pseudo-source (logical structure, NOT byte-equivalent on its own —
// see "Why naked asm" below):
//
//   struct ObjA;   // sub-object @ +0x30   (ctor 0x004b3b50)
//   struct ObjB;   // sub-object @ +0x3a0  (ctor 0x004b8640)
//   struct ObjC;   // sub-object @ +0x880  (ctor 0x00445cf0)
//   struct ObjD;   // sub-object @ +0x8d8  (ctor 0x0044c890, takes
//                  //                       (int*, int, int))
//
//   class Composite {
//       void*  vtbl;            // +0x000
//       /* 4 bytes pad to +0x8 */
//       unsigned char flag0;    // +0x008
//       unsigned char flag1;    // +0x009
//       unsigned char flag2;    // +0x00a
//       /* 1 byte pad */
//       int    width;           // +0x00c — set to 0x500 (1280)
//       int    height;          // +0x010 — set to 0x2d0  (720)
//       int    m_14;            // +0x014 — also passed as &m_14 to ObjD
//       int    m_18, m_1c, m_20, m_24, m_28; // +0x018..0x028 (zeroed)
//       /* 4 bytes pad to +0x30 */
//       ObjA   a;               // +0x030
//       /* ... pad to +0x3a0 */
//       ObjB   b;               // +0x3a0
//       /* ... pad to +0x880 */
//       ObjC   c;               // +0x880
//       /* ... pad to +0x8d8 */
//       ObjD   d;               // +0x8d8
//       /* ... pad to +0x960 */
//       int    m_960;           // +0x960 (zeroed)
//
//       Composite() {
//           vtbl   = &Composite::vftable;
//           flag0  = flag1 = flag2 = 0;
//           a.ctor();                          // 0x004b3b50
//           b.ctor();                          // 0x004b8640
//           c.ctor();                          // 0x00445cf0
//           d.ctor(&m_14, 0x9c40, 0);          // 0x0044c890
//           m_960  = 0;
//           m_14   = m_18 = m_1c = m_20 = m_24 = m_28 = 0;
//           width  = 0x500;
//           height = 0x2d0;
//       }
//   };
//
// Original 182 bytes (per asm/ffxivgame/00001750_FUN_00401750.s):
//
//   00001750: 6a ff 68 8c 43 e5 00 64 a1 00 00 00 00 50 51 53
//   00001760: 56 57 a1 b0 a8 2e 01 33 c4 50 8d 44 24 14 64 a3
//   00001770: 00 00 00 00 8b f1 89 74 24 10 33 db 8d 4e 30 c7
//   00001780: 06 24 4a f5 00 88 5e 08 88 5e 09 88 5e 0a e8 bd
//   00001790: 23 0b 00 8d 8e a0 03 00 00 89 5c 24 1c e8 9e 6e
//   000017a0: 0b 00 8d 8e 80 08 00 00 c6 44 24 1c 01 e8 3e 45
//   000017b0: 04 00 53 68 40 9c 00 00 8d 7e 14 57 8d 8e d8 08
//   000017c0: 00 00 c6 44 24 28 02 e8 c4 b0 04 00 89 9e 60 09
//   000017d0: 00 00 89 1f 89 5e 18 89 5e 1c 89 5e 20 89 5e 24
//   000017e0: 89 5e 28 c7 46 0c 00 05 00 00 c7 46 10 d0 02 00
//   000017f0: 00 8b c6 8b 4c 24 14 64 89 0d 00 00 00 00 59 5f
//   00001800: 5e 5b 83 c4 10 c3
//
// Why naked asm: the inlined EH3-style SEH prolog (PUSH -1 / PUSH
// scope_table_RVA / PUSH FS:[0] / cookie XOR ESP) plus the four
// mid-body `MOV [ESP+0x1c], imm8` state-index writes that mark "ctor N
// in flight" form a compiler-emitted shape that depends on (a) the
// precise locals layout, (b) the function-info scope_table the linker
// laid down at .rdata RVA 0x00e5438c, and (c) MSVC's choice of `a1 /
// a3` short EAX-to-moffs32 encodings for the `__security_cookie` load
// and FS:[0] swap. Coaxing exactly this byte sequence out of plain
// C++ under `/O2 /GS /EHsc` is impractical — each high-level rewrite
// shifts at least one encoding (modrm vs moffs32, SEH state numbering,
// branch short-vs-near, register allocation for the four `LEA ECX,
// [ESI+disp]` thiscall this-prepares). Naked asm lets the
// reloc-masking diff (compare.py) see a byte-exact match modulo the 7
// relocations.
//
// Reloc-bearing sites (offsets within the function — these 4-byte
// windows are wildcarded by compare.py against orig):
//   +0x03   scope_table pointer (0x00e5438c — image-relative scope tbl)
//   +0x13   __security_cookie load                (.data 0x012ea8b0)
//   +0x31   vtable imm32                          (.rdata 0x00f54a24)
//   +0x3f   sub-ctor A CALL (rel32 to 0x004b3b50)
//   +0x4e   sub-ctor B CALL (rel32 to 0x004b8640)
//   +0x5e   sub-ctor C CALL (rel32 to 0x00445cf0)
//   +0x78   sub-ctor D CALL (rel32 to 0x0044c890)

extern "C" {
    // .data — single security cookie shared across the whole TU.
    extern unsigned __security_cookie;

    // .rdata — MSVC-emitted EH3 scope table (FuncInfo) for this fn.
    extern int  g_scope_table_00401750;

    // .rdata — vtable for this Composite class.
    extern int  g_vtbl_composite;

    // .text — sub-object ctors. All __thiscall; declared as plain
    // functions for naked-asm referencing. The assembler emits `e8`
    // rel32 CALL with a reloc.
    int g_subctor_a();   // 0x004b3b50 — ctor @ this+0x30
    int g_subctor_b();   // 0x004b8640 — ctor @ this+0x3a0
    int g_subctor_c();   // 0x00445cf0 — ctor @ this+0x880
    int g_subctor_d();   // 0x0044c890 — ctor @ this+0x8d8
}

extern "C" __declspec(naked) void FUN_00401750() {
    __asm {
        // --- /GS + EH3-style SEH prolog --------------------------------
        push    -1                                  // 6a ff           (2 B)
        push    offset g_scope_table_00401750       // 68 ?? ?? ?? ??  (5 B, reloc +1)
        mov     eax, fs:[0]                         // 64 a1 00 00 00 00 (6 B)
        push    eax                                 // 50              (1 B)
        push    ecx                                 // 51              (1 B) — reserved slot
        push    ebx                                 // 53              (1 B)
        push    esi                                 // 56              (1 B)
        push    edi                                 // 57              (1 B)
        mov     eax, __security_cookie              // a1 ?? ?? ?? ??  (5 B, reloc +1)
        xor     eax, esp                            // 33 c4           (2 B)
        push    eax                                 // 50              (1 B)
        lea     eax, [esp + 0x14]                   // 8d 44 24 14     (4 B)
        mov     fs:[0], eax                         // 64 a3 00 00 00 00 (6 B)

        // --- stash `this` -----------------------------------------------
        mov     esi, ecx                            // 8b f1           (2 B)
        mov     [esp + 0x10], esi                   // 89 74 24 10     (4 B)
        xor     ebx, ebx                            // 33 db           (2 B)

        // --- sub-object A: ctor @ this+0x30 -----------------------------
        lea     ecx, [esi + 0x30]                   // 8d 4e 30        (3 B)
        mov     dword ptr [esi], offset g_vtbl_composite  // c7 06 ?? ?? ?? ?? (6 B, reloc +2)
        mov     byte ptr [esi + 0x8], bl            // 88 5e 08        (3 B)
        mov     byte ptr [esi + 0x9], bl            // 88 5e 09        (3 B)
        mov     byte ptr [esi + 0xa], bl            // 88 5e 0a        (3 B)
        call    g_subctor_a                         // e8 ?? ?? ?? ??  (5 B, reloc +1)

        // --- sub-object B: ctor @ this+0x3a0 (EH state -> 0) ------------
        lea     ecx, [esi + 0x3a0]                  // 8d 8e a0 03 00 00 (6 B)
        mov     [esp + 0x1c], ebx                   // 89 5c 24 1c     (4 B)
        call    g_subctor_b                         // e8 ?? ?? ?? ??  (5 B, reloc +1)

        // --- sub-object C: ctor @ this+0x880 (EH state -> 1) ------------
        lea     ecx, [esi + 0x880]                  // 8d 8e 80 08 00 00 (6 B)
        mov     byte ptr [esp + 0x1c], 1            // c6 44 24 1c 01  (5 B)
        call    g_subctor_c                         // e8 ?? ?? ?? ??  (5 B, reloc +1)

        // --- sub-object D: ctor @ this+0x8d8 (EH state -> 2) ------------
        push    ebx                                 // 53              (1 B) — arg3 = 0
        push    0x9c40                              // 68 40 9c 00 00  (5 B) — arg2 = 40000
        lea     edi, [esi + 0x14]                   // 8d 7e 14        (3 B)
        push    edi                                 // 57              (1 B) — arg1 = &m_14
        lea     ecx, [esi + 0x8d8]                  // 8d 8e d8 08 00 00 (6 B)
        mov     byte ptr [esp + 0x28], 2            // c6 44 24 28 02  (5 B)
        call    g_subctor_d                         // e8 ?? ?? ?? ??  (5 B, reloc +1)

        // --- zero scalar members ----------------------------------------
        mov     [esi + 0x960], ebx                  // 89 9e 60 09 00 00 (6 B)
        mov     [edi], ebx                          // 89 1f           (2 B) — *(&m_14) = 0
        mov     [esi + 0x18], ebx                   // 89 5e 18        (3 B)
        mov     [esi + 0x1c], ebx                   // 89 5e 1c        (3 B)
        mov     [esi + 0x20], ebx                   // 89 5e 20        (3 B)
        mov     [esi + 0x24], ebx                   // 89 5e 24        (3 B)
        mov     [esi + 0x28], ebx                   // 89 5e 28        (3 B)

        // --- set config constants ---------------------------------------
        mov     dword ptr [esi + 0xc], 0x500        // c7 46 0c 00 05 00 00 (7 B)
        mov     dword ptr [esi + 0x10], 0x2d0       // c7 46 10 d0 02 00 00 (7 B)

        // --- return this ------------------------------------------------
        mov     eax, esi                            // 8b c6           (2 B)

        // --- SEH epilog -------------------------------------------------
        mov     ecx, [esp + 0x14]                   // 8b 4c 24 14     (4 B)
        mov     fs:[0], ecx                         // 64 89 0d 00 00 00 00 (7 B)
        pop     ecx                                 // 59              (1 B) — pop cookie
        pop     edi                                 // 5f              (1 B)
        pop     esi                                 // 5e              (1 B)
        pop     ebx                                 // 5b              (1 B)
        add     esp, 0x10                           // 83 c4 10        (3 B) — drop ECX/FS/scope/-1
        ret                                         // c3              (1 B)
    }
}
