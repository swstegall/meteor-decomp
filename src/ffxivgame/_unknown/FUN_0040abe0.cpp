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
// FUNCTION: ffxivgame 0x0000abe0 — find node in linked-list bucket by predicate
//                                   (__thiscall, 153 B / 0x99)
//
// __thiscall void* find_node(this, int index, SomeArg param_2)
//   ECX        : this  — pointer to an array of Node* bucket heads
//   [ESP+0x04] : int   index   — which bucket to search
//   [ESP+0x08] : param_2       — opaque arg forwarded to the predicate method
//
// Memory layout of Node (inferred from +0x1c offset, same as FUN_0040ab90):
//   struct Node { /* ... 0x1c bytes ... */ Node *next; };
//
// Behaviour:
//   Walks the singly-linked list at this->arr[index].  For each node it
//   constructs a local predicate object (via FUN_0040dd50), calls its test
//   method (FUN_0040ddd0, forwarding param_2) and then destroys it
//   (FUN_0040db10).  Returns the first node for which the predicate is
//   non-zero, or NULL if none is found.
//
// Calling convention: __thiscall, callee cleans 2 stack args (RET 0x8).
// Stack frame: 0x24 bytes of locals + SEH/C++ EH frame.
//   The SEH frame guards the local predicate object: state is set to 0
//   before FUN_0040ddd0 and -1 after so that an in-flight exception will
//   trigger FUN_0040db10 to clean up.
//
// The 6-byte `LEA EBX,[EBX]` at +0x2a is a NOP used to align the loop body.
//
// Reloc-bearing sites in the orig 153 bytes:
//     +0x04   PUSH imm32  → 0x00e54d8e  (__CxxFrameHandler3 / SEH handler)
//     +0x2d   CALL rel32  → FUN_0040dd50  (predicate object ctor)
//     +0x4a   CALL rel32  → FUN_0040ddd0  (predicate method / test)
//     +0x5d   CALL rel32  → FUN_0040db10  (predicate object dtor)

extern "C" __declspec(naked) void FUN_0040abe0() {
    __asm {
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH -0x1  (SEH state = -1)
        _emit 0xff
        _emit 0x68              // PUSH 0x00e54d8e  (SEH handler address)
        _emit 0x8e
        _emit 0x4d
        _emit 0xe5
        _emit 0x00
        _emit 0x50              // PUSH EAX  (old ExceptionList)
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]  (param_1 / index)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV dword ptr FS:[0x0], ESP  (install SEH frame)
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // SUB ESP, 0x24
        _emit 0xec
        _emit 0x24
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ECX + EAX*4]  (bucket head)
        _emit 0x34
        _emit 0x81
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x57              // PUSH EDI
        _emit 0x74              // JZ  null_return  (+0x47)
        _emit 0x47
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x44]  (param_2)
        _emit 0x7c
        _emit 0x24
        _emit 0x44
        _emit 0x8d              // LEA EBX, [EBX]  (6-byte NOP, loop-align pad)
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // loop_top: (offset 0x30)
        _emit 0x8d              // LEA ECX, [ESP+0x0C]  (&local predicate object)
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0xe8              // CALL FUN_0040dd50  (ctor, rel32 → 0x003137)
        _emit 0x37
        _emit 0x31
        _emit 0x00
        _emit 0x00
        _emit 0x57              // PUSH EDI  (push param_2 as arg)
        _emit 0x8d              // LEA ECX, [ESP+0x10]  (&local, after push)
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0xc7              // MOV dword ptr [ESP+0x3C], 0x0  (SEH state = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESP+0x14], ESI  (store node ptr)
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0xe8              // CALL FUN_0040ddd0  (predicate test, rel32 → 0x31a1)
        _emit 0xa1
        _emit 0x31
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP+0x0C]  (&local, after callee cleaned arg)
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x8a              // MOV BL, AL  (save return value)
        _emit 0xd8
        _emit 0xc7              // MOV dword ptr [ESP+0x38], 0xFFFFFFFF  (SEH state = -1)
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8              // CALL FUN_0040db10  (dtor, rel32 → 0x2ece)
        _emit 0xce
        _emit 0x2e
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST BL, BL
        _emit 0xdb
        _emit 0x75              // JNZ  found  (+0x1d)
        _emit 0x1d
        _emit 0x8b              // MOV ESI, dword ptr [ESI+0x1c]  (advance to next)
        _emit 0x76
        _emit 0x1c
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75              // JNZ  loop_top  (-0x3d)
        _emit 0xc3
        // null_return: (offset 0x6d)
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5b              // POP EBX
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x24]  (old ExceptionList)
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x64              // MOV dword ptr FS:[0x0], ECX  (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x30
        _emit 0xc4
        _emit 0x30
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        // found: (offset 0x83)
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x30]  (old ExceptionList)
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI  (return node ptr)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x64              // MOV dword ptr FS:[0x0], ECX  (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x30
        _emit 0xc4
        _emit 0x30
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
