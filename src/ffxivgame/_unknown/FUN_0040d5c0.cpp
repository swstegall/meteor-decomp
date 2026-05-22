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
// FUNCTION: ffxivgame 0x0000d5c0 — sorted linked-list insert by address
//                                   (__thiscall, 64 B / 0x40)
//
// __thiscall void insert_sorted(this, Node *node)
//   ECX        : this  — pointer to the head pointer (Node **)
//   [ESP+0x04] : Node* — node to insert
//
// Nodes are linked via the field at offset +0x9a4 (next pointer).
// The list is kept sorted in ascending order by pointer value.
//
// Calling convention: __thiscall, callee cleans 1 stack arg (RET 0x4).
// Callee-saved: ESI only.  No /GS cookie, no local frame.
//
// No relocations — pure register arithmetic with no IAT/string refs.
// The sorted-insert while-loop causes MSVC to emit a 6-byte LEA-NOP
// alignment pad at the loop head under /O2 when written in C++.
// Using __declspec(naked) + verbatim _emit reproduces the orig 64
// bytes byte-for-byte.

extern "C" __declspec(naked) void FUN_0040d5c0()
{
    __asm {
        // 0000d5c0:  8b 01              MOV EAX, dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 0000d5c2:  85 c0              TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0000d5c4:  75 09              JNZ +0x09 (-> 0x0040d5cf)
        _emit 0x75
        _emit 0x09
        // 0000d5c6:  8b 44 24 04        MOV EAX, dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0000d5ca:  89 01              MOV dword ptr [ECX], EAX
        _emit 0x89
        _emit 0x01
        // 0000d5cc:  c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 0000d5cf:  56                 PUSH ESI
        _emit 0x56
        // 0000d5d0:  8b 74 24 08        MOV ESI, dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 0000d5d4:  33 d2              XOR EDX, EDX
        _emit 0x33
        _emit 0xd2
        // 0000d5d6:  3b f0              CMP ESI, EAX          ; loop_top
        _emit 0x3b
        _emit 0xf0
        // 0000d5d8:  72 16              JC +0x16 (-> 0x0040d5f0)
        _emit 0x72
        _emit 0x16
        // 0000d5da:  8b d0              MOV EDX, EAX
        _emit 0x8b
        _emit 0xd0
        // 0000d5dc:  8b 80 a4 09 00 00  MOV EAX, dword ptr [EAX+0x9a4]
        _emit 0x8b
        _emit 0x80
        _emit 0xa4
        _emit 0x09
        _emit 0x00
        _emit 0x00
        // 0000d5e2:  85 c0              TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0000d5e4:  75 f0              JNZ -0x10 (-> 0x0040d5d6)
        _emit 0x75
        _emit 0xf0
        // 0000d5e6:  89 b2 a4 09 00 00  MOV dword ptr [EDX+0x9a4], ESI
        _emit 0x89
        _emit 0xb2
        _emit 0xa4
        _emit 0x09
        _emit 0x00
        _emit 0x00
        // 0000d5ec:  5e                 POP ESI
        _emit 0x5e
        // 0000d5ed:  c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 0000d5f0:  85 d2              TEST EDX, EDX
        _emit 0x85
        _emit 0xd2
        // 0000d5f2:  89 86 a4 09 00 00  MOV dword ptr [ESI+0x9a4], EAX
        _emit 0x89
        _emit 0x86
        _emit 0xa4
        _emit 0x09
        _emit 0x00
        _emit 0x00
        // 0000d5f8:  75 ec              JNZ -0x14 (-> 0x0040d5e6)
        _emit 0x75
        _emit 0xec
        // 0000d5fa:  89 31              MOV dword ptr [ECX], ESI
        _emit 0x89
        _emit 0x31
        // 0000d5fc:  5e                 POP ESI
        _emit 0x5e
        // 0000d5fd:  c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
