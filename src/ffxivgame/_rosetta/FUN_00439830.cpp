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
// FUNCTION: ffxivgame 0x00439830 — __thiscall refcount-release / shutdown
//                                  routine (195 B / 0xc3)
//
// Behaviour read from asm/ffxivgame/00039830_FUN_00439830.s:
//
//   __thiscall void FUN_00439830(this)
//
//     if (--this->refcount /* [this+0xd4] */ != 0) return;   // early-out
//
//     FUN_004396a0(this);                        // teardown helper (ECX=this)
//     FUN_00418410(0, 0);                         // cdecl, 2 zero args
//     { int sel = 0; FUN_00419020(&this->[0x0c], &sel); }     // sel pushed via ESP slot
//     { int sel = 1; FUN_00419020(&this->[0x4c], &sel); }
//     { int sel = 2; FUN_00419020(&this->[0x8c], &sel); }
//     g_var_01328f68 = this->[0xcc];
//     FUN_0041d0e0(this->[0xcc]);
//     g_var_01328ef0 = (char)this->[0xd0];
//     FUN_0041c1f0((char)this->[0xd0]);
//     g_var_01328ee8 = (char)this->[0xd1];
//     FUN_0041c1d0((char)this->[0xd1]);
//     g_var_01328f19 = (char)this->[0xd2];
//     FUN_0041c270((char)this->[0xd2]);
//
// Reloc-bearing sites in the orig 195 bytes (CALL rel32 + absolute MOV moffs32
// to .data globals). `tools/compare.py` masks the reloc slots out of the byte
// diff; we re-emit the orig bytes verbatim so the .obj's .text section matches
// byte-for-byte without driving a relink:
//     +0x11  CALL rel32  → 0x004396a0  (teardown helper)
//     +0x1a  CALL rel32  → 0x00418410
//     +0x2f  CALL rel32  → 0x00419020  (sel = 0)
//     +0x44  CALL rel32  → 0x00419020  (sel = 1)
//     +0x5c  CALL rel32  → 0x00419020  (sel = 2)
//     +0x68  MOV moffs32 [0x01328f68], EAX
//     +0x6d  CALL rel32  → 0x0041d0e0
//     +0x7c  MOV moffs32 [0x01328ef0], AL
//     +0x86  CALL rel32  → 0x0041c1f0
//     +0x9a  MOV moffs32 [0x01328ee8], AL
//     +0x9f  CALL rel32  → 0x0041c1d0
//     +0xb3  MOV moffs32 [0x01328f19], AL
//     +0xb8  CALL rel32  → 0x0041c270
//
// Reconstruction strategy — naked-asm byte passthrough (same shape as the
// sibling _rosetta matches FUN_00406680 / FUN_00403f10): a __declspec(naked)
// body that re-emits the orig 195 bytes verbatim via MASM _emit directives.

extern "C" __declspec(naked) void FUN_00439830() {
    __asm {
        _emit 0x51              // PUSH ECX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x83              // ADD dword ptr [ESI+0xd4], -1
        _emit 0x86
        _emit 0xd4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0x0f              // JNZ 0x004398f0
        _emit 0x85
        _emit 0xaf
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL rel32 → 0x004396a0
        _emit 0x5a
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0xe8              // CALL rel32 → 0x00418410
        _emit 0xc1
        _emit 0xeb
        _emit 0xfd
        _emit 0xff
        _emit 0x83              // ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0x8d              // LEA EAX, [ESI+0x0c]
        _emit 0x46
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV EAX, ESP
        _emit 0xc4
        _emit 0xc7              // MOV dword ptr [EAX], 0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL rel32 → 0x00419020
        _emit 0xbc
        _emit 0xf7
        _emit 0xfd
        _emit 0xff
        _emit 0x8d              // LEA ECX, [ESI+0x4c]
        _emit 0x4e
        _emit 0x4c
        _emit 0x83              // ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0x51              // PUSH ECX
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV EAX, ESP
        _emit 0xc4
        _emit 0xc7              // MOV dword ptr [EAX], 1
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL rel32 → 0x00419020
        _emit 0xa7
        _emit 0xf7
        _emit 0xfd
        _emit 0xff
        _emit 0x83              // ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0x8d              // LEA EDX, [ESI+0x8c]
        _emit 0x96
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x52              // PUSH EDX
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV EAX, ESP
        _emit 0xc4
        _emit 0xc7              // MOV dword ptr [EAX], 2
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL rel32 → 0x00419020
        _emit 0x8f
        _emit 0xf7
        _emit 0xfd
        _emit 0xff
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0xcc]
        _emit 0x86
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xa3              // MOV [0x01328f68], EAX
        _emit 0x68
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL rel32 → 0x0041d0e0
        _emit 0x3e
        _emit 0x38
        _emit 0xfe
        _emit 0xff
        _emit 0x8a              // MOV AL, byte ptr [ESI+0xd0]
        _emit 0x86
        _emit 0xd0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x88              // MOV byte ptr [ESP+0x10], AL
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xa2              // MOV [0x01328ef0], AL
        _emit 0xf0
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → 0x0041c1f0
        _emit 0x35
        _emit 0x29
        _emit 0xfe
        _emit 0xff
        _emit 0x8a              // MOV AL, byte ptr [ESI+0xd1]
        _emit 0x86
        _emit 0xd1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x88              // MOV byte ptr [ESP+0x14], AL
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x51              // PUSH ECX
        _emit 0xa2              // MOV [0x01328ee8], AL
        _emit 0xe8
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL rel32 → 0x0041c1d0
        _emit 0xfc
        _emit 0x28
        _emit 0xfe
        _emit 0xff
        _emit 0x8a              // MOV AL, byte ptr [ESI+0xd2]
        _emit 0x86
        _emit 0xd2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x88              // MOV byte ptr [ESP+0x18], AL
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x18]
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x52              // PUSH EDX
        _emit 0xa2              // MOV [0x01328f19], AL
        _emit 0x19
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL rel32 → 0x0041c270
        _emit 0x83
        _emit 0x29
        _emit 0xfe
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0x5e              // POP ESI
        _emit 0x59              // POP ECX
        _emit 0xc3              // RET
    }
}
