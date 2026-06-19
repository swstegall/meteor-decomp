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
// FUNCTION: ffxivgame 0x0044c510 — __thiscall constructor-like method; sets
//   vtable ptr at [this+0], computes n*4 with 32-bit overflow saturation
//   (n*4 → -1 on overflow), calls allocator FUN_009d04ac, stores result at
//   [this+4], returns this.  107 bytes.
//
// Calling convention: __thiscall (ECX = this, one stack arg `n`, RET 4).
// Stack frame (after prologue, ESP = orig-24):
//   [ESP+0x00]  GS cookie (security_cookie ^ ESP)
//   [ESP+0x04]  saved ESI
//   [ESP+0x08]  saved ECX (this, also written by MOV [ESP+8],ESI after copy)
//   [ESP+0x0C]  old FS:[0]   ← EXCEPTION_REGISTRATION_RECORD.prev
//   [ESP+0x10]  0xe579c8     ← EXCEPTION_REGISTRATION_RECORD.handler
//   [ESP+0x14]  -1 → 0       ← trylevel (set to 0 on entry of try-block)
//   [ESP+0x18]  return address
//   [ESP+0x1C]  arg n
//
// Body:
//   EAX = n (=[ESP+0x1C]); ECX = 0; EDX = 4
//   MUL EDX  →  EAX = n*4 (low), EDX = n*4 (high) — OF set iff overflow
//   SETO CL  →  CL = 1 if OF (multiply overflowed into EDX)
//   [ESP+0x14] = 0  (enter SEH try-block state 0)
//   [ESI+0] = 0xf6735c  (set vtable)
//   NEG ECX  →  ECX = 0 or 0xFFFFFFFF
//   OR  ECX, EAX  →  ECX = n*4 | (-overflow)  (saturates to -1 on overflow)
//   PUSH ECX; CALL FUN_009d04ac  (operator new[] or malloc-like, cdecl-compat)
//   [ESI+4] = EAX  (store allocated buffer pointer)
//   return ESI (= this)
//
// Reloc-bearing sites (compare.py masks these):
//   +0x03  PUSH imm32 → 0xe579c8   (EH scope table, .rdata)
//   +0x12  MOV EAX,[imm32] → 0x012ea8b0  (__security_cookie, .data)
//   +0x1d  MOV FS:[0],EAX (imm32 = 0x00000000, FS-override abs)
//   +0x3b  MOV dword ptr [ESI],0xf6735c  (vtable, .rdata — abs imm32 at +0x3c)
//   +0x46  CALL rel32 → 0x009d04ac  (FUN_009d04ac, allocator)
//   +0x58  MOV FS:[0],ECX (imm32 = 0x00000000, FS-override abs)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The SEH3+/GS prologue ordering (PUSH -1 / PUSH scopetable / MOV FS:[0] /
//   PUSH ECX (this) / PUSH ESI / XOR-cookie / LEA / install) cannot be
//   reproduced byte-for-byte from source-level C++; the MSVC 2005 compiler
//   schedules the cookie XOR and register saves in a compiler-internal order
//   that source code cannot control.  Naked-asm passthrough is the standard
//   approach used by all SEH-wrapped siblings in this _rosetta tree.

extern "C" __declspec(naked) void FUN_0044c510() {
    __asm {
        _emit 0x6a  // PUSH -1
        _emit 0xff
        _emit 0x68  // PUSH 0xe579c8  (SEH scope table)
        _emit 0xc8
        _emit 0x79
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX  (chain old FS:[0])
        _emit 0x51  // PUSH ECX  (save this)
        _emit 0x56  // PUSH ESI
        _emit 0xa1  // MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX  (GS cookie ^ ESP)
        _emit 0x8d  // LEA EAX, [ESP+0x0c]  (ptr to EXCEPTION_REGISTRATION_RECORD)
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x64  // MOV FS:[0], EAX  (install SEH frame)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV ESI, ECX  (ESI = this)
        _emit 0xf1
        _emit 0x89  // MOV dword ptr [ESP+0x8], ESI  (refresh saved-this slot)
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x8b  // MOV EAX, dword ptr [ESP+0x1c]  (load arg n)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x33  // XOR ECX, ECX  (zero high-overflow sentinel)
        _emit 0xc9
        _emit 0xba  // MOV EDX, 4
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf7  // MUL EDX  (EAX = n*4; OF set if overflowed)
        _emit 0xe2
        _emit 0x0f  // SETO CL  (CL = 1 if MUL overflowed)
        _emit 0x90
        _emit 0xc1
        _emit 0xc7  // MOV dword ptr [ESP+0x14], 0  (enter SEH try-block state 0)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7  // MOV dword ptr [ESI], 0xf6735c  (set vtable)
        _emit 0x06
        _emit 0x5c
        _emit 0x73
        _emit 0xf6
        _emit 0x00
        _emit 0xf7  // NEG ECX  (0 → 0, 1 → 0xFFFFFFFF)
        _emit 0xd9
        _emit 0x0b  // OR ECX, EAX  (saturate size to -1 on overflow)
        _emit 0xc8
        _emit 0x51  // PUSH ECX  (push size arg)
        _emit 0xe8  // CALL 0x009d04ac  (FUN_009d04ac — allocator)
        _emit 0x4c
        _emit 0x3f
        _emit 0x58
        _emit 0x00
        _emit 0x89  // MOV dword ptr [ESI+4], EAX  (store allocated buffer)
        _emit 0x46
        _emit 0x04
        _emit 0x83  // ADD ESP, 4  (pop size arg)
        _emit 0xc4
        _emit 0x04
        _emit 0x8b  // MOV EAX, ESI  (return this)
        _emit 0xc6
        _emit 0x8b  // MOV ECX, dword ptr [ESP+0x0c]  (reload old FS:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x64  // MOV FS:[0], ECX  (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX  (discard GS cookie)
        _emit 0x5e  // POP ESI
        _emit 0x83  // ADD ESP, 0x10  (pop: saved this, old FS:[0], handler, trylevel)
        _emit 0xc4
        _emit 0x10
        _emit 0xc2  // RET 4  (callee cleans one stack arg)
        _emit 0x04
        _emit 0x00
    }
}
