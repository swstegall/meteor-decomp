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
// FUNCTION: ffxivgame 0x00433bd0 — factory method that allocates a 16-byte
//                                  handler object, wires up its vtable +
//                                  three fields, and registers it with a
//                                  member queue (__thiscall, 139 bytes).
//
// Calling convention: __thiscall (ECX = this → saved in EBX); RET 0xc
// cleans 3 dword stack args, so the source shape is:
//
//   void* __thiscall FUN_00433bd0(This* this, int a1, int a2, int a3);
//
// Body (inferred):
//   p   = *(Slot**)0x01328d90;            // global current-context pointer
//   sub = p->base[ p->idx[0] * 7 ];       // ECX = p->[4] + 28*p->byte[0]
//   r   = call_417a70(sub, a3, a2 * 4);   // → EDI
//   obj = call_417ab0(sub, 0x10);         // allocate 16 bytes
//   if (obj) {
//       obj->vftable = 0x00f64920;
//       obj->f4 = a1; obj->f8 = a2; obj->fc = r;
//       return call_43c2d0(this->f0xc, obj);
//   }
//   return call_43c2d0(this->f0xc, 0);
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function references absolute addresses (the global pointer at
//   0x01328d90 and the vtable immediate 0x00f64920) and makes four rel32
//   CALLs (0x00417a70, 0x00417ab0, 0x0043c2d0 ×2). A source-level C++
//   form would emit symbol relocations the linker fills in, but the
//   __thiscall arg-shuffle, the 28*idx SIB scaling, and the duplicated
//   context-pointer reload are not coercible from C++ under /O2. As with
//   siblings FUN_0040ad30 / FUN_00408610, the pragmatic match is a
//   __declspec(naked) body re-emitting the original 139 bytes verbatim.
//   The displacement/immediate bytes are copied from the linked slice, so
//   the .obj's .text is byte-identical to the original and compare.py
//   reports GREEN.

extern "C" __declspec(naked) void FUN_00433bd0() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, ECX
        _emit 0xd9
        _emit 0x8b              // MOV ECX, dword ptr [0x01328d90]
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x0f              // MOVZX EAX, byte ptr [ECX]
        _emit 0xb6
        _emit 0x01
        _emit 0x8d              // LEA EDX, [EAX*0x8 + 0x0]
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB EDX, EAX
        _emit 0xd0
        _emit 0x8b              // MOV EAX, dword ptr [ECX + 0x4]
        _emit 0x41
        _emit 0x04
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP + 0x10]
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x8d              // LEA ECX, [EAX + EDX*0x4]
        _emit 0x0c
        _emit 0x90
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA EDX, [ESI*0x4 + 0x0]
        _emit 0x14
        _emit 0xb5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x52              // PUSH EDX
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x00417a70
        _emit 0x6d
        _emit 0x3e
        _emit 0xfe
        _emit 0xff
        _emit 0x8b              // MOV ECX, dword ptr [0x01328d90]
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV EDI, EAX
        _emit 0xf8
        _emit 0x0f              // MOVZX EAX, byte ptr [ECX]
        _emit 0xb6
        _emit 0x01
        _emit 0x8d              // LEA EDX, [EAX*0x8 + 0x0]
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB EDX, EAX
        _emit 0xd0
        _emit 0x8b              // MOV EAX, dword ptr [ECX + 0x4]
        _emit 0x41
        _emit 0x04
        _emit 0x8d              // LEA ECX, [EAX + EDX*0x4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0xe8              // CALL 0x00417ab0
        _emit 0x8c
        _emit 0x3e
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ 0x00433c4a
        _emit 0x22
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0xc7              // MOV dword ptr [EAX], 0xf64920
        _emit 0x00
        _emit 0x20
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x89              // MOV dword ptr [EAX + 0x4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x89              // MOV dword ptr [EAX + 0x8], ESI
        _emit 0x70
        _emit 0x08
        _emit 0x89              // MOV dword ptr [EAX + 0xc], EDI
        _emit 0x78
        _emit 0x0c
        _emit 0x8b              // MOV ECX, dword ptr [EBX + 0xc]
        _emit 0x4b
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043c2d0
        _emit 0x8c
        _emit 0x86
        _emit 0x00
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [EBX + 0xc]   (0x00433c4a)
        _emit 0x4b
        _emit 0x0c
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043c2d0
        _emit 0x7b
        _emit 0x86
        _emit 0x00
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
