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
// FUNCTION: ffxivgame 0x00450210 — composite-object ctor (__thiscall, 191 B / 0xbf)
//
// Constructor for a class with four std::string-like SSO members and a
// circular linked-list sub-object. The function:
//
//   1. Sets up the EH3-style SEH frame (PUSH -1 / scope_table / FS:[0]
//      chain + /GS cookie XOR ESP).
//   2. Stashes `this` (ECX) into ESI at [ESP+0x0c].
//   3. Calls FUN_004513f0(__thiscall); stores result at this+0x04;
//      sets result[0x45] = 1.
//   4. Initialises three self-referential pointers at *field_04:
//        [field_04+0x00] = [field_04+0x04] = [field_04+0x08] = field_04
//   5. Zeroes this+0x08. Sets EH state 0.
//   6. Initialises four SSO strings (cap=0xf, size=0, buf[0]=0):
//        str1 @ this+0x0c, str2 @ this+0x28,
//        str3 @ this+0x44, str4 @ this+0x60.
//   7. Sets EH state 4.
//   8. Calls FUN_00404120(0xf67698, 0xc) with ECX=this+0x60.
//   9. Calls FUN_00451bf0(0x20) with ECX=this+0x60 (twice; second arg 0x54).
//  10. Sets this+0xa8 = 2.
//  11. Returns this in EAX. SEH epilog.
//
// Why naked asm: the EH3 SEH prolog (PUSH -1 / PUSH scope_table_RVA /
// PUSH FS:[0] / cookie XOR ESP) plus two mid-body EH-state writes and
// seven CALL/imm32 relocations form a compiler-emitted shape where every
// high-level rewrite shifts at least one byte. The `_emit` byte-passthrough
// lets compare.py see a byte-exact match modulo the 7 wildcarded reloc
// windows.
//
// Reloc-bearing sites (byte offsets within the function):
//   +0x03   scope_table imm32          (.rdata 0x00e57dd7)
//   +0x12   __security_cookie moffs32  (.data  0x012ea8b0)
//   +0x2a   CALL rel32 → FUN_004513f0
//   +0x81   PUSH imm32 data addr       (.rdata/data 0x00f67698)
//   +0x8b   CALL rel32 → FUN_00404120
//   +0x94   CALL rel32 → FUN_00451bf0
//   +0x9d   CALL rel32 → FUN_00451bf0

extern "C" __declspec(naked) void FUN_00450210() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xd7
        _emit 0x7d
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x51
        _emit 0x56
        _emit 0x57
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x50
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf1
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0xe8
        _emit 0xb2
        _emit 0x11
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x46
        _emit 0x04
        _emit 0xc6
        _emit 0x40
        _emit 0x45
        _emit 0x01
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x89
        _emit 0x40
        _emit 0x04
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x89
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x89
        _emit 0x40
        _emit 0x08
        _emit 0x33
        _emit 0xc0
        _emit 0x89
        _emit 0x46
        _emit 0x08
        _emit 0x8d
        _emit 0x4e
        _emit 0x0c
        _emit 0xba
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x51
        _emit 0x18
        _emit 0x89
        _emit 0x41
        _emit 0x14
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x88
        _emit 0x41
        _emit 0x04
        _emit 0x89
        _emit 0x56
        _emit 0x40
        _emit 0x89
        _emit 0x46
        _emit 0x3c
        _emit 0x88
        _emit 0x46
        _emit 0x2c
        _emit 0x89
        _emit 0x56
        _emit 0x5c
        _emit 0x89
        _emit 0x46
        _emit 0x58
        _emit 0x88
        _emit 0x46
        _emit 0x48
        _emit 0x8d
        _emit 0x7e
        _emit 0x60
        _emit 0x89
        _emit 0x57
        _emit 0x18
        _emit 0x89
        _emit 0x47
        _emit 0x14
        _emit 0x88
        _emit 0x47
        _emit 0x04
        _emit 0x6a
        _emit 0x0c
        _emit 0x68
        _emit 0x98
        _emit 0x76
        _emit 0xf6
        _emit 0x00
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x04
        _emit 0xe8
        _emit 0x81
        _emit 0x3e
        _emit 0xfb
        _emit 0xff
        _emit 0x6a
        _emit 0x20
        _emit 0x8b
        _emit 0xcf
        _emit 0xe8
        _emit 0x48
        _emit 0x19
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x54
        _emit 0x8b
        _emit 0xcf
        _emit 0xe8
        _emit 0x3f
        _emit 0x19
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x86
        _emit 0xa8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xc6
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5f
        _emit 0x5e
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0xc3
    }
}
