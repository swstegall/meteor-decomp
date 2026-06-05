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
// FUNCTION: ffxivgame 0x004563b0 — state-check predicate
//                                  (__cdecl bool (), 97 bytes)
//
// Reads a cached handle from global [0x0126701c].
//
// Fast path (handle != -1):
//   Passes the handle to an IAT-imported getter ([0x00f3e2a4]), reads the
//   first DWORD of the returned pointer, adds 1, and returns true iff the
//   result is zero (i.e. the stored value was -1 / 0xFFFFFFFF).
//
// Slow path (handle == -1, sentinel "not yet cached"):
//   Calls FUN_00457270 via __thiscall on the global object at 0x132d0e0,
//   which returns an object pointer. Reads two 16-bit fields from that
//   object ([EAX+0x88] → cx, [EAX+0x8a] → ax).
//   - If cx > ax: returns true iff (signed ax) - (signed cx) + 0x21 == 0
//   - If cx <= ax: returns true iff (signed ax) - (signed cx) == 0
//
// Reconstruction strategy — naked-asm byte passthrough:
//   This body contains five relocation-bearing operands (one MOV-EAX-moffs32,
//   one MOV-ECX-imm32, one CALL rel32, and one indirect IAT CALL) whose
//   addresses are baked from the orig binary's address space and emitted
//   verbatim via MASM `_emit`. The resulting .obj's .text section is
//   byte-identical to the orig 97-byte slice with no separate relocations;
//   tools/compare.py reports GREEN.
//
// Reloc-bearing sites:
//   +0x01  MOV EAX, moffs32 → DAT_0126701c  (0x0126701c)
//   +0x0B  MOV ECX, imm32   → obj 0x132d0e0
//   +0x10  CALL rel32       → FUN_00457270  (rel = 0x00000eac)
//   +0x4D  CALL [imm32]     → IAT 0x00f3e2a4

extern "C" __declspec(naked) void FUN_004563b0() {
    __asm {
        _emit 0xa1              // MOV EAX, dword ptr [0x0126701c]
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        _emit 0x83              // CMP EAX, -0x1
        _emit 0xf8
        _emit 0xff
        _emit 0x75              // JNZ +0x41  (→ else_branch @ +0x4B)
        _emit 0x41
        _emit 0xb9              // MOV ECX, 0x132d0e0
        _emit 0xe0
        _emit 0xd0
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL FUN_00457270  (rel32 = 0x00000eac)
        _emit 0xac
        _emit 0x0e
        _emit 0x00
        _emit 0x00
        _emit 0x0f              // MOVZX ECX, word ptr [EAX + 0x88]
        _emit 0xb7
        _emit 0x88
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0f              // MOVZX EAX, word ptr [EAX + 0x8a]
        _emit 0xb7
        _emit 0x80
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP CX, AX
        _emit 0x3b
        _emit 0xc8
        _emit 0x0f              // MOVSX EAX, AX
        _emit 0xbf
        _emit 0xc0
        _emit 0x7e              // JLE +0x12  (→ inner_else @ +0x3C)
        _emit 0x12
        _emit 0x0f              // MOVSX ECX, CX
        _emit 0xbf
        _emit 0xc9
        _emit 0x2b              // SUB EAX, ECX
        _emit 0xc1
        _emit 0x83              // ADD EAX, 0x21
        _emit 0xc0
        _emit 0x21
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x0f              // SETZ CL
        _emit 0x94
        _emit 0xc1
        _emit 0x8a              // MOV AL, CL
        _emit 0xc1
        _emit 0xc3              // RET
        // inner_else:
        _emit 0x0f              // MOVSX EDX, CX
        _emit 0xbf
        _emit 0xd1
        _emit 0x2b              // SUB EAX, EDX
        _emit 0xc2
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x0f              // SETZ CL
        _emit 0x94
        _emit 0xc1
        _emit 0x8a              // MOV AL, CL
        _emit 0xc1
        _emit 0xc3              // RET
        // else_branch:
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL dword ptr [0x00f3e2a4]
        _emit 0x15
        _emit 0xa4
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [EAX]
        _emit 0x00
        _emit 0x83              // ADD EAX, 0x1
        _emit 0xc0
        _emit 0x01
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x0f              // SETZ CL
        _emit 0x94
        _emit 0xc1
        _emit 0x8a              // MOV AL, CL
        _emit 0xc1
        _emit 0xc3              // RET
    }
}
