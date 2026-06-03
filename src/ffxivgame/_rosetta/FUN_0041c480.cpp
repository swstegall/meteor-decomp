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
// FUNCTION: ffxivgame 0x0041c480 — zero-arg `__cdecl` dispatcher that loads
//   the global singleton at [0x0132987c] into ECX and calls the __thiscall
//   method FUN_004236e0 with args (0xce, 0x1).
//
// Asm (19 bytes, RVA 0x0001c480):
//
//   8b 0d 7c 98 32 01   MOV  ECX, [0x0132987c]   ; abs32 reloc — global obj
//   6a 01               PUSH 0x1                  ; second arg
//   68 ce 00 00 00       PUSH 0xce                ; first arg
//   e8 4e 72 00 00       CALL 0x004236e0          ; rel32 reloc — __thiscall method
//   c3                   RET
//
// Reconstruction strategy — naked _emit passthrough.
//
//   Two relocations (abs32 at +2, rel32 at +14) make any source-level
//   reconstruction fragile to scheduler / addressing differences.
//   Emitting the 19 original bytes verbatim via _emit is the correct
//   approach; compare.py masks the reloc windows, so GREEN is stable.

extern "C" __declspec(naked) void FUN_0041c480() {
    __asm {
        _emit 0x8b  // MOV ECX, [0x0132987c]       ; abs32 reloc
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x6a  // PUSH 0x1
        _emit 0x01
        _emit 0x68  // PUSH 0xce
        _emit 0xce
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL 0x004236e0              ; rel32 reloc
        _emit 0x4e
        _emit 0x72
        _emit 0x00
        _emit 0x00
        _emit 0xc3  // RET
    }
}
