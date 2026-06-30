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
// FUNCTION: ffxivgame 0x0005e9c0 — asn1_check_eoc (34 B)
//
// Checks whether the buffer pointed to by *ECX starts with an ASN.1
// End-of-Contents (EOC) marker (two zero bytes). If len (first stack arg)
// is at least 2 and the first two bytes at *ECX are both 0x00, advances
// *ECX past them and returns 1; otherwise returns 0.
//
// Calling convention: ECX = const unsigned char ** (read and written),
// [ESP+4] = long len; caller cleans stack (bare RET, not RET 4).
// This hybrid convention does not map to a named ABI — naked + inline
// asm is the only way to reproduce the exact encoding.
//
// Asm (34 bytes @ orig RVA 0x0005e9c0):
//   83 7c 24 04 02    CMP dword ptr [ESP+0x4],0x2
//   7c 18             JL  short fail
//   8b 01             MOV EAX,dword ptr [ECX]
//   80 38 00          CMP byte ptr [EAX],0x0
//   75 11             JNZ short fail
//   80 78 01 00       CMP byte ptr [EAX+0x1],0x0
//   75 0b             JNZ short fail
//   83 c0 02          ADD EAX,0x2
//   89 01             MOV dword ptr [ECX],EAX
//   b8 01 00 00 00    MOV EAX,0x1
//   c3                RET
//   33 c0             XOR EAX,EAX      ; fail:
//   c3                RET

extern "C" __declspec(naked) void FUN_0045e9c0()
{
    __asm {
        cmp dword ptr [esp + 0x4], 0x2
        jl  short fail
        mov eax, dword ptr [ecx]
        cmp byte ptr [eax], 0x0
        jnz short fail
        cmp byte ptr [eax + 0x1], 0x0
        jnz short fail
        add eax, 0x2
        mov dword ptr [ecx], eax
        mov eax, 0x1
        ret
    fail:
        xor eax, eax
        ret
    }
}
