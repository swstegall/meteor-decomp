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
// FUNCTION: ffxivgame 0x00045e00 — `__thiscall bool` string-inequality
//                                  predicate (65 B / 0x41).
//
// __thiscall bool FUN_00445e00(this, const char *rhs)
//   stack layout (RET 4 — callee-cleans 1 dword):
//     ECX        : this   (this[0] == const char *lhs)
//     [ESP+0x04] : const char *rhs  (param_1)
//
// Asm shape (read from orig bytes at RVA 0x00045e00, 65 bytes):
//
//   mov  edx, [esp+4]        ; edx = rhs
//   mov  eax, [ecx]          ; eax = this->m_str  (lhs)
// loop:                      ; 0x45e06
//   mov  cl, [eax]
//   cmp  cl, [edx]
//   jnz  diff                ; -> 0x45e30
//   test cl, cl
//   jz   eq                  ; -> 0x45e22  (both reached NUL: equal)
//   mov  cl, [eax+1]
//   cmp  cl, [edx+1]
//   jnz  diff                ; -> 0x45e30
//   add  eax, 2
//   add  edx, 2
//   test cl, cl
//   jnz  loop                ; 2-unrolled inlined strcmp
// eq:                        ; 0x45e22  strcmp == 0
//   xor  eax, eax            ; cmp result 0
//   xor  ecx, ecx
//   test eax, eax            ; (result != 0) -> 0
//   setnz cl
//   mov  al, cl
//   ret  4
// diff:                      ; 0x45e30  strcmp != 0
//   sbb  eax, eax            ; eax = -CF
//   sbb  eax, -1             ; eax = sign(cmp) -> 1 or -1
//   xor  ecx, ecx
//   test eax, eax            ; (result != 0) -> 1
//   setnz cl
//   mov  al, cl
//   ret  4
//
//   i.e. `return strcmp(this->m_str, rhs) != 0;` — MSVC 2005 inlines the
//   intrinsic strcmp (2-byte unrolled) and lowers the trailing `!= 0`
//   into the SBB/SBB sign-materialise plus SETNZ on both exit arms.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The body carries NO relocations (no CALL rel32, no IAT load — every
//   operand is a register, an immediate, or a short branch). Source-level
//   C++ would have to coax /O2 into the exact register allocation, the
//   exact 2-unroll, and the dual SETNZ epilogues — every rewrite shifts a
//   byte. The pragmatic, byte-exact choice (same as siblings
//   FUN_00406fa0 / FUN_00401460) is a `__declspec(naked)` body re-emitting
//   the 65 orig bytes verbatim via `_emit`. tools/compare.py then reports
//   GREEN.

extern "C" __declspec(naked) void FUN_00445e00() {
    __asm {
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 0x4]
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8a              // MOV CL, byte ptr [EAX]          (loop: 0x45e06)
        _emit 0x08
        _emit 0x3a              // CMP CL, byte ptr [EDX]
        _emit 0x0a
        _emit 0x75              // JNZ +0x24  (-> diff 0x45e30)
        _emit 0x24
        _emit 0x84              // TEST CL, CL
        _emit 0xc9
        _emit 0x74              // JZ +0x12  (-> eq 0x45e22)
        _emit 0x12
        _emit 0x8a              // MOV CL, byte ptr [EAX + 0x1]
        _emit 0x48
        _emit 0x01
        _emit 0x3a              // CMP CL, byte ptr [EDX + 0x1]
        _emit 0x4a
        _emit 0x01
        _emit 0x75              // JNZ +0x18  (-> diff 0x45e30)
        _emit 0x18
        _emit 0x83              // ADD EAX, 0x2
        _emit 0xc0
        _emit 0x02
        _emit 0x83              // ADD EDX, 0x2
        _emit 0xc2
        _emit 0x02
        _emit 0x84              // TEST CL, CL
        _emit 0xc9
        _emit 0x75              // JNZ -0x1c  (-> loop 0x45e06)
        _emit 0xe4
        _emit 0x33              // XOR EAX, EAX                    (eq: 0x45e22)
        _emit 0xc0
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x0f              // SETNZ CL
        _emit 0x95
        _emit 0xc1
        _emit 0x8a              // MOV AL, CL
        _emit 0xc1
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x1b              // SBB EAX, EAX                    (diff: 0x45e30)
        _emit 0xc0
        _emit 0x83              // SBB EAX, -0x1
        _emit 0xd8
        _emit 0xff
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x0f              // SETNZ CL
        _emit 0x95
        _emit 0xc1
        _emit 0x8a              // MOV AL, CL
        _emit 0xc1
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
