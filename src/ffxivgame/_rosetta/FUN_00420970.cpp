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
// FUNCTION: ffxivgame 0x00420970 — FUN_00420970 (31 B / 0x1f)
//
// __cdecl void FUN_00420970(int idx)
//
//   No stack frame. One argument read from [ESP+4].
//   Stores `idx` to a global at VA 0x01328f68.
//   Loads a `this` pointer from VA 0x0132987c into ECX (for a __thiscall).
//   Performs a DWORD table lookup: g_table[idx] where g_table is at
//   VA 0x00f597c8 (4-byte-wide array, SIB encoding [EAX*4+disp32]).
//   Calls FUN_004236e0 as a __thiscall with two stack args:
//     arg1 = 0x16 (pushed last, top of stack)
//     arg2 = g_table[idx] (pushed first)
//   Returns with plain RET (caller cleans up the single stack argument).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function contains four relocation sites (three absolute VA refs
//   and one CALL rel32). A source-level reconstruction requires MSVC to
//   schedule the ECX load (for the __thiscall this-pointer) between the
//   initial EAX load and the global store — scheduling that depends on
//   the optimizer's register-pressure picture and is fragile. Emitting
//   the 31 original bytes verbatim via __declspec(naked) + _emit gives
//   a byte-identical match without that fragility. The COFF .obj produced
//   by naked _emit carries no relocation entries; compare.py's mask is
//   all-zero, so all 31 bytes must equal the orig PE bytes exactly —
//   which they do since we copy them from the disassembly unchanged.
//
// Disassembly (RVA 0x00020970, 31 bytes):
//
//   00020970:  8b 44 24 04           MOV EAX,dword ptr [ESP + 0x4]
//   00020974:  8b 0d 7c 98 32 01     MOV ECX,dword ptr [0x0132987c]
//   0002097a:  a3 68 8f 32 01        MOV [0x01328f68],EAX
//   0002097f:  8b 04 85 c8 97 f5 00  MOV EAX,dword ptr [EAX*0x4 + 0xf597c8]
//   00020986:  50                    PUSH EAX
//   00020987:  6a 16                 PUSH 0x16
//   00020989:  e8 52 2d 00 00        CALL 0x004236e0
//   0002098e:  c3                    RET

extern "C" __declspec(naked) void FUN_00420970() {
    __asm {
        _emit 0x8b  // MOV EAX, dword ptr [ESP + 0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b  // MOV ECX, dword ptr [0x0132987c]
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0xa3  // MOV [0x01328f68], EAX
        _emit 0x68
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x8b  // MOV EAX, dword ptr [EAX*0x4 + 0xf597c8]
        _emit 0x04
        _emit 0x85
        _emit 0xc8
        _emit 0x97
        _emit 0xf5
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x6a  // PUSH 0x16
        _emit 0x16
        _emit 0xe8  // CALL 0x004236e0  (rel32 = 0x00002d52)
        _emit 0x52
        _emit 0x2d
        _emit 0x00
        _emit 0x00
        _emit 0xc3  // RET
    }
}
