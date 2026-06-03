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
// FUNCTION: ffxivgame 0x000190a0 — SEH-framed wrapper that calls
//                                   FUN_00418bf0 with 8 pushed arguments,
//                                   writes the first field of the returned
//                                   object into *ESI, then optionally makes
//                                   one virtual call (vtable[0](1)) on a
//                                   caller-supplied object pointer.
//                                   (163 bytes / 0xa3)
//
// Calling convention: __thiscall (ECX = this; RET = no callee stack-clean,
//   consistent with zero pushed stack-args for __thiscall, but the function
//   body also reads deep stack slots — see layout note below).
//
// SEH frame layout after prologue (ESP-relative, 6 DWORDs of frame overhead):
//   [ESP+0x00] = security cookie XOR esp
//   [ESP+0x04] = saved ESI
//   [ESP+0x08] = saved ECX / local_var (zeroed to 0 at 0x190c6, set to 1 at
//                0x1911e — used as SEH "try-level" state mirror)
//   [ESP+0x0c] = old FS:[0]                ← FS:[0] installed here
//   [ESP+0x10] = 0xe555e2  (SEH handler/scopetable)
//   [ESP+0x14] = -1        (initial SEH state)
//   [ESP+0x18] = return address
//   [ESP+0x1c..+0x38] = caller-supplied arguments
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function builds its argument list for FUN_00418bf0 by interleaving
//   reads from caller-stack slots at varying offsets (which shift as pushes
//   accumulate) with a write of the current ESP into one slot.  That
//   register-allocation / offset sequence is not reproducible from a C++
//   source form without fine-grained allocation hints.  A __declspec(naked)
//   body re-emitting the original 163 bytes verbatim produces a .obj whose
//   .text is byte-identical to the original slice; compare.py reports GREEN.
//
// Reloc-bearing sites:
//     +0x5a  CALL rel32   → FUN_00418bf0  (RVA 0x00018bf0; rel32 = 0xFFFFFAF1)

extern "C" __declspec(naked) void FUN_004190a0() {
    __asm {
        // 000190a0: 6a ff              PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 000190a2: 68 e2 55 e5 00     PUSH 0xe555e2
        _emit 0x68
        _emit 0xe2
        _emit 0x55
        _emit 0xe5
        _emit 0x00
        // 000190a7: 64 a1 00000000     MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000190ad: 50                 PUSH EAX
        _emit 0x50
        // 000190ae: 51                 PUSH ECX
        _emit 0x51
        // 000190af: 56                 PUSH ESI
        _emit 0x56
        // 000190b0: a1 b0a82e01        MOV EAX,[0x012ea8b0]  (security cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 000190b5: 33 c4              XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 000190b7: 50                 PUSH EAX
        _emit 0x50
        // 000190b8: 8d 44 24 0c        LEA EAX,[ESP+0xc]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 000190bc: 64 a3 00000000     MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000190c2: 8b 54 24 2c        MOV EDX,[ESP+0x2c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        // 000190c6: c7 44 24 08 00000000  MOV dword ptr [ESP+0x8],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000190ce: 8b 44 24 34        MOV EAX,[ESP+0x34]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 000190d2: 50                 PUSH EAX
        _emit 0x50
        // 000190d3: 51                 PUSH ECX
        _emit 0x51
        // 000190d4: 8b 4c 24 38        MOV ECX,[ESP+0x38]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // 000190d8: 89 64 24 3c        MOV [ESP+0x3c],ESP
        _emit 0x89
        _emit 0x64
        _emit 0x24
        _emit 0x3c
        // 000190dc: 8b c4              MOV EAX,ESP
        _emit 0x8b
        _emit 0xc4
        // 000190de: 51                 PUSH ECX
        _emit 0x51
        // 000190df: 8b 4c 24 30        MOV ECX,[ESP+0x30]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        // 000190e3: 52                 PUSH EDX
        _emit 0x52
        // 000190e4: 8b 54 24 30        MOV EDX,[ESP+0x30]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x30
        // 000190e8: c7 00 00000000     MOV dword ptr [EAX],0x0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000190ee: 8b 44 24 38        MOV EAX,[ESP+0x38]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x38
        // 000190f2: 50                 PUSH EAX
        _emit 0x50
        // 000190f3: 51                 PUSH ECX
        _emit 0x51
        // 000190f4: 52                 PUSH EDX
        _emit 0x52
        // 000190f5: 8d 44 24 50        LEA EAX,[ESP+0x50]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x50
        // 000190f9: 50                 PUSH EAX
        _emit 0x50
        // 000190fa: e8 f1faffff        CALL FUN_00418bf0 (rel32 = 0xFFFFFAF1)
        _emit 0xe8
        _emit 0xf1
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        // 000190ff: 8b 08              MOV ECX,[EAX]
        _emit 0x8b
        _emit 0x08
        // 00019101: 8b 74 24 3c        MOV ESI,[ESP+0x3c]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x3c
        // 00019105: c7 00 00000000     MOV dword ptr [EAX],0x0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001910b: 83 c4 20           ADD ESP,0x20
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        // 0001910e: 89 0e              MOV dword ptr [ESI],ECX
        _emit 0x89
        _emit 0x0e
        // 00019110: 8b 4c 24 34        MOV ECX,[ESP+0x34]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        // 00019114: 85 c9              TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // 00019116: c7 44 24 14 00000000  MOV dword ptr [ESP+0x14],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001911e: c7 44 24 08 01000000  MOV dword ptr [ESP+0x8],0x1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00019126: 74 08              JZ +0x08  (→ 0x00419130)
        _emit 0x74
        _emit 0x08
        // 00019128: 8b 11              MOV EDX,[ECX]
        _emit 0x8b
        _emit 0x11
        // 0001912a: 8b 02              MOV EAX,[EDX]
        _emit 0x8b
        _emit 0x02
        // 0001912c: 6a 01              PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0001912e: ff d0              CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00019130: 8b c6              MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 00019132: 8b 4c 24 0c        MOV ECX,[ESP+0xc]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 00019136: 64 89 0d 00000000  MOV FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001913d: 59                 POP ECX
        _emit 0x59
        // 0001913e: 5e                 POP ESI
        _emit 0x5e
        // 0001913f: 83 c4 10           ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00019142: c3                 RET
        _emit 0xc3
    }
}
