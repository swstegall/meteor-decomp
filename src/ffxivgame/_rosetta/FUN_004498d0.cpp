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
// FUNCTION: ffxivgame 0x004498d0 — front half of a `std::vector`-style
//                                  reallocate-and-grow helper (134 B),
//                                  __thiscall, under an MSVC C++ SEH /GS
//                                  unwind frame.
//
// Ghidra carved this 134-byte slice out of a larger routine: the tail
// instruction at body offset 0x84 is `eb 33` (JMP +0x33) whose target
// (orig 0x00449989) lies *past* the recorded function end (0x00449956),
// i.e. it jumps into the rest of the parent function that the symbol
// table split off. That makes a source-level C++ reconstruction
// impossible from this slice alone — there is no self-contained control
// flow to model. So we reproduce the exact 134 bytes verbatim, the same
// `__declspec(naked)` byte-passthrough idiom used by siblings
// FUN_00408610 / FUN_004089f0 for code MSVC won't reproduce from source.
//
// What the slice does (reconstructed from the asm, for the record):
//
//   void * __thiscall reallocate(This *this /*ecx*/, size_t n /*[ebp+8]*/) {
//       // standard /GS + C++ SEH prologue, initial unwind state -1
//       size_t cap = n | 0xf;                 // round request up
//       if (cap <= 0xfffffffe) {              // no overflow
//           unsigned half = this->member_18;  // [edi+0x18]
//           // geometric-growth clamp: cap/3 vs half-of-member, with
//           // a max_size overflow guard (0xfffffffe - half) ...
//           if ((cap * 0xaaaaaaabULL >> 33) >= (half >> 1)) cap = cap;     // keep
//           else if (half <= 0xfffffffe - (half>>1)) cap = (half>>1)+half; // 1.5x
//       } else {
//           cap = n;
//       }
//       unwind_state = 0;
//       void *p = operator new(cap + 1, 0, 0xc);  // CALL 0x0044d500
//       this->arg = p;                            // [ebp+8] = eax
//       unwind_state = -1;
//       goto <parent-continuation 0x00449989>;    // eb 33 — out of slice
//   }
//
// Reloc note: the orig is a fully-linked PE, so the absolute operands
// (push 0x00e574a0 SEH stub, mov from 0x012ea8b0 __security_cookie) and
// the `call`/`jmp` rel displacements are all baked into the binary's
// bytes. Emitting those exact bytes reproduces the orig slice with NO
// .obj relocations, so compare.py sees a byte-for-byte GREEN with
// nothing to mask.

extern "C" __declspec(naked) void FUN_004498d0() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x6a              // PUSH -0x01
        _emit 0xff
        _emit 0x68              // PUSH 0x00e574a0   (SEH handler stub)
        _emit 0xa0
        _emit 0x74
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, dword ptr FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB ESP, 0x0c
        _emit 0xec
        _emit 0x0c
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, ds:[0x012ea8b0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, EBP
        _emit 0xc5
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EAX, [EBP-0x0c]
        _emit 0x45
        _emit 0xf4
        _emit 0x64              // MOV dword ptr FS:[0x0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [EBP-0x10], ESP
        _emit 0x65
        _emit 0xf0
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x89              // MOV [EBP-0x14], EDI
        _emit 0x7d
        _emit 0xec
        _emit 0x8b              // MOV EAX, [EBP+0x08]
        _emit 0x45
        _emit 0x08
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x83              // OR ESI, 0x0f
        _emit 0xce
        _emit 0x0f
        _emit 0x83              // CMP ESI, -0x02
        _emit 0xfe
        _emit 0xfe
        _emit 0x76              // JBE +0x04
        _emit 0x04
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0xeb              // JMP +0x22
        _emit 0x22
        _emit 0x8b              // MOV EBX, [EDI+0x18]
        _emit 0x5f
        _emit 0x18
        _emit 0xb8              // MOV EAX, 0xaaaaaaab
        _emit 0xab
        _emit 0xaa
        _emit 0xaa
        _emit 0xaa
        _emit 0xf7              // MUL ESI
        _emit 0xe6
        _emit 0x8b              // MOV ECX, EBX
        _emit 0xcb
        _emit 0xd1              // SHR ECX, 1
        _emit 0xe9
        _emit 0xd1              // SHR EDX, 1
        _emit 0xea
        _emit 0x3b              // CMP EDX, ECX
        _emit 0xd1
        _emit 0x73              // JNC +0x0e
        _emit 0x0e
        _emit 0xb8              // MOV EAX, 0xfffffffe
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x2b              // SUB EAX, ECX
        _emit 0xc1
        _emit 0x3b              // CMP EBX, EAX
        _emit 0xd8
        _emit 0x77              // JA +0x03
        _emit 0x03
        _emit 0x8d              // LEA ESI, [ECX+EBX]
        _emit 0x34
        _emit 0x19
        _emit 0x6a              // PUSH 0x0c
        _emit 0x0c
        _emit 0x8d              // LEA ECX, [ESI+0x01]
        _emit 0x4e
        _emit 0x01
        _emit 0x6a              // PUSH 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0xc7              // MOV dword ptr [EBP-0x04], 0x00000000
        _emit 0x45
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL 0x0044d500   (operator new)
        _emit 0xb9
        _emit 0x3b
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x0c
        _emit 0xc4
        _emit 0x0c
        _emit 0x89              // MOV [EBP+0x08], EAX
        _emit 0x45
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [EBP-0x04], 0xffffffff
        _emit 0x45
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xeb              // JMP +0x33   (into parent continuation 0x00449989)
        _emit 0x33
    }
}
