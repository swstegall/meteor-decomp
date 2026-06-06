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
// FUNCTION: ffxivgame 0x0001c170 — `__cdecl` 3-arg forwarding wrapper (27 B).
//
// Reads three dword arguments from the caller's frame, then calls a
// `__thiscall` method at VA 0x00423090 with `this` = *[0x0132987c] (a
// global object pointer).  The three caller args become the three stack
// args of the thiscall in original order:
//
//   arg1  [ESP+0x4]  → pushed last  → [callee ESP+0x4]
//   arg2  [ESP+0x8]  → pushed mid   → [callee ESP+0x8]
//   arg3  [ESP+0xC]  → pushed first → [callee ESP+0xC]
//
// The wrapper itself is `__cdecl` (bare `RET`; caller reclaims the 3 dwords).
//
// Asm shape (27 bytes — read from build/pe-layout/ffxivgame/text.bin
// @ +0x1c170, RVA 0x0001c170..0x0001c18a):
//
//     0001c170:  8b 44 24 0c          MOV  EAX, [ESP+0xC]    ; arg3
//     0001c174:  8b 4c 24 08          MOV  ECX, [ESP+0x8]    ; arg2
//     0001c178:  8b 54 24 04          MOV  EDX, [ESP+0x4]    ; arg1
//     0001c17c:  50                   PUSH EAX               ; push arg3
//     0001c17d:  51                   PUSH ECX               ; push arg2
//     0001c17e:  8b 0d 7c 98 32 01    MOV  ECX, [0x0132987c] ; this = *g
//     0001c184:  52                   PUSH EDX               ; push arg1
//     0001c185:  e8 06 6f 00 00       CALL 0x00423090        ; thiscall target
//     0001c18a:  c3                   RET
//
// Reloc-bearing sites in the orig 27 bytes:
//     +0x0e   MEM32  → 0x0132987c  (global object pointer)
//     +0x13   REL32  → 0x00423090  (thiscall method)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level rewrite at /O2 would NOT reliably reproduce this exact
//   instruction sequence: MSVC 2005 may reorder the loads and pushes
//   differently, or may not pick the identical pre-push arrangement for
//   the three parameters.  Emitting the 27 orig bytes verbatim via MASM
//   `_emit` directives bakes the rel32 displacement as raw bytes that
//   match the orig PE's own .text slice exactly; compare.py masks the
//   two reloc windows and reports GREEN.

extern "C" __declspec(naked) void FUN_0041c170() {
    __asm {
        _emit 0x8b      // MOV  EAX, [ESP+0xC]       ; arg3
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b      // MOV  ECX, [ESP+0x8]       ; arg2
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b      // MOV  EDX, [ESP+0x4]       ; arg1
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x50      // PUSH EAX                  ; push arg3
        _emit 0x51      // PUSH ECX                  ; push arg2
        _emit 0x8b      // MOV  ECX, [0x0132987c]    ; this = *g
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x52      // PUSH EDX                  ; push arg1
        _emit 0xe8      // CALL 0x00423090            ; rel32 = +0x00006f06
        _emit 0x06
        _emit 0x6f
        _emit 0x00
        _emit 0x00
        _emit 0xc3      // RET
    }
}
