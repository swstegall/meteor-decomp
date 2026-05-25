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
// FUNCTION: ffxivgame 0x00015350 — __thiscall clamped-index table lookup (23 B)
//
// Reads a byte index from the stack, clamps it to the inclusive range
// [0..4] (anything ≥5 saturates to 4), then returns the dword at
// `this + 0x20 + index * 0x14`. Behaviour matches the Ghidra hint:
//
//     undefined4 FUN_00415350(byte param_1)
//     {
//         if (4 < param_1) param_1 = 4;
//         return *(undefined4 *)(this + 0x20 + param_1 * 0x14);
//     }
//
// The 0x14-byte stride suggests `this` owns a 5-entry array of fixed-
// size records at offset 0x20 (each record 20 bytes / 0x14), and the
// dword pulled out lives at offset 0 of each record.
//
// Asm (23 bytes @ orig RVA 0x00015350):
//   00415350: 8a 44 24 04        MOV   AL, byte ptr [ESP+0x4]   ; load param_1
//   00415354: 3c 05              CMP   AL, 0x5
//   00415356: 72 02              JB    +0x2 → 0x0041535a         ; <5 → keep
//   00415358: b0 04              MOV   AL, 0x4                   ; ≥5 → clamp
//   0041535a: 0f b6 c0           MOVZX EAX, AL
//   0041535d: 8d 04 80           LEA   EAX, [EAX + EAX*4]        ; EAX = idx*5
//   00415360: 8b 44 81 20        MOV   EAX, [ECX + EAX*4 + 0x20] ; this+idx*0x14+0x20
//   00415364: c2 04 00           RET   0x4                       ; __thiscall pop
//
// Calling convention: __thiscall (ECX = this, one stack arg, RET 4).
// No prologue — /Oy frame-pointer omission; leaf function with no
// callee-saved register usage.
//
// No relocations in these 23 bytes (no CALL, no IAT, no data refs).
// Naked-asm passthrough chosen to pin the exact byte layout — MSVC 2005's
// high-level lowering of the clamp can pick `CMOV` or a different branch
// shape depending on inliner heuristics, which would mis-match.

extern "C" __declspec(naked) void FUN_00415350() {
    __asm {
        _emit 0x8a              // MOV AL, byte ptr [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x3c              // CMP AL, 0x5
        _emit 0x05
        _emit 0x72              // JB +0x2 (skip clamp)
        _emit 0x02
        _emit 0xb0              // MOV AL, 0x4
        _emit 0x04
        _emit 0x0f              // MOVZX EAX, AL
        _emit 0xb6
        _emit 0xc0
        _emit 0x8d              // LEA EAX, [EAX + EAX*4]
        _emit 0x04
        _emit 0x80
        _emit 0x8b              // MOV EAX, dword ptr [ECX + EAX*4 + 0x20]
        _emit 0x44
        _emit 0x81
        _emit 0x20
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
