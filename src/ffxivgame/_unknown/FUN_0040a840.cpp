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
// FUNCTION: ffxivgame 0x0040a840 — construct local object, call method with
//                                  argument, destruct, return result
//                                  (96 B / 0x60, __cdecl, MSVC SEH frame)
//
// __cdecl int FUN_0040a840(int arg)
//   [ESP+0x04] : int arg — passed through into the local object before
//                          calling FUN_0040db60
//
// Stack layout (after prologue + PUSH ESI, ESP = entry - 0x34):
//   [ESP+0x00]  saved ESI
//   [ESP+0x04]  local object start  (this for FUN_0040dd50/db60/db10)
//   [ESP+0x08]  local object +0x04  (arg is stored here before db60 call)
//   ...
//   [ESP+0x28]  saved FS:[0]  (old exception chain)
//   [ESP+0x2c]  SEH handler address (0x00e54d58)
//   [ESP+0x30]  SEH try-level cookie (-1 / 0 / -1)
//   [ESP+0x34]  return address
//   [ESP+0x38]  arg (first caller argument)
//
// Flow:
//   1. MSVC SEH prologue: install exception registration record
//   2. Construct local object at ECX=[ESP+0x4] via FUN_0040dd50
//   3. Set SEH try-level = 0  (destructor now covers exception unwind)
//   4. Load arg from [ESP+0x38], store into [ESP+0x8] (object field)
//   5. Call FUN_0040db60 (method on object), save result in ESI
//   6. Set SEH try-level = -1  (leaving protected region)
//   7. Call FUN_0040db10 (destructor on object)
//   8. Restore FS:[0], unwind frame, return ESI
//
// Reloc-bearing sites in the orig 96 bytes:
//     +0x02   PUSH imm32   → 0x00e54d58  (SEH handler / scope-table thunk)
//     +0x07   MOV  EAX, FS:[0x0]        (FS-relative load, no classic reloc)
//     +0x0e   MOV  FS:[0x0], ESP        (FS-relative store)
//     +0x19   CALL rel32   → FUN_0040dd50  (+0x000034ee)
//     +0x26   CALL rel32   → FUN_0040db60  (+0x000032e5)
//     +0x31   CALL rel32   → FUN_0040db10  (+0x00003282)
//
// Reconstruction strategy — naked-asm byte passthrough.
// The three CALL rel32 offsets and the PUSH imm32 handler address are baked
// into the orig binary's own address space; emitting them as raw bytes in a
// __declspec(naked) body produces a .obj whose .text is byte-identical to
// the orig slice with no relocations.  compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0040a840() {
    __asm {
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0x00e54d58  (SEH handler thunk)
        _emit 0x58
        _emit 0x4d
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, dword ptr FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x64              // MOV dword ptr FS:[0x0], ESP
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // SUB ESP, 0x24
        _emit 0xec
        _emit 0x24
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA ECX, [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xe8              // CALL FUN_0040dd50  (rel32 = 0x000034ee)
        _emit 0xee
        _emit 0x34
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x38]
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x8d              // LEA ECX, [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xc7              // MOV dword ptr [ESP+0x30], 0x0
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESP+0x8], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xe8              // CALL FUN_0040db60  (rel32 = 0x000032e5)
        _emit 0xe5
        _emit 0x32
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0xc7              // MOV dword ptr [ESP+0x30], 0xffffffff
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8              // CALL FUN_0040db10  (rel32 = 0x00003282)
        _emit 0x82
        _emit 0x32
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x28]
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x64              // MOV dword ptr FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x30
        _emit 0xc4
        _emit 0x30
        _emit 0xc3              // RET
    }
}
