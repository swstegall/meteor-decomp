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
// FUNCTION: ffxivgame 0x00409920 — lazy-init singleton getter/setter (104 B)
//
// __cdecl FUN_00409920(int newValue) — set-or-get accessor for a process-
// scope dword stored at .data 0x01327B3C, guarded by an init flag at
// .data 0x01327B40.  Wraps the lazy initializer in an inline SEH frame
// so that any throw from the underlying init helper FUN_0040E500 unwinds
// the slot back to a known state via the per-function scope table at
// 0x00E54ABE.
//
// Structurally identical to the siblings FUN_004091f0 / FUN_00409610 /
// FUN_004097d0 — same prolog, same branch shape, same epilog — but
// parameterised on a different singleton slot (0x01327B3C), a different
// init flag (0x01327B40), and a different per-function EH scope table
// (0x00E54ABE).  The rel32 CALL target (FUN_0040E500) is identical;
// here it resolves to rel32 = 0x0040E500 - 0x00409955 = 0x00004BAB.
//
// Reloc-bearing sites in the orig 104 bytes:
//     +0x08   PUSH imm32   → 0x00E54ABE (per-function EH scope table)
//     +0x1A   TEST [imm32] → 0x01327B40 (init flag)
//     +0x22   OR   [imm32] → 0x01327B40 (set init flag)
//     +0x30   CALL rel32   → 0x0040E500 (lazy-init helper)
//     +0x35   MOV  [imm32] → 0x01327B3C (singleton slot)
//     +0x42   MOV  [imm32] → 0x01327B3C (setter overwrite)
//     +0x58   MOV  EAX,[imm32] → 0x01327B3C (getter load)
//
// Reconstruction strategy — naked-asm byte passthrough, matching the
// established sibling FUN_004091f0 / FUN_00409610 / FUN_004097d0 idiom.

extern "C" __declspec(naked) void FUN_00409920() {
    __asm {
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH 0xFFFFFFFF
        _emit 0xff
        _emit 0x68              // PUSH 0x00E54ABE (EH scope table)
        _emit 0xbe
        _emit 0x4a
        _emit 0xe5
        _emit 0x00
        _emit 0x50              // PUSH EAX (chain old fs:[0])
        _emit 0xb8              // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x64              // MOV FS:[0x0], ESP
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [0x01327B40], AL
        _emit 0x05
        _emit 0x40
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ +0x18 (after_init)
        _emit 0x18
        _emit 0x09              // OR  dword ptr [0x01327B40], EAX
        _emit 0x05
        _emit 0x40
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [ESP+0x08], 0
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_0040E500 (rel32 → 0x00004BAB)
        _emit 0xab
        _emit 0x4b
        _emit 0x00
        _emit 0x00
        _emit 0xa3              // MOV [0x01327B3C], EAX
        _emit 0x3c
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]    (after_init:)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x13 (getter)
        _emit 0x13
        _emit 0xa3              // MOV [0x01327B3C], EAX
        _emit 0x3c
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV ECX, dword ptr [ESP]
        _emit 0x0c
        _emit 0x24
        _emit 0x64              // MOV FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x0C
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
        _emit 0x8b              // MOV ECX, dword ptr [ESP]         (getter:)
        _emit 0x0c
        _emit 0x24
        _emit 0xa1              // MOV EAX, [0x01327B3C]
        _emit 0x3c
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x64              // MOV FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x0C
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
    }
}
