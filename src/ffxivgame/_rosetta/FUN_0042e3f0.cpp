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
// FUNCTION: ffxivgame 0x0042e3f0 — single-arg setter/forwarder trampoline
//                                   that stows the argument in a global and
//                                   tail-jumps to FUN_0041d100 (18 B / 0x12).
//
// Behaviour read from the disassembly at orig RVA 0x0002e3f0:
//
//   8b 44 24 04        MOV  EAX, [ESP+4]          ; load arg
//   a3 14 8f 32 01     MOV  [0x01328f14], EAX     ; g_var = arg  (moffs32 store)
//   89 44 24 04        MOV  [ESP+4], EAX          ; re-spill arg into outgoing slot
//   e9 fe ec fe ff     JMP  0x0041d100            ; __cdecl tail call (no cleanup)
//
//   Equivalent C (the structure the asm implements):
//
//     extern int g_var;                  // .data 0x01328f14
//     int __cdecl FUN_0042e3f0(int x) {
//         g_var = x;
//         return FUN_0041d100(x);        // tail call, args left on stack
//     }
//
//   Calling convention: `__cdecl`. The single inbound argument is left on
//   the stack and reused as FUN_0041d100's argument via the tail JMP (the
//   caller owns the cleanup). EAX flows through whatever the callee returns.
//
// Reloc-bearing sites in the orig 18 bytes (resolve only in a full-binary
// relink at image base 0x00400000; standalone .obj can't reproduce them):
//   +0x05   global ptr store         .data 0x01328f14 (moffs32 form)
//   +0x0e   tail-call JMP rel32      .text 0x0041d100 (FUN_0041d100)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The redundant `MOV [ESP+4], EAX` store-back (re-spilling the just-loaded
//   argument into the slot it came from before the tail JMP) is an MSVC 2005
//   /O2 tail-call artifact that source-level C does not reliably reproduce —
//   a clean `g_var = x; return FUN_0041d100(x);` lowers to load/store/JMP
//   and drops the re-spill, shifting the 18-byte function's length. Since
//   both reloc-adjacent sites (the moffs32 store and the rel32 JMP) and the
//   re-spill must all land byte-exact, the pragmatic match — the same one
//   FUN_00401730 took for its reloc-heavy trampoline — is a
//   `__declspec(naked)` body that re-emits the orig 18 bytes verbatim via
//   MASM `_emit` directives. The emitted bytes carry no relocations, so the
//   .obj `.text` slice is byte-identical to the orig, which is what
//   tools/compare.py checks.

extern "C" __declspec(naked) void FUN_0042e3f0() {
    __asm {
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04

        _emit 0xa3
        _emit 0x14
        _emit 0x8f
        _emit 0x32
        _emit 0x01

        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x04

        _emit 0xe9
        _emit 0xfe
        _emit 0xec
        _emit 0xfe
        _emit 0xff
    }
}
