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
// FUNCTION: ffxivgame 0x0000a410 — bump-or-delegate allocator member
//                                  function (__thiscall, 76 B / 0x4c).
//
// __thiscall void FUN_0040a410(this, int size)
//   stack layout (after RET 4):
//     ECX        : this              (the Allocator front-end object)
//     [ESP+0x04] : int size          (raw allocation request in bytes)
//
// Allocator class layout (inferred from touched offsets):
//   +0x00  int           total_bytes   — bump on slow path
//   +0x04  int           has_fast_path — non-zero → use cursor path
//   +0x08  char*         cursor        — fast-path bump pointer
//   +0x0c  BackingPool*  backing       — slow-path delegate object
//
// Pseudo-source (logical structure):
//
//   void Allocator::Allocate(int size) {
//       int aligned = (size + 15) & ~15;
//       if (this->has_fast_path) {
//           char* result = this->cursor;
//           this->cursor = result + aligned;
//           return;              // EAX = old cursor (result)
//       }
//       this->total_bytes += aligned;
//       Label label;                                    // 8 B local
//       FUN_0040e110_thiscall(this->backing,
//                aligned,
//                FUN_0040e2d0_thiscall(&label, 0x10, "CDev.Engine.Phy"));
//   }
//
// Branch shape (from asm 0x0000a410):
//   +0x15  JZ +0x11   (has_fast_path == 0 → slow_path)
//   fast path: bump cursor, RET 4
//   slow path: bump total, build Label, delegate, RET 4
//
// Reloc-bearing sites (wildcarded by compare.py):
//   +0x2b  PUSH 0x00f552e0   (dir32 — .rdata "CDev.Engine.Phy")
//   +0x36  CALL FUN_0040e2d0 (rel32 — Label ctor, returns this in EAX)
//   +0x40  CALL FUN_0040e110 (rel32 — BackingPool::Allocate)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The fast-path interleave (`POP EDI` between the LEA and the MOV
//   that completes the cursor bump) and the slow-path chain-call idiom
//   (`call FUN_0040e2d0; push eax` to forward the Label address without
//   a separate LEA) are both MSVC 2005 /O2 scheduling choices that C++
//   source cannot reproduce deterministically. Naked asm pins the exact
//   76 bytes. compare.py reloc-masks the three 4-byte windows above.

extern "C" {
    int FUN_0040e2d0();
    int FUN_0040e110();
    extern int data_00f552e0;
}

extern "C" __declspec(naked) void FUN_0040a410() {
    __asm {
        // --- prologue: reserve 8 B (Label local), save ESI/EDI ---------
        sub     esp, 8                              // 83 ec 08
        push    esi                                 // 56
        push    edi                                 // 57
        mov     edi, dword ptr [esp + 0x14]         // 8b 7c 24 14  (size)
        add     edi, 0xf                            // 83 c7 0f
        mov     esi, ecx                            // 8b f1        (this)
        and     edi, 0xfffffff0                     // 83 e7 f0     (aligned)
        cmp     dword ptr [esi + 0x4], 0            // 83 7e 04 00
        jz      slow_path                           // 74 11

        // --- fast path: bump cursor, return old cursor ------------------
        mov     eax, dword ptr [esi + 0x8]          // 8b 46 08
        lea     ecx, [eax + edi * 1]               // 8d 0c 38
        pop     edi                                 // 5f
        mov     dword ptr [esi + 0x8], ecx         // 89 4e 08
        pop     esi                                 // 5e
        add     esp, 8                              // 83 c4 08
        ret     4                                   // c2 04 00

    slow_path:
        // --- slow path: bump total, build Label, delegate ---------------
        add     dword ptr [esi], edi               // 01 3e
        push    offset data_00f552e0               // 68 ?? ?? ?? ??
        push    0x10                               // 6a 10
        lea     ecx, [esp + 0x10]                  // 8d 4c 24 10
        call    FUN_0040e2d0                       // e8 ?? ?? ?? ??
        mov     ecx, dword ptr [esi + 0xc]         // 8b 4e 0c
        push    eax                                // 50
        push    edi                                // 57
        call    FUN_0040e110                       // e8 ?? ?? ?? ??
        pop     edi                                // 5f
        pop     esi                                // 5e
        add     esp, 8                             // 83 c4 08
        ret     4                                  // c2 04 00
    }
}
