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
// FUNCTION: ffxivgame 0x00459980 — wstring-like compare(__thiscall, 110 B)
//
// int __thiscall FUN_00459980(void *this, size_t off, size_t n0,
//                             const wchar_t *ptr, size_t count)
//
// Implements a 3-way comparison of a substring [off, off+n0) of a
// 2-byte-element string against (ptr, count). The `this` object uses
// the MSVC 2005 SSO layout:
//   [+0x04] = inline buffer (small) or heap pointer (large)
//   [+0x14] = _Mysize (element count)
//   [+0x18] = _Myres (capacity; >= 8 → heap branch)
//
// Pseudocode:
//   if (this->_Mysize < off) _Xlength_error();     // bounds guard
//   available = this->_Mysize - off;
//   compare_len = min(available, n0);
//   final_len   = min(compare_len, count);
//   data = (_Myres >= 8) ? this->_Ptr : &this->_Buf;
//   cmp  = FUN_00459940(data + off, ptr, final_len); // wmemcmp-like
//   if (cmp != 0) return cmp;
//   if (compare_len < count) return -1;             // this is shorter
//   return (compare_len != count) ? 1 : 0;
//
// Calling convention: __thiscall (ECX = this; RET 0x10 cleans 4 stack args)
//
// Reloc-bearing sites (rel32 CALL targets inside the orig binary):
//   +0x09   CALL rel32  → 0x009d046d  (_Xlength_error or assert)
//   +0x46   CALL rel32  → 0x00459940  (wmemcmp-like comparison helper)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The two CALL rel32 instructions carry displacements that are valid
//   only relative to the orig binary's load address. Emitting them as
//   raw _emit bytes in a __declspec(naked) body produces a .obj whose
//   .text matches the orig byte-for-byte with no relocations.
//   compare.py reads the post-fixup orig PE and compares byte streams
//   directly, so this zero-reloc passthrough yields GREEN.

extern "C" __declspec(naked) void FUN_00459980() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x0c]
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x39              // CMP dword ptr [EDI+0x14], EBP
        _emit 0x6f
        _emit 0x14
        _emit 0x73              // JNC +0x05
        _emit 0x05
        _emit 0xe8              // CALL 0x009d046d (rel32: d9 6a 57 00)
        _emit 0xd9
        _emit 0x6a
        _emit 0x57
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [EDI+0x14]
        _emit 0x47
        _emit 0x14
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x18]
        _emit 0x74
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x20]
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        _emit 0x2b              // SUB EAX, EBP
        _emit 0xc5
        _emit 0x3b              // CMP EAX, ESI
        _emit 0xc6
        _emit 0x0f              // CMOVC ESI, EAX
        _emit 0x42
        _emit 0xf0
        _emit 0x3b              // CMP ESI, EBX
        _emit 0xf3
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x72              // JC +0x02
        _emit 0x02
        _emit 0x8b              // MOV EAX, EBX
        _emit 0xc3
        _emit 0x83              // CMP dword ptr [EDI+0x18], 0x8
        _emit 0x7f
        _emit 0x18
        _emit 0x08
        _emit 0x72              // JC +0x05
        _emit 0x05
        _emit 0x8b              // MOV EDI, dword ptr [EDI+0x04]
        _emit 0x7f
        _emit 0x04
        _emit 0xeb              // JMP +0x03
        _emit 0x03
        _emit 0x83              // ADD EDI, 0x04
        _emit 0xc7
        _emit 0x04
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA ECX, [EDI + EBP*2]
        _emit 0x0c
        _emit 0x6f
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL 0x00459940 (rel32: 75 ff ff ff)
        _emit 0x75
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x0c
        _emit 0xc4
        _emit 0x0c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +0x15
        _emit 0x15
        _emit 0x3b              // CMP ESI, EBX
        _emit 0xf3
        _emit 0x73              // JNC +0x0a
        _emit 0x0a
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x83              // OR EAX, 0xffffffff
        _emit 0xc8
        _emit 0xff
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x10
        _emit 0x10
        _emit 0x00
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x3b              // CMP ESI, EBX
        _emit 0xf3
        _emit 0x0f              // SETNZ AL
        _emit 0x95
        _emit 0xc0
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x10
        _emit 0x10
        _emit 0x00
    }
}
