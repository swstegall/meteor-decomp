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
// FUNCTION: ffxivgame 0x0041d050 — four-registration dispatcher; 107 bytes.
//
// __cdecl void FUN_0041d050(int a, int b, int c, int d)
//
// Calls FUN_004236e0 (a __thiscall method on object at [0x0132987c]) four
// times, once per argument, with a unique action-ID constant each time.
// Each arg is used as an index into a dword pointer array based at
// 0x00f596e0. The four action IDs used are: 0x13 (19), 0x14 (20),
// 0xcf (207), 0xd0 (208).
//
// Pattern (per invocation):
//   ECX = *array_base[arg_N]   // array[arg_N * 4 + 0x00f596e0]
//   push ECX                   // object/data pointer
//   ECX = [0x0132987c]         // global this-ptr for FUN_004236e0
//   push action_id             // constant
//   call FUN_004236e0          // __thiscall — callee cleans the 2 stack args
//
// After the 4th call the function returns (plain RET — __cdecl, caller cleans).
//
// Reloc-bearing sites (compare.py wildcard-masks these 4-byte windows):
//   +0x07  imm32: 0x00f596e0   array base (SIB displacement)    × 4
//   +0x0e  imm32: 0x0132987c   global this-ptr                  × 4
//   +0x14  rel32: 0x00006677   call → FUN_004236e0 (RVA 0x236e0)
//   +0x2d  rel32: 0x0000665e   call → FUN_004236e0
//   +0x49  rel32: 0x00006642   call → FUN_004236e0
//   +0x65  rel32: 0x00006626   call → FUN_004236e0
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Source-level C++ would hoist [0x0132987c] into a saved register and
//   share it across calls; MSVC 2005 reloads it four times. The
//   SIB-encoded [reg*4+imm32] addressing, the particular register
//   allocation (EAX/ECX/EDX/ECX pattern), and the four distinct rel32
//   CALL offsets (same target, different displacement depending on
//   instruction position) all interact too subtly to recover
//   source-level. The naked-asm byte passthrough is the safe path.

extern "C" __declspec(naked) void FUN_0041d050() {
    __asm {
        // call 1 — arg a  ([esp+4]), action-id 0x13
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x04]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [EAX*4 + 0x00f596e0]
        _emit 0x0c
        _emit 0x85
        _emit 0xe0              // imm32: 0x00f596e0
        _emit 0x96
        _emit 0xf5
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, dword ptr [0x0132987c]
        _emit 0x0d
        _emit 0x7c              // imm32: 0x0132987c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x6a              // PUSH 0x13
        _emit 0x13
        _emit 0xe8              // CALL FUN_004236e0 (rel32)
        _emit 0x77
        _emit 0x66
        _emit 0x00
        _emit 0x00

        // call 2 — arg b ([esp+8]), action-id 0x14
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x08]
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [EDX*4 + 0x00f596e0]
        _emit 0x04
        _emit 0x95
        _emit 0xe0              // imm32: 0x00f596e0
        _emit 0x96
        _emit 0xf5
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [0x0132987c]
        _emit 0x0d
        _emit 0x7c              // imm32: 0x0132987c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0x6a              // PUSH 0x14
        _emit 0x14
        _emit 0xe8              // CALL FUN_004236e0 (rel32)
        _emit 0x5e
        _emit 0x66
        _emit 0x00
        _emit 0x00

        // call 3 — arg c ([esp+0xc]), action-id 0xcf (207)
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x0c]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x8b              // MOV EDX, dword ptr [ECX*4 + 0x00f596e0]
        _emit 0x14
        _emit 0x8d
        _emit 0xe0              // imm32: 0x00f596e0
        _emit 0x96
        _emit 0xf5
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [0x0132987c]
        _emit 0x0d
        _emit 0x7c              // imm32: 0x0132987c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x52              // PUSH EDX
        _emit 0x68              // PUSH 0xcf
        _emit 0xcf
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_004236e0 (rel32)
        _emit 0x42
        _emit 0x66
        _emit 0x00
        _emit 0x00

        // call 4 — arg d ([esp+0x10]), action-id 0xd0 (208)
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV ECX, dword ptr [EAX*4 + 0x00f596e0]
        _emit 0x0c
        _emit 0x85
        _emit 0xe0              // imm32: 0x00f596e0
        _emit 0x96
        _emit 0xf5
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, dword ptr [0x0132987c]
        _emit 0x0d
        _emit 0x7c              // imm32: 0x0132987c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x68              // PUSH 0xd0
        _emit 0xd0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_004236e0 (rel32)
        _emit 0x26
        _emit 0x66
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
    }
}
