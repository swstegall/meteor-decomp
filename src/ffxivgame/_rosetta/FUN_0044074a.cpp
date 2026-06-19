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
// FUNCTION: ffxivgame 0x0044074a — wstring-assign body fragment (110 B).
//
// Fragment of a std::basic_string<wchar_t> assign method body.  The prologue
// (SEH-frame installation, PUSH EBX/ESI/EDI, SUB ESP) lives before RVA 0x4074a.
// This 110-byte window begins with the first statement and ends with RET 0x8.
//
// Logic (recovered from asm):
//   EBX  = count ([EBP+0xC])
//   EDI  = this  (string object)
//   ESI  = new capacity
//   [EBP+0x8] = source data pointer
//
//   if (count > 0) {
//       char *buf = ([EDI+0x18] >= 8) ? [EDI+0x4] : &[EDI+0x4]; // SSO
//       FUN_009d17f3(src, buf, ESI*2+2, EBX*2);  // __cdecl copy helper
//   }
//   if ([EDI+0x18] >= 8)
//       FUN_009d1b17([EDI+0x4]);                  // free old heap buffer
//   // store new fields
//   [EDI+0x4 as word] = 0;  [EDI+0x4] = src;
//   [EDI+0x18] = ESI;  [EDI+0x14] = EBX;
//   if (ESI >= 8) EAX = src;                     // else EAX = &[EDI+0x4]
//   [EAX + EBX*2] as word = 0;                   // null-terminate (wchar_t)
//   // epilogue: restore FS:[0], pop regs, RET 0x8
//
// Both CALL rel32 targets are hard-coded binary VAs; emitting them verbatim
// via _emit produces a .text that matches the orig byte-for-byte.
// Reloc-bearing sites:
//   +0x23   CALL rel32 → 0x009d17f3  (wchar copy helper)
//   +0x35   CALL rel32 → 0x009d1b17  (heap-free helper)

extern "C" __declspec(naked) void FUN_0044074a() {
    __asm {
        _emit 0x8b  // MOV EBX, dword ptr [EBP+0xC]
        _emit 0x5d
        _emit 0x0c
        _emit 0x85  // TEST EBX, EBX
        _emit 0xdb
        _emit 0x76  // JBE +0x24
        _emit 0x24
        _emit 0x83  // CMP dword ptr [EDI+0x18], 0x8
        _emit 0x7f
        _emit 0x18
        _emit 0x08
        _emit 0x72  // JC +0x5
        _emit 0x05
        _emit 0x8b  // MOV EAX, dword ptr [EDI+0x4]
        _emit 0x47
        _emit 0x04
        _emit 0xeb  // JMP +0x3
        _emit 0x03
        _emit 0x8d  // LEA EAX, [EDI+0x4]
        _emit 0x47
        _emit 0x04
        _emit 0x8b  // MOV ECX, dword ptr [EBP+0x8]
        _emit 0x4d
        _emit 0x08
        _emit 0x8d  // LEA EDX, [EBX+EBX*1]
        _emit 0x14
        _emit 0x1b
        _emit 0x52  // PUSH EDX
        _emit 0x50  // PUSH EAX
        _emit 0x8d  // LEA EAX, [ESI+ESI*1+0x2]
        _emit 0x44
        _emit 0x36
        _emit 0x02
        _emit 0x50  // PUSH EAX
        _emit 0x51  // PUSH ECX
        _emit 0xe8  // CALL 0x009d17f3
        _emit 0x81
        _emit 0x10
        _emit 0x59
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x83  // CMP dword ptr [EDI+0x18], 0x8
        _emit 0x7f
        _emit 0x18
        _emit 0x08
        _emit 0x72  // JC +0xC
        _emit 0x0c
        _emit 0x8b  // MOV EDX, dword ptr [EDI+0x4]
        _emit 0x57
        _emit 0x04
        _emit 0x52  // PUSH EDX
        _emit 0xe8  // CALL 0x009d1b17
        _emit 0x93
        _emit 0x13
        _emit 0x59
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x4  (caller-clean for PUSH EDX)
        _emit 0xc4
        _emit 0x04
        _emit 0x83  // CMP ESI, 0x8
        _emit 0xfe
        _emit 0x08
        _emit 0x8b  // MOV ECX, dword ptr [EBP+0x8]
        _emit 0x4d
        _emit 0x08
        _emit 0x8d  // LEA EAX, [EDI+0x4]
        _emit 0x47
        _emit 0x04
        _emit 0x66  // MOV word ptr [EAX], 0x0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89  // MOV dword ptr [EAX], ECX
        _emit 0x08
        _emit 0x89  // MOV dword ptr [EDI+0x18], ESI
        _emit 0x77
        _emit 0x18
        _emit 0x89  // MOV dword ptr [EDI+0x14], EBX
        _emit 0x5f
        _emit 0x14
        _emit 0x72  // JC +0x2
        _emit 0x02
        _emit 0x8b  // MOV EAX, ECX
        _emit 0xc1
        _emit 0x66  // MOV word ptr [EAX+EBX*2], 0x0
        _emit 0xc7
        _emit 0x04
        _emit 0x58
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV ECX, dword ptr [EBP-0xC]
        _emit 0x4d
        _emit 0xf4
        _emit 0x64  // MOV dword ptr FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5b  // POP EBX
        _emit 0x8b  // MOV ESP, EBP
        _emit 0xe5
        _emit 0x5d  // POP EBP
        _emit 0xc2  // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
