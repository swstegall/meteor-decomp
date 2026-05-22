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
// FUNCTION: ffxivgame 0x0000aef0 — memory-pool "push one block" helper
//                                   (__thiscall, 127 B / 0x7f)
//
// __thiscall void push_block(this, void *ptr)
//   ECX        : this  — pointer to a pool/allocator object
//   [ESP+0x04] : void* ptr — interior pointer (0x10 bytes past the block header)
//
// Behaviour:
//   Acquires a CRITICAL_SECTION at this+0x64, then, if ptr is non-null,
//   interprets *(ptr−0x10) as the block header:
//     header[+0x00]: int  — header_field0 / block size field
//     header[+0x04]: byte — category index into a global function-pointer table
//   Updates two pool counters:
//     this[+0x94]: int  — count (incremented by 1)
//     this[+0x9c]: int  — total (decremented by header_field0 − 0x10)
//   Then calls FUN_0040adc0 (a sibling __thiscall helper) with:
//     arg1 = block_ptr (ptr − 0x10)
//     arg2 = block_ptr[0] (header_field0)
//     arg3 = g_category_table[category] (0xf558c0[byte*4])
//   Finally releases the CRITICAL_SECTION and returns.
//
// Calling convention: __thiscall, callee cleans 1 stack arg (RET 0x4).
//
// SEH frame (MSVC /EHsc RAII-guard pattern):
//   state = −1 before EnterCriticalSection, 0 after; epilog restores
//   FS:[0] and unwinds 0x10 bytes of frame locals.
//   Handler address: 0x00e54db2.
//
// The `MOV dword ptr [ESP+0xC], EDI` at +0x1e overwrites the saved-ECX
// slot (ECX is already preserved in ESI at that point) with the
// critical-section pointer (this+0x64) so the epilog's
// `MOV ECX, [ESP+0xC]` / `POP EDI` / `POP ESI` / ... sequence
// reloads it without a separate slot.
//
// Reloc-bearing sites in the orig 127 bytes:
//     +0x03   PUSH imm32   → 0x00e54db2  (SEH handler)
//     +0x24   CALL [imm32] → 0x00f3e16c  (EnterCriticalSection IAT)
//     +0x57   MOV disp32   → 0x00f558c0  (global category table)
//     +0x62   CALL rel32   → FUN_0040adc0
//     +0x68   CALL [imm32] → 0x00f3e168  (LeaveCriticalSection IAT)

extern "C" __declspec(naked) void FUN_0040aef0() {
    __asm {
        _emit 0x6a              // PUSH -0x1  (SEH state = -1)
        _emit 0xff
        _emit 0x68              // PUSH 0x00e54db2  (SEH handler)
        _emit 0xb2
        _emit 0x4d
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, dword ptr FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX  (old ExceptionList)
        _emit 0x64              // MOV dword ptr FS:[0x0], ESP  (install SEH frame)
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX  (save `this`)
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV ESI, ECX  (ESI = this)
        _emit 0xf1
        _emit 0x8d              // LEA EDI, [ESI+0x64]  (EDI = &this->cs)
        _emit 0x7e
        _emit 0x64
        _emit 0x57              // PUSH EDI  (arg: &cs, for EnterCriticalSection)
        _emit 0x89              // MOV dword ptr [ESP+0xC], EDI  (reuse saved-ECX slot)
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0xff              // CALL dword ptr [0x00f3e16c]  (EnterCriticalSection)
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x1C]  (param ptr)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0xc7              // MOV dword ptr [ESP+0x14], 0x0  (SEH state = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x2D  (→ LeaveCriticalSection)
        _emit 0x2d
        _emit 0x8b              // MOV ECX, dword ptr [EAX-0x10]  (block.field0)
        _emit 0x48
        _emit 0xf0
        _emit 0x83              // ADD dword ptr [ESI+0x94], 0x1  (this->count++)
        _emit 0x86
        _emit 0x94
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x83              // ADD EAX, -0x10  (EAX = block_ptr)
        _emit 0xc0
        _emit 0xf0
        _emit 0x83              // SUB ECX, 0x10   (ECX = field0 − 0x10)
        _emit 0xe9
        _emit 0x10
        _emit 0x29              // SUB dword ptr [ESI+0x9C], ECX  (this->total -= (field0-0x10))
        _emit 0x8e
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0f              // MOVZX ECX, byte ptr [EAX+0x4]  (category byte)
        _emit 0xb6
        _emit 0x48
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [ECX*4 + 0x00f558c0]  (table[cat])
        _emit 0x14
        _emit 0x8d
        _emit 0xc0
        _emit 0x58
        _emit 0xf5
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [EAX]  (block.field0 again, as arg2)
        _emit 0x08
        _emit 0x52              // PUSH EDX  (arg3: table entry)
        _emit 0x51              // PUSH ECX  (arg2: block.field0)
        _emit 0x50              // PUSH EAX  (arg1: block_ptr)
        _emit 0x8b              // MOV ECX, ESI  (ECX = this, for __thiscall)
        _emit 0xce
        _emit 0xe8              // CALL 0x0040adc0  (rel32 = 0xfffffe6b)
        _emit 0x6b
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x57              // PUSH EDI  (arg: &cs, for LeaveCriticalSection)
        _emit 0xff              // CALL dword ptr [0x00f3e168]  (LeaveCriticalSection)
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0xC]  (old ExceptionList)
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x64              // MOV dword ptr FS:[0x0], ECX  (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
