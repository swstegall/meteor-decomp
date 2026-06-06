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
// FUNCTION: ffxivgame 0x00016e90 — aligned-size calculator (41 B / 0x29)
//
// int __cdecl FUN_00416e90(int a, int b, int c, int d, int e)
//
// Computes: (c + b + d) * a + align4(e)
// where align4(e) = (e + 3) & ~3 (round e up to the next multiple of 4).
// Returns 0 immediately if a == 0.
//
// Stack layout ([ESP+N] after CALL, no frame pointer — /Oy):
//   [ESP+0x04] : int a  (param 1 — the row-count / multiplier; tested for zero)
//   [ESP+0x08] : int b  (param 2 — summand)
//   [ESP+0x0c] : int c  (param 3 — summand, loaded into EAX first)
//   [ESP+0x10] : int d  (param 4 — summand, added from memory)
//   [ESP+0x14] : int e  (param 5 — base for the 4-byte alignment bump)
//
// Calling convention: __cdecl (caller cleans; plain RET at every exit).
// Frame: none.  No callee-saves (no ESI/EDI/EBX push/pop).
//
// Register use:
//   ECX — holds a (loaded first for the early-zero test; reloaded with e later)
//   EAX — accumulator for the sum then the multiply result
//   EDX — scratch for b (MSVC 2005 /O2 pipelines the two loads before the ADD)
//
// No external calls; no absolute-address relocations.  All 41 bytes are
// literal arithmetic — naked _emit passthrough is the safest match strategy.
//
// Asm (41 bytes @ orig RVA 0x00016e90):
//   8b 4c 24 04   MOV ECX, dword ptr [ESP+0x04]    ; a
//   85 c9         TEST ECX, ECX
//   75 03         JNZ  +0x03 (→ 0x00416e9b)
//   33 c0         XOR  EAX, EAX                     ; return 0
//   c3            RET
//   8b 44 24 0c   MOV EAX, dword ptr [ESP+0x0c]    ; c
//   8b 54 24 08   MOV EDX, dword ptr [ESP+0x08]    ; b → EDX (pipelined)
//   03 c2         ADD  EAX, EDX                     ; EAX = c + b
//   03 44 24 10   ADD  EAX, dword ptr [ESP+0x10]   ; EAX += d
//   0f af c1      IMUL EAX, ECX                     ; EAX *= a
//   8b 4c 24 14   MOV ECX, dword ptr [ESP+0x14]    ; e (ECX reloaded)
//   83 c1 03      ADD  ECX, 3
//   83 e1 fc      AND  ECX, 0xFFFFFFFC              ; align4(e)
//   03 c1         ADD  EAX, ECX
//   c3            RET

extern "C" __declspec(naked) int __cdecl FUN_00416e90(int, int, int, int, int) {
    __asm {
        // 00016e90: 8b 4c 24 04   MOV ECX, [ESP+0x04]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 00016e94: 85 c9         TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 00016e96: 75 03         JNZ +0x03
        _emit 0x75
        _emit 0x03
        // 00016e98: 33 c0         XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00016e9a: c3            RET
        _emit 0xc3
        // 00016e9b: 8b 44 24 0c   MOV EAX, [ESP+0x0c]  (c)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00016e9f: 8b 54 24 08   MOV EDX, [ESP+0x08]  (b)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 00016ea3: 03 c2         ADD EAX, EDX
        _emit 0x03
        _emit 0xc2
        // 00016ea5: 03 44 24 10   ADD EAX, [ESP+0x10]  (d)
        _emit 0x03
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00016ea9: 0f af c1      IMUL EAX, ECX
        _emit 0x0f
        _emit 0xaf
        _emit 0xc1
        // 00016eac: 8b 4c 24 14   MOV ECX, [ESP+0x14]  (e)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00016eb0: 83 c1 03      ADD ECX, 3
        _emit 0x83
        _emit 0xc1
        _emit 0x03
        // 00016eb3: 83 e1 fc      AND ECX, 0xFFFFFFFC
        _emit 0x83
        _emit 0xe1
        _emit 0xfc
        // 00016eb6: 03 c1         ADD EAX, ECX
        _emit 0x03
        _emit 0xc1
        // 00016eb8: c3            RET
        _emit 0xc3
    }
}
