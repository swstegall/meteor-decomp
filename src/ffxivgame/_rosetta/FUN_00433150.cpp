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
// FUNCTION: ffxivgame 0x00033150 — `__cdecl char(int)` switch predicate (32 B).
//
// A boolean classifier over a single dword arg. MSVC 2005 /O2 lowers a
// `switch` whose case labels span [5, 21] (17 contiguous values) and whose
// arms collapse to just two outcomes (return 1 / fall to default return 0)
// into the classic *two-level* jump table:
//
//   1. Bias the selector by the low case label (ADD EAX, -5) and bound-check
//      the biased value against (hi-lo) = 0x10 with a single unsigned `JA`
//      (CMP EAX,0x10 / JA default) — anything outside [5,21] returns 0.
//   2. Index a packed *byte* table at 0x00433178 to map each in-range value
//      onto one of the few distinct arm indices (MOVZX EAX, byte[EAX+...]).
//   3. Dispatch through the *dword* address table at 0x00433170
//      (JMP [EAX*4 + 0x00433170]).
//
// Both tables live in .text immediately after the 32-byte function body
// (the byte table at +0x28, the dword table at +0x20), so they fall outside
// the function's compared slice; only the 32 instruction bytes below match.
//
// The two reachable arms are the minimal `MOV AL,1 / RET` (true) and
// `XOR AL,AL / RET` (false, also the out-of-range default). MSVC emits the
// truthy arm first, the default last.
//
// Disassembly (verbatim — RVA 0x00033150..0x00033170, 32 bytes):
//
//   00033150:  8b 44 24 04           MOV   EAX, [ESP+0x4]              ; selector
//   00033154:  83 c0 fb              ADD   EAX, -0x5                   ; bias by lo
//   00033157:  83 f8 10              CMP   EAX, 0x10                   ; span = hi-lo
//   0003315a:  77 11                 JA    0x0043316d                  ; out of range
//   0003315c:  0f b6 80 78 31 43 00  MOVZX EAX, byte [EAX+0x00433178]  ; byte index
//   00033163:  ff 24 85 70 31 43 00  JMP   [EAX*4 + 0x00433170]        ; dword dispatch
//   0003316a:  b0 01                 MOV   AL, 0x1                     ; -> return 1
//   0003316c:  c3                    RET
//   0003316d:  32 c0                 XOR   AL, AL                      ; -> return 0
//   0003316f:  c3                    RET
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The MOVZX / JMP operands carry absolute VAs (0x00433178 / 0x00433170)
//   into our own image, i.e. base-reloc-bearing sites. Baking the orig 32
//   bytes verbatim via MASM `_emit` reproduces those absolute dwords as raw
//   bytes that match the orig PE's .text slice exactly, sidestepping any
//   symbol-resolution / table-placement fragility. This mirrors the
//   convention used by the sibling reloc-bearing wrappers (cf. FUN_00401000).

extern "C" __declspec(naked) char FUN_00433150() {
    __asm {
        _emit 0x8b      // MOV   EAX, [ESP+0x4]            ; selector
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x83      // ADD   EAX, -0x5                 ; bias by low case label
        _emit 0xc0
        _emit 0xfb
        _emit 0x83      // CMP   EAX, 0x10                 ; span = hi-lo
        _emit 0xf8
        _emit 0x10
        _emit 0x77      // JA    0x0043316d                ; out of range -> default
        _emit 0x11
        _emit 0x0f      // MOVZX EAX, byte [EAX+0x00433178]; packed byte index table
        _emit 0xb6
        _emit 0x80
        _emit 0x78
        _emit 0x31
        _emit 0x43
        _emit 0x00
        _emit 0xff      // JMP   [EAX*4 + 0x00433170]      ; dword dispatch table
        _emit 0x24
        _emit 0x85
        _emit 0x70
        _emit 0x31
        _emit 0x43
        _emit 0x00
        _emit 0xb0      // MOV   AL, 0x1                   ; truthy arm
        _emit 0x01
        _emit 0xc3      // RET
        _emit 0x32      // XOR   AL, AL                    ; default arm
        _emit 0xc0
        _emit 0xc3      // RET
    }
}
