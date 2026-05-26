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
// FUNCTION: ffxivgame 0x0000d880 — memory-space assign/free on a layout object
//                                   (__thiscall, 2 stack args, 136 B / 0x88)
//
// Signature (inferred):
//   void __thiscall FUN_0040d880(this, int param1, unsigned short param2)
//
// ECX = this (pointer to object)
// Stack args:
//   [ESP+4]  param1  — DWORD, stored at this[0]
//   [ESP+8]  param2  — WORD (tested as BX), controls memory-space size
//
// Calling convention: __thiscall, RET 0x8 (callee pops 8 bytes of stack args).
// Callee-saved: EBX, EBP, ESI, EDI. 8-byte local frame.
//
// Logic:
//   1. If this[0] != 0 AND this[0x28] != 0, call FUN_0040df70(this[0x28])
//      (release/free the old memory space handle)
//   2. this[0] = param1
//   3. If (short)param2 == 0: this[0x28] = 0; return.
//   4. Call FUN_0040e230 on a local 8-byte object with the global at
//      [0x012652f8] and the string "CDev.Engine.Lay.Mem.Space" to
//      look up or initialise a named memory space descriptor.
//   5. Call FUN_0040e110(__thiscall on param1) with size = param2*0x1002
//      to allocate a block from the named space; store result at this[0x28].
//   6. If allocation succeeded, call FUN_0040dd90 on sub-object this[4]
//      with args (alloc_ptr, alloc_ptr + param2*0x1000, 0x1000, param2)
//      to configure the sub-object's mapped range.
//
// Globals:
//   [0x012652f8]  — pointer to global memory manager object
//   [0x00f55864]  — string literal "CDev.Engine.Lay.Mem.Space"
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Source-level C++ cannot reproduce the exact register schedule here:
//   MSVC holds the MOVZX'd param2 in EDI across the FUN_0040e230 call,
//   then uses both EDI<<0xc (for the page-rounded end address) and the
//   original BX (for the param2 word passed to FUN_0040dd90) without
//   reloading. The LEA ECX,[ESP+0x18] thiscall on a stack-allocated
//   8-byte local, combined with the IMUL ECX,ECX,0x1002 idiom and the
//   two-epilogue structure (one at 0xd8b3, one at 0xd901), makes a
//   source-level match infeasible. Naked-asm passthrough is used.

extern "C" __declspec(naked) void FUN_0040d880()
{
    __asm {
        // 0x00: SUB ESP, 0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0x03: PUSH EBX
        _emit 0x53
        // 0x04: PUSH EBP
        _emit 0x55
        // 0x05: PUSH ESI
        _emit 0x56
        // 0x06: MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 0x08: MOV ECX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x0e
        // 0x0a: PUSH EDI
        _emit 0x57
        // 0x0b: XOR EDI, EDI
        _emit 0x33
        _emit 0xff
        // 0x0d: CMP ECX, EDI
        _emit 0x3b
        _emit 0xcf
        // 0x0f: JZ +0x10  (→ 0x0040d8a1)
        _emit 0x74
        _emit 0x10
        // 0x11: MOV EAX, dword ptr [ESI + 0x28]
        _emit 0x8b
        _emit 0x46
        _emit 0x28
        // 0x14: CMP EAX, EDI
        _emit 0x3b
        _emit 0xc7
        // 0x16: JZ +0x09  (→ 0x0040d8a1)
        _emit 0x74
        _emit 0x09
        // 0x18: PUSH EAX
        _emit 0x50
        // 0x19: CALL FUN_0040df70  (rel32 = 0x000006d2; __stdcall — callee pops 1 arg)
        _emit 0xe8
        _emit 0xd2
        _emit 0x06
        _emit 0x00
        _emit 0x00
        // 0x1e: MOV dword ptr [ESI + 0x28], EDI  (this->field28 = 0 after free)
        _emit 0x89
        _emit 0x7e
        _emit 0x28
        // --- 0x0040d8a1 ---
        // 0x1e: MOV EBX, dword ptr [ESP + 0x20]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        // 0x22: CMP BX, DI
        _emit 0x66
        _emit 0x3b
        _emit 0xdf
        // 0x25: MOV EBP, dword ptr [ESP + 0x1c]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        // 0x29: MOV dword ptr [ESI], EBP
        _emit 0x89
        _emit 0x2e
        // 0x2b: JNZ +0x0d  (→ 0x0040d8bd)
        _emit 0x75
        _emit 0x0d
        // 0x2d: MOV dword ptr [ESI + 0x28], EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x28
        // 0x30: POP EDI
        _emit 0x5f
        // 0x31: POP ESI
        _emit 0x5e
        // 0x32: POP EBP
        _emit 0x5d
        // 0x33: POP EBX
        _emit 0x5b
        // 0x34: ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0x37: RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // --- 0x0040d8bd ---
        // 0x3a: MOV EAX, [0x012652f8]
        _emit 0xa1
        _emit 0xf8
        _emit 0x52
        _emit 0x26
        _emit 0x01
        // 0x3f: PUSH 0xf55864
        _emit 0x68
        _emit 0x64
        _emit 0x58
        _emit 0xf5
        _emit 0x00
        // 0x44: PUSH EAX
        _emit 0x50
        // 0x45: LEA ECX, [ESP + 0x18]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0x49: MOVZX EDI, BX
        _emit 0x0f
        _emit 0xb7
        _emit 0xfb
        // 0x4c: CALL FUN_0040e230  (rel32 = 0x0000095c)
        _emit 0xe8
        _emit 0x5c
        _emit 0x09
        _emit 0x00
        _emit 0x00
        // 0x51: MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 0x53: IMUL ECX, ECX, 0x1002
        _emit 0x69
        _emit 0xc9
        _emit 0x02
        _emit 0x10
        _emit 0x00
        _emit 0x00
        // 0x59: PUSH EAX
        _emit 0x50
        // 0x5a: PUSH ECX
        _emit 0x51
        // 0x5b: MOV ECX, EBP
        _emit 0x8b
        _emit 0xcd
        // 0x5d: CALL FUN_0040e110  (rel32 = 0x0000082b)
        _emit 0xe8
        _emit 0x2b
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0x62: TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0x64: MOV dword ptr [ESI + 0x28], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x28
        // 0x67: JZ +0x15  (→ epilogue at 0x0040d901)
        _emit 0x74
        _emit 0x15
        // 0x69: PUSH EBX
        _emit 0x53
        // 0x6a: SHL EDI, 0xc
        _emit 0xc1
        _emit 0xe7
        _emit 0x0c
        // 0x6d: PUSH 0x1000
        _emit 0x68
        _emit 0x00
        _emit 0x10
        _emit 0x00
        _emit 0x00
        // 0x72: ADD EDI, EAX
        _emit 0x03
        _emit 0xf8
        // 0x74: PUSH EDI
        _emit 0x57
        // 0x75: PUSH EAX
        _emit 0x50
        // 0x76: LEA ECX, [ESI + 0x4]
        _emit 0x8d
        _emit 0x4e
        _emit 0x04
        // 0x79: CALL FUN_0040dd90  (rel32 = 0x0000048f)
        _emit 0xe8
        _emit 0x8f
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // --- epilogue (0x0040d901) ---
        // 0x7e: POP EDI
        _emit 0x5f
        // 0x7f: POP ESI
        _emit 0x5e
        // 0x80: POP EBP
        _emit 0x5d
        // 0x81: POP EBX
        _emit 0x5b
        // 0x82: ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0x85: RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
