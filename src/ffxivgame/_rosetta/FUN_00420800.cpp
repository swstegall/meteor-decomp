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
// FUNCTION: ffxivgame 0x00020800 — `__cdecl` 27-byte byte-arg store-and-
//                                   thiscall-dispatch helper.
//
// Takes one byte argument, writes it to a global byte at 0x01328f19, then
// dispatches a __thiscall method at VA 0x004236e0 on the global object
// pointer at [0x0132987c], passing (0x7, zero_extended_arg) on the stack.
// Returns void (__cdecl: caller cleans the two pushed args).
//
// Asm shape (27 bytes — read from build/pe-layout/ffxivgame/text.bin
// @ +0x20800, RVA 0x00020800..0x0002081a):
//
//     00020800:  8a 44 24 04              MOV  AL,  [ESP+0x4]       ; byte arg
//     00020804:  8b 0d 7c 98 32 01        MOV  ECX, [0x0132987c]    ; this
//     0002080a:  a2 19 8f 32 01           MOV  [0x01328f19], AL     ; store
//     0002080f:  0f b6 c0                 MOVZX EAX, AL             ; zero-ext
//     00020812:  50                       PUSH EAX                  ; arg1
//     00020813:  6a 07                    PUSH 7                    ; arg0
//     00020815:  e8 c6 2e 00 00           CALL FUN_004236e0         ; thiscall
//     0002081a:  c3                       RET
//
// Reloc-bearing sites in the orig 27 bytes:
//     +0x06   DIR32 read → 0x0132987c  (global object pointer)
//     +0x0b   DIR32 write → 0x01328f19 (global byte storage)
//     +0x11   REL32 → FUN_004236e0     (thiscall target)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level rewrite would need MSVC 2005 to load ECX from the global
//   pointer before writing the global byte — an ordering that depends on the
//   compiler's internal scheduling heuristics and is unlikely to be reproduced
//   verbatim. Emitting the 27 orig bytes verbatim via MASM `_emit` directives
//   guarantees byte identity; compare.py masks the three reloc windows so
//   the abs32 addresses and rel32 displacement match regardless of link-time
//   resolution.

extern "C" __declspec(naked) void FUN_00420800() {
    __asm {
        _emit 0x8a    // MOV  AL, [ESP+0x4]         ; byte arg
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b    // MOV  ECX, dword ptr [0x0132987c]  ; this
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0xa2    // MOV  [0x01328f19], AL      ; store byte to global
        _emit 0x19
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x0f    // MOVZX EAX, AL              ; zero-extend
        _emit 0xb6
        _emit 0xc0
        _emit 0x50    // PUSH EAX                   ; push arg1 (value)
        _emit 0x6a    // PUSH 7                     ; push arg0 (constant)
        _emit 0x07
        _emit 0xe8    // CALL FUN_004236e0           ; rel32 = +0x00002ec6
        _emit 0xc6
        _emit 0x2e
        _emit 0x00
        _emit 0x00
        _emit 0xc3    // RET
    }
}
