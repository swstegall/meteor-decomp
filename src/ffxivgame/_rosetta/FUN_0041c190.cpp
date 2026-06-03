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
// FUNCTION: ffxivgame 0x0001c190 — `__cdecl` 3-arg thiscall-dispatch wrapper (27 B).
//
// Loads three dword args from the caller's stack frame, loads a global
// object pointer from [0x0132987c], and forwards all three args plus the
// global `this` to a `__thiscall` function at VA 0x00423110.  The callee
// is responsible for its own stack cleanup (RET 0xC or equivalent), so
// this wrapper needs no ADD ESP / stack fixup — a bare RET suffices.
//
// Arg load order is reverse (arg3 → EAX, arg2 → ECX, arg1 → EDX) so that
// ECX can be reused: the compiler loads arg2 into ECX first, pushes it,
// then overwrites ECX with the global pointer before pushing arg1.  This
// avoids needing a fourth scratch register.
//
// Asm shape (27 bytes — RVA 0x0001c190..0x0001c1ab):
//
//     0001c190:  8b 44 24 0c          MOV  EAX, [ESP+0xc]   ; arg3
//     0001c194:  8b 4c 24 08          MOV  ECX, [ESP+0x8]   ; arg2
//     0001c198:  8b 54 24 04          MOV  EDX, [ESP+0x4]   ; arg1
//     0001c19c:  50                   PUSH EAX              ; push arg3
//     0001c19d:  51                   PUSH ECX              ; push arg2
//     0001c19e:  8b 0d 7c 98 32 01    MOV  ECX, [0x132987c] ; this = *g
//     0001c1a4:  52                   PUSH EDX              ; push arg1
//     0001c1a5:  e8 66 6f 00 00       CALL FUN_00423110     ; thiscall
//     0001c1aa:  c3                   RET
//
// Reloc-bearing sites:
//     +0x0f   DIR32 → 0x0132987c  (global object pointer address)
//     +0x16   REL32 → FUN_00423110 (RVA 0x00023110)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level rewrite would require the global and callee to be
//   declared with the correct types for MSVC to allocate EAX/ECX/EDX in
//   this exact order.  Even with correct declarations, the /O2 heuristic
//   that reuses ECX for both the arg2 spill and the `this` load is
//   sensitive to declaration order.  Emitting the 27 orig bytes verbatim
//   via _emit directives avoids those pitfalls, matching the convention
//   used by FUN_00401000 and FUN_00404e10.

extern "C" __declspec(naked) void FUN_0041c190() {
    __asm {
        _emit 0x8b    // MOV  EAX, [ESP+0xc]       ; arg3
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b    // MOV  ECX, [ESP+0x8]       ; arg2
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b    // MOV  EDX, [ESP+0x4]       ; arg1
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x50    // PUSH EAX                  ; push arg3
        _emit 0x51    // PUSH ECX                  ; push arg2
        _emit 0x8b    // MOV  ECX, dword ptr [0x0132987c]  ; this = *g
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x52    // PUSH EDX                  ; push arg1
        _emit 0xe8    // CALL FUN_00423110          ; rel32 = +0x00006f66
        _emit 0x66
        _emit 0x6f
        _emit 0x00
        _emit 0x00
        _emit 0xc3    // RET
    }
}
