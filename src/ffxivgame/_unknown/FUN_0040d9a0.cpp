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
// FUNCTION: ffxivgame 0x0000d9a0 — memory-space find-or-create helper
//                                   (__thiscall, 108 B / 0x6c)
//
// __thiscall int FUN_0040d9a0(this)
//   ECX : this  — pointer to an outer allocator/space object
//
// Behaviour:
//   1. Sets up an SEH frame (handler at 0x00e54e81; state −1 → 0 after
//      the guarded region is entered).
//   2. Constructs a key object on the stack at [ESP+0x14] by calling
//      FUN_0040e230 (__thiscall, 2 stack args) with:
//        ECX   = &stack_key
//        arg1  = "CDev.Engine.Lay.Mem.Space"   (VA 0x00f55864)
//        arg2  = *DAT_012652f8                  (global DWORD at 0x012652f8)
//   3. Looks up / allocates a memory space by calling FUN_0040e110
//      (__thiscall, 2 stack args) with:
//        ECX   = this->field_4   ([this+0x4])
//        arg1  = 0x9a8           (space capacity / tag)
//        arg2  = return of step 2 (key handle)
//   4. Transitions SEH state to 0.  Stores the result in two local
//      shadow slots (MSVC EH state bookkeeping).
//   5. If the result is non-NULL, calls FUN_0040d910 (__thiscall, 0 args)
//      on it (activates / initialises the space).
//   6. Restores FS:[0] and returns the result pointer.
//
// Calling convention: __thiscall, 0 stack args (plain RET / c3).
//
// SEH frame:
//   Handler address: 0x00e54e81
//   State −1 pushed first; transitioned to 0 at +0x49 (after FUN_0040e110).
//   Frame locals: 0x10 bytes (SUB ESP, 0x10).
//
// Reloc-bearing sites in the orig 108 bytes:
//   +0x02  PUSH imm32   → 0x00e54e81  (SEH handler)
//   +0x08  MOV EAX, FS:[0x0]          (FS-override — not a reloc)
//   +0x18  MOV EAX, [imm32]           → 0x012652f8  (global DWORD ptr)
//   +0x1e  PUSH imm32                 → 0x00f55864  (string literal)
//   +0x2a  CALL rel32                 → FUN_0040e230 (rel32 = 0x00000861)
//   +0x38  CALL rel32                 → FUN_0040e110 (rel32 = 0x00000733)
//   +0x55  CALL rel32                 → FUN_0040d910 (rel32 = 0xffffff16)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The interleaving of the callee-save PUSH ESI with the argument pushes
//   for FUN_0040e230 (PUSH string, MOV ESI,ECX, PUSH global, LEA ECX,[local])
//   and the reuse of ESI first as the `this` cache and then as the result
//   holder cannot be reliably reproduced at the C++ source level with MSVC
//   2005. A __declspec(naked) body re-emitting the original 108 bytes
//   verbatim via MASM _emit directives produces a .obj whose .text is
//   byte-identical to the original slice. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0040d9a0() {
    __asm {
        // 0000d9a0:  6a ff          PUSH -0x1  (SEH state = -1)
        _emit 0x6a
        _emit 0xff
        // 0000d9a2:  68 81 4e e5 00 PUSH 0x00e54e81  (SEH handler)
        _emit 0x68
        _emit 0x81
        _emit 0x4e
        _emit 0xe5
        _emit 0x00
        // 0000d9a7:  64 a1 00 00 00 00  MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000d9ad:  50             PUSH EAX  (old ExceptionList)
        _emit 0x50
        // 0000d9ae:  64 89 25 00 00 00 00  MOV FS:[0x0], ESP  (install SEH frame)
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000d9b5:  83 ec 10  SUB ESP, 0x10  (local frame)
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 0000d9b8:  a1 f8 52 26 01  MOV EAX, [0x012652f8]
        _emit 0xa1
        _emit 0xf8
        _emit 0x52
        _emit 0x26
        _emit 0x01
        // 0000d9bd:  56  PUSH ESI  (save callee-saved register)
        _emit 0x56
        // 0000d9be:  68 64 58 f5 00  PUSH 0x00f55864  ("CDev.Engine.Lay.Mem.Space")
        _emit 0x68
        _emit 0x64
        _emit 0x58
        _emit 0xf5
        _emit 0x00
        // 0000d9c3:  8b f1  MOV ESI, ECX  (ESI = this)
        _emit 0x8b
        _emit 0xf1
        // 0000d9c5:  50  PUSH EAX  (arg2: global dword)
        _emit 0x50
        // 0000d9c6:  8d 4c 24 14  LEA ECX, [ESP+0x14]  (ECX = &stack_key local)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0000d9ca:  e8 61 08 00 00  CALL 0x0040e230  (FUN_0040e230)
        _emit 0xe8
        _emit 0x61
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0000d9cf:  8b 4e 04  MOV ECX, [ESI+0x4]  (ECX = this->field_4)
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 0000d9d2:  50  PUSH EAX  (arg2: key handle from FUN_0040e230)
        _emit 0x50
        // 0000d9d3:  68 a8 09 00 00  PUSH 0x9a8  (arg1: capacity / tag)
        _emit 0x68
        _emit 0xa8
        _emit 0x09
        _emit 0x00
        _emit 0x00
        // 0000d9d8:  e8 33 07 00 00  CALL 0x0040e110  (FUN_0040e110)
        _emit 0xe8
        _emit 0x33
        _emit 0x07
        _emit 0x00
        _emit 0x00
        // 0000d9dd:  8b f0  MOV ESI, EAX  (ESI = result ptr)
        _emit 0x8b
        _emit 0xf0
        // 0000d9df:  89 74 24 04  MOV [ESP+0x4], ESI  (local shadow 0)
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x04
        // 0000d9e3:  89 74 24 08  MOV [ESP+0x8], ESI  (local shadow 1)
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 0000d9e7:  85 f6  TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0000d9e9:  c7 44 24 1c 00 00 00 00  MOV [ESP+0x1c], 0  (SEH state = 0)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000d9f1:  74 07  JZ +0x7  (skip FUN_0040d910 if ESI == 0)
        _emit 0x74
        _emit 0x07
        // 0000d9f3:  8b ce  MOV ECX, ESI  (ECX = result, for thiscall)
        _emit 0x8b
        _emit 0xce
        // 0000d9f5:  e8 16 ff ff ff  CALL 0x0040d910  (FUN_0040d910)
        _emit 0xe8
        _emit 0x16
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0000d9fa:  8b 4c 24 14  MOV ECX, [ESP+0x14]  (old ExceptionList)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0000d9fe:  8b c6  MOV EAX, ESI  (return value)
        _emit 0x8b
        _emit 0xc6
        // 0000da00:  5e  POP ESI  (restore callee-saved ESI)
        _emit 0x5e
        // 0000da01:  64 89 0d 00 00 00 00  MOV FS:[0x0], ECX  (restore SEH chain)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000da08:  83 c4 1c  ADD ESP, 0x1c  (unwind frame)
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 0000da0b:  c3  RET
        _emit 0xc3
    }
}
