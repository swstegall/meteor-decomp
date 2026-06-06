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
// FUNCTION: ffxivgame 0x00418220 — `__thiscall` struct initializer (31 B).
//
// Initialises a small object (ECX = this):
//   1. Zeros three consecutive dword fields at offsets 0x04, 0x08, 0x0c
//      using XOR EAX,EAX + three MOV [ESI+n],EAX stores.
//   2. Calls FUN_0041b950 (no args) → result stored at [this+0x00].
//   3. Calls FUN_0041b970 (no args) → result stored at [this+0x10].
//   Returns void (plain RET).
//
// Calling convention: __thiscall (ECX = this; no additional stack args).
// Frame: PUSH ESI / POP ESI bracket only (no alloca, no /GS cookie).
//
// Disassembly (verbatim from orig PE slice, RVA 0x00018220, 31 bytes):
//
//   00018220:  33 c0              xor     eax, eax
//   00018222:  56                 push    esi
//   00018223:  8b f1              mov     esi, ecx
//   00018225:  89 46 04           mov     [esi+0x04], eax
//   00018228:  89 46 08           mov     [esi+0x08], eax
//   0001822b:  89 46 0c           mov     [esi+0x0c], eax
//   0001822e:  e8 1d 37 00 00     call    FUN_0041b950   ; rel32 → 0x0001b950
//   00018233:  89 06              mov     [esi], eax
//   00018235:  e8 36 37 00 00     call    FUN_0041b970   ; rel32 → 0x0001b970
//   0001823a:  89 46 10           mov     [esi+0x10], eax
//   0001823d:  5e                 pop     esi
//   0001823e:  c3                 ret
//
// Reconstruction strategy:
//   Naked asm with named CALL targets. MASM encodes each CALL as a 5-byte
//   e8 rel32 instruction; compare.py masks the two 4-byte COFF reloc sites
//   so only the non-reloc bytes need to match — and they do one-for-one.

extern "C" void FUN_0041b950();
extern "C" void FUN_0041b970();

extern "C" __declspec(naked) void FUN_00418220() {
    __asm {
        xor     eax, eax                   ; 33 c0
        push    esi                        ; 56
        mov     esi, ecx                   ; 8b f1
        mov     dword ptr [esi+0x04], eax  ; 89 46 04
        mov     dword ptr [esi+0x08], eax  ; 89 46 08
        mov     dword ptr [esi+0x0c], eax  ; 89 46 0c
        call    FUN_0041b950               ; e8 [rel32]
        mov     dword ptr [esi], eax       ; 89 06
        call    FUN_0041b970               ; e8 [rel32]
        mov     dword ptr [esi+0x10], eax  ; 89 46 10
        pop     esi                        ; 5e
        ret                               ; c3
    }
}
