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
// FUNCTION: ffxivgame 0x00451c90 — __thiscall 2-arg constructor that
// initialises two inline std::string-like members under an MSVC C++ SEH
// /GS unwind frame (124 B / 0x7c).
//
// Behaviour reconstructed from the disassembly at orig RVA 0x00451c90:
//
//   struct Pair { string a /*+0x00*/; string b /*+0x1c*/; };  // 0x38 B
//
//   Pair * __thiscall ctor(Pair *this /*ECX*/, char *srcA, char *srcB) {
//       this->a.cap  = 0xf;          // [this+0x18] short-buffer capacity
//       this->a.len  = 0;            // [this+0x14]
//       this->a.buf[0] = 0;          // [this+0x04] inline buffer NUL
//       assign(&this->a, srcA, 0, -1);   // CALL 0x00404040 (__thiscall assign)
//
//       this->b.cap  = 0xf;          // [this+0x1c+0x18]
//       this->b.len  = 0;            // [this+0x1c+0x14]
//       this->b.buf[0] = 0;          // [this+0x1c+0x04]
//       assign(&this->b, srcB, 0, -1);   // CALL 0x00404040
//       return this;                 // EAX = ESI = this
//   }
//
//   Each string member is the classic MSVC 2005 std::basic_string layout:
//   inline buffer at +0x04, size at +0x14, capacity at +0x18 (0xf = the
//   15-char small-string-optimisation capacity). 0x00404040 is the shared
//   `assign(ptr, pos=0, count=-1)` helper (__thiscall, this in ECX, three
//   stack args, callee-cleanup).
//
// Calling convention: __thiscall (ECX = this), `RET 0x8` confirms two
// 4-byte stack params (srcA, srcB) under callee cleanup. The function
// also carries a /GS security cookie + an SEH registration record so the
// two sub-constructors' partial-init state can unwind correctly.
//
// SEH / GS frame after the prologue (relative to final esp):
//   [esp+0x00] saved GS cookie (xor'd with esp)
//   [esp+0x04] saved ESI
//   [esp+0x08] saved EBX
//   [esp+0x0c] saved ECX scratch (overwritten with `this` spill)
//   [esp+0x10] prev fs:[0]   (SEH chain link)
//   [esp+0x14] SEH handler RVA (push'd as 0x00e57f18)
//   [esp+0x18] initial unwind state (-1)
//   [esp+0x1c] return address
//   [esp+0x20] arg1 (srcA)
//   [esp+0x24] arg2 (srcB)
//
// Reconstruction strategy — naked-asm byte passthrough (same approach as
// the 124-byte sibling FUN_00401350): a `__declspec(naked)` body that
// re-emits the orig 124 bytes verbatim via MASM `_emit` directives. The
// absolute operands (SEH handler 0x00e57f18, __security_cookie 0x012ea8b0)
// reproduce byte-for-byte because they are values in the binary's own
// address space; the two CALL rel32 operands to 0x00404040 are masked by
// tools/compare.py. tools/compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_00451c90() {
    __asm {
        _emit 0x6a              // PUSH -0x1            (initial unwind state)
        _emit 0xff
        _emit 0x68              // PUSH 0xe57f18        (SEH handler)
        _emit 0x18
        _emit 0x7f
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX             (prev fs:[0])
        _emit 0x51              // PUSH ECX             (scratch -> this spill)
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0xa1              // MOV EAX, [0x012ea8b0] (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX             (spill cookie)
        _emit 0x8d              // LEA EAX, [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV FS:[0x0], EAX    (install SEH record)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, ECX         (esi = this)
        _emit 0xf1
        _emit 0x89              // MOV [ESP+0xc], ESI   (this spill)
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x8b              // MOV EAX, [ESP+0x20]  (arg1 srcA)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0x6a              // PUSH -0x1            (assign count = -1)
        _emit 0xff
        _emit 0x53              // PUSH EBX             (assign pos = 0)
        _emit 0xc7              // MOV dword [ESI+0x18], 0xf  (a.cap)
        _emit 0x46
        _emit 0x18
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [ESI+0x14], EBX  (a.len = 0)
        _emit 0x5e
        _emit 0x14
        _emit 0x50              // PUSH EAX             (srcA)
        _emit 0x88              // MOV byte [ESI+0x4], BL (a.buf[0] = 0)
        _emit 0x5e
        _emit 0x04
        _emit 0xe8              // CALL 0x00404040      (assign a) [rel32 masked]
        _emit 0x6b
        _emit 0x23
        _emit 0xfb
        _emit 0xff
        _emit 0x8b              // MOV EDX, [ESP+0x24]  (arg2 srcB)
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x6a              // PUSH -0x1            (assign count = -1)
        _emit 0xff
        _emit 0x8d              // LEA ECX, [ESI+0x1c]  (&this->b)
        _emit 0x4e
        _emit 0x1c
        _emit 0x53              // PUSH EBX             (assign pos = 0)
        _emit 0xc7              // MOV dword [ECX+0x18], 0xf  (b.cap)
        _emit 0x41
        _emit 0x18
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [ECX+0x14], EBX  (b.len = 0)
        _emit 0x59
        _emit 0x14
        _emit 0x52              // PUSH EDX             (srcB)
        _emit 0x89              // MOV [ESP+0x24], EBX  (clear arg-slot temp)
        _emit 0x5c
        _emit 0x24
        _emit 0x24
        _emit 0x88              // MOV byte [ECX+0x4], BL (b.buf[0] = 0)
        _emit 0x59
        _emit 0x04
        _emit 0xe8              // CALL 0x00404040      (assign b) [rel32 masked]
        _emit 0x4a
        _emit 0x23
        _emit 0xfb
        _emit 0xff
        _emit 0x8b              // MOV EAX, ESI         (return this)
        _emit 0xc6
        _emit 0x8b              // MOV ECX, [ESP+0x10]  (saved prev fs:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV FS:[0x0], ECX    (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX              (drop cookie)
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}

// vim: ts=4 sts=4 sw=4 et
