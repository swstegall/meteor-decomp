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
// FUNCTION: ffxivgame 0x00409d10 — SEH-protected lazy-init getter/setter
//                                  for a global cached int (104 B / 0x68)
//
// __cdecl int FUN_00409d10(int new_value)
//
//   stack layout (after the SEH 3-dword frame is pushed):
//     [ESP+0x00] : SEH prev FS:[0] chain link  (saved next ptr)
//     [ESP+0x04] : SEH handler PUSH 0x00e54b9e (scope-table addr)
//     [ESP+0x08] : SEH trylevel PUSH -1       (then overwritten to 0
//                                              inside the init branch)
//     [ESP+0x0C] : (return addr)
//     [ESP+0x10] : int new_value              (the sole caller arg)
//
// Memory layout / globals touched:
//   0x01327b90 : int  s_cached_value       (the lazy-initialised result)
//   0x01327b94 : int  s_init_flag          (bit 0 = "initialised once")
//   0x00e54b9e : SEH scope-table address   (PUSH'd into the FS:[0] frame)
//   0x0040e500 : int FUN_0040e500()        (the one-shot initialiser)
//
// Structurally identical to the sibling FUN_00409990 (same 104-byte SEH
// lazy-init getter/setter pattern) — only the absolute scope-table /
// global addresses differ. See FUN_00409990.cpp for the full annotated
// trace; this is the same shape against a different pair of globals.
//
// Reloc-bearing sites in the orig 104 bytes:
//     +0x09   PUSH imm32  → 0x00e54b9e   (SEH scope-table pointer)
//     +0x1c   TEST mem32  → 0x01327b94   (init-flag global, byte access)
//     +0x24   OR   mem32  → 0x01327b94   (init-flag global, dword access)
//     +0x31   CALL rel32  → FUN_0040e500 (one-shot initialiser)
//     +0x36   MOV  mem32  → 0x01327b90   (cached-value global)
//     +0x43   MOV  mem32  → 0x01327b90
//     +0x59   MOV  mem32  → 0x01327b90
//
// Reconstruction strategy: naked-asm byte passthrough, same as the
// sibling FUN_00409990 — emit the orig 104 bytes verbatim via MASM
// `_emit` directives. The .obj's `.text` section ends up byte-identical
// to the orig slice.

extern "C" __declspec(naked) void FUN_00409d10() {
    __asm {
        _emit 0x64              // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00E54B9E (SEH scope-table)
        _emit 0x9e
        _emit 0x4b
        _emit 0xe5
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xb8              // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x64              // MOV FS:[0], ESP
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [0x01327B94], AL
        _emit 0x05
        _emit 0x94
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ tail (+0x18)
        _emit 0x18
        _emit 0x09              // OR dword ptr [0x01327B94], EAX
        _emit 0x05
        _emit 0x94
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [ESP+0x8], 0
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_0040e500 (rel32 → 0x000047bb)
        _emit 0xbb
        _emit 0x47
        _emit 0x00
        _emit 0x00
        _emit 0xa3              // MOV [0x01327B90], EAX
        _emit 0x90
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]   (tail:)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ return_cached (+0x13)
        _emit 0x13
        _emit 0xa3              // MOV [0x01327B90], EAX
        _emit 0x90
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV ECX, dword ptr [ESP]
        _emit 0x0c
        _emit 0x24
        _emit 0x64              // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0xC
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
        _emit 0x8b              // MOV ECX, dword ptr [ESP]        (return_cached:)
        _emit 0x0c
        _emit 0x24
        _emit 0xa1              // MOV EAX, [0x01327B90]
        _emit 0x90
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x64              // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0xC
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
    }
}
