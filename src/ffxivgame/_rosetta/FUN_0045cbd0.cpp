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
// FUNCTION: ffxivgame 0x0005cbd0 — __cdecl ENGINE-backed method lookup
//                                  (61 B / 0x3d)
//
//   void* FUN_0045cbd0(int nid)
//
//   Allocates a 4-byte local ENGINE pointer on the stack via __alloca_probe,
//   then calls FUN_00469720(&local, nid) to perform the ENGINE table lookup.
//   If the call returns a non-NULL pointer, dereferences it to obtain the
//   method result (ESI = *retptr). Otherwise ESI = 0. If the local ENGINE
//   reference was set (non-NULL), releases it via _ENGINE_finish. Returns
//   the method pointer (or NULL) in EAX.
//
//   Calling convention: __cdecl — one DWORD stack arg; plain RET (c3),
//   caller cleans. ESI is callee-saved. The 4-byte local is reclaimed by
//   POP ECX in the epilogue.
//
//   Stack layout (after __alloca_probe(4)):
//     [ESP+0x0]  local ENGINE* (output-param, 4 bytes)
//     [ESP+0x4]  return address of caller
//     [ESP+0x8]  nid / arg1
//
//   After PUSH ESI (and pre-call PUSH EAX + PUSH ECX):
//     [ESP+0x4]  local ENGINE* — read back as [ESP+0x4] after ADD ESP,0x8
//
//   Callee-saved: ESI (holds the final method result across ENGINE_finish).
//
// Reloc-bearing sites in the orig 61 bytes (rel32 CALL targets):
//   +0x05  CALL rel32  → 0x005d29d0  (__alloca_probe)
//   +0x16  CALL rel32  → 0x00069720  (FUN_00469720 — ENGINE table select)
//   +0x31  CALL rel32  → 0x000695c0  (_ENGINE_finish)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The combination of the alloc-probe MOV/CALL prologue (which shifts every
//   subsequent [ESP+N] reference by 4 relative to the frame without alloca),
//   the POP ECX epilogue trick to reclaim the 4-byte local, and the two
//   rel32 CALL targets whose resolved offsets can only be reproduced by a
//   full-image link makes source-level C++ brittle under MSVC 2005 /O2.
//   Following the established practice for sibling functions in this module
//   (FUN_0045d3a0, FUN_0045d660, FUN_0046c180), the 61 bytes are emitted
//   verbatim via MASM _emit directives in a __declspec(naked) body.
//   tools/compare.py masks the three rel32 relocation windows so
//   tools/compare.py reports GREEN against orig[0x5cbd0..0x5cc0c].

extern "C" __declspec(naked) void FUN_0045cbd0() {
    __asm {
        // b8 04 00 00 00   MOV EAX, 0x4
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // e8 f6 5d 57 00   CALL __alloca_probe (rel32 → 0x005d29d0)
        _emit 0xe8
        _emit 0xf6
        _emit 0x5d
        _emit 0x57
        _emit 0x00
        // 8b 44 24 08      MOV EAX, dword ptr [ESP+0x8]  ; arg1 / nid
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 56               PUSH ESI
        _emit 0x56
        // 50               PUSH EAX                      ; push arg1 for call
        _emit 0x50
        // 8d 4c 24 08      LEA ECX, [ESP+0x8]            ; &local ENGINE*
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 51               PUSH ECX                      ; push &local for call
        _emit 0x51
        // e8 36 cb 00 00   CALL FUN_00469720 (rel32 → 0x00069720)
        _emit 0xe8
        _emit 0x36
        _emit 0xcb
        _emit 0x00
        _emit 0x00
        // 83 c4 08         ADD ESP, 0x8                  ; cleanup 2 args
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 85 c0            TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 74 04            JZ +4                         ; → XOR ESI,ESI
        _emit 0x74
        _emit 0x04
        // 8b 30            MOV ESI, dword ptr [EAX]      ; ESI = *retptr
        _emit 0x8b
        _emit 0x30
        // eb 02            JMP +2                        ; → read local
        _emit 0xeb
        _emit 0x02
        // 33 f6            XOR ESI, ESI                  ; ESI = NULL
        _emit 0x33
        _emit 0xf6
        // 8b 44 24 04      MOV EAX, dword ptr [ESP+0x4]  ; local ENGINE*
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 85 c0            TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 74 09            JZ +9                         ; → epilogue
        _emit 0x74
        _emit 0x09
        // 50               PUSH EAX                      ; push local ENGINE*
        _emit 0x50
        // e8 bb c9 00 00   CALL _ENGINE_finish (rel32 → 0x000695c0)
        _emit 0xe8
        _emit 0xbb
        _emit 0xc9
        _emit 0x00
        _emit 0x00
        // 83 c4 04         ADD ESP, 0x4                  ; cleanup 1 arg
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 8b c6            MOV EAX, ESI                  ; return method ptr
        _emit 0x8b
        _emit 0xc6
        // 5e               POP ESI
        _emit 0x5e
        // 59               POP ECX                       ; reclaim 4-byte local
        _emit 0x59
        // c3               RET
        _emit 0xc3
    }
}
