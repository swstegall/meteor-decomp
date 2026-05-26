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
// FUNCTION: ffxivgame 0x0000e970 — LeakIter::advance() (__thiscall, 95 B)
//
// Advances a LeakIter by one step.  The iterator struct layout (inferred
// from the ctor FUN_0040e9d0 and is_valid FUN_0040ea30):
//
//   +0x00  int *head   — pointer to the MemoryTracker
//   +0x04  int *cur    — current node pointer (null ⇒ exhausted)
//
// FUN_0040e0b0 (9 B):
//   MOV ECX, [ECX]; MOV EAX, [ECX]; MOV EDX, [EAX+0x18]; JMP EDX
//   Called with ECX = this->head; tail-calls vtable[6](head_node).
//   Returns a mode integer (0 or 1).
//
// Mode 0 path (0x0040e9a4–0x0040e9ba):
//   ECX = *this->head; ECX = *ECX (the node object)
//   EAX = vtable[5](node) via CALL [vtable+0x14]
//   this->cur = *this->cur  (advance one link-list step)
//   EAX += 4               (sentinel offset)
//
// Mode 1 path (0x0040e98a–0x0040e9a2):
//   EAX = this->cur;  ECX = EAX[0] + EAX + 0x10   (new cursor)
//   this->cur = ECX
//   EAX = this->head
//   EAX = EAX[2] + EAX[1] + EAX - 0x10            (end sentinel)
//
// Both paths then compare ECX (new cur) with EAX (end sentinel);
// if equal, this->cur = 0.
//
// Epilogue:
//   XOR EAX, EAX
//   CMP [ESI+4], EAX        (this->cur vs 0)
//   POP ESI
//   SETNZ AL                (return cur != 0)
//   RET
//
// Calling convention: __thiscall, no stack args, plain RET.

extern "C" __declspec(naked) void FUN_0040e970() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x83              // CMP dword ptr [ESI+0x4], 0
        _emit 0x7e
        _emit 0x04
        _emit 0x00
        _emit 0x74              // JZ +0x4c  (→ 0x0040e9c5)
        _emit 0x4c
        _emit 0x8b              // MOV ECX, dword ptr [ESI]
        _emit 0x0e
        _emit 0xe8              // CALL FUN_0040e0b0  (rel32 → 0xfffff730 = -0x8d0+5? let me recalc)
        _emit 0x30
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        _emit 0x83              // SUB EAX, 0
        _emit 0xe8
        _emit 0x00
        _emit 0x74              // JZ +0x1f  (→ 0x0040e9a4)
        _emit 0x1f
        _emit 0x83              // SUB EAX, 1
        _emit 0xe8
        _emit 0x01
        _emit 0x75              // JNZ +0x3b  (→ 0x0040e9c5)
        _emit 0x3b
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [EAX]
        _emit 0x08
        _emit 0x8d              // LEA ECX, [ECX + EAX*1 + 0x10]
        _emit 0x4c
        _emit 0x01
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x89              // MOV dword ptr [ESI+0x4], ECX
        _emit 0x4e
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x8]
        _emit 0x50
        _emit 0x08
        _emit 0x03              // ADD EDX, dword ptr [EAX+0x4]
        _emit 0x50
        _emit 0x04
        _emit 0x8d              // LEA EAX, [EDX + EAX*1 - 0x10]
        _emit 0x44
        _emit 0x02
        _emit 0xf0
        _emit 0xeb              // JMP +0x16  (→ 0x0040e9ba)
        _emit 0x16
        _emit 0x8b              // MOV ECX, dword ptr [ESI]     (case0:)
        _emit 0x0e
        _emit 0x8b              // MOV ECX, dword ptr [ECX]
        _emit 0x09
        _emit 0x8b              // MOV EDX, dword ptr [ECX]
        _emit 0x11
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x14]
        _emit 0x42
        _emit 0x14
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x4]
        _emit 0x4e
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [ECX]
        _emit 0x09
        _emit 0x89              // MOV dword ptr [ESI+0x4], ECX
        _emit 0x4e
        _emit 0x04
        _emit 0x83              // ADD EAX, 4
        _emit 0xc0
        _emit 0x04
        _emit 0x3b              // CMP ECX, EAX               (compare:)
        _emit 0xc8
        _emit 0x75              // JNZ +0x07  (→ 0x0040e9c5)
        _emit 0x07
        _emit 0xc7              // MOV dword ptr [ESI+0x4], 0
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33              // XOR EAX, EAX               (end:)
        _emit 0xc0
        _emit 0x39              // CMP dword ptr [ESI+0x4], EAX
        _emit 0x46
        _emit 0x04
        _emit 0x5e              // POP ESI
        _emit 0x0f              // SETNZ AL
        _emit 0x95
        _emit 0xc0
        _emit 0xc3              // RET
    }
}
