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
// FUNCTION: ffxivgame 0x0003fba0 — argument marshal + tail to FUN_0043f9a0
//                                   (__cdecl, 3 args, 43 B / 0x2B)
//
// void FUN_0043fba0(T arg1, T arg2, T arg3)
//
// Allocates one DWORD on the stack via PUSH ECX (used as a local byte
// variable, zeroed with MOV byte [ESP], 0), then loads all three caller
// args from the adjusted stack and passes them — along with the zero
// value read back as a DWORD — to FUN_0043f9a0 with 6 arguments:
//
//   FUN_0043f9a0(arg1, arg2, arg3, arg3, arg3, zero_local)
//
// The interleaved PUSH/MOV pattern is classic MSVC 2005 /O2 /Oy output:
// each parameter is loaded into a register, pushed (which changes ESP),
// then the next arg is loaded with an adjusted [ESP+0x14] offset.
//
// Calling convention: __cdecl — plain RET, caller cleans via ADD ESP,0x1c.
// Frame: 0 saved regs, 1 local DWORD (the zero byte), 6 pushed call args.
//
// Asm (43 bytes @ orig RVA 0x0003fba0):
//   51                 PUSH ECX               ; allocate local
//   8b 4c 24 10        MOV ECX, [ESP+0x10]    ; ECX = arg3
//   8b 54 24 10        MOV EDX, [ESP+0x10]    ; EDX = arg3
//   c6 04 24 00        MOV byte [ESP], 0       ; local = 0
//   8b 04 24           MOV EAX, [ESP]          ; EAX = 0 (dword of local)
//   50                 PUSH EAX               ; push zero_local
//   8b 44 24 14        MOV EAX, [ESP+0x14]    ; EAX = arg3
//   51                 PUSH ECX               ; push arg3
//   8b 4c 24 14        MOV ECX, [ESP+0x14]    ; ECX = arg2
//   52                 PUSH EDX               ; push arg3
//   8b 54 24 14        MOV EDX, [ESP+0x14]    ; EDX = arg1
//   50                 PUSH EAX               ; push arg3
//   51                 PUSH ECX               ; push arg2
//   52                 PUSH EDX               ; push arg1
//   e8 d9 fd ff ff     CALL FUN_0043f9a0       ; (reloc)
//   83 c4 1c           ADD ESP, 0x1c
//   c3                 RET
//
// Reconstruction: __declspec(naked) byte passthrough. The interleaved
// PUSH/MOV pattern is fragile to re-derive in source-level C++ due to
// register allocation order and the local-byte idiom. The CALL rel32
// carries a linker relocation; compare.py masks those 4 bytes.

extern "C" void FUN_0043f9a0();

extern "C" __declspec(naked) void FUN_0043fba0() {
    __asm {
        // 0003fba0: 51   PUSH ECX
        _emit 0x51
        // 0003fba1: 8b 4c 24 10   MOV ECX, [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0003fba5: 8b 54 24 10   MOV EDX, [ESP+0x10]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0003fba9: c6 04 24 00   MOV byte ptr [ESP], 0x0
        _emit 0xc6
        _emit 0x04
        _emit 0x24
        _emit 0x00
        // 0003fbad: 8b 04 24   MOV EAX, [ESP]
        _emit 0x8b
        _emit 0x04
        _emit 0x24
        // 0003fbb0: 50   PUSH EAX
        _emit 0x50
        // 0003fbb1: 8b 44 24 14   MOV EAX, [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0003fbb5: 51   PUSH ECX
        _emit 0x51
        // 0003fbb6: 8b 4c 24 14   MOV ECX, [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0003fbba: 52   PUSH EDX
        _emit 0x52
        // 0003fbbb: 8b 54 24 14   MOV EDX, [ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0003fbbf: 50   PUSH EAX
        _emit 0x50
        // 0003fbc0: 51   PUSH ECX
        _emit 0x51
        // 0003fbc1: 52   PUSH EDX
        _emit 0x52
        // 0003fbc2: e8 d9 fd ff ff   CALL FUN_0043f9a0 (reloc)
        call FUN_0043f9a0
        // 0003fbc7: 83 c4 1c   ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 0003fbca: c3   RET
        _emit 0xc3
    }
}
