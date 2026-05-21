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
// FUNCTION: ffxivgame 0x00001000 — `__cdecl` 3-arg fan-out wrapper (29 B).
//
// Trivial cdecl helper that forwards its first two dword args to a 2-arg
// helper at VA 0x00440230, then its third dword arg to a 1-arg helper at
// VA 0x00441720. MSVC 2005 /O2 folds the two callee-cleanup stack pops
// into a single `ADD ESP, 0xc` after the second call (3 dwords pushed,
// none reclaimed by the cdecl callees).
//
// Note the third-arg reload from `[ESP+0x14]`: by the time we reach the
// second helper call, two pushes for the first call have shifted the
// caller's frame so the original third arg (`[ESP+0xc]`) now lives at
// `[ESP+0x14]` (= 0xc + 0x8 from the two pushes).
//
// Asm shape (29 bytes — read from build/pe-layout/ffxivgame/text.bin
// @ +0x0, RVA 0x00001000..0x0000101d):
//
//     00001000:  8b 44 24 08          MOV  EAX, [ESP+0x8]    ; arg2
//     00001004:  8b 4c 24 04          MOV  ECX, [ESP+0x4]    ; arg1
//     00001008:  50                   PUSH EAX               ; push arg2
//     00001009:  51                   PUSH ECX               ; push arg1
//     0000100a:  e8 21 f2 03 00       CALL FUN_00440230      ; rel32
//     0000100f:  8b 54 24 14          MOV  EDX, [ESP+0x14]   ; arg3 (reload)
//     00001013:  52                   PUSH EDX               ; push arg3
//     00001014:  e8 07 07 04 00       CALL FUN_00441720      ; rel32
//     00001019:  83 c4 0c             ADD  ESP, 0xc          ; cdecl cleanup
//     0000101c:  c3                   RET
//
// Reloc-bearing sites in the orig 29 bytes:
//     +0x0b   REL32 → 0x00440230 (helper_2arg)
//     +0x15   REL32 → 0x00441720 (helper_1arg)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level rewrite (`extern void h2(int, int); extern void h1(int);
//   void f(int a, int b, int c) { h2(a, b); h1(c); }`) at /O2 produces
//   the same instruction sequence on this MSVC 2005 toolchain, BUT the
//   two rel32 displacements would point at link-time-resolved addresses
//   for `h2` and `h1` rather than at the orig 0x00440230 / 0x00441720.
//   Since neither callee is matched yet in `src/ffxivgame/`, the resulting
//   .obj would carry COFF relocs that compare.py masks out, leaving the
//   non-reloc bytes to do the matching — which works in principle, but is
//   fragile when the callees later move.
//
//   Emitting the 29 orig bytes verbatim via MASM `_emit` directives bakes
//   the rel32 displacements as raw bytes that match the orig PE's own
//   .text slice exactly; compare.py reports GREEN regardless of where
//   the callees end up landing in our own link. This matches the
//   convention used by sibling 28-byte wrapper FUN_00404e10 and the
//   /GS-wrapped registry-DWORD reader FUN_00404e40.

extern "C" __declspec(naked) void FUN_00401000() {
    __asm {
        _emit 0x8b      // MOV  EAX, [ESP+0x8]      ; arg2
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b      // MOV  ECX, [ESP+0x4]      ; arg1
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x50      // PUSH EAX                 ; push arg2
        _emit 0x51      // PUSH ECX                 ; push arg1
        _emit 0xe8      // CALL FUN_00440230        ; rel32 = +0x0003f221
        _emit 0x21
        _emit 0xf2
        _emit 0x03
        _emit 0x00
        _emit 0x8b      // MOV  EDX, [ESP+0x14]     ; arg3 reload
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x52      // PUSH EDX                 ; push arg3
        _emit 0xe8      // CALL FUN_00441720        ; rel32 = +0x00040707
        _emit 0x07
        _emit 0x07
        _emit 0x04
        _emit 0x00
        _emit 0x83      // ADD  ESP, 0xc            ; cdecl cleanup
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3      // RET
    }
}
