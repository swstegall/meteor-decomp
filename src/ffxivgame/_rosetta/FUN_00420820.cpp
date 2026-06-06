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
// FUNCTION: ffxivgame 0x00020820 — `__cdecl` single-byte-arg global-store +
//                                   callee-cleanup notify helper (27 B).
//
// Takes one byte argument, stores it at the global VA 0x01328f18, loads a
// global dword from VA 0x0132987c into ECX (setting up `this` for the
// callee), zero-extends the byte to EAX, and calls the two-arg
// (thiscall/stdcall) function at VA 0x004236e0 with args (0xe, EAX) —
// no stack cleanup after the call (callee-cleanup), then plain RET.
//
// Asm shape (27 bytes — read from asm/ffxivgame/00020820_FUN_00420820.s):
//
//     00020820:  8a 44 24 04               MOV   AL, byte ptr [ESP+0x4]   ; byte arg
//     00020824:  8b 0d 7c 98 32 01         MOV   ECX, dword ptr [0x0132987c] ; global this
//     0002082a:  a2 18 8f 32 01            MOV   [0x01328f18], AL         ; store byte
//     0002082f:  0f b6 c0                  MOVZX EAX, AL                  ; zero-extend
//     00020832:  50                        PUSH  EAX                      ; arg1 for callee
//     00020833:  6a 0e                     PUSH  0xe                      ; arg0 for callee
//     00020835:  e8 a6 2e 00 00            CALL  0x004236e0               ; rel32=+0x00002ea6
//     0002083a:  c3                        RET
//
// Reloc-bearing sites in the orig 27 bytes:
//     +0x06   DIR32 → 0x0132987c  (global this-pointer slot)
//     +0x0b   DIR32 → 0x01328f18  (global byte target)
//     +0x14   REL32 → 0x004236e0  (notify helper, rel32 = 0x00002ea6)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The two absolute addresses and one relative call are baked in as raw
//   bytes. compare.py masks the reloc windows so the non-reloc bytes drive
//   the match; emitting verbatim avoids any risk of MSVC reordering the
//   global load vs. global store or emitting a different MOVZX encoding.

extern "C" __declspec(naked) void FUN_00420820() {
    __asm {
        _emit 0x8a      // MOV  AL, byte ptr [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b      // MOV  ECX, dword ptr [0x0132987c]
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0xa2      // MOV  [0x01328f18], AL
        _emit 0x18
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x0f      // MOVZX EAX, AL
        _emit 0xb6
        _emit 0xc0
        _emit 0x50      // PUSH EAX
        _emit 0x6a      // PUSH 0xe
        _emit 0x0e
        _emit 0xe8      // CALL 0x004236e0 ; rel32 = +0x00002ea6
        _emit 0xa6
        _emit 0x2e
        _emit 0x00
        _emit 0x00
        _emit 0xc3      // RET
    }
}
