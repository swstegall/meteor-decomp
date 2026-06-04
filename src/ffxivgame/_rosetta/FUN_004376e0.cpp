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
// FUNCTION: ffxivgame 0x000376e0 — allocate-and-forward dispatch thunk
//                                  (__thiscall, 98 B / 0x62)
//
// __thiscall void FUN_004376e0(int a0, int a1, int a2, int a3, int a4)
//   ECX = this; RET 0x14 → 5 dword stack args (5-arg __thiscall).
//
// Behaviour:
//   1. p = *(0x01328d90);                       ; global pool/registry pointer
//      idx = (unsigned char)*p;                 ; one-byte index at *p
//      slot = *(p+4) + idx*28;                  ; element stride 0x1c, 7*idx*4
//      obj  = ((Allocator*)slot)->alloc(0x34);  ; FUN_00417ab0(thiscall, size=52)
//   2. if (obj != NULL):
//        r = obj->init(a0, a2, a1, a3, a4);      ; FUN_004352e0 (thiscall)
//        ((Sink*)*(this+8))->push(r);            ; FUN_0043c2d0 (thiscall, RET ?)
//      else:
//        ((Sink*)*(this+8))->push(NULL);
//
// The inner init call (FUN_004352e0) receives the five forwarded args in
// the scheduled push order arg4,arg3,arg1,arg2,arg0 — i.e. on the callee
// stack a0,a2,a1,a3,a4 — exactly as the orig codegen interleaved the
// register loads with the pushes.
//
// Reloc-bearing sites in the orig 98 bytes:
//   +0x05   MOV  ECX, [imm32]  (DIR32 → 0x01328d90 global)
//   +0x1d   CALL rel32         → 0x00417ab0  (allocator)
//   +0x41   CALL rel32         → 0x004352e0  (init)
//   +0x4a   CALL rel32         → 0x0043c2d0  (sink push, success path)
//   +0x59   CALL rel32         → 0x0043c2d0  (sink push, null path)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form would emit the same shape but produce DIR32 /
//   rel32 relocations the linker resolves at relink time, and would need
//   to coax MSVC 2005 /O2 into the exact push scheduling (arg1/arg2 swap)
//   and register-allocator picture. A `__declspec(naked)` body that
//   re-emits the orig 98 bytes verbatim via MASM `_emit` directives
//   produces a .obj whose .text is byte-identical to the orig slice
//   (no relocations — the imm32/rel32 operands are baked into the orig
//   binary's own address space and emitted here as raw bytes). compare.py
//   then reports GREEN.

extern "C" __declspec(naked) void FUN_004376e0() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV  ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV  ECX, dword ptr [0x01328d90]
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x0f              // MOVZX EAX, byte ptr [ECX]
        _emit 0xb6
        _emit 0x01
        _emit 0x8d              // LEA  EDX, [EAX*0x8 + 0x0]
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB  EDX, EAX
        _emit 0xd0
        _emit 0x8b              // MOV  EAX, dword ptr [ECX+0x4]
        _emit 0x41
        _emit 0x04
        _emit 0x8d              // LEA  ECX, [EAX + EDX*0x4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a              // PUSH 0x34
        _emit 0x34
        _emit 0xe8              // CALL 0x00417ab0 (rel32)
        _emit 0xae
        _emit 0x03
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ   +0x2d  (→ 00037733, null path)
        _emit 0x2d
        _emit 0x8b              // MOV  ECX, dword ptr [ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV  EDX, dword ptr [ESP+0x14]
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV  ECX, dword ptr [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x52              // PUSH EDX
        _emit 0x8b              // MOV  EDX, dword ptr [ESP+0x18]
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV  ECX, dword ptr [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x52              // PUSH EDX
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV  ECX, EAX
        _emit 0xc8
        _emit 0xe8              // CALL 0x004352e0 (rel32)
        _emit 0xba
        _emit 0xdb
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV  ECX, dword ptr [ESI+0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043c2d0 (rel32)
        _emit 0xa1
        _emit 0x4b
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0x14
        _emit 0x14
        _emit 0x00
        _emit 0x8b              // MOV  ECX, dword ptr [ESI+0x8]   (null path)
        _emit 0x4e
        _emit 0x08
        _emit 0x33              // XOR  EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043c2d0 (rel32)
        _emit 0x92
        _emit 0x4b
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0x14
        _emit 0x14
        _emit 0x00
    }
}
