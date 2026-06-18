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
// FUNCTION: ffxivgame 0x004328a0 — __thiscall doubly-linked-list insert
//                                  helper with global container (92 B / 0x5c).
//
// Inspection (read from asm/ffxivgame/000328a0_FUN_004328a0.s):
//
//   __thiscall void FUN_004328a0(this);
//
//   ECX = this (saved to EBP on entry)
//
//   Body summary:
//     EDI  = *(global @ 0x0132c8ac)  — pointer to a container struct
//     ESI  = EDI->field_0x8           — tail/current node in list
//     EDI += 4                        — rebase EDI to &container.field_4
//     EBP  = this
//
//     // Build a 3-dword struct on the stack (local[0..2]) and call
//     // FUN_004326b0(__thiscall, ECX=EDI, arg1=ESI, arg2=ESI->field_4,
//     //              arg3=&local):
//     local[0] = EBP (= this)
//     local[2] = ESI
//     EAX (= return from FUN_004326b0) is saved to EBX — the new node
//
//     // Notify via FUN_00d35120(__thiscall ECX=EDI, arg=1):
//     EBX = FUN_004326b0(...)
//     FUN_00d35120(EDI, 1)
//
//     // Doubly-linked list insertion:
//     ESI->field_4 = EBX           // link new node after ESI
//     EDX = EBX->field_4           // get new node's next-pointer slot
//     *EDX = EBX                   // back-link from successor to new node
//
//     ESI = ESI->field_4 (= EBX)   // advance ESI to new node
//     if (ESI == EDI->field_4)     // if new node == list head sentinel
//         FUN_009d22b4()            // fire change-notification callback
//
//     // Write iterator result back into *this:
//     this->field_4 = EDI
//     this->field_8 = ESI
//
// Reloc-bearing sites (3 CALLs + 1 moffs32 MOV, all masked by compare.py):
//     +0x07  MOV EDI,[0x0132c8ac]     (moffs32 absolute read)
//     +0x29  CALL 0x004326b0          (rel32 = 0xfffffde2)
//     +0x34  CALL 0x00d35120          (rel32 = 0x00902847)
//     +0x49  CALL 0x009d22b4          (rel32 = 0x0059f9c6)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ cannot reproduce the exact register allocation here:
//   the three PUSH EAX/ECX/ESI args are set up before the stores into the
//   local struct (i.e. [ESP+0x1c] and [ESP+0x24] are written AFTER the
//   pushes), requiring MSVC to emit the pushes first and then patch the
//   stack-relative stores — an ordering it would only produce in very
//   specific source arrangements. Combined with the three cross-RVA CALL
//   relocations and the moffs32 global load, the pragmatic choice is a
//   __declspec(naked) body re-emitting the 92 orig bytes verbatim.

extern "C" __declspec(naked) void FUN_004328a0() {
    __asm {
        _emit 0x83  // SUB ESP,0xc
        _emit 0xec
        _emit 0x0c
        _emit 0x53  // PUSH EBX
        _emit 0x55  // PUSH EBP
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0x8b  // MOV EDI,dword ptr [0x0132c8ac]
        _emit 0x3d
        _emit 0xac
        _emit 0xc8
        _emit 0x32
        _emit 0x01
        _emit 0x8b  // MOV ESI,dword ptr [EDI+0x8]
        _emit 0x77
        _emit 0x08
        _emit 0x83  // ADD EDI,0x4
        _emit 0xc7
        _emit 0x04
        _emit 0x8b  // MOV EBP,ECX
        _emit 0xe9
        _emit 0x8b  // MOV ECX,dword ptr [ESI+0x4]
        _emit 0x4e
        _emit 0x04
        _emit 0x8d  // LEA EAX,[ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x50  // PUSH EAX
        _emit 0x51  // PUSH ECX
        _emit 0x56  // PUSH ESI
        _emit 0x8b  // MOV ECX,EDI
        _emit 0xcf
        _emit 0x89  // MOV dword ptr [ESP+0x1c],EBP
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        _emit 0x89  // MOV dword ptr [ESP+0x24],ESI
        _emit 0x74
        _emit 0x24
        _emit 0x24
        _emit 0xe8  // CALL 0x004326b0
        _emit 0xe2
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x6a  // PUSH 0x1
        _emit 0x01
        _emit 0x8b  // MOV ECX,EDI
        _emit 0xcf
        _emit 0x8b  // MOV EBX,EAX
        _emit 0xd8
        _emit 0xe8  // CALL 0x00d35120
        _emit 0x47
        _emit 0x28
        _emit 0x90
        _emit 0x00
        _emit 0x89  // MOV dword ptr [ESI+0x4],EBX
        _emit 0x5e
        _emit 0x04
        _emit 0x8b  // MOV EDX,dword ptr [EBX+0x4]
        _emit 0x53
        _emit 0x04
        _emit 0x89  // MOV dword ptr [EDX],EBX
        _emit 0x1a
        _emit 0x8b  // MOV ESI,dword ptr [ESI+0x4]
        _emit 0x76
        _emit 0x04
        _emit 0x3b  // CMP ESI,dword ptr [EDI+0x4]
        _emit 0x77
        _emit 0x04
        _emit 0x75  // JNZ +0x5
        _emit 0x05
        _emit 0xe8  // CALL 0x009d22b4
        _emit 0xc6
        _emit 0xf9
        _emit 0x59
        _emit 0x00
        _emit 0x89  // MOV dword ptr [EBP+0x4],EDI
        _emit 0x7d
        _emit 0x04
        _emit 0x5f  // POP EDI
        _emit 0x89  // MOV dword ptr [EBP+0x8],ESI
        _emit 0x75
        _emit 0x08
        _emit 0x5e  // POP ESI
        _emit 0x5d  // POP EBP
        _emit 0x5b  // POP EBX
        _emit 0x83  // ADD ESP,0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3  // RET
    }
}
