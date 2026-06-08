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
// FUNCTION: ffxivgame 0x00024c40 — singleton-guarded dispatch thunk (49 B / 0x31)
//
// Calling convention: __cdecl — no prologue, bare RET, six DWORD/float stack
// parameters accessed via [ESP+0x10..0x18] (args 4-6); args 1-3 are accessed
// lazily after the stack grows during the inner PUSH sequence.
//
// Behaviour:
//   void __cdecl FUN_00424c40(int a1, int a2, int a3, int a4, int a5, float a6)
//   {
//       void *obj = *(void **)0x0132c9d0;   // global singleton pointer
//       if (obj)                             // NULL guard
//           FUN_00439ed0(a3, a1, a2, a4, a5, a6);   // ECX = obj (__thiscall)
//   }
//
// Two non-trivial lowering choices prevent clean source-level reconstruction:
//
//   1. Float-push trick — MSVC pushes float a6 (a memory operand) via:
//         FLD float ptr [ESP+0x18]   ; load into ST(0)
//         PUSH ECX                   ; make room (1-byte opcode vs sub esp,4)
//         FSTP float ptr [ESP]       ; store ST(0) at [ESP], overwriting ECX
//      ECX (global obj) survives in the register for the __thiscall dispatch.
//
//   2. Arg-order reload — the compiler pre-loads a5 into EAX and a4 into EDX
//      before the first PUSH, then re-reads a2 and a1 from the (shifted) stack
//      with MOV EAX/EDX,[ESP+0x10] after two and three pushes respectively,
//      and finally re-reads a3 via [ESP+0x1c] after four pushes. Replicating
//      this exact register scheduling via C++ is fragile across MSVC 2005
//      optimisation passes, so the byte-passthrough approach is used instead.
//
//   3. No stack cleanup after CALL — FUN_00439ed0 is __thiscall or __stdcall
//      and cleans its own 24 bytes of stack args; the outer function therefore
//      falls directly to RET without an ADD ESP,N.
//
// Relocation sites in the 49-byte sequence:
//   +0x01  MOV imm32 → 0x0132c9d0       (global singleton pointer slot)
//   +0x2c  CALL rel32 → FUN_00439ed0    (compare.py masks these bytes)

extern "C" __declspec(naked) void FUN_00424c40() {
    __asm {
        // 00024c40: 8b 0d d0 c9 32 01   MOV ECX, dword ptr [0x0132c9d0]
        _emit 0x8b
        _emit 0x0d
        _emit 0xd0
        _emit 0xc9
        _emit 0x32
        _emit 0x01
        // 00024c46: 85 c9               TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 00024c48: 74 26               JZ +0x26 (→ RET at 00024c70)
        _emit 0x74
        _emit 0x26
        // 00024c4a: d9 44 24 18         FLD float ptr [ESP+0x18]
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 00024c4e: 8b 44 24 14         MOV EAX, dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00024c52: 8b 54 24 10         MOV EDX, dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 00024c56: 51                  PUSH ECX
        _emit 0x51
        // 00024c57: d9 1c 24            FSTP float ptr [ESP]
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // 00024c5a: 50                  PUSH EAX
        _emit 0x50
        // 00024c5b: 8b 44 24 10         MOV EAX, dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00024c5f: 52                  PUSH EDX
        _emit 0x52
        // 00024c60: 8b 54 24 10         MOV EDX, dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 00024c64: 50                  PUSH EAX
        _emit 0x50
        // 00024c65: 8b 44 24 1c         MOV EAX, dword ptr [ESP+0x1c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00024c69: 52                  PUSH EDX
        _emit 0x52
        // 00024c6a: 50                  PUSH EAX
        _emit 0x50
        // 00024c6b: e8 60 52 01 00      CALL FUN_00439ed0 (rel32)
        _emit 0xe8
        _emit 0x60
        _emit 0x52
        _emit 0x01
        _emit 0x00
        // 00024c70: c3                  RET
        _emit 0xc3
    }
}
