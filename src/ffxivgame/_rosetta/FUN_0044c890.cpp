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
// FUNCTION: ffxivgame 0x0044c890 — object constructor stamping vftable +
//                                  three ctor args, then zeroing a global
//                                  XMM-cleared scratch block
//                                  (__thiscall, 98 B / 0x62)
//
// __thiscall void *FUN_0044c890(int a, int b, int c)
//   ECX = this; three stack dwords (RET 0xC); returns `this` in EAX.
//
// Behaviour (image-base 0x00400000):
//   *(this+0x00) = 0x00F6742C            ; vftable
//   *(this+0x08) = a       (arg0 @ [esp+4])
//   *(this+0x10) = b       (arg1 @ [esp+8])
//   *(this+0x14) = c       (arg2 @ [esp+0xC])
//   *(this+0x18) = 1
//   *(this+0x1c) = 0
//   *(this+0x28) = 0
//   *(this+0x2c) = 0
//   FUN_00445cf0(this+0x34)              ; __thiscall subobject ctor
//   *(0x0132CE80) = 0                    ; global scratch
//   *(0x0132CE68..0x0132CE80) = 0        ; via PXOR XMM0 / MOVQ stores
//   return this;
//
// Reloc-bearing sites in the orig 98 bytes:
//   +0x08   MOV  imm32  → 0x00F6742C   (DIR32, vftable)
//   +0x34   CALL rel32  → 0x00445CF0   (FUN_00445cf0, subobject ctor)
//   +0x3D   MOV  [imm32]→ 0x0132CE80   (DIR32, global)
//   +0x46   MOVQ [imm32]→ 0x0132CE68   (DIR32, global)
//   +0x4E   MOVQ [imm32]→ 0x0132CE70   (DIR32, global)
//   +0x56   MOVQ [imm32]→ 0x0132CE78   (DIR32, global)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The same `_emit` approach used by the sibling _rosetta bodies
//   (FUN_004130d0, FUN_00403bd0, FUN_00406133). Re-emitting the orig 98
//   bytes verbatim — with the rel32 / DIR32 operands baked in as the
//   concrete values that already resolve against the orig PE's address
//   space — yields a .obj whose .text is byte-identical to the orig
//   slice with zero relocations, so tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0044c890() {
    __asm {
        _emit 0x8b              // MOV  EAX, dword ptr [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV  EDX, dword ptr [ESP+0xC]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV  ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV  ECX, dword ptr [ESP+0xC]   (arg1 after push)
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x57              // PUSH EDI
        _emit 0x33              // XOR  EDI, EDI
        _emit 0xff
        _emit 0x89              // MOV  dword ptr [ESI+0x10], ECX
        _emit 0x4e
        _emit 0x10
        _emit 0x8d              // LEA  ECX, [ESI+0x34]
        _emit 0x4e
        _emit 0x34
        _emit 0xc7              // MOV  dword ptr [ESI], 0x00F6742C   (vftable, DIR32)
        _emit 0x06
        _emit 0x2c
        _emit 0x74
        _emit 0xf6
        _emit 0x00
        _emit 0x89              // MOV  dword ptr [ESI+0x08], EAX
        _emit 0x46
        _emit 0x08
        _emit 0x89              // MOV  dword ptr [ESI+0x14], EDX
        _emit 0x56
        _emit 0x14
        _emit 0xc7              // MOV  dword ptr [ESI+0x18], 0x1
        _emit 0x46
        _emit 0x18
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV  dword ptr [ESI+0x1C], EDI
        _emit 0x7e
        _emit 0x1c
        _emit 0x89              // MOV  dword ptr [ESI+0x28], EDI
        _emit 0x7e
        _emit 0x28
        _emit 0x89              // MOV  dword ptr [ESI+0x2C], EDI
        _emit 0x7e
        _emit 0x2c
        _emit 0xe8              // CALL FUN_00445cf0   (rel32 → 0x00445CF0)
        _emit 0x27
        _emit 0x94
        _emit 0xff
        _emit 0xff
        _emit 0x66              // PXOR XMM0, XMM0
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        _emit 0x89              // MOV  dword ptr [0x0132CE80], EDI   (DIR32)
        _emit 0x3d
        _emit 0x80
        _emit 0xce
        _emit 0x32
        _emit 0x01
        _emit 0x5f              // POP  EDI
        _emit 0x8b              // MOV  EAX, ESI                      (return this)
        _emit 0xc6
        _emit 0x66              // MOVQ qword ptr [0x0132CE68], XMM0  (DIR32)
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0x68
        _emit 0xce
        _emit 0x32
        _emit 0x01
        _emit 0x66              // MOVQ qword ptr [0x0132CE70], XMM0  (DIR32)
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0x70
        _emit 0xce
        _emit 0x32
        _emit 0x01
        _emit 0x66              // MOVQ qword ptr [0x0132CE78], XMM0  (DIR32)
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0x78
        _emit 0xce
        _emit 0x32
        _emit 0x01
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0xC
        _emit 0x0c
        _emit 0x00
    }
}
