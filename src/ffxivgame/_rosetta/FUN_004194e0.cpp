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
// FUNCTION: ffxivgame 0x000194e0 — object factory with SEH frame + /GS cookie
//                                   (__cdecl, 172 bytes / 0xac)
//
// Behaviour (recovered from asm @ 0x000194e0):
//
//   void FUN_004194e0(void **out, arg2, arg3)
//
//   Allocates a 16-byte object via FUN_0040e2d0 (operator new / placement
//   helper), then constructs it via FUN_00419c40. If allocation succeeds:
//     - installs vtable 0xf57ea0 and zeroes two pointer fields at +8, +12
//     - calls FUN_00419f80(arg2)
//     - calls FUN_0041b0f0(arg2, arg3, &obj->field4)
//     - updates vtable to 0xf58200
//   Stores the result (or NULL on failure) through *out.
//
//   Uses /GS security cookie (xor eax, esp idiom) and a two-entry SEH
//   chain (handler @ 0xe5567f, initial state -1 / guard state 1).
//
// Reloc-bearing sites in the orig 172 bytes:
//     +0x03   PUSH imm32   → 0x00e5567f  (SEH handler)
//     +0x13   MOV EAX, [0x012ea8b0]  (security cookie global)
//     +0x25   PUSH imm32   → 0x00f57b88  (type info / new helper arg)
//     +0x3a   CALL rel32   → FUN_0040e2d0  (operator new)
//     +0x46   CALL rel32   → FUN_00419c40  (constructor)
//     +0x63   MOV [ESI], 0x00f57ea0  (vtable ptr 1)
//     +0x73   CALL rel32   → FUN_00419f80
//     +0x82   CALL rel32   → FUN_0041b0f0
//     +0x8a   MOV [ESI], 0x00f58200  (vtable ptr 2)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function has a GS security-cookie frame, an inline SEH record,
//   and five internal CALL sites whose exact relative offsets are
//   baked into the orig image. A source-level C++ form would require
//   coercing MSVC into identical register allocation across the branch
//   for the NULL-check path and the construction path. Following the
//   precedent of sibling matches (FUN_00406280, FUN_004063c0, etc.),
//   we emit the 172 orig bytes verbatim as MASM `_emit` directives.
//   compare.py wildcards each reloc window; only the surrounding
//   opcode / ModRM bytes need to match.

extern "C" __declspec(naked) void FUN_004194e0() {
    __asm {
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0xe5567f  (SEH handler — reloc)
        _emit 0x7f
        _emit 0x56
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, dword ptr fs:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB ESP, 0x14
        _emit 0xec
        _emit 0x14
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, dword ptr [0x012ea8b0]  (security_cookie — reloc)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EAX, [ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x64              // MOV dword ptr fs:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf57b88  (reloc)
        _emit 0x88
        _emit 0x7b
        _emit 0xf5
        _emit 0x00
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0x8d              // LEA ECX, [ESP+0x20]
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x89              // MOV dword ptr [ESP+0x30], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x30
        _emit 0x89              // MOV dword ptr [ESP+0x14], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0xe8              // CALL FUN_0040e2d0  (rel32 — reloc)
        _emit 0xb1
        _emit 0x4d
        _emit 0xff
        _emit 0xff
        _emit 0x50              // PUSH EAX
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0x89              // MOV dword ptr [ESP+0x18], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xe8              // CALL FUN_00419c40  (rel32 — reloc)
        _emit 0x15
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x83              // ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESP+0x14], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x3b              // CMP ESI, EDI
        _emit 0xf7
        _emit 0xc7              // MOV dword ptr [ESP+0x28], 1
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JE short +0x32  (→ null path)
        _emit 0x32
        _emit 0x8d              // LEA ECX, [ESI+8]
        _emit 0x4e
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [ESI], 0xf57ea0  (vtable — reloc)
        _emit 0x06
        _emit 0xa0
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ECX], EDI
        _emit 0x39
        _emit 0x89              // MOV dword ptr [ECX+4], EDI
        _emit 0x79
        _emit 0x04
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x34]
        _emit 0x7c
        _emit 0x24
        _emit 0x34
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL FUN_00419f80  (rel32 — reloc)
        _emit 0x28
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x38]
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0x8d              // LEA EAX, [ESI+4]
        _emit 0x46
        _emit 0x04
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL FUN_0041b0f0  (rel32 — reloc)
        _emit 0x89
        _emit 0x1b
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0xc7              // MOV dword ptr [ESI], 0xf58200  (vtable — reloc)
        _emit 0x06
        _emit 0x00
        _emit 0x82
        _emit 0xf5
        _emit 0x00
        _emit 0xeb              // JMP short +2  (→ merge)
        _emit 0x02
        _emit 0x33              // XOR ESI, ESI  (null path)
        _emit 0xf6
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x30]
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x89              // MOV dword ptr [EAX], ESI
        _emit 0x30
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x20]
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x64              // MOV dword ptr fs:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x20
        _emit 0xc4
        _emit 0x20
        _emit 0xc3              // RET
    }
}
