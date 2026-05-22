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
// FUNCTION: ffxivgame 0x0000a410 — labelled bump-or-delegate allocator
// member function (__thiscall, 76 B).
//
// Member function on an allocator-front-end class with this layout:
//
//   class Allocator {
//       int            total_bytes;     // 0x00  — bump on overflow path
//       int            has_fast_path;   // 0x04  — when non-zero, bump
//       char*          cursor;          // 0x08  — fast-path bump pointer
//       BackingPool*   backing;         // 0x0c  — slow-path delegate
//   };
//
// Pseudo-source (logical structure):
//
//   void* __thiscall Allocator::Allocate(int size) {
//       int aligned = (size + 15) & ~15;
//       if (this->has_fast_path) {
//           void* result = this->cursor;
//           this->cursor = (char*)result + aligned;
//           return result;
//       }
//       this->total_bytes += aligned;
//       Label label;                                    // 8 B local
//       return this->backing->Allocate(
//                   aligned,
//                   label.Init(0x10, "CDev.Engine.Phy"));
//   }
//
// `Label::Init` (FUN_0040e2d0, 18 B) is a 2-field constructor that
// stores (priority, category-name) into the two int slots of the local
// and returns `this` in EAX — the standard MSVC trick that lets the
// label be chained as an argument to the next call.
//
// `BackingPool::Allocate` (FUN_0040e110, 250 B) takes (size, Label*)
// and returns the new block.
//
// Sibling functions FUN_0040a460 / FUN_0040a4b0 in `engine_memory` are
// the public entry points: they consult a pair of globals at
// 0x01328034 / 0x01328038 and either delegate to this thiscall (when
// the fast-path Allocator is hot), pass-through the "CDev.Engine.Phy"
// label, or upgrade the label to "CDev.Engine.Phy.Init" (RVA 0xf552f0).
//
// Branch shape (per asm/ffxivgame/0000a410_FUN_0040a410.s):
//
//   +0x15  JZ +0x11   (has_fast_path == 0 → slow_path)
//   first arm: bump cursor, return old cursor, ret 4
//   slow_path: bump total, build Label, delegate, ret 4
//
// Why naked asm: the slow-path uses MSVC's "constructor returns this"
// idiom (`call FUN_0040e2d0; push eax`) to chain the Label address
// into the next call without a separate `lea`. C++ source-level
// won't reproduce this byte-for-byte without a same-shape helper, and
// the small register interleave (`POP EDI` between `LEA ECX, [EAX+EDI]`
// and `MOV [ESI+8], ECX` in the fast path) is exactly MSVC 2005's
// /O2 instruction-schedule choice — hard to coerce out of source.
// Naked asm pins the 76 bytes; compare.py reloc-masks the three
// 4-byte windows (1 DIR32 + 2 REL32).
//
// Reloc-bearing sites (4-byte windows, wildcarded by compare.py):
//   +0x2b  PUSH offset data_00f552e0   (dir32, .rdata "CDev.Engine.Phy")
//   +0x36  CALL FUN_0040e2d0           (rel32, Label::Init)
//   +0x40  CALL FUN_0040e110           (rel32, BackingPool::Allocate)

extern "C" {
    // .text — RVA 0x0040e2d0. Label::Init(int, const char*) — __thiscall,
    // returns this in EAX (chainable into the next CALL).
    int FUN_0040e2d0();

    // .text — RVA 0x0040e110. BackingPool::Allocate(size, Label*) —
    // __thiscall, returns void* in EAX.
    int FUN_0040e110();

    // .rdata — "CDev.Engine.Phy" category-name string.
    extern int data_00f552e0;
}

extern "C" __declspec(naked) void FUN_0040a410() {
    __asm {
        // --- prologue: 8 B local (the Label), save ESI/EDI ------------
        sub     esp, 8                                // 83 ec 08
        push    esi                                   // 56
        push    edi                                   // 57
        mov     edi, dword ptr [esp + 0x14]           // 8b 7c 24 14   size
        add     edi, 0xf                              // 83 c7 0f
        mov     esi, ecx                              // 8b f1         this
        and     edi, 0xfffffff0                       // 83 e7 f0      aligned
        cmp     dword ptr [esi + 0x4], 0              // 83 7e 04 00
        jz      slow_path                             // 74 11

        // --- fast path: bump cursor, return old cursor ---------------
        mov     eax, dword ptr [esi + 0x8]            // 8b 46 08
        lea     ecx, [eax + edi * 1]                  // 8d 0c 38
        pop     edi                                    // 5f
        mov     dword ptr [esi + 0x8], ecx            // 89 4e 08
        pop     esi                                    // 5e
        add     esp, 8                                 // 83 c4 08
        ret     4                                      // c2 04 00

    slow_path:
        // --- slow path: bump total, build label, delegate ------------
        add     dword ptr [esi], edi                  // 01 3e
        push    offset data_00f552e0                  // 68 ?? ?? ?? ??
        push    0x10                                   // 6a 10
        lea     ecx, [esp + 0x10]                     // 8d 4c 24 10
        call    FUN_0040e2d0                          // e8 ?? ?? ?? ??
        mov     ecx, dword ptr [esi + 0xc]            // 8b 4e 0c
        push    eax                                    // 50
        push    edi                                    // 57
        call    FUN_0040e110                          // e8 ?? ?? ?? ??
        pop     edi                                    // 5f
        pop     esi                                    // 5e
        add     esp, 8                                 // 83 c4 08
        ret     4                                      // c2 04 00
    }
}
