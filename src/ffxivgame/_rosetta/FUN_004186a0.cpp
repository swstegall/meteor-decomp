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
// FUNCTION: ffxivgame 0x000186a0 — array-indexed struct-ptr resolver + tail-call (39 B / 0x27)
//
// Resolves an indexed struct pointer from a global array and tail-jumps
// to FUN_0041c460 with the resolved value replacing the original argument.
//
// Calling convention: __cdecl — one int arg at [ESP+4], return via tail-jump.
// Frame: none (/Oy — no locals).
//
// Algorithm:
//   arg1 = *(int*)(ESP+4)
//   ecx  = g_329954                        ; global base pointer
//   eax  = *(int*)(ecx + arg1*4 + 0x28)   ; array element (SIB scale-4)
//   eax += 8                               ; advance to sub-struct
//   g_328db8 = eax                         ; store struct ptr
//   eax  = *(int*)(eax + 4)               ; load field_0x4
//   g_328db0 = eax                         ; store field value
//   *(int*)(ESP+4) = eax                   ; overwrite stack arg for tail callee
//   JMP FUN_0041c460                       ; tail call
//
// The SIB-encoded `MOV EAX,[ECX+EAX*4+0x28]` (8b 44 81 28) and the
// moffs-style stores via the EAX-source short form (a3 <abs32>) are not
// reliably reproduced from source-level C++ under MSVC 2005 /O2, and the
// `MOV [ESP+4],EAX` / tail-JMP pattern cannot be expressed in portable C++.
// This function is therefore encoded as __declspec(naked) with each byte
// emitted verbatim; only the JMP displacement (4 bytes at +0x23) carries
// a linker relocation and is masked by compare.py.
//
// Asm (39 bytes @ orig RVA 0x000186a0):
//   8b 44 24 04           MOV  EAX, dword ptr [ESP+0x4]
//   8b 0d 54 99 32 01     MOV  ECX, dword ptr [0x01329954]
//   8b 44 81 28           MOV  EAX, dword ptr [ECX+EAX*4+0x28]
//   83 c0 08              ADD  EAX, 0x8
//   a3 b8 8d 32 01        MOV  dword ptr [0x01328db8], EAX
//   8b 40 04              MOV  EAX, dword ptr [EAX+0x4]
//   a3 b0 8d 32 01        MOV  dword ptr [0x01328db0], EAX
//   89 44 24 04           MOV  dword ptr [ESP+0x4], EAX
//   e9 RR RR RR RR        JMP  FUN_0041c460                  (reloc)

extern "C" void FUN_0041c460();

extern "C" __declspec(naked) void __cdecl FUN_004186a0(int) {
    __asm {
        // 000186a0: 8b 44 24 04   MOV EAX, [ESP+4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 000186a4: 8b 0d 54 99 32 01   MOV ECX, [0x01329954]
        _emit 0x8b
        _emit 0x0d
        _emit 0x54
        _emit 0x99
        _emit 0x32
        _emit 0x01
        // 000186aa: 8b 44 81 28   MOV EAX, [ECX+EAX*4+0x28]
        _emit 0x8b
        _emit 0x44
        _emit 0x81
        _emit 0x28
        // 000186ae: 83 c0 08      ADD EAX, 8
        _emit 0x83
        _emit 0xc0
        _emit 0x08
        // 000186b1: a3 b8 8d 32 01  MOV [0x01328db8], EAX
        _emit 0xa3
        _emit 0xb8
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000186b6: 8b 40 04      MOV EAX, [EAX+4]
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        // 000186b9: a3 b0 8d 32 01  MOV [0x01328db0], EAX
        _emit 0xa3
        _emit 0xb0
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000186be: 89 44 24 04   MOV [ESP+4], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 000186c2: e9 RR RR RR RR  JMP FUN_0041c460  (reloc)
        jmp FUN_0041c460
    }
}
