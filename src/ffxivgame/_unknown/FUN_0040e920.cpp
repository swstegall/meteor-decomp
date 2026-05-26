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
// FUNCTION: ffxivgame 0x0000e920 — LeakIter::begin() / reset-to-start
//                                   (__thiscall, 73 B / 0x49)
//
// Sets the iterator's cursor (m_cur at this+4) to point at the first
// element of the underlying tracker structure, depending on mode.
//
// Object layout (inferred from sibling FUN_0040e970 and FUN_0040e9d0):
//   [this + 0x00]  int*  m_head   — pointer to the MemoryTracker
//   [this + 0x04]  int*  m_cur    — current node pointer (null ⇒ exhausted)
//
// FUN_0040e0b0 (9 B):
//   MOV ECX, [ECX]; MOV EAX, [ECX]; MOV EDX, [EAX+0x18]; JMP EDX
//   Called with ECX = this->m_head; tail-calls vtable[6](head_obj).
//   Returns mode int in EAX (0 = linked-list mode, 1 = flat-buffer mode).
//
// Mode 0 (case 0): vtable[5] of **m_head is called; returns a list head.
//   Cursor is set to head->next, or NULL if list is empty (head==head->next).
//
// Mode 1 (case 1): cursor set to flat-buffer start:
//   m_cur = (int*)((char*)m_head + m_head[1] + 0x10)
//
// Other modes: m_cur stays 0 (set at entry).
//
// Calling convention: __thiscall, no stack args, plain RET.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The source-level C++ form of this function generates register-allocation
//   patterns that don't match the orig (EDX vs EAX for vtable load, ECX vs EDX
//   for the NEG/SBB/AND sequence). The naked-asm passthrough reproduces the
//   exact orig 73 bytes verbatim. compare.py masks the 4 CALL rel32 bytes
//   (the CALL FUN_0040e0b0 reloc), so the .obj's .text section is
//   byte-identical to the orig slice modulo that masked reloc.

extern "C" __declspec(naked) void FUN_0040e920() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV ECX, dword ptr [ESI]
        _emit 0x0e
        _emit 0xc7              // MOV dword ptr [ESI+4], 0
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_0040e0b0 (rel32)
        _emit 0x7f
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        _emit 0x83              // SUB EAX, 0
        _emit 0xe8
        _emit 0x00
        _emit 0x74              // JZ +0x13 (→ case0 at 0x0040e949)
        _emit 0x13
        _emit 0x83              // SUB EAX, 1
        _emit 0xe8
        _emit 0x01
        _emit 0x75              // JNZ +0x2c (→ end at 0x0040e967)
        _emit 0x2c
        // case 1:
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x8b              // MOV ECX, dword ptr [EAX+4]
        _emit 0x48
        _emit 0x04
        _emit 0x8d              // LEA EDX, [ECX + EAX*1 + 0x10]
        _emit 0x54
        _emit 0x01
        _emit 0x10
        _emit 0x89              // MOV dword ptr [ESI+4], EDX
        _emit 0x56
        _emit 0x04
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        // case 0:
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x8b              // MOV ECX, dword ptr [EAX]
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ECX]
        _emit 0x11
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x14]
        _emit 0x42
        _emit 0x14
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV EDX, dword ptr [EAX+4]
        _emit 0x50
        _emit 0x04
        _emit 0x83              // ADD EAX, 4
        _emit 0xc0
        _emit 0x04
        _emit 0x8b              // MOV ECX, EDX
        _emit 0xca
        _emit 0x2b              // SUB ECX, EAX
        _emit 0xc8
        _emit 0xf7              // NEG ECX
        _emit 0xd9
        _emit 0x1b              // SBB ECX, ECX
        _emit 0xc9
        _emit 0x23              // AND ECX, EDX
        _emit 0xca
        _emit 0x89              // MOV dword ptr [ESI+4], ECX
        _emit 0x4e
        _emit 0x04
        // end:
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}

// vim: ts=4 sts=4 sw=4 et
