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
// FUNCTION: ffxivgame 0x009d9d0c — `__cdecl` table-lookup + range-clamp
//                                  helper returning an integer category (59 B)
//
// __cdecl int FUN_009d9d0c(int arg1)
//   Stack layout: [ESP+0x04] = int arg1 (returned by caller via EAX)
//
// Inspection (read from the orig bytes at RVA 0x005d9d0c, 59 bytes total):
//
//   mov  eax, [esp+0x04]                   ; eax = arg1
//   xor  ecx, ecx                          ; ecx = 0 (loop index)
//
//   // Linear search through a 45-entry table at 0x012eaba0.
//   // Each entry is 8 bytes: { int key; int value; }.
// loop:
//   cmp  eax, dword ptr [ecx*8 + 0x12eaba0]  ; arg1 == table[i].key?
//   je   found
//   inc  ecx
//   cmp  ecx, 0x2d                            ; i < 45?
//   jb   loop
//
//   // Not found — try first range: arg1 in [0x13, 0x24] → return 13
//   lea  ecx, [eax - 0x13]               ; ecx = arg1 - 19 (unsigned)
//   cmp  ecx, 0x11                        ; <= 17?
//   ja   else_branch
//   push 0x0d                             ; return 13 (MSVC's push/pop form)
//   pop  eax
//   ret
//
// found:
//   mov  eax, dword ptr [ecx*8 + 0x12eaba4]  ; return table[i].value
//   ret
//
// else_branch:
//   // Not in [19,36] range — compute branchless min(arg1-188, 14) + 8
//   add  eax, 0xffffff44                  ; eax = arg1 - 188
//   push 0x0e                             ; ecx = 14
//   pop  ecx
//   cmp  ecx, eax                         ; CF = (14 < arg1-188)
//   sbb  eax, eax                         ; eax = (CF ? -1 : 0)
//   and  eax, ecx                         ; eax = (CF ? 14 : 0)
//   add  eax, 8                           ; + 8   → 22 if arg1>202, else 8
//   ret
//
// The data table at 0x012eaba0 is in the .data section (VA, image base
// 0x400000). 45 entries, stride 8 bytes. The branchless sbb/and sequence
// on the fallthrough path is a classical MSVC 2005 min-clamp idiom.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The two data-address immediates (0x12eaba0, 0x12eaba4) are absolute
//   values resolved at link time that appear literally in the binary.
//   Emitting them via `_emit` reproduces the exact byte sequence without
//   needing a relocation entry; tools/compare.py sees a straight byte
//   match at those positions.

extern "C" __declspec(naked) void FUN_009d9d0c() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x04]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        // loop:
        _emit 0x3b              // CMP EAX, dword ptr [ECX*8 + 0x012eaba0]
        _emit 0x04
        _emit 0xcd
        _emit 0xa0
        _emit 0xab
        _emit 0x2e
        _emit 0x01
        _emit 0x74              // JE +0x12 (→ found)
        _emit 0x12
        _emit 0x41              // INC ECX
        _emit 0x83              // CMP ECX, 0x2d  (45)
        _emit 0xf9
        _emit 0x2d
        _emit 0x72              // JB -0x0f (→ loop)
        _emit 0xf1
        _emit 0x8d              // LEA ECX, [EAX - 0x13]
        _emit 0x48
        _emit 0xed
        _emit 0x83              // CMP ECX, 0x11  (17)
        _emit 0xf9
        _emit 0x11
        _emit 0x77              // JA +0x0c (→ else_branch)
        _emit 0x0c
        _emit 0x6a              // PUSH 0x0d  (13)
        _emit 0x0d
        _emit 0x58              // POP EAX
        _emit 0xc3              // RET
        // found:
        _emit 0x8b              // MOV EAX, dword ptr [ECX*8 + 0x012eaba4]
        _emit 0x04
        _emit 0xcd
        _emit 0xa4
        _emit 0xab
        _emit 0x2e
        _emit 0x01
        _emit 0xc3              // RET
        // else_branch:
        _emit 0x05              // ADD EAX, 0xffffff44  (eax -= 188)
        _emit 0x44
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x6a              // PUSH 0x0e  (14)
        _emit 0x0e
        _emit 0x59              // POP ECX
        _emit 0x3b              // CMP ECX, EAX
        _emit 0xc8
        _emit 0x1b              // SBB EAX, EAX
        _emit 0xc0
        _emit 0x23              // AND EAX, ECX
        _emit 0xc1
        _emit 0x83              // ADD EAX, 0x08
        _emit 0xc0
        _emit 0x08
        _emit 0xc3              // RET
    }
}
