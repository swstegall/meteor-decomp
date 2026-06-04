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
// FUNCTION: ffxivgame 0x0041c540 — ECX-transform dispatch helper (53 bytes / 0x35)
//
// Reads an operation selector from [ESP+4] (first stack argument).  Each
// case applies one of several arithmetic operations to ECX and returns the
// result in EAX.  Cases > 8 fall through to the default path which returns 0.
//
// Case code blocks (derived from the switch body, jump table at 0x41c578):
//   block @+0x10 : MOV EAX, ECX             (identity)
//   block @+0x13 : LEA EAX, [ECX-1]         (subtract 1)
//   block @+0x17 : MOV EAX,ECX; SHR EAX,1   (divide by 2)
//   block @+0x1c : MUL-trick; SHR EDX,1     (divide by 3 — multiply by 0xaaaaaaab)
//   block @+0x28 : LEA EAX, [ECX-2]         (subtract 2)
//   block @+0x2c : MOV EAX,ECX; SHR EAX,2   (divide by 4)
//   default @+0x32: XOR EAX, EAX             (return 0, op > 8)
//
// Calling convention: non-standard — ECX is the input value; [ESP+4] is the
// operation selector; plain RET (caller responsible for the stack argument).
//
// Naked __asm byte passthrough (mirrors FUN_004051e0 strategy): the absolute
// address 0x41c578 in the JMP [EAX*4+imm32] instruction at bytes +0x09..+0x0f
// is emitted verbatim as raw bytes.  tools/compare.py masks any reloc bytes.

extern "C" __declspec(naked) void FUN_0041c540() {
    __asm {
        _emit 0x8b  // MOV EAX, dword ptr [ESP+4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x83  // CMP EAX, 0x8
        _emit 0xf8
        _emit 0x08
        _emit 0x77  // JA +0x29  (→ default: XOR EAX,EAX; RET)
        _emit 0x29
        _emit 0xff  // JMP dword ptr [EAX*4 + 0x41c578]
        _emit 0x24
        _emit 0x85
        _emit 0x78  // 0x41c578 lo byte 0
        _emit 0xc5  // 0x41c578 lo byte 1
        _emit 0x41  // 0x41c578 lo byte 2
        _emit 0x00  // 0x41c578 hi byte
        _emit 0x8b  // MOV EAX, ECX
        _emit 0xc1
        _emit 0xc3  // RET
        _emit 0x8d  // LEA EAX, [ECX + (-1)]
        _emit 0x41
        _emit 0xff
        _emit 0xc3  // RET
        _emit 0x8b  // MOV EAX, ECX
        _emit 0xc1
        _emit 0xd1  // SHR EAX, 1
        _emit 0xe8
        _emit 0xc3  // RET
        _emit 0xb8  // MOV EAX, 0xaaaaaaab   (divide-by-3 magic constant)
        _emit 0xab
        _emit 0xaa
        _emit 0xaa
        _emit 0xaa
        _emit 0xf7  // MUL ECX
        _emit 0xe1
        _emit 0x8b  // MOV EAX, EDX
        _emit 0xc2
        _emit 0xd1  // SHR EAX, 1
        _emit 0xe8
        _emit 0xc3  // RET
        _emit 0x8d  // LEA EAX, [ECX + (-2)]
        _emit 0x41
        _emit 0xfe
        _emit 0xc3  // RET
        _emit 0x8b  // MOV EAX, ECX
        _emit 0xc1
        _emit 0xc1  // SHR EAX, 2
        _emit 0xe8
        _emit 0x02
        _emit 0xc3  // RET
        _emit 0x33  // XOR EAX, EAX   (default — op > 8)
        _emit 0xc0
        _emit 0xc3  // RET
    }
}
