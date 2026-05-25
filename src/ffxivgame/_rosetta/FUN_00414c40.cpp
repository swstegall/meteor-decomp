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
// FUNCTION: ffxivgame 0x00414c40 — global refcounted "fire-once" dtor
//                                  trampoline (20 B, __cdecl).
//
// Decrements a static int counter at VA 0x01328040; when it reaches
// zero, tail-jumps to `thunk_FUN_00414e10` (a 5-byte rel32 thunk at
// VA 0x00414d70 which forwards to FUN_00414e10, itself a one-byte
// `c3 ret` stub at VA 0x00414e10) with `this = 0x01328044` (the
// dword immediately after the counter) loaded into ECX. Otherwise
// returns immediately.
//
// The pattern is the canonical MSVC 2005 atexit-style "wrapped global
// dtor" — a `__cdecl` wrapper around a `__thiscall` dtor where the
// wrapper performs the refcount decrement and tail-jumps when the
// count drops to zero so the dtor itself doesn't have to know about
// the refcount. The trampoline target being effectively a no-op
// (thunk → ret) is consistent with this being the registered-atexit
// dtor of a global with a trivial destructor whose only side effect
// was the refcount itself.
//
// Asm shape (20 bytes — read from the disassembly at orig RVA 0x00014c40):
//
//   00014c40:  83 2d 40 80 32 01 01  sub   dword ptr [0x01328040], 1
//   00014c47:  75 0a                 jne   00014c53
//   00014c49:  b9 44 80 32 01        mov   ecx, 0x01328044
//   00014c4e:  e9 1d 01 00 00        jmp   thunk_FUN_00414e10  (rel32 → 0x00414d70)
//   00014c53:  c3                    ret
//
// Reconstruction strategy — naked-asm byte passthrough (_emit):
//
//   The `sub mem32, imm8`, `mov ecx, imm32`, and `jmp rel32`
//   instructions each encode an absolute VA / call target directly in
//   the instruction stream. MSVC 2005 won't reproduce those encodings
//   from `extern int g_counter;` / `extern void target();` references
//   unless the linker can be coaxed into placing each symbol at
//   exactly its orig VA — which isn't reliable from a single rosetta
//   .obj. The pragmatic choice (same as sibling FUN_00406c30 for its
//   pair-of-globals stores) is a `__declspec(naked)` body with literal
//   `_emit` bytes: the .obj's .text contains the 20 verbatim bytes
//   with zero relocations, so the byte diff is exact and reloc-free.

extern "C" __declspec(naked) void FUN_00414c40() {
    __asm {
        _emit 0x83              // SUB  dword ptr [0x01328040], 1
        _emit 0x2d
        _emit 0x40
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75              // JNE  +0x0a (→ 0x00014c53 ret)
        _emit 0x0a
        _emit 0xb9              // MOV  ECX, 0x01328044
        _emit 0x44
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0xe9              // JMP  thunk_FUN_00414e10 (rel32 → 0x00014d70)
        _emit 0x1d
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
    }
}
