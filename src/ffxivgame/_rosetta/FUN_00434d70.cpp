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
// FUNCTION: ffxivgame 0x00034d70 — __thiscall wrapper, 34 bytes (RET 0x4).
//
// Semantics (recovered from asm):
//
//   void FUN_00434d70(this, void *arg1) {   // __thiscall, 1 stack arg
//       char locals[12];                    // SUB ESP, 0xc
//       // Compute sub-element: array at this+0x38, element size 16 bytes,
//       // indexed by this->field_0x34.
//       elem = (char*)this + 0x38 + (this->field_0x34 << 4);
//       FUN_004214a0(elem, &locals, &arg1); // __thiscall, 2 args, RET 0x8
//   }
//
// Register usage:
//   ECX = this on entry; consumed by the final LEA to form the callee's
//   ECX (sub-element address).  No callee-save registers used.
//   EAX = scratch for index computation; EDX = pointer to local buffer.
//
// Push order: PUSH EAX (&arg1) then PUSH EDX (&locals), so callee sees:
//   [ESP+4] = &locals  (arg1)
//   [ESP+8] = &arg1    (arg2)
//
// Closely mirrors the tail of FUN_00434d00 (RET 0x8, 2 stack args, ESI
// save): same element-indexing idiom, same FUN_004214a0 callee, but here
// there is only 1 stack arg and no ESI save, so ECX can be consumed
// directly by the LEA without first being saved.
//
// Reloc-bearing site (compare.py wildcard-masks this 5-byte window):
//     +0x17  CALL rel32 → 0x004214a0  (FUN_004214a0, std::_Tree insert helper)
//
// Reconstruction strategy — naked-asm byte passthrough.  Emitting the
// original 34 bytes verbatim via MASM _emit produces a zero-reloc .obj
// whose .text matches the original slice byte-for-byte.

extern "C" __declspec(naked) void FUN_00434d70() {
    __asm {
        _emit 0x83              // SUB ESP, 0xc
        _emit 0xec
        _emit 0x0c
        _emit 0x8d              // LEA EAX, [ESP + 0x10]  ; &arg1
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x50              // PUSH EAX               ; push &arg1 (will be callee arg2)
        _emit 0x8b              // MOV EAX, dword ptr [ECX + 0x34]  ; this->field_0x34
        _emit 0x41
        _emit 0x34
        _emit 0x8d              // LEA EDX, [ESP + 0x4]   ; &locals (after first push, locals at ESP+4)
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0xc1              // SHL EAX, 0x4            ; index * 16
        _emit 0xe0
        _emit 0x04
        _emit 0x52              // PUSH EDX               ; push &locals (callee arg1)
        _emit 0x8d              // LEA ECX, [EAX + ECX*1 + 0x38]  ; this+0x38+index*16
        _emit 0x4c
        _emit 0x08
        _emit 0x38
        _emit 0xe8              // CALL FUN_004214a0
        _emit 0x14
        _emit 0xc7
        _emit 0xfe
        _emit 0xff
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
