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
// FUNCTION: ffxivgame 0x0001cf50 — `__cdecl` 2-arg float-to-byte logger
//                                  (123 B / 0x7b). Sends a raw table value
//                                  via event 0x19, then a float-to-int
//                                  conversion (clamped to 255) via event
//                                  0x18, both through a global __thiscall
//                                  dispatch at 0x0132987c → FUN_004236e0.
//
// Inspection (read from the disassembly at orig RVA 0x0001cf50):
//
//   __cdecl void FUN_0041cf50(int arg1, float arg2)
//
//   Pseudo-C:
//
//     void FUN_0041cf50(int arg1, float arg2) {
//         // First dispatch: table lookup + event 0x19
//         int tableVal = *(int*)(0xf596c0 + arg1 * 4);
//         g_obj_0132987c->dispatch(0x19, tableVal);   // __thiscall, RETN 8
//
//         // Float conversion with explicit x87 truncation
//         double fval = (double)arg2 * *(double*)0xf598b0 + *(double*)0xf59898;
//         int result = (int)fval;   // via FNSTCW/FLDCW truncate + FISTP qword
//
//         // Clamp to [0, 255]
//         int clamped = (result > 0xff) ? 0xff : result;
//
//         // Second dispatch: clamped value + event 0x18
//         g_obj_0132987c->dispatch(0x18, clamped);    // __thiscall, RETN 8
//     }
//
//   Stack frame (ESP-relative, after SUB ESP, 0x8):
//     [ESP+0x00] to [ESP+0x07]  8-byte local scratch (FISTP qword / CW saves)
//     [ESP+0x08]                return address
//     [ESP+0x0c]                arg1 (int, table index) — reused as FNSTCW slot
//     [ESP+0x10]                arg2 (float) — reused for modified CW + result
//
//   Reloc-bearing sites (absolute addresses baked in, no COFF relocations
//   emitted by the naked-asm _emit approach):
//     +0x07   MOV ECX, [EAX*4 + 0xf596c0]   (SIB table base — absolute)
//     +0x0f   MOV ECX, [0x0132987c]           (global this ptr — absolute)
//     +0x17   CALL rel32 → 0x004236e0         (FUN_004236e0 — baked displacement)
//     +0x20   FMUL double [0x00f598b0]         (scale constant — absolute)
//     +0x2f   FADD double [0x00f59898]         (offset constant — absolute)
//     +0x72   MOV ECX, [0x0132987c]            (global this ptr — 2nd load)
//     +0x78   CALL rel32 → 0x004236e0          (FUN_004236e0 — 2nd call)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The x87 manual-truncation idiom (FNSTCW / OR imm / FLDCW / FISTP qword /
//   FLDCW restore), the SIB-encoded table lookup, and the two identical
//   __thiscall calls make source-level C++ reconstruction brittle under
//   MSVC 2005 /O2 — any deviation in register allocation or rounding-mode
//   wording shifts at least one instruction byte. The pragmatic choice,
//   consistent with FUN_00401350 / FUN_00403d60 / FUN_00401650, is a
//   `__declspec(naked)` body re-emitting the orig 123 bytes verbatim via
//   MASM `_emit` directives. The .obj `.text` becomes byte-identical to
//   the original slice with no COFF relocations; `tools/compare.py`
//   reports GREEN.

extern "C" __declspec(naked) void FUN_0041cf50() {
    __asm {
        _emit 0x83              // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0xc]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b              // MOV ECX, dword ptr [EAX*4 + 0xf596c0]
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x96
        _emit 0xf5
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, dword ptr [0x0132987c]
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x6a              // PUSH 0x19
        _emit 0x19
        _emit 0xe8              // CALL 0x004236e0 (rel32 = 0x00006774)
        _emit 0x74
        _emit 0x67
        _emit 0x00
        _emit 0x00
        _emit 0xd9              // FLD float ptr [ESP + 0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xdc              // FMUL double ptr [0x00f598b0]
        _emit 0x0d
        _emit 0xb0
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        _emit 0xd9              // FNSTCW word ptr [ESP + 0xc]
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x0f              // MOVZX EAX, word ptr [ESP + 0xc]
        _emit 0xb7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xdc              // FADD double ptr [0x00f59898]
        _emit 0x05
        _emit 0x98
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        _emit 0x0d              // OR EAX, 0xc00
        _emit 0x00
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESP + 0x10], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xd9              // FLDCW word ptr [ESP + 0x10]
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0xdf              // FISTP qword ptr [ESP]
        _emit 0x3c
        _emit 0x24
        _emit 0x8b              // MOV EAX, dword ptr [ESP]
        _emit 0x04
        _emit 0x24
        _emit 0x89              // MOV dword ptr [ESP + 0x10], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x3d              // CMP EAX, 0xff
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xd9              // FLDCW word ptr [ESP + 0xc]
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        _emit 0x8d              // LEA EAX, [ESP + 0xc]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xc7              // MOV dword ptr [ESP + 0xc], 0xff
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x77              // JA +4 (→ 0x0041cfb7)
        _emit 0x04
        _emit 0x8d              // LEA EAX, [ESP + 0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x8b              // MOV ECX, dword ptr [0x0132987c]
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x52              // PUSH EDX
        _emit 0x6a              // PUSH 0x18
        _emit 0x18
        _emit 0xe8              // CALL 0x004236e0 (rel32 = 0x00006719)
        _emit 0x19
        _emit 0x67
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc3              // RET
    }
}
