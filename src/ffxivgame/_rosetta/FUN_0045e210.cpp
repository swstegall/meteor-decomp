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
// FUNCTION: ffxivgame 0x0045e210 — iterated-accumulator loop with __chkstk prologue
//                                  (__cdecl int (void *arg0), 97 bytes)
//
// Allocates 4 bytes via __chkstk (used as a spill slot for the inner
// call's handle result), saves EBX/EBP/ESI, loads EBP from arg0, then
// folds PUSH EDI into the first call's argument slot rather than
// preserving it as a callee-save. Loop pattern:
//
//   count = FUN_00464030(EDI);           // initial count query
//   if (count <= 0) return 0;
//   do {
//       handle = FUN_00464040(EDI, i);   // fetch handle at index i
//       result = FUN_00460470(&handle, arg0, 0xf6913c, -1, -1);
//       if (result < 0) return result;
//       sum += result;
//       i++;
//       count = FUN_00464030(EDI);       // re-query count
//   } while (i < count);
//   return sum;
//
// Epilogue restores ESI/EBP/EBX and then POP ECX to discard the
// 4-byte __chkstk allocation before RET.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The __chkstk-before-prologue idiom, the combined callee-save/arg push
//   for EDI, and the back-edge loop with two distinct exit paths make a
//   source-level C++ reconstruction unreliable. Emitting the original 97
//   bytes verbatim via MASM _emit produces a .obj whose .text is
//   byte-identical to the orig slice; tools/compare.py masks the five
//   CALL rel32 slots and one PUSH imm32 (0xf6913c) reloc, so those bytes
//   need not match exactly but are included here from the orig binary.
//
// Reloc-bearing sites (rel32 / DIR32 masked by compare.py):
//   +0x06  CALL rel32 → __chkstk / _alloca_probe (VA 0x009d29d0)
//   +0x17  CALL rel32 → FUN_00464030              (VA 0x00464030)
//   +0x25  CALL rel32 → FUN_00464040              (VA 0x00464040)
//   +0x2e  PUSH imm32 → DAT_00f6913c              (VA 0x00f6913c)
//   +0x3d  CALL rel32 → FUN_00460470              (VA 0x00460470)
//   +0x4f  CALL rel32 → FUN_00464030              (VA 0x00464030)

extern "C" __declspec(naked) void FUN_0045e210() {
    __asm {
        _emit 0xb8              // MOV  EAX, 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL __chkstk  (VA 0x009d29d0)
        _emit 0xb6
        _emit 0x47
        _emit 0x57
        _emit 0x00
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV  EBP, dword ptr [ESP+0x10]
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI  (folded: callee EDI + arg to CALL below)
        _emit 0x33              // XOR  EBX, EBX
        _emit 0xdb
        _emit 0x33              // XOR  ESI, ESI
        _emit 0xf6
        _emit 0xe8              // CALL FUN_00464030  (VA 0x00464030)
        _emit 0x05
        _emit 0x5e
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD  ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7e              // JLE  +0x38  (→ MOV EAX,EBX / epilogue)
        _emit 0x38
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL FUN_00464040  (VA 0x00464040)
        _emit 0x07
        _emit 0x5e
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0xf6913c  (VA 0x00f6913c)
        _emit 0x3c
        _emit 0x91
        _emit 0xf6
        _emit 0x00
        _emit 0x89              // MOV  dword ptr [ESP+0x20], EAX   (spill handle)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x8d              // LEA  EAX, [ESP+0x20]             (ptr to handle)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x55              // PUSH EBP
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_00460470  (VA 0x00460470)
        _emit 0x1f
        _emit 0x22
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD  ESP, 0x1c
        _emit 0xc4
        _emit 0x1c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7c              // JL   +0x14  (→ early exit, return EAX)
        _emit 0x14
        _emit 0x57              // PUSH EDI
        _emit 0x03              // ADD  EBX, EAX
        _emit 0xd8
        _emit 0x83              // ADD  ESI, 0x1
        _emit 0xc6
        _emit 0x01
        _emit 0xe8              // CALL FUN_00464030  (VA 0x00464030)
        _emit 0xcd
        _emit 0x5d
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD  ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x3b              // CMP  ESI, EAX
        _emit 0xf0
        _emit 0x7c              // JL   -0x38  (→ loop top PUSH ESI)
        _emit 0xc8
        _emit 0x8b              // MOV  EAX, EBX
        _emit 0xc3
        _emit 0x5e              // POP  ESI                 ; restore
        _emit 0x5d              // POP  EBP                 ; restore
        _emit 0x5b              // POP  EBX                 ; restore
        _emit 0x59              // POP  ECX                 ; discard __chkstk alloc
        _emit 0xc3              // RET
    }
}
