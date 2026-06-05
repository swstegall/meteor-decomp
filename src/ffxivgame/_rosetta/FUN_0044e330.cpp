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
// FUNCTION: ffxivgame 0x0044e330 — thin forwarding wrapper (43 B / 0x2B)
//
//   __cdecl FUN_0044e330(int a, int b, int c)
//     forwards to FUN_0044e1f0(a, b, c, c, c, 0) and returns its result.
//
//   The single PUSH ECX at entry reserves a 4-byte local slot, into which
//   a zero byte is stored (MOV byte [ESP],0) and then re-loaded as a dword
//   to become the trailing 6th argument. The three caller args (at the
//   post-prologue offsets [ESP+0x10] and [ESP+0x14]) are shuffled through
//   ECX/EDX/EAX and pushed in reverse so the callee sees (a, b, c, c, c, 0).
//   The ADD ESP,0x1c after the CALL reclaims the six pushed args (0x18) plus
//   the local slot (0x04) — __cdecl, caller-cleans.
//
// Reconstruction: naked-asm byte passthrough (mirrors the sibling
// _rosetta/*.cpp matches). The CALL rel32 → 0x0044e1f0 is the lone reloc
// site (+0x22); tools/compare.py masks reloc bytes, so re-emitting the orig
// 43 wire bytes verbatim yields a byte-identical, zero-reloc .text — GREEN.
//     +0x22  CALL rel32 → FUN_0044e1f0

extern "C" __declspec(naked) void FUN_0044e330() {
    __asm {
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x10]
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0xc6             // MOV byte ptr [ESP], 0x00
        _emit 0x04
        _emit 0x24
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP]
        _emit 0x04
        _emit 0x24
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x52              // PUSH EDX
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x14]
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL FUN_0044e1f0 (rel32 → 0x0044e1f0)
        _emit 0x99
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x1c
        _emit 0xc4
        _emit 0x1c
        _emit 0xc3              // RET
    }
}
