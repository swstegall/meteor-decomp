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
// FUNCTION: ffxivgame 0x00037a20 — __thiscall guarded dispatch helper
//                                  (149 B / 0x95, ret 4).
//
// __thiscall void FUN_00437a20(this):
//   ECX = this. Computes a signed element count from a container span
//   (`(this->_0x18 - this->_0x14) / 10`, the classic 0x66666667 magic
//   divide-by-10 idiom, guarded by a null-begin test at +0x14). If that
//   count is > 0, it lazily initialises a global function pointer at
//   0x0132390c (set once, gated by a one-bit flag at 0x01323910) and
//   fires it with five literal args (an assert/log-style call). Then it
//   unconditionally:
//     - calls FUN_0043bf50(this=this->_0x8),
//     - reloads a stack-resident object pointer, virtual-dispatches its
//       vtable slot +0xc with five zero args + the object,
//     - hands the result to this's own vtable slot +0xa0,
//     - returns this->_0x24.
//
// Reloc-bearing sites (DIR32 data + REL32 calls; compare.py masks them):
//   +0x29  TEST byte ptr [data_01323910], al   (dir32, init flag)
//   +0x31  OR   dword ptr [data_01323910], eax (dir32, init flag)
//   +0x37  MOV  dword ptr [data_0132390c], offset FUN_00436b00 (2× dir32)
//   +0x41  PUSH offset data_00f659d8           (dir32)
//   +0x4b  PUSH offset data_00f65a58           (dir32)
//   +0x50  PUSH offset data_00f657e7           (dir32)
//   +0x55  PUSH offset data_00f65a34           (dir32)
//   +0x5a  CALL dword ptr [data_0132390c]      (dir32, fn-ptr global)
//   +0x66  CALL FUN_0043bf50                   (rel32)
//
// Per the ffxivgame convention, this lowers via __declspec(naked) so the
// 149 .text bytes match orig byte-for-byte modulo the reloc windows.

extern "C" {
    // .text — internal direct-call target (REL32).
    int FUN_0043bf50();
    // .text — function pointer literal stored into the global at 0x0132390c.
    int FUN_00436b00();
    // .data — lazy-init flag (bit 0) and function-pointer slot.
    extern int data_01323910;
    extern int data_0132390c;
    // .rdata — literal arg pointers (assert/log strings & descriptors).
    extern int data_00f659d8;
    extern int data_00f65a58;
    extern int data_00f657e7;
    extern int data_00f65a34;
}

extern "C" __declspec(naked) void FUN_00437a20() {
    __asm {
        push    esi                                     // 56
        mov     esi, ecx                                // 8b f1
        mov     eax, dword ptr [esi + 0x14]             // 8b 46 14
        test    eax, eax                                // 85 c0
        jz      have_count                              // 74 16
        mov     ecx, dword ptr [esi + 0x18]             // 8b 4e 18
        sub     ecx, eax                                // 2b c8
        mov     eax, 0x66666667                         // b8 67 66 66 66
        imul    ecx                                     // f7 e9
        sar     edx, 3                                  // c1 fa 03
        mov     eax, edx                                // 8b c2
        shr     eax, 0x1f                               // c1 e8 1f
        add     eax, edx                                // 03 c2

    have_count:
        test    eax, eax                                // 85 c0
        jbe     do_dispatch                             // 76 3f
        mov     eax, 1                                  // b8 01 00 00 00
        test    byte ptr [data_01323910], al           // 84 05 10 39 32 01
        jnz     fp_ready                                // 75 10
        or      dword ptr [data_01323910], eax          // 09 05 10 39 32 01
        mov     dword ptr [data_0132390c], offset FUN_00436b00  // c7 05 0c 39 32 01 00 6b 43 00

    fp_ready:
        push    offset data_00f659d8                    // 68 d8 59 f6 00
        push    0x1b8                                   // 68 b8 01 00 00
        push    offset data_00f65a58                    // 68 58 5a f6 00
        push    offset data_00f657e7                    // 68 e7 57 f6 00
        push    offset data_00f65a34                    // 68 34 5a f6 00
        call    dword ptr [data_0132390c]               // ff 15 0c 39 32 01
        add     esp, 0x14                               // 83 c4 14

    do_dispatch:
        mov     ecx, dword ptr [esi + 8]                // 8b 4e 08
        call    FUN_0043bf50                            // e8 ?? ?? ?? ??
        mov     eax, dword ptr [esp + 8]                // 8b 44 24 08
        mov     ecx, dword ptr [eax]                    // 8b 08
        mov     edx, dword ptr [ecx + 0xc]              // 8b 51 0c
        push    0                                       // 6a 00
        push    0                                       // 6a 00
        push    0                                       // 6a 00
        push    0                                       // 6a 00
        push    0                                       // 6a 00
        push    eax                                     // 50
        call    edx                                     // ff d2
        mov     edx, dword ptr [esi]                    // 8b 16
        push    eax                                     // 50
        mov     eax, dword ptr [edx + 0xa0]             // 8b 82 a0 00 00 00
        mov     ecx, esi                                // 8b ce
        call    eax                                     // ff d0
        mov     eax, dword ptr [esi + 0x24]             // 8b 46 24
        pop     esi                                     // 5e
        ret     4                                       // c2 04 00
    }
}
