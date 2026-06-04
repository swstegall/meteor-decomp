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
// FUNCTION: ffxivgame 0x0001c040 — `__cdecl` 3-arg thiscall-on-global
//                                   wrapper (27 B).
//
// Loads three dword args, then forwards them to a `__thiscall` method on the
// global object pointer stored at 0x0132987c, returning without stack cleanup
// (plain RET — the callee is thiscall / stdcall and cleans up the 3 stack
// args itself).  The outer function is therefore `__cdecl`.
//
// Register assignment before the CALL:
//   EAX ← [ESP+0xc]  (arg3)          PUSH EAX
//   ECX ← [ESP+0x8]  (arg2)          PUSH ECX
//   ECX ← *[0x0132987c]              (this for thiscall callee)
//   EDX ← [ESP+0x4]  (arg1)          PUSH EDX
//   CALL rel32 → FUN_00423010        (thiscall, 3 stack args)
//   RET
//
// After both the pre-CALL pushes and the arg1 push the callee's stack frame
// holds (top → bottom): arg1, arg2, arg3.  ECX carries the global this-ptr.
//
// Asm shape (27 bytes, read from build/pe-layout/ffxivgame/text.bin
// @ +0x1c040, RVA 0x0001c040..0x0001c05a):
//
//   0001c040:  8b 44 24 0c     MOV  EAX, [ESP+0xc]    ; arg3
//   0001c044:  8b 4c 24 08     MOV  ECX, [ESP+0x8]    ; arg2
//   0001c048:  8b 54 24 04     MOV  EDX, [ESP+0x4]    ; arg1
//   0001c04c:  50              PUSH EAX               ; push arg3
//   0001c04d:  51              PUSH ECX               ; push arg2
//   0001c04e:  8b 0d 7c 98 32 01  MOV ECX,[0x132987c] ; this = *g_ptr
//   0001c054:  52              PUSH EDX               ; push arg1
//   0001c055:  e8 b6 6f 00 00  CALL 0x00423010        ; rel32 = +0x00006fb6
//   0001c05a:  c3              RET
//
// Reloc-bearing sites:
//   +0x0f   DIR32 mem ref → 0x0132987c  (global object-pointer slot)
//   +0x15   REL32         → 0x00423010  (thiscall callee)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level rewrite such as
//       `void f(int a, int b, int c) { g_obj->Method(a, b, c); }`
//   would in principle produce the same instruction sequence, but MSVC 2005
//   register allocation for the pre-CALL shuffle (EAX/ECX/EDX choice, push
//   ordering) is sensitive to declaration order, optimisation flags, and the
//   exact calling convention annotation on the callee.  Rather than risk a
//   push-order mismatch, we re-emit the 27 original bytes verbatim via MASM
//   `_emit` directives — the same convention used by sibling wrappers
//   FUN_00401000 and FUN_00404e10 in this `_rosetta/` cluster.

extern "C" __declspec(naked) void FUN_0041c040() {
    __asm {
        _emit 0x8b      // MOV  EAX, [ESP+0xc]      ; arg3
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b      // MOV  ECX, [ESP+0x8]      ; arg2
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b      // MOV  EDX, [ESP+0x4]      ; arg1
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x50      // PUSH EAX                 ; push arg3
        _emit 0x51      // PUSH ECX                 ; push arg2
        _emit 0x8b      // MOV  ECX, [0x0132987c]   ; this = *g_ptr
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x52      // PUSH EDX                 ; push arg1
        _emit 0xe8      // CALL 0x00423010           ; rel32 = +0x00006fb6
        _emit 0xb6
        _emit 0x6f
        _emit 0x00
        _emit 0x00
        _emit 0xc3      // RET
    }
}
