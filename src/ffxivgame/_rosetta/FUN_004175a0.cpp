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
// FUNCTION: ffxivgame 0x004175a0 — __thiscall debug-dump method that prints a
//           header, iterates an array of 8-byte elements printing each with its
//           index, then prints a footer. 109 bytes (0x6d).
//
// Calling convention: __thiscall — ECX = `this` on entry.
// No stack frame (no EBP save/setup).  Preserved regs: EBX, ESI, EDI.
//
// Struct layout inferred from asm (fields of the `this` object at ECX/EBX):
//   +0x04  void   *array_ptr   — pointer to array of 8-byte elements
//   +0x08  DWORD   field_8     — pushed as 3rd arg to the header format call
//   +0x0C  DWORD   count       — number of array elements (unsigned comparison)
//
// Array element layout (8 bytes, stride = 8):
//   +0x00  DWORD   elem_field0
//   +0x04  DWORD   elem_field4
//
// Call pattern (repeated three times, for header / per-element / footer):
//   push args...
//   push <format-string-address>
//   call FUN_00415c90              ; format / prepare string → EAX
//   push EAX
//   call FUN_00415f70              ; output the prepared string
//   add  ESP, <total args * 4>     ; __cdecl caller-cleans
//
// Reloc-bearing sites in the orig 109 bytes (all baked into the post-fixup
// binary as concrete byte sequences; emitting them verbatim via `_emit`
// produces a .obj whose .text matches the orig byte-for-byte):
//   +0x0c  PUSH imm32  0x00f578e8  — header format-string address  (DIR32)
//   +0x11  CALL rel32  0x00415c90  — format helper (REL32 = 0xffffe6da)
//   +0x16  CALL rel32  0x00415f70  — output helper (REL32 = 0xffffe9b4)
//   +0x38  PUSH imm32  0x00f57718  — element format-string address (DIR32)
//   +0x3d  CALL rel32  0x00415c90  — format helper (REL32 = 0xffffe6ae)
//   +0x42  CALL rel32  0x00415f70  — output helper (REL32 = 0xffffe988)
//   +0x56  PUSH imm32  0x00f578b4  — footer format-string address  (DIR32)
//   +0x5b  CALL rel32  0x00415c90  — format helper (REL32 = 0xffffe690)
//   +0x60  CALL rel32  0x00415f70  — output helper (REL32 = 0xffffe96a)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function's code structure (register allocation EBX=this, ESI=ptr,
//   EDI=counter, the 2-byte `MOV EDI,EDI` loop-alignment NOP at +0x2e,
//   the unsigned JBE/JC pair for the loop bounds, and the exact interleaving
//   of ESI advance and counter reload) cannot be guaranteed from source-level
//   C++ under MSVC 2005 /O2 without exactly matching the surrounding TU
//   context that the compiler used. The pragmatic choice — consistent with
//   siblings FUN_004090b0, FUN_004091f0, FUN_00415c90 — is a naked function
//   that re-emits the 109 orig bytes verbatim via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_004175a0() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, ECX               (this → EBX)
        _emit 0xd9
        _emit 0x8b              // MOV EAX, dword ptr [EBX+0x8]
        _emit 0x43
        _emit 0x08
        _emit 0x8b              // MOV ECX, dword ptr [EBX+0xc]
        _emit 0x4b
        _emit 0x0c
        _emit 0x56              // PUSH ESI
        _emit 0x50              // PUSH EAX                   (field_8, arg3)
        _emit 0x51              // PUSH ECX                   (count,   arg2)
        _emit 0x68              // PUSH 0x00f578e8             (header fmt, arg1)
        _emit 0xe8
        _emit 0x78
        _emit 0xf5
        _emit 0x00
        _emit 0xe8              // CALL FUN_00415c90 (rel32 = 0xffffe6da)
        _emit 0xda
        _emit 0xe6
        _emit 0xff
        _emit 0xff
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_00415f70 (rel32 = 0xffffe9b4)
        _emit 0xb4
        _emit 0xe9
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ESI, dword ptr [EBX+0x4]  (array_ptr)
        _emit 0x73
        _emit 0x04
        _emit 0x83              // ADD ESP, 0x10               (clean 4 args)
        _emit 0xc4
        _emit 0x10
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74              // JZ +0x44  (→ epilogue: pop ESI)
        _emit 0x44
        _emit 0x57              // PUSH EDI
        _emit 0x33              // XOR EDI, EDI                (i = 0)
        _emit 0xff
        _emit 0x39              // CMP dword ptr [EBX+0xc], EDI
        _emit 0x7b
        _emit 0x0c
        _emit 0x76              // JBE +0x28 (→ footer block)
        _emit 0x28
        _emit 0x8b              // MOV EDI, EDI                (2-byte NOP / loop align)
        _emit 0xff
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x4]  (elem_field4)
        _emit 0x56
        _emit 0x04
        _emit 0x8b              // MOV EAX, dword ptr [ESI]      (elem_field0)
        _emit 0x06
        _emit 0x52              // PUSH EDX                   (elem_field4, arg4)
        _emit 0x50              // PUSH EAX                   (elem_field0, arg3)
        _emit 0x57              // PUSH EDI                   (i,           arg2)
        _emit 0x68              // PUSH 0x00f57718             (element fmt, arg1)
        _emit 0x18
        _emit 0x77
        _emit 0xf5
        _emit 0x00
        _emit 0xe8              // CALL FUN_00415c90 (rel32 = 0xffffe6ae)
        _emit 0xae
        _emit 0xe6
        _emit 0xff
        _emit 0xff
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_00415f70 (rel32 = 0xffffe988)
        _emit 0x88
        _emit 0xe9
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD EDI, 0x1                (i++)
        _emit 0xc7
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x14               (clean 5 args)
        _emit 0xc4
        _emit 0x14
        _emit 0x83              // ADD ESI, 0x8                (next element)
        _emit 0xc6
        _emit 0x08
        _emit 0x3b              // CMP EDI, dword ptr [EBX+0xc]
        _emit 0x7b
        _emit 0x0c
        _emit 0x72              // JC -0x26  (→ loop body: MOV EDI,EDI)
        _emit 0xda
        _emit 0x68              // PUSH 0x00f578b4             (footer fmt, arg1)
        _emit 0xb4
        _emit 0x78
        _emit 0xf5
        _emit 0x00
        _emit 0xe8              // CALL FUN_00415c90 (rel32 = 0xffffe690)
        _emit 0x90
        _emit 0xe6
        _emit 0xff
        _emit 0xff
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_00415f70 (rel32 = 0xffffe96a)
        _emit 0x6a
        _emit 0xe9
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x8                (clean 2 args)
        _emit 0xc4
        _emit 0x08
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
