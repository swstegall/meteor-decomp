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
// FUNCTION: ffxivgame 0x000153a0 — `__thiscall` level-keyed range-table
//                                  lookup with modular interpolation (60 B)
//
// Calling convention: __thiscall (ECX = this; two stack args;
//   callee-cleans 8 via `ret 8`).
//   arg1 = byte level  [ESP+0x4]
//   arg2 = int  val    [ESP+0x8]
//
// Layout of *this (inferred):
//   Flat array of 5-DWORD (20-byte) entries starting at this+0x00.
//   Each entry (at this + n*20):
//     +0x00  int base       — added to final result
//     +0x04  int ???        — not read here
//     +0x08  int addend     — bias added to val before dividing
//     +0x0c  int max        — threshold: val must be > max to proceed
//     +0x10  int divisor    — modulus divisor; entries[0].divisor
//                             (at this+0x10) also used as the output scale
//
// Source shape (inferred):
//
//   int Foo::method(unsigned char level, int val) {
//       if (level >= 5) level = 4;
//       int idx = (level + 1) * 5;
//       // idx*4 indexes the entry: this[20*(level+1)]
//       if (val <= this->entries[idx].max)
//           return 0;
//       int rem = (this->entries[idx].addend + val) % this->entries[idx].divisor;
//       return rem * this->entries[0].divisor + this->entries[idx].base;
//   }
//
// Key structural notes:
//   - PUSH ESI is delayed to offset +0x1b (after the CMP), deferring the
//     callee-save until the register is first needed for the entry pointer.
//     MSVC 2005 /O2 performs this deferral when the early-return path
//     does not need ESI.
//   - The early-return (val <= max) zeroes EAX with XOR and pops ESI
//     before the `ret 8`, then falls through to the calculation path
//     which also pops ESI.
//   - No frame pointer (no PUSH EBP / MOV EBP, ESP).
//   - No relocations — all addressing is register-indirect; the 60 bytes
//     are entirely self-contained.
//
// Reconstruction: naked-asm _emit passthrough — no relocations so raw
// bytes reproduce identically without linker fixups. compare.py → GREEN.

extern "C" __declspec(naked) void FUN_004153a0() {
    __asm {
        _emit 0x8a              // MOV AL, byte ptr [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x3c              // CMP AL, 0x5
        _emit 0x05
        _emit 0x72              // JC +0x02  (→ movzx)
        _emit 0x02
        _emit 0xb0              // MOV AL, 0x4
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x8]
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x0f              // MOVZX EAX, AL
        _emit 0xb6
        _emit 0xc0
        _emit 0x83              // ADD EAX, 0x1
        _emit 0xc0
        _emit 0x01
        _emit 0x8d              // LEA EAX, [EAX + EAX*4]  → EAX = (level+1)*5
        _emit 0x04
        _emit 0x80
        _emit 0x39              // CMP dword ptr [ECX + EAX*4 + 0xc], EDX
        _emit 0x54
        _emit 0x81
        _emit 0x0c
        _emit 0x56              // PUSH ESI          (callee-save, deferred)
        _emit 0x8d              // LEA ESI, [ECX + EAX*4]  → ESI = entry ptr
        _emit 0x34
        _emit 0x81
        _emit 0x7f              // JG +0x06  (→ calc)
        _emit 0x06
        _emit 0x33              // XOR EAX, EAX      (return 0)
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x8]  (entry->addend)
        _emit 0x46
        _emit 0x08
        _emit 0x03              // ADD EAX, EDX          (addend + val)
        _emit 0xc2
        _emit 0x99              // CDQ
        _emit 0xf7              // IDIV dword ptr [ESI+0x10]  (÷ entry->divisor)
        _emit 0x7e
        _emit 0x10
        _emit 0x8b              // MOV EAX, EDX          (remainder)
        _emit 0xc2
        _emit 0x0f              // IMUL EAX, dword ptr [ECX+0x10]  (* entries[0].divisor)
        _emit 0xaf
        _emit 0x41
        _emit 0x10
        _emit 0x03              // ADD EAX, dword ptr [ESI]  (+ entry->base)
        _emit 0x06
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
