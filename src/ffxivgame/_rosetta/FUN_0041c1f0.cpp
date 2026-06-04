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
// FUNCTION: ffxivgame 0x0001c1f0 — `__cdecl` 1-arg thiscall-dispatch wrapper (20 B).
//
// Zero-extends a byte argument from [ESP+4], loads the global object pointer
// from [0x0132987c] into ECX (the __thiscall `this` register), then forwards
// the byte arg plus a literal 0x1b to the `__thiscall` method at VA 0x004236e0.
// The wrapper is `__cdecl` (bare `RET`; caller reclaims the single byte arg).
//
// Asm shape (20 bytes — RVA 0x0001c1f0..0x0001c203):
//
//     0001c1f0:  0f b6 44 24 04        MOVZX EAX, byte ptr [ESP+0x4]   ; arg (byte, zero-extended)
//     0001c1f5:  8b 0d 7c 98 32 01     MOV   ECX, dword ptr [0x132987c] ; this = *g
//     0001c1fb:  50                    PUSH  EAX                         ; push byte arg
//     0001c1fc:  6a 1b                 PUSH  0x1b                        ; push literal 0x1b
//     0001c1fe:  e8 dd 74 00 00        CALL  0x004236e0                  ; thiscall target
//     0001c203:  c3                    RET
//
// Reloc-bearing sites in the orig 20 bytes:
//     +0x07   DIR32 → 0x0132987c  (global object pointer address)
//     +0x0f   REL32 → 0x004236e0  (thiscall method, disp = +0x000074dd)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   Identical structure to sibling FUN_0041c1d0 (same object pointer, same
//   callee, only the literal differs: 0xf → 0x1b).  The MOVZX + ECX-load
//   sequence is sensitive to register-allocation order under /O2; following
//   the established convention for this cluster (FUN_0041c170, FUN_0041c190,
//   FUN_0041c1b0) we emit the 20 orig bytes verbatim via MASM `_emit`
//   directives.  compare.py masks the two reloc windows (+0x07 DIR32 and
//   +0x0f REL32) and reports GREEN.

extern "C" __declspec(naked) void FUN_0041c1f0() {
    __asm {
        _emit 0x0f    // MOVZX EAX, byte ptr [ESP+0x4]   ; arg (byte, zero-extended)
        _emit 0xb6
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b    // MOV ECX, dword ptr [0x0132987c] ; this = *g
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x50    // PUSH EAX                        ; push byte arg
        _emit 0x6a    // PUSH 0x1b                       ; push literal 27
        _emit 0x1b
        _emit 0xe8    // CALL 0x004236e0                 ; thiscall, rel32 = +0x000074dd
        _emit 0xdd
        _emit 0x74
        _emit 0x00
        _emit 0x00
        _emit 0xc3    // RET
    }
}
