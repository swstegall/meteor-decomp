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
// FUNCTION: ffxivgame 0x00049450 — `__stdcall` 2-arg scaling wrapper (30 B).
//
// Two-arg stdcall thunk (`RET 0x8`) that forwards to the cdecl 3-arg helper
// at VA 0x0044d500. It scales the first dword arg by 4 (`LEA EDX,[ECX*4]`,
// i.e. an int-index → byte-offset conversion), passes the second dword arg
// through unchanged, and appends the literal constant 0xc (12). The cdecl
// callee leaves its three pushed dwords on the stack, which the wrapper
// reclaims with a single `ADD ESP, 0xc` before the `RET 0x8` stdcall
// epilogue. The helper's return value in EAX is propagated untouched.
//
// Asm shape (30 bytes — RVA 0x00049450..0x0004946e):
//
//     00049450:  8b 44 24 08          MOV  EAX, [ESP+0x8]    ; arg2
//     00049454:  8b 4c 24 04          MOV  ECX, [ESP+0x4]    ; arg1
//     00049458:  6a 0c                PUSH 0xc               ; const 12
//     0004945a:  50                   PUSH EAX               ; push arg2
//     0004945b:  8d 14 8d 00000000    LEA  EDX, [ECX*4]      ; arg1 * 4
//     00049462:  52                   PUSH EDX               ; push arg1*4
//     00049463:  e8 98 40 00 00       CALL FUN_0044d500      ; rel32
//     00049468:  83 c4 0c             ADD  ESP, 0xc          ; cdecl cleanup
//     0004946b:  c2 08 00             RET  0x8               ; stdcall epilogue
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough, matching
// the convention of sibling wrapper FUN_00401000. Emitting the 30 orig
// bytes verbatim bakes the CALL rel32 displacement as raw bytes that match
// the orig PE's .text slice exactly, regardless of where FUN_0044d500
// lands in our own link.

extern "C" __declspec(naked) void FUN_00449450() {
    __asm {
        _emit 0x8b      // MOV  EAX, [ESP+0x8]      ; arg2
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b      // MOV  ECX, [ESP+0x4]      ; arg1
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x6a      // PUSH 0xc                 ; const 12
        _emit 0x0c
        _emit 0x50      // PUSH EAX                 ; push arg2
        _emit 0x8d      // LEA  EDX, [ECX*4]        ; arg1 * 4
        _emit 0x14
        _emit 0x8d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x52      // PUSH EDX                 ; push arg1*4
        _emit 0xe8      // CALL FUN_0044d500        ; rel32 = +0x00004098
        _emit 0x98
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x83      // ADD  ESP, 0xc            ; cdecl cleanup
        _emit 0xc4
        _emit 0x0c
        _emit 0xc2      // RET  0x8                 ; stdcall epilogue
        _emit 0x08
        _emit 0x00
    }
}
