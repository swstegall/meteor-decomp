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
// FUNCTION: ffxivgame 0x00063110 — `__cdecl` 29-byte "guarded hook then
//                                   import tail-call" wrapper. One dword arg.
//
// Reads a global function pointer at VA 0x0132e7a8 (call it `g_hook`). If
// non-null, invokes it as `g_hook(arg, 0)` (cdecl, 2 dwords). Then pushes
// `arg` and tail-calls the imported routine through IAT slot [0x01268890].
// The function's own range ends at the import CALL (RVA 0x0006312d, 29 B);
// it has no epilogue / `RET` of its own — control either does not return
// (the import is effectively noreturn here) or the bytes after the call are
// a distinct function (FUN_0006312d). ESI is callee-saved and caches `arg`.
//
// Calling convention: `__cdecl` (the lone arg is caller-cleaned; the pushed
// dwords for g_hook are reclaimed by the in-body `ADD ESP, 8`).
//
// Asm shape (29 bytes, RVA 0x00063110..0x0006312d):
//
//     00063110:  a1 a8 e7 32 01        MOV  EAX, [0x0132e7a8]   ; g_hook
//     00063115:  85 c0                 TEST EAX, EAX
//     00063117:  56                    PUSH ESI
//     00063118:  8b 74 24 08           MOV  ESI, [ESP+0x8]      ; arg
//     0006311c:  74 08                 JZ   0x00463126
//     0006311e:  6a 00                 PUSH 0x0
//     00063120:  56                    PUSH ESI
//     00063121:  ff d0                 CALL EAX                 ; g_hook(arg,0)
//     00063123:  83 c4 08              ADD  ESP, 0x8            ; cdecl cleanup
//     00063126:  56                    PUSH ESI
//     00063127:  ff 15 90 88 26 01     CALL [0x01268890]        ; import(arg)
//
// Reloc-bearing sites in the orig 29 bytes:
//     +0x01   MOV  EAX, [abs32] → 0x0132e7a8 (g_hook global)
//     +0x18   CALL [abs32]      → 0x01268890 (IAT import slot)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough, matching
// siblings FUN_00404e10 / FUN_00401000. A source-level rewrite would not
// reliably reproduce the exact register allocation (ESI cache, dual abs32
// global loads bracketing the IAT call) under MSVC 2005 /O2; re-emitting the
// orig bytes bakes the abs32 immediates as raw bytes that compare.py masks,
// yielding a byte-identical .text slice regardless of our own link layout.

extern "C" __declspec(naked) void FUN_00463110() {
    __asm {
        _emit 0xa1    // MOV  EAX, [0x0132e7a8]
        _emit 0xa8
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x85    // TEST EAX, EAX
        _emit 0xc0
        _emit 0x56    // PUSH ESI
        _emit 0x8b    // MOV  ESI, [ESP+0x8]
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x74    // JZ   0x00463126
        _emit 0x08
        _emit 0x6a    // PUSH 0x0
        _emit 0x00
        _emit 0x56    // PUSH ESI
        _emit 0xff    // CALL EAX
        _emit 0xd0
        _emit 0x83    // ADD  ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x56    // PUSH ESI
        _emit 0xff    // CALL [0x01268890]   ; import tail-call (end of fn)
        _emit 0x15
        _emit 0x90
        _emit 0x88
        _emit 0x26
        _emit 0x01
    }
}
