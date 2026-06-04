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
// FUNCTION: ffxivgame 0x00415bf0 — SQEX::CDev::Engine::Vfx::Common::Io::
//                                   Printer::Printer(int cols, int rows)
//                                   constructor (75 B / 0x4b)
//
// __thiscall member constructor: this in ECX, two int stack args (cols,
// rows). Callee pops args via ret 8. Returns this in EAX (standard MSVC
// constructor calling convention).
//
// Behaviour:
//
//   Printer::Printer(int cols, int rows) {
//       this->m_flag_4   = 1;          // [+0x04] owns-m_data flag
//       this->m_flag_5   = 1;          // [+0x05] owns-m_meta flag
//       this->m_data     = NULL;        // [+0x08]
//       this->m_meta     = NULL;        // [+0x0c]
//       this->m_cols     = 0;           // [+0x10]
//       this->m_field_78 = 0;           // [+0x78]
//       this->m_byte_81  = 0;           // [+0x81]
//       this->m_subobj   = NULL;        // [+0x7c]
//       this->m_byte_91  = 0;           // [+0x91]
//       this->m_byte_80  = 0;           // [+0x80]
//       this->vftable    = &Printer_vftable;  // [+0x00] = 0x00f5763c
//       SetLimit(cols, rows);           // FUN_004158b0
//   }
//
// Register allocation:
//   ESI = this (saved across the whole function)
//   ECX = cols (loaded from [ESP+8] early, pushed as the inner arg to
//         SetLimit then immediately overwritten with this before the call)
//   EAX/AL = used for the constant stores (1 → m_flag_4/5; 0 via XOR
//            → all NULL/zero fields)
//
// The unusual store order (0x81 before 0x7c, 0x91 before 0x80) reflects
// the source-level field declaration order in the Printer class, not the
// numeric offset order.
//
// Reloc-bearing sites (4-byte windows, wildcarded by compare.py):
//   +0x3c  MOV [ESI], offset Printer_vftable  (DIR32, .rdata vftable)
//   +0x41  CALL FUN_004158b0                  (REL32, SetLimit)

extern "C" {
    // vftable for Printer class — .rdata @ 0x00f5763c
    extern int Printer_vftable;

    // FUN_004158b0 — Printer::SetLimit(int cols, int rows), __thiscall, ret 8
    void FUN_004158b0();
}

extern "C" __declspec(naked) void FUN_00415bf0() {
    __asm {
        push    esi                                     // 56
        mov     esi, ecx                                // 8b f1          this → ESI
        mov     ecx, dword ptr [esp + 0x8]              // 8b 4c 24 08    cols → ECX
        mov     al, 1                                   // b0 01
        mov     byte ptr [esi + 0x4], al                // 88 46 04       m_flag_4 = 1
        mov     byte ptr [esi + 0x5], al                // 88 46 05       m_flag_5 = 1
        xor     eax, eax                                // 33 c0
        mov     dword ptr [esi + 0x8], eax              // 89 46 08       m_data = 0
        mov     dword ptr [esi + 0xc], eax              // 89 46 0c       m_meta = 0
        mov     dword ptr [esi + 0x10], eax             // 89 46 10       m_cols = 0
        mov     dword ptr [esi + 0x78], eax             // 89 46 78       m_field_78 = 0
        mov     byte ptr [esi + 0x81], al               // 88 86 81000000 m_byte_81 = 0
        mov     dword ptr [esi + 0x7c], eax             // 89 46 7c       m_subobj = 0
        mov     byte ptr [esi + 0x91], al               // 88 86 91000000 m_byte_91 = 0
        mov     byte ptr [esi + 0x80], al               // 88 86 80000000 m_byte_80 = 0
        mov     eax, dword ptr [esp + 0xc]              // 8b 44 24 0c    rows → EAX
        push    eax                                     // 50             push rows
        push    ecx                                     // 51             push cols (ECX)
        mov     ecx, esi                                // 8b ce          ECX = this
        mov     dword ptr [esi], offset Printer_vftable // c7 06 ????????  vftable
        call    FUN_004158b0                            // e8 ????????    SetLimit(cols,rows)
        mov     eax, esi                                // 8b c6          return this
        pop     esi                                     // 5e
        ret     8                                       // c2 08 00
    }
}
