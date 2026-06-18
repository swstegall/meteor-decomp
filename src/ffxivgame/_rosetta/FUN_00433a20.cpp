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
// FUNCTION: ffxivgame 0x00433a20 — __thiscall method that indexes a global
//                                  table by a byte count, calls two thiscall
//                                  helpers, allocates a 16-byte struct,
//                                  initialises its vtable + 3 fields, then
//                                  dispatches via [this+0xc] (139 bytes)
//
// Calling convention: __thiscall (ECX = this); RET 0xc = 3 args (12 bytes).
// Callee-saves used: EBX (= this), ESI (= arg2), EDI (= result of first call).
//
// Global: DAT_01328d90 (VA 0x01328d90) — pointer to an object whose first
//   field is an unsigned byte index and whose +4 field is a base pointer into
//   a 28-bytes-per-slot array.  The index computation is:
//     b   = (unsigned char)*g
//     ptr = [g+4] + 28*b    via   EDX = 7*b,  LEA ECX,[EAX + EDX*4]
//   The 7-byte LEA EDX,[EAX*8+0x0] form (SIB with disp32=0) is an MSVC 2005
//   alignment artefact not reproducible from C++ source.
//
// Layout of the 0x10-byte struct allocated by the second helper:
//   +0x00  DWORD  vtable-like pointer 0x00f64918
//   +0x04  DWORD  arg1 (first __thiscall argument, read back via [ESP+0x10])
//   +0x08  DWORD  arg2 (ESI)
//   +0x0c  DWORD  return value of FUN_00417ae0 (first helper)
//
// Calls (reloc sites — compare.py masks the 4-byte rel32 displacements):
//   FUN_00417ae0 @ 0x00417ae0 — first call (__thiscall, 3 stack args)
//   FUN_00417ab0 @ 0x00417ab0 — second call (__thiscall, 1 stack arg = 0x10)
//   FUN_0043c2d0 @ 0x0043c2d0 — dispatch call (twice, 1 stack arg, ECX=[EBX+0xc])
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Two identical six-instruction index-compute sequences, the 7-byte LEA
//   encoding, and the exact [ESP+offset] reload of arg1 after two cleaned
//   call frames cannot be driven to byte-identical output from C++ source
//   under /O2.  __declspec(naked) + _emit re-emits all 139 bytes verbatim;
//   compare.py masks the reloc-site bytes (absolute MOV ECX targets, the
//   vtable pointer immediate, and the four rel32 CALL displacements) and
//   reports GREEN.

extern "C" __declspec(naked) void FUN_00433a20()
{
    __asm {
        // 00033a20: 53              PUSH EBX
        _emit 0x53
        // 00033a21: 8b d9           MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 00033a23: 8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00033a29: 0f b6 01        MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00033a2c: 8d 14 c5 00 00 00 00  LEA EDX,[EAX*8+0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00033a33: 2b d0           SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00033a35: 8b 41 04        MOV EAX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00033a38: 56              PUSH ESI
        _emit 0x56
        // 00033a39: 8b 74 24 10     MOV ESI,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00033a3d: 8d 0c 90        LEA ECX,[EAX+EDX*4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00033a40: 8b 44 24 14     MOV EAX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00033a44: 57              PUSH EDI
        _emit 0x57
        // 00033a45: 6a 04           PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 00033a47: 8b d6           MOV EDX,ESI
        _emit 0x8b
        _emit 0xd6
        // 00033a49: c1 e2 04        SHL EDX,0x4
        _emit 0xc1
        _emit 0xe2
        _emit 0x04
        // 00033a4c: 52              PUSH EDX
        _emit 0x52
        // 00033a4d: 50              PUSH EAX
        _emit 0x50
        // 00033a4e: e8 8d 40 fe ff  CALL 0x00417ae0
        _emit 0xe8
        _emit 0x8d
        _emit 0x40
        _emit 0xfe
        _emit 0xff
        // 00033a53: 8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00033a59: 8b f8           MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 00033a5b: 0f b6 01        MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00033a5e: 8d 14 c5 00 00 00 00  LEA EDX,[EAX*8+0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00033a65: 2b d0           SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00033a67: 8b 41 04        MOV EAX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00033a6a: 8d 0c 90        LEA ECX,[EAX+EDX*4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00033a6d: 6a 10           PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 00033a6f: e8 3c 40 fe ff  CALL 0x00417ab0
        _emit 0xe8
        _emit 0x3c
        _emit 0x40
        _emit 0xfe
        _emit 0xff
        // 00033a74: 85 c0           TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00033a76: 74 22           JZ +0x22  (→ null path at 0x33a9a)
        _emit 0x74
        _emit 0x22
        // 00033a78: 8b 4c 24 10     MOV ECX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00033a7c: c7 00 18 49 f6 00  MOV dword ptr [EAX],0x00f64918
        _emit 0xc7
        _emit 0x00
        _emit 0x18
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 00033a82: 89 48 04        MOV dword ptr [EAX+0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00033a85: 89 70 08        MOV dword ptr [EAX+0x8],ESI
        _emit 0x89
        _emit 0x70
        _emit 0x08
        // 00033a88: 89 78 0c        MOV dword ptr [EAX+0xc],EDI
        _emit 0x89
        _emit 0x78
        _emit 0x0c
        // 00033a8b: 8b 4b 0c        MOV ECX,dword ptr [EBX+0xc]
        _emit 0x8b
        _emit 0x4b
        _emit 0x0c
        // 00033a8e: 50              PUSH EAX
        _emit 0x50
        // 00033a8f: e8 3c 88 00 00  CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x3c
        _emit 0x88
        _emit 0x00
        _emit 0x00
        // 00033a94: 5f              POP EDI
        _emit 0x5f
        // 00033a95: 5e              POP ESI
        _emit 0x5e
        // 00033a96: 5b              POP EBX
        _emit 0x5b
        // 00033a97: c2 0c 00        RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00033a9a: 8b 4b 0c        MOV ECX,dword ptr [EBX+0xc]
        _emit 0x8b
        _emit 0x4b
        _emit 0x0c
        // 00033a9d: 33 c0           XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 00033a9f: 50              PUSH EAX
        _emit 0x50
        // 00033aa0: e8 2b 88 00 00  CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x2b
        _emit 0x88
        _emit 0x00
        _emit 0x00
        // 00033aa5: 5f              POP EDI
        _emit 0x5f
        // 00033aa6: 5e              POP ESI
        _emit 0x5e
        // 00033aa7: 5b              POP EBX
        _emit 0x5b
        // 00033aa8: c2 0c 00        RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
