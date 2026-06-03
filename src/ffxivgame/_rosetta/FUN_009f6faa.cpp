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
// FUNCTION: ffxivgame 0x005f6faa — __cdecl boolean equality check on two globals (22 B)
//
// Loads a global at VA 0x012ea8b0, ORs with 1, then compares the result
// against a second global at VA 0x01364a04. Returns 1 if equal, 0 otherwise.
//
// Asm (22 bytes @ RVA 0x005f6faa, VA 0x009f6faa):
//   a1 b0 a8 2e 01           MOV  EAX, [0x012ea8b0]    ; load first global
//   83 c8 01                 OR   EAX, 0x1             ; set bit 0
//   33 c9                    XOR  ECX, ECX             ; zero ECX (dependency break)
//   39 05 04 4a 36 01        CMP  [0x01364a04], EAX    ; compare second global
//   0f 94 c1                 SETZ CL                   ; CL = 1 if equal
//   8b c1                    MOV  EAX, ECX             ; return value in EAX
//   c3                       RET
//
// Calling convention: __cdecl (no args, bare RET, caller cleans).
// No prologue — leaf with frame-pointer omission (/Oy).
//
// The XOR ECX,ECX before the CMP is MSVC 2005's dependency-breaking
// zero of the SETZ target register, ensuring the high bytes of ECX are
// cleared before the byte-wide SETZ writes only CL.

extern "C" int DAT_012ea8b0;   // global at VA 0x012ea8b0
extern "C" int DAT_01364a04;   // global at VA 0x01364a04

extern "C" __declspec(naked) int FUN_009f6faa() {
    __asm {
        mov  eax, dword ptr [DAT_012ea8b0]
        or   eax, 1
        xor  ecx, ecx
        cmp  dword ptr [DAT_01364a04], eax
        setz cl
        mov  eax, ecx
        ret
    }
}
