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
// FUNCTION: ffxivgame 0x00423ce0 — __thiscall 4-DWORD setter using
//                                  16-byte-aligned SSE2 store (53 bytes / 0x35).
//
// Layout (inferred from the asm):
//   this (ECX): pointer to a 16-byte-aligned structure whose first 16 bytes
//               are overwritten by the four DWORD parameters.
//
// Source shape:
//
//   void Foo::Set(int a, int b, int c, int d) {
//       struct { int x, y, z, w; } __declspec(align(16)) tmp = { a, b, c, d };
//       *reinterpret_cast<__m128i *>(this) = *reinterpret_cast<__m128i *>(&tmp);
//   }
//
// Calling convention: __thiscall (ECX = this; four DWORD stack args;
// callee cleans 0x10 via `ret 0x10`).
//
// Frame:
//   PUSH EBP
//   MOV  EBP, ESP
//   AND  ESP, 0xfffffff0        ; align stack to 16 bytes for MOVDQA
//   SUB  ESP, 0x10              ; 16-byte aligned scratch storage
//
// SSE2 instructions used:
//   MOVDQA XMM0, xmmword ptr [ESP]   ; load 16 bytes from aligned scratch
//   MOVDQA xmmword ptr [EAX], XMM0  ; store 16 bytes to *this (EAX=ECX saved)
//
// _emit passthrough: no relocations in this function, and MASM 8.0 inline
// assembler SSE2 encoding is not guaranteed to match the orig wire bytes for
// MOVDQA. All 53 bytes are emitted verbatim.
//
// Byte layout:
//   +0x00  55              PUSH EBP
//   +0x01  8b ec           MOV  EBP, ESP
//   +0x03  83 e4 f0        AND  ESP, 0xfffffff0
//   +0x06  83 ec 10        SUB  ESP, 0x10
//   +0x09  8b 55 0c        MOV  EDX, [EBP+0xc]
//   +0x0c  8b c1           MOV  EAX, ECX
//   +0x0e  8b 4d 08        MOV  ECX, [EBP+0x8]
//   +0x11  89 0c 24        MOV  [ESP], ECX
//   +0x14  8b 4d 10        MOV  ECX, [EBP+0x10]
//   +0x17  89 54 24 04     MOV  [ESP+0x4], EDX
//   +0x1b  8b 55 14        MOV  EDX, [EBP+0x14]
//   +0x1e  89 4c 24 08     MOV  [ESP+0x8], ECX
//   +0x22  89 54 24 0c     MOV  [ESP+0xc], EDX
//   +0x26  66 0f 6f 04 24  MOVDQA XMM0, [ESP]
//   +0x2b  66 0f 7f 00     MOVDQA [EAX], XMM0
//   +0x2f  8b e5           MOV  ESP, EBP
//   +0x31  5d              POP  EBP
//   +0x32  c2 10 00        RET  0x10

extern "C" __declspec(naked) void FUN_00423ce0() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x83              // AND ESP, 0xfffffff0
        _emit 0xe4
        _emit 0xf0
        _emit 0x83              // SUB ESP, 0x10
        _emit 0xec
        _emit 0x10
        _emit 0x8b              // MOV EDX, [EBP+0xc]
        _emit 0x55
        _emit 0x0c
        _emit 0x8b              // MOV EAX, ECX
        _emit 0xc1
        _emit 0x8b              // MOV ECX, [EBP+0x8]
        _emit 0x4d
        _emit 0x08
        _emit 0x89              // MOV [ESP], ECX
        _emit 0x0c
        _emit 0x24
        _emit 0x8b              // MOV ECX, [EBP+0x10]
        _emit 0x4d
        _emit 0x10
        _emit 0x89              // MOV [ESP+0x4], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV EDX, [EBP+0x14]
        _emit 0x55
        _emit 0x14
        _emit 0x89              // MOV [ESP+0x8], ECX
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x89              // MOV [ESP+0xc], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x66              // MOVDQA XMM0, xmmword ptr [ESP]
        _emit 0x0f
        _emit 0x6f
        _emit 0x04
        _emit 0x24
        _emit 0x66              // MOVDQA xmmword ptr [EAX], XMM0
        _emit 0x0f
        _emit 0x7f
        _emit 0x00
        _emit 0x8b              // MOV ESP, EBP
        _emit 0xe5
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x10
        _emit 0x10
        _emit 0x00
    }
}
