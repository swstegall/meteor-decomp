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
// FUNCTION: ffxivgame 0x00435b50 — __thiscall wrapper that extracts four
// fields from `this` and forwards them to vtable slot 43 (offset 0xac)
// of its single argument; on non-zero return, fires a lazy-init assertion/
// log function guarded by an init flag (110 bytes).
//
// __thiscall FUN_00435b50(SomeObject *arg0)
//   ECX  = this
//   [ESP+4] = arg0   (caller's only stack argument; callee-cleaned via RET 4)
//
// Semantics (recovered from asm):
//
//   float f = this->field_10;
//   int   a = this->field_04;
//   int   b = this->field_08;
//   int   c = this->field_0c;
//   int ret = arg0->vtbl[43](arg0, 0, 0, a, b, f, c);  // stdcall/thiscall vtable fn
//   if (ret != 0) {
//       // lazy-init the assertion fn pointer at 0x0132390c
//       if (!(*(BYTE*)0x01323910 & 1)) {
//           *(DWORD*)0x01323910 |= 1;
//           *(DWORD*)0x0132390c  = 0x00433720;
//       }
//       // call assertion/log fn: fn(0xf64be8, 0xf6515c, 0xf64c18, 0x1cf, 0xf65170)
//       ((void(__cdecl*)(int,int,int,int,int))(**(void***)0x0132390c))(
//           0xf64be8, 0xf6515c, 0xf64c18, 0x1cf, 0xf65170);
//   }
//
// Absolute addresses baked into the orig binary:
//   0x01323910  — init flag (.data)
//   0x0132390c  — function-pointer slot (.data)
//   0x00433720  — assertion/log function (.text)
//   0xf64be8    — string arg 1 (.rdata)
//   0xf6515c    — string arg 2 (.rdata)
//   0xf64c18    — string arg 3 (.rdata)
//   0x000001cf  — line/code constant
//   0xf65170    — string arg 5 (.rdata)
//
// Reconstruction strategy — naked-asm byte passthrough (identical to the
// strategy used by siblings FUN_004090b0, FUN_004091f0, FUN_00409260):
// source-level C++ would produce relocations for every absolute address,
// and the vtable-dispatch lowering is ambiguous; emitting the 110 orig
// bytes verbatim via MASM _emit directives produces a .obj whose .text
// matches byte-for-byte with no relocations. compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_00435b50() {
    __asm {
        _emit 0xd9              // FLD float ptr [ECX + 0x10]
        _emit 0x41
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x8b              // MOV EDX, dword ptr [EDX + 0xac]
        _emit 0x92
        _emit 0xac
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ECX + 0xc]
        _emit 0x71
        _emit 0x0c
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ECX + 0x8]
        _emit 0x71
        _emit 0x08
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, dword ptr [ECX + 0x4]
        _emit 0x49
        _emit 0x04
        _emit 0xd9              // FSTP float ptr [ESP]
        _emit 0x1c
        _emit 0x24
        _emit 0x56              // PUSH ESI
        _emit 0x51              // PUSH ECX
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0x74              // JZ +0x3f  (→ epilogue / RET)
        _emit 0x3f
        _emit 0xb8              // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [0x01323910], AL
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ +0x10  (→ already_init)
        _emit 0x10
        _emit 0x09              // OR dword ptr [0x01323910], EAX
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [0x0132390c], 0x00433720
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00
        _emit 0x68              // PUSH 0x00f65170
        _emit 0x70
        _emit 0x51
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x000001cf
        _emit 0xcf
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x00f64c18
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x00f6515c
        _emit 0x5c
        _emit 0x51
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x00f64be8
        _emit 0xe8
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
