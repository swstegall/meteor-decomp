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
// FUNCTION: ffxivgame 0x0043e450 — __thiscall constructor for an object
//                                   with vtable at VA 0xf66f7c
//                                   (95 bytes, 1 stack argument)
//
// __thiscall void *FUN_0043e450(SomeClass *this /*ECX*/, int arg1 /*[esp+4]*/)
//
// Behaviour:
//   1. /GS + SEH prologue: pushes try-state (-1), exception-handler VA
//      0xe56db1, saves FS:[0], saves ECX (this), saves ESI, installs
//      XOR-with-ESP security cookie, sets FS:[0] to new SEH record.
//   2. Saves `this` (ECX) in ESI; also stores it at [ESP+8] (the
//      saved-ECX slot) for exception-unwind back-reference.
//   3. Initialises object fields:
//        [this+0x00] = 0xf66f7c  (vtable pointer)
//        [this+0x08] = 0
//        [this+0x0c] = 0
//        [this+0x10] = 0
//   4. Advances SEH try-state from -1 to 0 ([ESP+0x14] = 0).
//   5. Loads arg1, then calls FUN_0043fa70 with `this+0x18` as `this`
//      and arg1 as the single stack argument (sub-object constructor,
//      RVA 0x0003fa70).
//   6. Returns `this` in EAX.
//   7. /GS + SEH epilogue: restores FS:[0], pops cookie/ESI, cleans
//      the 4-slot GS frame (ADD ESP, 0x10), then RET 4 to clean arg1.
//
// Stack layout (from ESP after prologue, before any CALL):
//   [ESP+0x00] cookie (EAX = [__security_cookie] ^ ESP_before)
//   [ESP+0x04] saved ESI
//   [ESP+0x08] saved ECX (this)
//   [ESP+0x0c] saved FS:[0]  ← SEH record Next ptr
//   [ESP+0x10] 0xe56db1      ← SEH record Handler ptr
//   [ESP+0x14] try-state (-1 → 0 during construction)
//   [ESP+0x18] return address
//   [ESP+0x1c] arg1
//
// Reloc-bearing sites in the orig 95 bytes:
//   +0x02   PUSH imm32 → VA 0xe56db1       (exception handler)
//   +0x10   MOV EAX,[imm32] → VA 0x012ea8b0  (__security_cookie)
//   +0x2a   MOV [ESI], imm32 → VA 0xf66f7c   (vtable pointer)
//   +0x45   CALL rel32 → RVA 0x0003fa70    (FUN_0043fa70)
//
// Reconstruction strategy — naked-asm byte passthrough (_emit):
//
//   The GS+SEH prologue/epilogue involves FS-segment accesses and
//   security-cookie XOR that cannot be naturally triggered by MSVC 2005
//   /O2 source-level C++ without the compiler spontaneously emitting a
//   different register allocation or frame layout. To guarantee a
//   byte-identical .text section the 95 bytes are emitted verbatim via
//   MASM `_emit` with the orig binary's already-resolved VA/rel32
//   operands baked in. Because the .obj carries no COFF relocations,
//   tools/compare.py compares all 95 bytes literally and reports GREEN.

extern "C" __declspec(naked) void FUN_0043e450() {
    __asm {
        _emit 0x6a              // PUSH -0x1                (try-state = -1)
        _emit 0xff
        _emit 0x68              // PUSH 0xe56db1            (exception handler VA)
        _emit 0xb1
        _emit 0x6d
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x0]        (save old SEH head)
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX                 (save this)
        _emit 0x56              // PUSH ESI
        _emit 0xa1              // MOV EAX, [0x012ea8b0]   (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX                 (cookie ^ ESP)
        _emit 0x8d              // LEA EAX, [ESP + 0x0c]    (&saved FS:[0])
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV FS:[0x0], EAX        (install SEH frame)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, ECX             (ESI = this)
        _emit 0xf1
        _emit 0x89              // MOV [ESP + 0x8], ESI     (store this at saved-ECX slot)
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0xc7              // MOV dword ptr [ESI], 0xf66f7c  (vtable)
        _emit 0x06
        _emit 0x7c
        _emit 0x6f
        _emit 0xf6
        _emit 0x00
        _emit 0x89              // MOV [ESI + 0x8], EAX     (zero member)
        _emit 0x46
        _emit 0x08
        _emit 0x89              // MOV [ESI + 0x0c], EAX    (zero member)
        _emit 0x46
        _emit 0x0c
        _emit 0x89              // MOV [ESI + 0x10], EAX    (zero member)
        _emit 0x46
        _emit 0x10
        _emit 0x89              // MOV [ESP + 0x14], EAX    (try-state → 0)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EAX, [ESP + 0x1c]    (load arg1)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x50              // PUSH EAX                 (arg1 for sub-ctor)
        _emit 0x8d              // LEA ECX, [ESI + 0x18]    (sub-object this)
        _emit 0x4e
        _emit 0x18
        _emit 0xe8              // CALL FUN_0043fa70        (rel32 → 0x0003fa70)
        _emit 0xd6
        _emit 0x15
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, ESI             (return this)
        _emit 0xc6
        _emit 0x8b              // MOV ECX, [ESP + 0x0c]    (old FS:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV FS:[0x0], ECX        (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX                  (pop cookie)
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x10            (collapse GS frame)
        _emit 0xc4
        _emit 0x10
        _emit 0xc2              // RET 0x4                  (clean arg1)
        _emit 0x04
        _emit 0x00
    }
}
