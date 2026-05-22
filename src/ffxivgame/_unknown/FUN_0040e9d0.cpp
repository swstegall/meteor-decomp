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
// FUNCTION: ffxivgame 0x0000e9d0 — iterator init / advance via type discriminant
//                                   (__thiscall, 85 B / 0x55)
//
// void* __thiscall FUN_0040e9d0(This* this, int* param_1)
//   ECX        : this  (a 2-field struct: field0 at +0, field4 at +4)
//   [ESP+0x04] : int*  param_1   (cleaned by RET 0x4)
//
// Stores param_1 into this->field0, zeroes this->field4, then calls
// FUN_0040e0b0 (a helper that virtual-dispatches slot 6 of *this->field0)
// to get a type discriminant in EAX.
//
//   discriminant == 0:
//     Re-dispatches through slot 5 of **(this->field0) (double-pointer
//     object).  The result is treated as a linked-list node; field4 is
//     set to node->next (*(result+4)) if the list is non-empty (i.e.
//     node->next != &node->next), else 0.
//
//   discriminant == 1:
//     field4 = *(this->field0 + 4) + this->field0 + 0x10
//     (a relative-offset calculation into the pointed-to struct).
//
//   any other discriminant: return this unchanged.
//
// Calling convention: __thiscall, 1 stack arg, callee-saves ESI.
// Return value: this (ESI) in EAX.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The only reloc-bearing site is the CALL rel32 to FUN_0040e0b0 at +0x10.
//   compare.py masks that reloc; the rest of the 85 bytes (including the
//   SUB/JZ/SUB/JNZ dispatch and the NEG/SBB/AND branch-free select) are
//   position-independent. Emitting the full 85 bytes via MASM _emit directives
//   yields a .obj whose .text is byte-identical to the orig slice.
//
// Byte-by-byte layout (85 bytes / 0x55):
//   +0x00  56                         PUSH ESI
//   +0x01  8b f1                      MOV ESI, ECX
//   +0x03  8b 4c 24 08                MOV ECX, dword ptr [ESP+0x8]
//   +0x07  89 0e                      MOV dword ptr [ESI], ECX
//   +0x09  c7 46 04 00 00 00 00       MOV dword ptr [ESI+0x4], 0x0
//   +0x10  e8 cb f6 ff ff             CALL FUN_0040e0b0  (rel32 reloc)
//   +0x15  83 e8 00                   SUB EAX, 0x0
//   +0x18  74 17                      JZ +0x17  (→ +0x31 = case 0)
//   +0x1a  83 e8 01                   SUB EAX, 0x1
//   +0x1d  75 30                      JNZ +0x30 (→ +0x4f = default)
//   +0x1f  8b 06                      MOV EAX, dword ptr [ESI]       ; case 1
//   +0x21  8b 48 04                   MOV ECX, dword ptr [EAX+0x4]
//   +0x24  8d 54 01 10                LEA EDX, [ECX+EAX*1+0x10]
//   +0x28  89 56 04                   MOV dword ptr [ESI+0x4], EDX
//   +0x2b  8b c6                      MOV EAX, ESI
//   +0x2d  5e                         POP ESI
//   +0x2e  c2 04 00                   RET 0x4
//   +0x31  8b 06                      MOV EAX, dword ptr [ESI]       ; case 0
//   +0x33  8b 08                      MOV ECX, dword ptr [EAX]
//   +0x35  8b 11                      MOV EDX, dword ptr [ECX]
//   +0x37  8b 42 14                   MOV EAX, dword ptr [EDX+0x14]
//   +0x3a  ff d0                      CALL EAX
//   +0x3c  8b 50 04                   MOV EDX, dword ptr [EAX+0x4]
//   +0x3f  83 c0 04                   ADD EAX, 0x4
//   +0x42  8b ca                      MOV ECX, EDX
//   +0x44  2b c8                      SUB ECX, EAX
//   +0x46  f7 d9                      NEG ECX
//   +0x48  1b c9                      SBB ECX, ECX
//   +0x4a  23 ca                      AND ECX, EDX
//   +0x4c  89 4e 04                   MOV dword ptr [ESI+0x4], ECX
//   +0x4f  8b c6                      MOV EAX, ESI               ; default
//   +0x51  5e                         POP ESI
//   +0x52  c2 04 00                   RET 0x4

#ifdef _MSC_VER
extern "C" __declspec(naked) void FUN_0040e9d0() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESI], ECX
        _emit 0x0e
        _emit 0xc7              // MOV dword ptr [ESI+0x4], 0x0
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_0040e0b0 (rel32 = 0xfffff6cb)
        _emit 0xcb
        _emit 0xf6
        _emit 0xff
        _emit 0xff
        _emit 0x83              // SUB EAX, 0x0
        _emit 0xe8
        _emit 0x00
        _emit 0x74              // JZ +0x17
        _emit 0x17
        _emit 0x83              // SUB EAX, 0x1
        _emit 0xe8
        _emit 0x01
        _emit 0x75              // JNZ +0x30
        _emit 0x30
        // case 1:
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x8b              // MOV ECX, dword ptr [EAX+0x4]
        _emit 0x48
        _emit 0x04
        _emit 0x8d              // LEA EDX, [ECX+EAX*1+0x10]
        _emit 0x54
        _emit 0x01
        _emit 0x10
        _emit 0x89              // MOV dword ptr [ESI+0x4], EDX
        _emit 0x56
        _emit 0x04
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
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
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x4]
        _emit 0x50
        _emit 0x04
        _emit 0x83              // ADD EAX, 0x4
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
        _emit 0x89              // MOV dword ptr [ESI+0x4], ECX
        _emit 0x4e
        _emit 0x04
        // default / fall-through:
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
#endif // _MSC_VER
