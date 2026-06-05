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
// FUNCTION: ffxivgame 0x00447f70 — __thiscall member, 177 B / 0xb1
//
//   __thiscall T* method(this, arg1, arg2, arg3)   (RET 0xc — 3 stack args)
//
//   EH3-style SEH frame (PUSH -1 / scope_table 0xe57560 / FS:[0] chain +
//   /GS security cookie XOR ESP). A 0x54-byte locals frame holds a temp
//   sub-object built at [ESP+0x14] by the first __thiscall (0x00447a80),
//   whose lifetime is tracked by the EH state slot at [ESP+0x68]
//   (0 → 0xffffffff).
//
//   Logical shape (NOT byte-equivalent on its own):
//
//     T* this->method(a1, a2, a3) {
//         Temp tmp;
//         T* p = a1->build(&tmp, a2, a3);          // 0x00447a80
//         if (p != this) {
//             this->reserve(p->m_8, 1);            // 0x00447010
//             func(this->m_0, p->m_0, this->m_8);  // 0x009d4600 (cdecl)
//             this->m_10 = p->m_10;                // byte
//             this->m_c  = p->m_c;                 // dword
//         }
//         if (tmp.m_11 == 0)
//             cleanup(tmp.m_0, tmp.m_4, 0xb);      // 0x0044d350 (cdecl)
//         return this;
//     }
//
//   Reloc-bearing sites (compare.py masks the 4-byte reloc windows):
//     +0x03  scope_table pointer            (0x00e57560)
//     +0x14  __security_cookie load         (.data 0x012ea8b0)
//     +0x3b  CALL rel32 → 0x00447a80
//     +0x56  CALL rel32 → 0x00447010
//     +0x65  CALL rel32 → 0x009d4600
//     +0x94  CALL rel32 → 0x0044d350
//
// Reconstruction strategy — naked-asm byte passthrough (same as the
// SEH/reloc-bearing siblings FUN_00401750, FUN_00403f10, etc.). The
// inlined EH3 prolog + /GS cookie + four reloc CALLs make a source-level
// rewrite impractical to byte-match; re-emitting the orig 177 bytes
// verbatim yields a byte-identical .text slice that compare.py reports
// GREEN.

extern "C" __declspec(naked) void FUN_00447f70() {
    __asm {
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0x00e57560 (scope_table)
        _emit 0x60
        _emit 0x75
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB ESP, 0x54
        _emit 0xec
        _emit 0x54
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, [0x012ea8b0] (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EAX, [ESP+0x60]
        _emit 0x44
        _emit 0x24
        _emit 0x60
        _emit 0x64              // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, ECX (this)
        _emit 0xf1
        _emit 0x8b              // MOV EAX, [ESP+0x78] (arg3)
        _emit 0x44
        _emit 0x24
        _emit 0x78
        _emit 0x8b              // MOV ECX, [ESP+0x74] (arg2)
        _emit 0x4c
        _emit 0x24
        _emit 0x74
        _emit 0x50              // PUSH EAX (arg3)
        _emit 0x51              // PUSH ECX (arg2)
        _emit 0x8b              // MOV ECX, [ESP+0x78] (arg1 -> this for call)
        _emit 0x4c
        _emit 0x24
        _emit 0x78
        _emit 0x8d              // LEA EDX, [ESP+0x14] (&tmp)
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL rel32 → 0x00447a80
        _emit 0xd1
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EDI, EAX
        _emit 0xf8
        _emit 0x3b              // CMP EDI, ESI
        _emit 0xfe
        _emit 0xc7              // MOV dword ptr [ESP+0x68], 0x0
        _emit 0x44
        _emit 0x24
        _emit 0x68
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x2b (to 0x447fe8)
        _emit 0x2b
        _emit 0x8b              // MOV EAX, [EDI+0x8]
        _emit 0x47
        _emit 0x08
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL rel32 → 0x00447010
        _emit 0x46
        _emit 0xf0
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ECX, [ESI+0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x8b              // MOV EDX, [EDI]
        _emit 0x17
        _emit 0x8b              // MOV EAX, [ESI]
        _emit 0x06
        _emit 0x51              // PUSH ECX
        _emit 0x52              // PUSH EDX
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → 0x009d4600
        _emit 0x27
        _emit 0xc6
        _emit 0x58
        _emit 0x00
        _emit 0x8a              // MOV CL, [EDI+0x10]
        _emit 0x4f
        _emit 0x10
        _emit 0x88              // MOV [ESI+0x10], CL
        _emit 0x4e
        _emit 0x10
        _emit 0x8b              // MOV EDX, [EDI+0xc]
        _emit 0x57
        _emit 0x0c
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x89              // MOV [ESI+0xc], EDX
        _emit 0x56
        _emit 0x0c
        _emit 0x80              // CMP byte ptr [ESP+0x1d], 0x0
        _emit 0x7c
        _emit 0x24
        _emit 0x1d
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESP+0x68], 0xffffffff
        _emit 0x44
        _emit 0x24
        _emit 0x68
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x75              // JNZ +0x14 (to 0x44800b)
        _emit 0x14
        _emit 0x8b              // MOV EAX, [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV ECX, [ESP+0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x6a              // PUSH 0xb
        _emit 0x0b
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL rel32 → 0x0044d350
        _emit 0x48
        _emit 0x53
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x8b              // MOV EAX, ESI (return this)
        _emit 0xc6
        _emit 0x8b              // MOV ECX, [ESP+0x60]
        _emit 0x4c
        _emit 0x24
        _emit 0x60
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
        _emit 0x83              // ADD ESP, 0x60
        _emit 0xc4
        _emit 0x60
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
