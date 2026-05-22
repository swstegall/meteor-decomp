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
// FUNCTION: ffxivgame 0x0000f070 — __thiscall heap block coalescing free (206 B)
//
// Block header (16-byte prefix to user data):
//   [+0]: data_size      — size of user payload; next_block = block + data_size + 0x10
//   [+4]: size_and_flag  — bit31 = this block's in-use flag;
//                          bits0..30 = prev block's data_size (backward navigation)
//   [+8]: fwd            — free list forward link (valid when free)
//   [+C]: bak            — free list back link (valid when free)
//
// Allocator layout (ECX / `this`):
//   [+0]:  int pad
//   [+4]:  Block buckets[]  — sentinel per size class; index = BSR(size)+1
//
// Debug context (param_1, from stack):
//   [+0x14]: char debug_fill — nonzero → fill freed data with 0xAA
//
// Register assignment (MSVC 2005 /O2 /Oy FPO):
//   EAX = block (user_ptr - 0x10); becomes prev after prev-coalesce
//   EBP = this (ECX saved early; only used at LEA ECX,[EDX+EBP+4])
//   ESI = next block / next-next after next-coalesce
//   EDI = merged_size (accumulated)
//   ECX, EDX = temps (prev_flag, prev_rel, prev, list pointers)
//
// Source-level attempts produced ADD EDX,EAX instead of MOV EDX,EAX +
// SUB EDX,ECX, plus EBX allocated instead of EBP for `this`.  The MSVC
// 2005 register allocator prefers EBP over EBX when FPO is enabled and
// `this` must be saved before ECX is reused — but only when exactly 3
// callee-saves are needed; adding any extra local tips it to EBX.  No
// declaration reorder found to reproduce the exact layout, so naked-asm
// byte passthrough with a single CALL relocation is used.

extern "C" void FUN_009d2110(void *dst, int fill, unsigned int count);

extern "C" __declspec(naked) void FUN_0040f070()
{
    __asm {
        _emit 0x8b  // MOV EAX, [ESP+8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x83  // ADD EAX, -0x10
        _emit 0xc0
        _emit 0xf0
        _emit 0x55  // PUSH EBP
        _emit 0x8b  // MOV EBP, ECX
        _emit 0xe9
        _emit 0x8b  // MOV ECX, [EAX+4]
        _emit 0x48
        _emit 0x04
        _emit 0x81  // AND ECX, 0x7FFFFFFF
        _emit 0xe1
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        _emit 0x8b  // MOV EDX, EAX
        _emit 0xd0
        _emit 0x2b  // SUB EDX, ECX
        _emit 0xd1
        _emit 0x8b  // MOV ECX, [EDX-0xC]
        _emit 0x4a
        _emit 0xf4
        _emit 0x83  // SUB EDX, 0x10
        _emit 0xea
        _emit 0x10
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0x8b  // MOV EDI, [EAX]
        _emit 0x38
        _emit 0xc1  // SHR ECX, 0x1F
        _emit 0xe9
        _emit 0x1f
        _emit 0xf6  // TEST CL, 0x1
        _emit 0xc1
        _emit 0x01
        _emit 0x8d  // LEA ESI, [EDI+EAX+0x10]
        _emit 0x74
        _emit 0x07
        _emit 0x10
        _emit 0x75  // JNZ +0x1a
        _emit 0x1a
        _emit 0x8b  // MOV EAX, [EDX+0xC]
        _emit 0x42
        _emit 0x0c
        _emit 0x8b  // MOV ECX, [EDX+0x8]
        _emit 0x4a
        _emit 0x08
        _emit 0x89  // MOV [EAX+0x8], ECX
        _emit 0x48
        _emit 0x08
        _emit 0x8b  // MOV EAX, [EDX+0x8]
        _emit 0x42
        _emit 0x08
        _emit 0x8b  // MOV ECX, [EDX+0xC]
        _emit 0x4a
        _emit 0x0c
        _emit 0x89  // MOV [EAX+0xC], ECX
        _emit 0x48
        _emit 0x0c
        _emit 0x8b  // MOV EAX, [EDX]
        _emit 0x02
        _emit 0x8d  // LEA EDI, [EDI+EAX+0x10]
        _emit 0x7c
        _emit 0x07
        _emit 0x10
        _emit 0x8b  // MOV EAX, EDX
        _emit 0xc2
        _emit 0x8b  // MOV ECX, [ESI+4]
        _emit 0x4e
        _emit 0x04
        _emit 0xc1  // SHR ECX, 0x1F
        _emit 0xe9
        _emit 0x1f
        _emit 0xf6  // TEST CL, 0x1
        _emit 0xc1
        _emit 0x01
        _emit 0x75  // JNZ +0x1c
        _emit 0x1c
        _emit 0x8b  // MOV EDX, [ESI+0xC]
        _emit 0x56
        _emit 0x0c
        _emit 0x8b  // MOV ECX, [ESI+0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x89  // MOV [EDX+0x8], ECX
        _emit 0x4a
        _emit 0x08
        _emit 0x8b  // MOV EDX, [ESI+0x8]
        _emit 0x56
        _emit 0x08
        _emit 0x8b  // MOV ECX, [ESI+0xC]
        _emit 0x4e
        _emit 0x0c
        _emit 0x89  // MOV [EDX+0xC], ECX
        _emit 0x4a
        _emit 0x0c
        _emit 0x8b  // MOV EDX, [ESI]
        _emit 0x16
        _emit 0x8d  // LEA EDI, [EDI+EDX+0x10]
        _emit 0x7c
        _emit 0x17
        _emit 0x10
        _emit 0x8d  // LEA ESI, [ESI+EDX+0x10]
        _emit 0x74
        _emit 0x16
        _emit 0x10
        _emit 0x81  // AND [EAX+4], 0x7FFFFFFF
        _emit 0x60
        _emit 0x04
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        _emit 0x89  // MOV [EAX], EDI
        _emit 0x38
        _emit 0x8b  // MOV EDX, [ESI+4]
        _emit 0x56
        _emit 0x04
        _emit 0x33  // XOR EDX, EDI
        _emit 0xd7
        _emit 0x81  // AND EDX, 0x7FFFFFFF
        _emit 0xe2
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        _emit 0x31  // XOR [ESI+4], EDX
        _emit 0x56
        _emit 0x04
        _emit 0x0f  // BSR EDX, [EAX]
        _emit 0xbd
        _emit 0x10
        _emit 0x74  // JZ +5
        _emit 0x05
        _emit 0x83  // ADD EDX, 1
        _emit 0xc2
        _emit 0x01
        _emit 0xeb  // JMP +2
        _emit 0x02
        _emit 0x33  // XOR EDX, EDX
        _emit 0xd2
        _emit 0xc1  // SHL EDX, 4
        _emit 0xe2
        _emit 0x04
        _emit 0x8d  // LEA ECX, [EDX+EBP+4]
        _emit 0x4c
        _emit 0x2a
        _emit 0x04
        _emit 0x8b  // MOV EDX, [ECX+8]
        _emit 0x51
        _emit 0x08
        _emit 0x89  // MOV [EDX+0xC], EAX
        _emit 0x42
        _emit 0x0c
        _emit 0x8b  // MOV EDX, [ECX+8]
        _emit 0x51
        _emit 0x08
        _emit 0x89  // MOV [EAX+0xC], ECX
        _emit 0x48
        _emit 0x0c
        _emit 0x89  // MOV [EAX+8], EDX
        _emit 0x50
        _emit 0x08
        _emit 0x89  // MOV [ECX+8], EAX
        _emit 0x41
        _emit 0x08
        _emit 0x8b  // MOV ECX, [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x80  // CMP byte ptr [ECX+0x14], 0
        _emit 0x79
        _emit 0x14
        _emit 0x00
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5d  // POP EBP
        _emit 0x74  // JZ +0x14
        _emit 0x14
        _emit 0x8b  // MOV EDX, [EAX]
        _emit 0x10
        _emit 0x52  // PUSH EDX
        _emit 0x83  // ADD EAX, 0x10
        _emit 0xc0
        _emit 0x10
        _emit 0x68  // PUSH 0xAA (imm32)
        _emit 0xaa
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        call FUN_009d2110
        _emit 0x83  // ADD ESP, 0xC
        _emit 0xc4
        _emit 0x0c
        _emit 0xc2  // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
