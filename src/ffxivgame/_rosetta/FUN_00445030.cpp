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
// FUNCTION: ffxivgame 0x00045030 — `__thiscall` method (32 B).
//
// Saves `this` (ECX) into ESI, calls a base-class or initialisation method
// at 0x00456250 (still with the original ECX = this), then asserts that the
// field at [this+0x4] is non-zero; if zero it calls an error/assert handler
// at 0x00456060 passing the encoded line/id value 0x29d5 as a single cdecl
// arg.  Finally returns the dword at [this+0x8].
//
// Asm (32 bytes, RVA 0x00045030..0x00045050):
//
//   00045030:  56                 PUSH ESI
//   00045031:  8b f1              MOV ESI,ECX
//   00045033:  e8 18 12 01 00     CALL 0x00456250       ; rel32 reloc
//   00045038:  83 7e 04 00        CMP dword ptr [ESI+0x4],0x0
//   0004503c:  75 0d              JNZ +0x0d             ; → 0x0044504b
//   0004503e:  68 d5 29 00 00     PUSH 0x29d5
//   00045043:  e8 18 10 01 00     CALL 0x00456060       ; rel32 reloc
//   00045048:  83 c4 04           ADD ESP,0x4
//   0004504b:  8b 46 08           MOV EAX,dword ptr [ESI+0x8]
//   0004504e:  5e                 POP ESI
//   0004504f:  c3                 RET
//
// Calling convention: __thiscall (ECX = this).
// Frame: PUSH ESI / POP ESI only — no local storage allocated.
// Return: dword from [this+0x8].
//
// Reconstruction: __declspec(naked) mnemonic asm. The two CALL operands are
// rel32 relocations; compare.py masks them so the non-reloc bytes match
// byte-for-byte.

extern "C" void FUN_00456250();
extern "C" void FUN_00456060(unsigned int);

extern "C" __declspec(naked) void FUN_00445030() {
    __asm {
        push    esi
        mov     esi, ecx
        call    FUN_00456250
        cmp     dword ptr [esi + 0x4], 0x0
        jnz     done
        push    0x29d5
        call    FUN_00456060
        add     esp, 0x4
    done:
        mov     eax, dword ptr [esi + 0x8]
        pop     esi
        ret
    }
}
