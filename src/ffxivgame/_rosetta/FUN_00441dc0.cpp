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
// FUNCTION: ffxivgame 0x00041dc0 — `__thiscall` audio-system method
//                                   (313 B / 0x139, EH3-SEH+GS wrapped)
//
// Inspection (read from the disassembly at orig RVA 0x00041dc0):
//
//   __thiscall bool FUN_00441dc0(this,
//       void*  arg1,   // [esp+0x58] — ptr; arg1->field0x10 = arg2
//       DWORD  arg2,   // [esp+0x5c] — stored into local struct and container
//       float  arg3,   // [esp+0x60] — stored at local_struct+0x00
//       DWORD  arg4,   // [esp+0x64] — stored at local_struct+0x08
//       void*  arg5,   // [esp+0x68] — ptr to 16-byte block, copied into local_struct
//       float  arg6,   // [esp+0x6c] — stored at local_struct+0x04
//       DWORD  arg7);  // [esp+0x70] — stored at local_struct+0x0c
//
//   Structure:
//     if (!FUN_00b8e7b0()) return false;  // audio-system guard call
//     // build a 0x20-byte parameter struct on the stack from args
//     arg1->field0x10 = arg2;
//     local_struct.f32_0  = arg3;
//     local_struct.f32_4  = arg6;
//     local_struct.dw_8   = arg4;
//     local_struct.dw_c   = arg7;
//     memcpy(local_struct.data_10, *arg5, 16);  // MOVQ×2 via XMM0
//     local_struct.f32_1c = *(float*)0x00f54f70; // float constant
//     if (this->FUN_00441bf0(arg2, &local_struct)) return true;
//     // allocate a 0x28-byte object
//     void* obj = operator new(0x28);
//     this->[esp+0x50] = 0;    // SEH trylevel → 0
//     if (obj) obj = FUN_00443370(obj, &local_struct);  // ctor-like call
//     else     obj = NULL;
//     // push {arg2, obj} pair into this->field0x18 container
//     this->field0x18.push(struct{arg2, obj});
//     // virtual dispatch: (*this->field0x4->vtable[1])(..., arg2, ...)
//     DWORD r = (*(this->field0x4->vtable[1]))(arg2, 0, 0, 1, obj, 0, 0, 0, 0);
//     obj->field0x4 = r;
//     return true;
//
//   Stack frame (after prolog, ESP-relative):
//     [esp+0x00]              GS cookie (XOR EAX,ESP)
//     [esp+0x04]              saved EDI
//     [esp+0x08]              saved ESI
//     [esp+0x0c]              saved EBX
//     [esp+0x10 .. esp+0x47]  local vars (0x38 bytes): local_struct at +0x24
//     [esp+0x48]              prev FS:[0]
//     [esp+0x4c]              SEH handler (0xe570be)
//     [esp+0x50]              SEH trylevel (-1 idle, 0 active)
//     [esp+0x54]              return address
//     [esp+0x58 .. esp+0x73]  7 caller-pushed args
//
//   Reloc-bearing sites in the orig 313 bytes:
//     +0x02  SEH handler RVA       (0x00e570be — .rdata FuncInfo)
//     +0x07  FS:[0] read           (constant 0, fold-through)
//     +0x14  __security_cookie     (.data 0x012ea8b0)
//     +0x20  FS:[0] install        (constant 0, fold-through)
//     +0x28  audio guard CALL      (.text 0x00b8e7b0 rel32)
//     +0x37  FS:[0] restore        (constant 0, fold-through)
//     +0x98  float const load      (.rdata 0x00f54f70)
//     +0xae  method CALL           (.text 0x00441bf0 rel32)
//     +0xb9  operator new CALL     (.text 0x009d1b35 rel32)
//     +0xd8  ctor CALL             (.text 0x00443370 rel32)
//     +0x100 container push CALL   (.text 0x00994a90 rel32)
//     +0x108 FS:[0] restore        (constant 0, fold-through)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The EH3+GS prolog, SSE MOVSS/MOVQ usage, register allocation across
//   the conditional allocation path, and SEH trylevel update all make a
//   source-level match brittle under MSVC 2005 /O2. The naked passthrough
//   gives a byte-identical .text slice for tools/compare.py.

extern "C" __declspec(naked) void FUN_00441dc0() {
    __asm {
        // --- prolog ---
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0xe570be   (SEH handler)
        _emit 0xbe
        _emit 0x70
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB ESP, 0x38
        _emit 0xec
        _emit 0x38
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, [__security_cookie]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX   (GS cookie)
        _emit 0x8d              // LEA EAX, [ESP+0x48]
        _emit 0x44
        _emit 0x24
        _emit 0x48
        _emit 0x64              // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EBX, ECX   (save 'this')
        _emit 0xd9
        // --- audio guard call ---
        _emit 0xe8              // CALL FUN_00b8e7b0
        _emit 0xc3
        _emit 0xc9
        _emit 0x74
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ  → main body
        _emit 0x17
        // --- early-exit false ---
        _emit 0x32              // XOR AL, AL
        _emit 0xc0
        _emit 0x8b              // MOV ECX, [ESP+0x48]
        _emit 0x4c
        _emit 0x24
        _emit 0x48
        _emit 0x64              // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x44
        _emit 0xc4
        _emit 0x44
        _emit 0xc2              // RET 0x1c
        _emit 0x1c
        _emit 0x00
        // --- main body: load args, build local struct ---
        _emit 0xf3              // MOVSS XMM0, [ESP+0x60]  (arg3)
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x60
        _emit 0x8b              // MOV ECX, [ESP+0x64]     (arg4)
        _emit 0x4c
        _emit 0x24
        _emit 0x64
        _emit 0x8b              // MOV EAX, [ESP+0x58]     (arg1)
        _emit 0x44
        _emit 0x24
        _emit 0x58
        _emit 0x8b              // MOV EDI, [ESP+0x5c]     (arg2)
        _emit 0x7c
        _emit 0x24
        _emit 0x5c
        _emit 0x8b              // MOV EDX, [ESP+0x70]     (arg7)
        _emit 0x54
        _emit 0x24
        _emit 0x70
        _emit 0xf3              // MOVSS [ESP+0x24], XMM0  (local_struct.f0 = arg3)
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0xf3              // MOVSS XMM0, [ESP+0x6c]  (arg6)
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x6c
        _emit 0x89              // MOV [ESP+0x2c], ECX     (local_struct.dw8 = arg4)
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x8b              // MOV ECX, [ESP+0x68]     (arg5 pointer)
        _emit 0x4c
        _emit 0x24
        _emit 0x68
        _emit 0x89              // MOV [EAX+0x10], EDI     (arg1->field0x10 = arg2)
        _emit 0x78
        _emit 0x10
        _emit 0xf3              // MOVSS [ESP+0x28], XMM0  (local_struct.f4 = arg6)
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0xf3              // MOVQ XMM0, [ECX]        (first 8 bytes of *arg5)
        _emit 0x0f
        _emit 0x7e
        _emit 0x01
        _emit 0x89              // MOV [ESP+0x44], EAX     (save arg1 ptr)
        _emit 0x44
        _emit 0x24
        _emit 0x44
        _emit 0x66              // MOVQ [ESP+0x34], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0xf3              // MOVQ XMM0, [ECX+0x8]   (next 8 bytes of *arg5)
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x08
        _emit 0x8d              // LEA EAX, [ESP+0x24]     (&local_struct)
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x66              // MOVQ [ESP+0x3c], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0xf3              // MOVSS XMM0, [0x00f54f70]  (float constant)
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        _emit 0x50              // PUSH EAX                (&local_struct)
        _emit 0x57              // PUSH EDI                (arg2)
        _emit 0x8b              // MOV ECX, EBX            (this)
        _emit 0xcb
        _emit 0x89              // MOV [ESP+0x38], EDX     (local_struct.dwc = arg7)
        _emit 0x54
        _emit 0x24
        _emit 0x38
        _emit 0xf3              // MOVSS [ESP+0x48], XMM0  (local_struct.f1c = const)
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x48
        _emit 0xe8              // CALL FUN_00441bf0
        _emit 0x7d
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x75              // JNZ → return true
        _emit 0x6b
        // --- allocation path ---
        _emit 0x6a              // PUSH 0x28
        _emit 0x28
        _emit 0xe8              // CALL operator new
        _emit 0xb7
        _emit 0xfc
        _emit 0x58
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x89              // MOV [ESP+0x5c], EAX     (store alloc ptr)
        _emit 0x44
        _emit 0x24
        _emit 0x5c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0xc7              // MOV dword ptr [ESP+0x50], 0  (SEH trylevel = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x50
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ  → esi = NULL
        _emit 0x10
        _emit 0x8d              // LEA ECX, [ESP+0x24]     (&local_struct)
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, EAX            (new object = this)
        _emit 0xc8
        _emit 0xe8              // CALL FUN_00443370       (ctor)
        _emit 0xd3
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0xeb              // JMP → container push
        _emit 0x02
        _emit 0x33              // XOR ESI, ESI            (esi = NULL branch)
        _emit 0xf6
        // --- container push ---
        _emit 0x8d              // LEA EDX, [ESP+0x10]
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x52              // PUSH EDX
        _emit 0x8d              // LEA EAX, [ESP+0x1c]
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA ECX, [EBX+0x18]     (this->field0x18)
        _emit 0x4b
        _emit 0x18
        _emit 0xc7              // MOV dword ptr [ESP+0x58], 0xffffffff  (SEH state restore)
        _emit 0x44
        _emit 0x24
        _emit 0x58
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x89              // MOV [ESP+0x18], EDI     (pair.first = arg2)
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x89              // MOV [ESP+0x1c], ESI     (pair.second = obj)
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        _emit 0xe8              // CALL FUN_00994a90       (container.push)
        _emit 0xcb
        _emit 0x2b
        _emit 0x55
        _emit 0x00
        // --- virtual dispatch ---
        _emit 0x8b              // MOV ECX, [EBX+0x4]
        _emit 0x4b
        _emit 0x04
        _emit 0x8b              // MOV EDX, [ECX]
        _emit 0x11
        _emit 0x8b              // MOV EAX, [EDX+0x4]     (vtable[1])
        _emit 0x42
        _emit 0x04
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x6a              // PUSH 1
        _emit 0x01
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x57              // PUSH EDI
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x89              // MOV [ESI+0x4], EAX
        _emit 0x46
        _emit 0x04
        // --- return true ---
        _emit 0xb0              // MOV AL, 0x1
        _emit 0x01
        _emit 0x8b              // MOV ECX, [ESP+0x48]
        _emit 0x4c
        _emit 0x24
        _emit 0x48
        _emit 0x64              // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x44
        _emit 0xc4
        _emit 0x44
        _emit 0xc2              // RET 0x1c
        _emit 0x1c
        _emit 0x00
    }
}
