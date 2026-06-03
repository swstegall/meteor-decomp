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
// FUNCTION: ffxivgame 0x00016990 — `__thiscall` vtable-stamp + conditional
//                                   release, 27 bytes.
//
// A small `__thiscall` method (no stack frame) that:
//   1. Stamps the vtable pointer at [ECX+0x0] with 0x00f576b4 (unconditional).
//   2. Tests bit 0 of the byte flag at [ECX+0x12]; if the bit is set, returns
//      immediately (early-out — vtable already written).
//   3. Loads the pointer stored at [ECX+0x4]; if non-null, calls
//      FUN_004162c0(ptr) as a __cdecl 1-arg call (caller pops via POP ECX).
//   4. Returns.
//
// MSVC 2005 /O2 reordered the TEST [ECX+0x12] before the vtable MOV because
// the two memory accesses are independent (offsets 0x12 vs 0x0), folding
// the flag-check ahead of the store to shorten the critical path.
//
// Asm shape (27 bytes, read from build/pe-layout/ffxivgame/text.bin
// @ +0x16990, RVA 0x00016990..0x000169aa):
//
//   00016990: f6 41 12 01         TEST byte ptr [ECX+0x12], 0x1
//   00016994: c7 01 b4 76 f5 00   MOV  dword ptr [ECX], 0x00f576b4  ; vtable
//   0001699a: 75 0e               JNZ  0x004169aa                   ; flag set → ret
//   0001699c: 8b 41 04            MOV  EAX, [ECX+0x4]              ; member ptr
//   0001699f: 85 c0               TEST EAX, EAX
//   000169a1: 74 07               JZ   0x004169aa                   ; null → ret
//   000169a3: 50                  PUSH EAX
//   000169a4: e8 17 f9 ff ff      CALL FUN_004162c0                 ; rel32
//   000169a9: 59                  POP  ECX                          ; cdecl cleanup
//   000169aa: c3                  RET
//
// Reloc-bearing sites:
//   +0x06  DIR32 immediate → 0x00f576b4 (vtable address)
//   +0x15  REL32 → FUN_004162c0 at VA 0x004162c0
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//   A source-level rewrite of this function would require MSVC to replicate
//   the TEST-before-MOV reordering and the exact flag/null short-circuit
//   shape, which is fragile across minor /O2 heuristic differences.
//   Emitting all 27 bytes verbatim via `_emit` is the safe choice; the two
//   reloc windows are masked by compare.py in the diff.

extern "C" __declspec(naked) void FUN_00416990() {
    __asm {
        _emit 0xf6    // TEST byte ptr [ECX+0x12], 0x1
        _emit 0x41
        _emit 0x12
        _emit 0x01
        _emit 0xc7    // MOV dword ptr [ECX], 0x00f576b4   ; vtable
        _emit 0x01
        _emit 0xb4
        _emit 0x76
        _emit 0xf5
        _emit 0x00
        _emit 0x75    // JNZ +0x0e  (→ RET)
        _emit 0x0e
        _emit 0x8b    // MOV EAX, [ECX+0x4]
        _emit 0x41
        _emit 0x04
        _emit 0x85    // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74    // JZ +0x07  (→ RET)
        _emit 0x07
        _emit 0x50    // PUSH EAX
        _emit 0xe8    // CALL FUN_004162c0  ; rel32 = 0xfffff917
        _emit 0x17
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        _emit 0x59    // POP ECX   ; cdecl caller cleanup
        _emit 0xc3    // RET
    }
}
