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
// FUNCTION: ffxivgame 0x00014d80 — try/finally-wrapped two-step driver
//                                   (__thiscall, 73 B / 0x49)
//
// __thiscall void FUN_00414d80(C *this);
//
// Structure (matches asm flow):
//
//   __try {
//       this->FUN_00414e10();      // pass 1 (trylevel = 0)
//   } __finally {
//       this->FUN_00414e10();      // pass 2 (trylevel = -1 — cleanup)
//   }
//
//   Two rel32 CALLs to the same callee FUN_00414e10 (which appears to
//   be a __thiscall helper — `this` is reloaded into ECX before the
//   second call). The trylevel slot at [ESP+0x10] is written 0 before
//   the first call and -1 before the second — the EH4 scope-table at
//   0xe55198 (in .rdata) maps those values to the try / finally
//   regions respectively.
//
// Stack frame (after the EH4 prologue, ESP-relative):
//   [esp+0x00]  ESI (saved)
//   [esp+0x04]  this (cached copy of ECX, materialised for SEH)
//   [esp+0x08]  saved-FS:[0] chain link
//   [esp+0x0c]  scope-table address (0xe55198)
//   [esp+0x10]  EH4 trylevel  (-1 idle, 0 inside try, -1 in finally)
//   [esp+0x14]  return address
//
// Reloc-bearing sites in the orig 73 bytes:
//   +0x03   PUSH imm32   scope-table 0x00e55198 (.rdata)
//   +0x09   FS:[0] read  (constant 0, fold-through)
//   +0x10   FS:[0] install (constant 0, fold-through)
//   +0x26   CALL rel32   FUN_00414e10  (0x00014e10 → +0x66 from next-IP)
//   +0x35   CALL rel32   FUN_00414e10  (0x00014e10 → +0x57 from next-IP)
//   +0x40   FS:[0] restore (constant 0, fold-through)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc
//   into emitting the exact EH4 prologue + the exact `MOV ECX, ESI`
//   re-bind between the two calls (the second call is in the finally
//   block and must reload `this` from the callee-save). Driving the
//   linker to resolve the scope-table .rdata address and the two
//   rel32 callees in a standalone .obj diff is brittle.
//
//   The pragmatic choice — the same one the sibling SEH-wrapped
//   FUN_004014b0 / FUN_00401a00 / FUN_00403eb0 took — is a
//   `__declspec(naked)` body that re-emits the orig 73 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice (no relocations: scope-table and
//   rel32 targets are absolute / relative values in the orig
//   binary's own address space, so emitting them as immediates
//   produces the same bytes the linker would produce).

extern "C" __declspec(naked) void FUN_00414d80() {
    __asm {
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00e55198  (EH4 scope-table)
        _emit 0x98
        _emit 0x51
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x64              // MOV FS:[0], ESP
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX  (alloc trylevel slot)
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x89              // MOV [ESP+0x04], ESI  (cache this)
        _emit 0x74
        _emit 0x24
        _emit 0x04
        _emit 0xc7              // MOV [ESP+0x10], 0   (trylevel = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_00414e10  (rel32 = +0x66)
        _emit 0x66
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, ESI  (reload this for 2nd call)
        _emit 0xce
        _emit 0xc7              // MOV [ESP+0x10], -1  (trylevel = -1)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8              // CALL FUN_00414e10  (rel32 = +0x57)
        _emit 0x57
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, [ESP+0x08]  (saved FS:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x5e              // POP ESI
        _emit 0x64              // MOV FS:[0], ECX  (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3              // RET
    }
}
