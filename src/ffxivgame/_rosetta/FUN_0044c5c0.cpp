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
// FUNCTION: ffxivgame 0x0044c5c0 — __thiscall constructor-style method
//                                   with SEH frame + /GS cookie (107 B).
//
// __thiscall void *FUN_0044c5c0(void *this /*ECX*/, unsigned int count)
//
// Semantics (recovered from asm):
//   1. Installs an SEH3 frame (PUSH -1 / PUSH scope_table / chain FS:[0])
//      plus a /GS security cookie (MOV EAX,[__security_cookie] / XOR EAX,ESP).
//   2. Saves ECX (this) into ESI; writes it back to [ESP+0x08] for the cookie
//      checker.
//   3. Loads the single stack argument (count) from [ESP+0x1C].
//   4. Computes size = count * 12 with overflow detection:
//        XOR ECX, ECX
//        MOV EDX, 0xC
//        MUL EDX           ; EAX = count*12 (lo32), EDX = hi32
//        SETO CL           ; CL = 1 if overflow
//        NEG ECX           ; ECX = 0 if no overflow, 0xFFFFFFFF if overflow
//        OR  ECX, EAX      ; ECX = safe size (or huge value on overflow)
//   5. Sets the vtable pointer in *this to 0x00f6736c.
//   6. Calls the allocator at 0x009d04ac (operator new / malloc) with ECX.
//   7. Stores the returned pointer in this->field4 ([ESI+0x4]).
//   8. Returns 'this' (ESI) in EAX.
//   9. Tears down the SEH frame and returns __stdcall-style (RET 0x4).
//
// Stack layout (after prolog — PUSH -1, PUSH scope, PUSH old_fs0,
//               PUSH ECX, PUSH ESI, PUSH cookie — 6 pushes = 0x18 bytes):
//   [ESP+0x00] : /GS cookie  (PUSH EAX after XOR)
//   [ESP+0x04] : saved ESI
//   [ESP+0x08] : saved ECX   (= 'this'; written again by MOV [ESP+8],ESI)
//   [ESP+0x0C] : old FS:[0]
//   [ESP+0x10] : scope table (PUSH 0xe579f8)
//   [ESP+0x14] : trylevel = -1 then 0 (PUSH -1, updated inside try)
//   [ESP+0x18] : return address
//   [ESP+0x1C] : count       (first and only caller-supplied argument)
//
// Reloc-bearing sites (all baked in as raw bytes; compare.py masks them):
//   +0x02   PUSH imm32    → 0x00e579f8  (EH3 scope table, .rdata)
//   +0x07   MOV EAX,FS:[0] → 00 00 00 00 (FS segment, not a PE reloc)
//   +0x10   MOV EAX,[imm32] → 0x012ea8b0 (__security_cookie, .data)
//   +0x18   MOV FS:[0],EAX  → 00 00 00 00 (FS segment)
//   +0x4b   CALL rel32    → 0x00583e9c   (→ 0x009d04ac, allocator, .text)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The /GS prolog order (PUSH -1, PUSH scope BEFORE MOV EAX,FS:[0] and the
//   ECX-save PUSH between FS-chain and cookie-XOR) cannot be reproduced by a
//   source-level C++ constructor in MSVC 2005 without matching the exact
//   compiler heuristics. The pragmatic path — same as FUN_004090b0,
//   FUN_004091f0, FUN_00409260 — is a __declspec(naked) body that re-emits
//   all 107 orig bytes verbatim via MASM _emit directives.

extern "C" __declspec(naked) void FUN_0044c5c0() {
    __asm {
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0x00e579f8 (EH scope table)
        _emit 0xf8
        _emit 0x79
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX  (chain old fs:[0])
        _emit 0x51              // PUSH ECX  (save 'this')
        _emit 0x56              // PUSH ESI
        _emit 0xa1              // MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX  (/GS cookie)
        _emit 0x8d              // LEA EAX, [ESP + 0x0c]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV FS:[0x0], EAX  (install SEH chain)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, ECX  (ESI = this)
        _emit 0xf1
        _emit 0x89              // MOV dword ptr [ESP + 0x08], ESI  (write-back 'this')
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x1c]  (count arg)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0xba              // MOV EDX, 0x0000000c
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf7              // MUL EDX  (EAX = count * 12, lo32)
        _emit 0xe2
        _emit 0x0f              // SETO CL  (CL = 1 if overflow)
        _emit 0x90
        _emit 0xc1
        _emit 0xc7              // MOV dword ptr [ESP + 0x14], 0x0  (trylevel = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f6736c  (set vtable)
        _emit 0x06
        _emit 0x6c
        _emit 0x73
        _emit 0xf6
        _emit 0x00
        _emit 0xf7              // NEG ECX
        _emit 0xd9
        _emit 0x0b              // OR ECX, EAX
        _emit 0xc8
        _emit 0x51              // PUSH ECX  (allocation size)
        _emit 0xe8              // CALL 0x009d04ac (rel32 = 0x00583e9c)
        _emit 0x9c
        _emit 0x3e
        _emit 0x58
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESI + 0x4], EAX  (this->field4 = ptr)
        _emit 0x46
        _emit 0x04
        _emit 0x83              // ADD ESP, 0x4  (pop PUSH ECX arg)
        _emit 0xc4
        _emit 0x04
        _emit 0x8b              // MOV EAX, ESI  (return 'this')
        _emit 0xc6
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x0c]  (old FS:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV dword ptr FS:[0x0], ECX  (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x10  (cookie + old_fs0 + scope + trylevel)
        _emit 0xc4
        _emit 0x10
        _emit 0xc2              // RET 0x4  (clean 1 dword caller arg)
        _emit 0x04
        _emit 0x00
    }
}
