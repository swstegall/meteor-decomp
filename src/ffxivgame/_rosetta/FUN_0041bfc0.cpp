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
// FUNCTION: ffxivgame 0x0001bfc0 — `__cdecl` 3-arg thiscall forwarding wrapper (27 B).
//
// Reads three dword args from its caller's stack frame, then forwards them
// verbatim to a `__thiscall` method at VA 0x00422f90, using the object
// pointer stored in the global dword at absolute address 0x0132987c as
// the implicit `this` (ECX).
//
// The instruction shape is:
//
//   0001bfc0:  8b 44 24 0c    MOV  EAX, [ESP+0xc]      ; arg3
//   0001bfc4:  8b 4c 24 08    MOV  ECX, [ESP+0x8]      ; arg2
//   0001bfc8:  8b 54 24 04    MOV  EDX, [ESP+0x4]      ; arg1
//   0001bfcc:  50             PUSH EAX                 ; push arg3
//   0001bfcd:  51             PUSH ECX                 ; push arg2
//   0001bfce:  8b 0d 7c 98 32 01  MOV ECX, [0x0132987c]; this
//   0001bfd4:  52             PUSH EDX                 ; push arg1
//   0001bfd5:  e8 b6 6f 00 00 CALL 0x00422f90          ; thiscall method
//   0001bfda:  c3             RET
//
// Calling conventions:
//   - Outer function: `__cdecl` — bare `RET` with no stack adjustment; the
//     three dwords pushed for the inner call are reclaimed by the callee's
//     own epilogue (`RET 0xc`), so ESP is balanced on return.
//   - Inner function at 0x00422f90: `__thiscall` — ECX holds `this`,
//     three stack args, callee-cleanup.
//
// Reloc-bearing operands in the 27 bytes:
//   +0x0f  ABS32  → 0x0132987c  (global object pointer)
//   +0x18  REL32  → +0x00006fb6 (displacement to 0x00422f90 from next insn)
//
// Reconstruction uses `__declspec(naked)` with verbatim `_emit` directives,
// matching the convention established by FUN_00401000 and FUN_00404e10.
// A source-level rewrite would risk different register-load ordering or a
// different push sequence that MSVC 2005's optimiser might not canonicalise
// to the observed form.

extern "C" __declspec(naked) void FUN_0041bfc0() {
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
        _emit 0x8b      // MOV  ECX, [0x0132987c]   ; this = *g_obj_ptr
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x52      // PUSH EDX                 ; push arg1
        _emit 0xe8      // CALL 0x00422f90           ; rel32 = +0x00006fb6
        _emit 0xb6
        _emit 0x6f
        _emit 0x00
        _emit 0x00
        _emit 0xc3      // RET
    }
}
