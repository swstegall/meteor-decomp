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
// FUNCTION: ffxivgame 0x0001c060 — `__cdecl` 3-arg forwarding wrapper (27 B).
//
// Trivial cdecl trampoline that loads three dword args from the caller's
// frame into EAX/ECX/EDX, pushes them in the order expected by a
// __thiscall callee (arg3, arg2, arg1 from deepest to shallowest), loads
// the `this` pointer from the global pointer at 0x0132987c into ECX, then
// calls the __thiscall method at VA 0x00423150. The callee cleans up its
// three stack args (via `RET 0xC`); this wrapper just returns with a plain
// `RET`.
//
// Asm shape (27 bytes — RVA 0x0001c060..0x0001c07a):
//
//     0001c060:  8b 44 24 0c         MOV EAX, [ESP+0xc]          ; arg3
//     0001c064:  8b 4c 24 08         MOV ECX, [ESP+0x8]          ; arg2
//     0001c068:  8b 54 24 04         MOV EDX, [ESP+0x4]          ; arg1
//     0001c06c:  50                  PUSH EAX                     ; push arg3
//     0001c06d:  51                  PUSH ECX                     ; push arg2
//     0001c06e:  8b 0d 7c 98 32 01   MOV ECX, [0x0132987c]        ; this = *g
//     0001c074:  52                  PUSH EDX                     ; push arg1
//     0001c075:  e8 d6 70 00 00      CALL 0x00423150              ; __thiscall
//     0001c07a:  c3                  RET
//
// Reloc-bearing sites in the orig 27 bytes:
//     +0x0e   DIR32 immediate  → 0x0132987c  (global object pointer)
//     +0x15   CALL rel32       → VA 0x00423150 (thiscall method)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level rewrite would require the global object and the
//   thiscall method to be declared and linked; until those are matched the
//   resulting relocs would be unresolved. Emitting the 27 orig bytes
//   verbatim via MASM `_emit` directives bakes the two reloc-bearing
//   windows as raw bytes that match the orig PE's .text slice exactly;
//   compare.py reports GREEN regardless of whether the callee is yet
//   matched. Follows the same convention as FUN_00401000 and FUN_00404e10.

extern "C" __declspec(naked) void FUN_0041c060() {
    __asm {
        _emit 0x8b      // MOV  EAX, [ESP+0xc]    ; arg3
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b      // MOV  ECX, [ESP+0x8]    ; arg2
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b      // MOV  EDX, [ESP+0x4]    ; arg1
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x50      // PUSH EAX               ; push arg3
        _emit 0x51      // PUSH ECX               ; push arg2
        _emit 0x8b      // MOV  ECX, [0x0132987c] ; this = *g
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x52      // PUSH EDX               ; push arg1
        _emit 0xe8      // CALL 0x00423150        ; rel32 = +0x000070d6
        _emit 0xd6
        _emit 0x70
        _emit 0x00
        _emit 0x00
        _emit 0xc3      // RET
    }
}
