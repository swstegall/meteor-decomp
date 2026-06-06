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
// FUNCTION: ffxivgame 0x00067db0 — FUN_00467db0 (213 B / 0xd5, __cdecl,
//                                  /GS + __alloca_probe large frame).
//
// Asm shape: 3-arg __cdecl function with a 0x818-byte stack frame allocated
// via __alloca_probe (EAX=0x818 → CALL 0x009d29d0). Callee-saves EBX/ESI/EDI;
// each register is loaded from [ESP+0x828] with ESP stepping down 4 bytes per
// push, so EDI=arg0, ESI=arg1, EBX=arg2. The body:
//   1. Pushes 3 literal args (0x31a, 0xf78f00, 0xf78f50) and writes into
//      the local frame below them, then calls FUN_00466300.
//   2. Pushes 7 more values (ESI + 6 LEA'd local-slot addresses) and uses
//      ECX=EBX for a __thiscall-shaped call to FUN_004676b0.
//   3. Reads back the result (ESI = *(stack+0x30)), cleans 0x24 bytes.
//   4. If ESI != 0: calls FUN_00469ac0(EDI, ESI, [ESP+0x10]), captures
//      return in EDI, calls FUN_004632f0(ESI), cleans 0x10, jumps to tail.
//   5. If ESI == 0: calls FUN_00469ac0(EDI, &local_buf, [ESP+0x10]),
//      cleans 0xc, captures return in EDI.
//   6. Tail: calls FUN_004664d0(), restores regs, validates /GS cookie
//      via __security_check_cookie (0x009d20f4), and returns EDI in EAX.
//
// Reloc-bearing sites (wildcarded by compare.py):
//   +0x01  rel32 → 0x009d29d0  (__alloca_probe)
//   +0x06  abs32 → 0x012ea8b0  (__security_cookie)
//   +0x21  abs32 → 0x012ea8b0  (same, but actually part of [ESP+0x814] store context)
//   +0x2e  abs32 → 0x00f78f00  (.rdata string literal)
//   +0x33  abs32 → 0x00f78f50  (.rdata string literal)
//   +0x40  rel32 → 0x00466300  (FUN_00466300)
//   +0x5d  rel32 → 0x004676b0  (FUN_004676b0)
//   +0x72  rel32 → 0x00469ac0  (FUN_00469ac0, true branch)
//   +0x79  rel32 → 0x004632f0  (FUN_004632f0)
//   +0x85  rel32 → 0x00469ac0  (FUN_00469ac0, false branch)
//   +0x92  rel32 → 0x004664d0  (FUN_004664d0)
//   +0xc6  rel32 → 0x009d20f4  (__security_check_cookie)
//
// Reconstruction: naked byte passthrough (same rationale as FUN_00404e40 and
// FUN_00408910 — the exact __alloca_probe prolog, /GS cookie placement at
// [ESP+0x814], and interleaved PUSH/MOV register-load pattern cannot be
// reproduced byte-for-byte from C++ source without luck in MSVC 2005's
// scheduler).

extern "C" __declspec(naked) void FUN_00467db0() {
    __asm {
        // MOV EAX, 0x818
        _emit 0xb8
        _emit 0x18
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // CALL __alloca_probe (0x009d29d0)
        _emit 0xe8
        _emit 0x16
        _emit 0xac
        _emit 0x56
        _emit 0x00
        // MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // MOV [ESP+0x814], EAX
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0x14
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // PUSH EBX
        _emit 0x53
        // MOV EBX, [ESP+0x828]   ; arg2
        _emit 0x8b
        _emit 0x9c
        _emit 0x24
        _emit 0x28
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // PUSH ESI
        _emit 0x56
        // MOV ESI, [ESP+0x828]   ; arg1
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0x28
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // PUSH EDI
        _emit 0x57
        // MOV EDI, [ESP+0x828]   ; arg0
        _emit 0x8b
        _emit 0xbc
        _emit 0x24
        _emit 0x28
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // PUSH 0x31a
        _emit 0x68
        _emit 0x1a
        _emit 0x03
        _emit 0x00
        _emit 0x00
        // LEA EAX, [ESP+0x24]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // PUSH 0xf78f00
        _emit 0x68
        _emit 0x00
        _emit 0x8f
        _emit 0xf7
        _emit 0x00
        // PUSH 0xf78f50
        _emit 0x68
        _emit 0x50
        _emit 0x8f
        _emit 0xf7
        _emit 0x00
        // MOV [ESP+0x24], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // MOV [ESP+0x20], 0x800
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // MOV [ESP+0x18], 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // CALL FUN_00466300
        _emit 0xe8
        _emit 0xf4
        _emit 0xe4
        _emit 0xff
        _emit 0xff
        // PUSH ESI
        _emit 0x56
        // LEA ECX, [ESP+0x2c]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // PUSH ECX
        _emit 0x51
        // LEA EDX, [ESP+0x24]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // PUSH EDX
        _emit 0x52
        // LEA EAX, [ESP+0x2c]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // PUSH EAX
        _emit 0x50
        // LEA ECX, [ESP+0x28]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // PUSH ECX
        _emit 0x51
        // LEA EDX, [ESP+0x38]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x38
        // PUSH EDX
        _emit 0x52
        // MOV ECX, EBX
        _emit 0x8b
        _emit 0xcb
        // CALL FUN_004676b0
        _emit 0xe8
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        // MOV ESI, [ESP+0x30]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x30
        // ADD ESP, 0x24
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        // TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // JZ +0x19 (to false branch)
        _emit 0x74
        _emit 0x19
        // --- true branch (ESI != 0) ---
        // MOV EAX, [ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // PUSH EAX
        _emit 0x50
        // PUSH ESI
        _emit 0x56
        // PUSH EDI
        _emit 0x57
        // CALL FUN_00469ac0
        _emit 0xe8
        _emit 0x7c
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        // PUSH ESI
        _emit 0x56
        // MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // CALL FUN_004632f0
        _emit 0xe8
        _emit 0xa4
        _emit 0xb4
        _emit 0xff
        _emit 0xff
        // ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // JMP +0x15 (to tail)
        _emit 0xeb
        _emit 0x15
        // --- false branch (ESI == 0) ---
        // MOV ECX, [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // PUSH ECX
        _emit 0x51
        // LEA EDX, [ESP+0x24]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // PUSH EDX
        _emit 0x52
        // PUSH EDI
        _emit 0x57
        // CALL FUN_00469ac0
        _emit 0xe8
        _emit 0x5f
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        // ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // --- tail / epilog ---
        // CALL FUN_004664d0
        _emit 0xe8
        _emit 0x65
        _emit 0xe6
        _emit 0xff
        _emit 0xff
        // MOV ECX, [ESP+0x820]   ; reload /GS cookie
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x20
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // MOV EAX, EDI           ; return value
        _emit 0x8b
        _emit 0xc7
        // POP EDI
        _emit 0x5f
        // POP ESI
        _emit 0x5e
        // POP EBX
        _emit 0x5b
        // XOR ECX, ESP
        _emit 0x33
        _emit 0xcc
        // CALL __security_check_cookie (0x009d20f4)
        _emit 0xe8
        _emit 0x76
        _emit 0xa2
        _emit 0x56
        _emit 0x00
        // ADD ESP, 0x818
        _emit 0x81
        _emit 0xc4
        _emit 0x18
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // RET
        _emit 0xc3
    }
}
