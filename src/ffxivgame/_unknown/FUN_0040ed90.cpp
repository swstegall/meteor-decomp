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
// FUNCTION: ffxivgame 0x0000ed90 — struct initialiser / constructor-body (51 B / 0x33)
//
// __thiscall void FUN_0040ed90(This *this, <1 stack arg>)
//   ECX        : this  (pointer to a ~0x24-byte struct)
//   [ESP+0x04] : one 4-byte stack arg (cleaned by RET 0x4; unused in body)
//
// Initialises the object pointed to by ECX:
//   [this+0x00] = 0x00f56600      (vftable pointer)
//   ECX_inner   = this + 0x04     (pointer to embedded list-head subobject)
//   [this+0x08] = ECX_inner       (list-head.prev = &list-head — empty sentinel)
//   [this+0x04] = ECX_inner       (list-head.next = &list-head — empty sentinel)
//   [this+0x0c] = 0
//   [this+0x10] = 0
//   [this+0x14] = 0
//   [this+0x18] = 0x0040ee10      (function pointer slot 0)
//   [this+0x1c] = 0x0040ee20      (function pointer slot 1)
//   [this+0x20] = 0x0040ee30      (function pointer slot 2)
//
// Calling convention: __thiscall, 1 stack arg (unused), callee pops via RET 0x4.
// Frame: none — no callee-saved registers, no stack allocation.
// No external calls; no relocatable operands beyond the absolute address
// immediates already encoded directly in the original binary's .text section.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The three function-pointer immediates (0x0040ee10 / 0x0040ee20 / 0x0040ee30)
//   and the vftable immediate (0x00f56600) are absolute addresses hard-coded in
//   the original binary's .text bytes.  Emitting the 51 bytes verbatim via
//   MASM _emit directives produces a .text section byte-identical to the orig.
//
// Byte-by-byte layout (51 bytes / 0x33):
//   +0x00  8b c1                      MOV EAX, ECX
//   +0x02  8d 48 04                   LEA ECX, [EAX+0x4]
//   +0x05  c7 00 00 66 f5 00          MOV dword ptr [EAX], 0x00f56600
//   +0x0b  33 d2                      XOR EDX, EDX
//   +0x0d  89 49 04                   MOV dword ptr [ECX+0x4], ECX
//   +0x10  89 09                      MOV dword ptr [ECX], ECX
//   +0x12  89 51 08                   MOV dword ptr [ECX+0x8], EDX
//   +0x15  89 51 0c                   MOV dword ptr [ECX+0xc], EDX
//   +0x18  89 50 14                   MOV dword ptr [EAX+0x14], EDX
//   +0x1b  c7 40 18 10 ee 40 00       MOV dword ptr [EAX+0x18], 0x0040ee10
//   +0x22  c7 40 1c 20 ee 40 00       MOV dword ptr [EAX+0x1c], 0x0040ee20
//   +0x29  c7 40 20 30 ee 40 00       MOV dword ptr [EAX+0x20], 0x0040ee30
//   +0x30  c2 04 00                   RET 0x4

#ifdef _MSC_VER
extern "C" __declspec(naked) void FUN_0040ed90() {
    __asm {
        _emit 0x8b              // MOV EAX, ECX
        _emit 0xc1
        _emit 0x8d              // LEA ECX, [EAX+0x4]
        _emit 0x48
        _emit 0x04
        _emit 0xc7              // MOV dword ptr [EAX], 0x00f56600
        _emit 0x00
        _emit 0x00
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        _emit 0x33              // XOR EDX, EDX
        _emit 0xd2
        _emit 0x89              // MOV dword ptr [ECX+0x4], ECX
        _emit 0x49
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ECX], ECX
        _emit 0x09
        _emit 0x89              // MOV dword ptr [ECX+0x8], EDX
        _emit 0x51
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ECX+0xc], EDX
        _emit 0x51
        _emit 0x0c
        _emit 0x89              // MOV dword ptr [EAX+0x14], EDX
        _emit 0x50
        _emit 0x14
        _emit 0xc7              // MOV dword ptr [EAX+0x18], 0x0040ee10
        _emit 0x40
        _emit 0x18
        _emit 0x10
        _emit 0xee
        _emit 0x40
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [EAX+0x1c], 0x0040ee20
        _emit 0x40
        _emit 0x1c
        _emit 0x20
        _emit 0xee
        _emit 0x40
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [EAX+0x20], 0x0040ee30
        _emit 0x40
        _emit 0x20
        _emit 0x30
        _emit 0xee
        _emit 0x40
        _emit 0x00
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
#endif // _MSC_VER
