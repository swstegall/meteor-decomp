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
// FUNCTION: ffxivgame 0x435620 — clear a container, destroying each element
// (__thiscall member function, 79 B).
//
// Iterates [this->begin (0x04) .. this->end (0x08)) of a contiguous
// container of 4-byte elements (pointers). For each element it loads the
// stored pointer (`mov ecx, [esi]`) and delegates to FUN_0043b590
// (__thiscall, the per-element teardown), then resets this->field_30 to 0.
//
//   void __thiscall clear(Container *this) {
//       for (T** it = this->begin; it != this->end; ++it)
//           release(*it);              // FUN_0043b590, this = *it
//       this->field_30 = 0;
//   }
//
// Why naked asm: this is a checked-iterator (_SCL_SECURE) build. Every
// dereference / comparison is guarded by an inlined range check that, on
// failure, tail-calls __invalid_parameter_noinfo (RVA 0x5d22b4). MSVC
// 2005 emits these guards (plus a 1-byte NOP pad before the loop head)
// from the `<vector>` debug-iterator machinery — clean C++ source won't
// reproduce the exact guard placement, register choice (ESI = cursor,
// EBX = cached end, EDI = this), or the NOP. Naked asm pins all 79 bytes;
// compare.py reloc-masks the four REL32 CALL windows.
//
// Reloc-bearing sites (4-byte windows, wildcarded by compare.py):
//   +0x0d  CALL __invalid_parameter_noinfo   (rel32)  begin > end guard
//   +0x1a  CALL __invalid_parameter_noinfo   (rel32)  begin > end guard
//   +0x29  CALL __invalid_parameter_noinfo   (rel32)  deref range guard
//   +0x30  CALL FUN_0043b590                 (rel32)  per-element release
//   +0x3a  CALL __invalid_parameter_noinfo   (rel32)  post-inc range guard

extern "C" {
    // .text — RVA 0x5d22b4. Secure-CRT invalid-parameter reporter.
    int FUN_009d22b4();
    // .text — RVA 0x3b590. Per-element teardown (__thiscall on *it).
    int FUN_0043b590();
}

extern "C" __declspec(naked) void FUN_00435620() {
    __asm {
        push    ebx                                   // 53
        push    esi                                   // 56
        push    edi                                   // 57
        mov     edi, ecx                              // 8b f9         this
        mov     esi, dword ptr [edi + 0x4]            // 8b 77 04      begin
        cmp     esi, dword ptr [edi + 0x8]            // 3b 77 08
        jbe     L632                                  // 76 05
        call    FUN_009d22b4                          // e8 ?? ?? ?? ??
    L632:
        mov     ebx, dword ptr [edi + 0x8]            // 8b 5f 08      end
        cmp     dword ptr [edi + 0x4], ebx            // 39 5f 04
        jbe     L640                                  // 76 06
        call    FUN_009d22b4                          // e8 ?? ?? ?? ??
        nop                                           // 90
    L640:
        cmp     esi, ebx                              // 3b f3
        jz      L664                                  // 74 20
        cmp     esi, dword ptr [edi + 0x8]            // 3b 77 08
        jb      L64e                                  // 72 05
        call    FUN_009d22b4                          // e8 ?? ?? ?? ??
    L64e:
        mov     ecx, dword ptr [esi]                  // 8b 0e         *it
        call    FUN_0043b590                          // e8 ?? ?? ?? ??
        cmp     esi, dword ptr [edi + 0x8]            // 3b 77 08
        jb      L65f                                  // 72 05
        call    FUN_009d22b4                          // e8 ?? ?? ?? ??
    L65f:
        add     esi, 0x4                              // 83 c6 04
        jmp     L640                                  // eb dc
    L664:
        mov     dword ptr [edi + 0x30], 0             // c7 47 30 00 00 00 00
        pop     edi                                   // 5f
        pop     esi                                   // 5e
        pop     ebx                                   // 5b
        ret                                           // c3
    }
}
