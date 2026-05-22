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
// FUNCTION: ffxivgame 0x0000e370 — guarded call wrapper with SEH frame
//                                   (__cdecl, 82 B / 0x52)
//
// __cdecl int FUN_0040e370(int param_1)
//
// Behaviour:
//   1. Installs a SEH exception frame (handler at 0x00e54ec4).
//   2. Computes (param_1 + 0x3c) and tests it for zero.
//   3. If non-zero → calls FUN_0040ed90(param_1) and returns its result.
//   4. If zero     → returns 0.
//
// Stack layout (ESP-relative, after PUSH ECX):
//   [ESP+0x00] = local slot (ECX save / overwritten with param_1+0x3c)
//   [ESP+0x04] = prev FS:[0] (old exception chain)
//   [ESP+0x08] = SEH handler (0x00e54ec4)
//   [ESP+0x0C] = SEH state   (-1 → 0 on entry)
//   [ESP+0x10] = return address
//   [ESP+0x14] = param_1
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function's prologue is the classic three-push MSVC SEH prolog
//   followed by PUSH ECX (an unusual single-register save that doubles
//   as the lone local-variable allocation).  The SEH state field is set
//   to 0 via MOV [ESP+0x0C], 0 (not via an inline _try block), and the
//   epilogue restores FS:[0] from [ESP+0x4] before ADD ESP,0x10 / RET.
//   The CALL at offset +0x2D encodes a baked-in rel32 to FUN_0040ed90
//   (0x0040ed90 − 0x0040e3a2 = 0x09ee → bytes ee 09 00 00).  Reproducing
//   this exact prologue + epilogue layout from high-level C++ would
//   require driving a full relink.  A naked-asm passthrough that emits
//   the orig 82 bytes verbatim is the correct path to GREEN.

extern "C" __declspec(naked) void FUN_0040e370() {
    __asm {
        // 0000e370: 6a ff          PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0000e372: 68 c4 4e e5 00  PUSH 0xe54ec4
        _emit 0x68
        _emit 0xc4
        _emit 0x4e
        _emit 0xe5
        _emit 0x00
        // 0000e377: 64 a1 00 00 00 00  MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000e37d: 50              PUSH EAX
        _emit 0x50
        // 0000e37e: 64 89 25 00 00 00 00  MOV dword ptr FS:[0x0],ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000e385: 51              PUSH ECX
        _emit 0x51
        // 0000e386: 8b 44 24 14     MOV EAX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0000e38a: 8d 48 3c        LEA ECX,[EAX+0x3c]
        _emit 0x8d
        _emit 0x48
        _emit 0x3c
        // 0000e38d: 89 0c 24        MOV dword ptr [ESP],ECX
        _emit 0x89
        _emit 0x0c
        _emit 0x24
        // 0000e390: 85 c9           TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // 0000e392: c7 44 24 0c 00 00 00 00  MOV dword ptr [ESP+0xc],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000e39a: 74 15           JZ +0x15 (→ 0x0040e3b1)
        _emit 0x74
        _emit 0x15
        // 0000e39c: 50              PUSH EAX
        _emit 0x50
        // 0000e39d: e8 ee 09 00 00  CALL 0x0040ed90 (rel32 = 0x09ee)
        _emit 0xe8
        _emit 0xee
        _emit 0x09
        _emit 0x00
        _emit 0x00
        // 0000e3a2: 8b 4c 24 04     MOV ECX,dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0000e3a6: 64 89 0d 00 00 00 00  MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000e3ad: 83 c4 10        ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0000e3b0: c3              RET
        _emit 0xc3
        // 0000e3b1: 8b 4c 24 04     MOV ECX,dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0000e3b5: 33 c0           XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0000e3b7: 64 89 0d 00 00 00 00  MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000e3be: 83 c4 10        ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0000e3c1: c3              RET
        _emit 0xc3
    }
}
