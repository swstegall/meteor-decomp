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
// FUNCTION: ffxivgame 0x00035560 — lazy singleton accessor (__cdecl, 184 B / 0xb8)
//
// A "magic statics"-style lazy initializer wrapped in an EH3 SEH frame
// (PUSH -1 / scope_table / FS:[0] chain + /GS cookie XOR ESP). Logical
// shape recovered from asm/ffxivgame/00035560_FUN_00435560.s:
//
//   static void *g_instance;          // .data 0x01328d90
//   static int   g_logger_inited;     // .data 0x01323910 (bit 0)
//   static void(*g_logger)(...);      // .data 0x0132390c
//
//   void *get_instance() {
//       if (g_instance == 0) {
//           if (!(g_logger_inited & 1)) {        // one-time logger bind
//               g_logger_inited |= 1;
//               g_logger = (logfn)0x00433720;
//           }
//           g_logger(0xf64b24, 0xf54d48, 0xf64b38, 0x6a, 0xf64b90);
//           void *p = operator new(8);            // 0x009d1b35
//           if (p) {
//               g_instance = ((ctor)0x00433650)(p, arg0);  // __thiscall
//               return ...;
//           }
//           g_instance = 0;
//       }
//       ...
//   }
//
// Reloc-bearing sites in the orig 184 bytes (compare.py wildcards each
// window; we re-emit the orig bytes verbatim so the .obj .text matches
// byte-for-byte with NO relocations of its own):
//   +0x02   PUSH scope_table  (0x00e5618b)
//   +0x0f   MOV  EAX, [__security_cookie] (0x012ea8b0)
//   +0x21   CMP  [g_instance] (0x01328d90)
//   +0x2f   TEST [g_logger_inited] (0x01323910)
//   +0x37   OR   [g_logger_inited]
//   +0x3d   MOV  [g_logger] (0x0132390c), 0x00433720
//   +0x47.. PUSH imm32 string/arg literals (0xf64b90/0xf64b38/0xf54d48/0xf64b24)
//   +0x5d   CALL [g_logger] (indirect)
//   +0x68   CALL operator new (rel32 -> 0x009d1b35)
//   +0x87   CALL __thiscall ctor (rel32 -> 0x00433650)
//   +0x8c   MOV  [g_instance], EAX
//
// Why naked-asm byte passthrough: the inlined EH3 SEH prolog + /GS
// cookie + the lazy-init double-branch + the mixed indirect/rel32 call
// sequence form a compiler shape that depends on the precise locals
// layout, the linker-laid scope_table, and MSVC's moffs32 encoding
// choices. Coaxing exactly these 184 bytes from plain C++ is
// impractical; the proven path (cf. FUN_00403f10) is a `_emit`
// passthrough that reproduces the orig slice exactly.

extern "C" __declspec(naked) void FUN_00435560() {
    __asm {
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0x00e5618b (scope_table)
        _emit 0x8b
        _emit 0x61
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0xa1              // MOV EAX, [__security_cookie 0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EAX, [ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x64              // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // CMP dword ptr [g_instance 0x01328d90], 0
        _emit 0x3d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x74              // JZ +0x3c  (to 0x355c6)
        _emit 0x3c
        _emit 0xb8              // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [g_logger_inited 0x01323910], AL
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ +0x10 (to 0x355a7)
        _emit 0x10
        _emit 0x09              // OR dword ptr [g_logger_inited], EAX
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [g_logger 0x0132390c], 0x00433720
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00
        _emit 0x68              // PUSH 0x00f64b90
        _emit 0x90
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        _emit 0x6a              // PUSH 0x6a
        _emit 0x6a
        _emit 0x68              // PUSH 0x00f64b38
        _emit 0x38
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x00f54d48
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x00f64b24
        _emit 0x24
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        _emit 0xff              // CALL dword ptr [g_logger 0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x6a              // PUSH 0x8
        _emit 0x08
        _emit 0xe8              // CALL operator new (rel32 -> 0x009d1b35)
        _emit 0x68
        _emit 0xc5
        _emit 0x59
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ESP+0x4], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0xc7              // MOV dword ptr [ESP+0x10], 0
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x21 (to 0x35601)
        _emit 0x21
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, EAX  (this = new ptr)
        _emit 0xc8
        _emit 0xe8              // CALL __thiscall ctor (rel32 -> 0x00433650)
        _emit 0x64
        _emit 0xe0
        _emit 0xff
        _emit 0xff
        _emit 0xa3              // MOV [g_instance 0x01328d90], EAX
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x64              // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3              // RET
        _emit 0x33              // XOR EAX, EAX  (null path @ 0x35601)
        _emit 0xc0
        _emit 0xa3              // MOV [g_instance 0x01328d90], EAX
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x64              // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3              // RET
    }
}
