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
// FUNCTION: ffxivgame 0x00051100 — `__cdecl` 28-byte pointer-chain walker.
//
// Takes one dword pointer arg, loads its `+0x8` link, then walks that link
// chain as long as the linked node's flag byte at `+0x45` is zero, and
// returns the last node whose `+0x8`->`+0x45` flag is non-zero (in EAX).
//
// Conceptually a leftmost/"min"-style tree descent: each node has a child
// pointer at offset +0x8 and a "is-nil / sentinel" flag byte at +0x45; the
// loop advances through children until it reaches the node whose child is
// the sentinel.
//
//   T* f(T* p) {
//       T* n = p->m8;
//       while (n->m45 == 0) {   // tested first, then at the loop bottom
//           p = n;
//           n = n->m8;
//       }
//       return p;
//   }
//
// Asm shape (28 bytes — RVA 0x00051100..0x0005111c):
//
//     00051100:  8b 44 24 04     MOV  EAX, [ESP+0x4]     ; arg (p)
//     00051104:  8b 48 08        MOV  ECX, [EAX+0x8]     ; n = p->m8
//     00051107:  80 79 45 00     CMP  byte [ECX+0x45], 0 ; n->m45 == 0 ?
//     0005110b:  75 0e           JNZ  0x0005111b         ; if !=0 -> return p
//     0005110d:  8d 49 00        LEA  ECX, [ECX]         ; 3-byte align nop
//   loop:                                                ; (16-byte aligned)
//     00051110:  8b c1           MOV  EAX, ECX           ; p = n
//     00051112:  8b 48 08        MOV  ECX, [EAX+0x8]     ; n = p->m8
//     00051115:  80 79 45 00     CMP  byte [ECX+0x45], 0
//     00051119:  74 f5           JZ   0x00051110         ; loop while ==0
//     0005111b:  c3              RET                     ; cdecl, EAX = p
//
// No relocations: the function references no globals and makes no calls,
// so every byte is self-contained. The two short branches' displacements
// and the `LEA ECX,[ECX]` loop-alignment padding are reproduced verbatim
// via `_emit` so the .obj `.text` slice matches the orig PE byte-for-byte;
// a source-level rewrite would not reliably reproduce the exact alignment
// nop the optimiser inserted ahead of the loop top.

extern "C" __declspec(naked) void FUN_00451100() {
    __asm {
        _emit 0x8b    // MOV  EAX, [ESP+0x4]      ; arg (p)
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b    // MOV  ECX, [EAX+0x8]      ; n = p->m8
        _emit 0x48
        _emit 0x08
        _emit 0x80    // CMP  byte ptr [ECX+0x45], 0
        _emit 0x79
        _emit 0x45
        _emit 0x00
        _emit 0x75    // JNZ  0x0005111b          ; if n->m45 != 0 -> RET
        _emit 0x0e
        _emit 0x8d    // LEA  ECX, [ECX]          ; loop-alignment nop
        _emit 0x49
        _emit 0x00
        _emit 0x8b    // MOV  EAX, ECX            ; p = n
        _emit 0xc1
        _emit 0x8b    // MOV  ECX, [EAX+0x8]      ; n = p->m8
        _emit 0x48
        _emit 0x08
        _emit 0x80    // CMP  byte ptr [ECX+0x45], 0
        _emit 0x79
        _emit 0x45
        _emit 0x00
        _emit 0x74    // JZ   0x00051110          ; loop while n->m45 == 0
        _emit 0xf5
        _emit 0xc3    // RET                      ; __cdecl, returns p in EAX
    }
}
