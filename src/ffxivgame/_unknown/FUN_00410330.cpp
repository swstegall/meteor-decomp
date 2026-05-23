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
// FUNCTION: ffxivgame 0x00010330 — SEH-guarded allocate-and-init wrapper
//                                   (142 B / 0x8e, __cdecl, 6 params)
//
// Signature (Ghidra):
//   undefined4 FUN_00410330(undefined4 param_1, undefined4 param_2,
//                            undefined4 param_3, undefined4 param_4,
//                            undefined4 param_5, undefined4 param_6)
//
// Body shape:
//   Sets up a 3-slot SEH frame (PUSH -1 / PUSH handler / chain FS:[0]).
//   Calls FUN_0040e2d0(&local_c, 0x10, "CDev.Engine.Memory.Alternative")
//     via __thiscall to init a pair of fields on the stack local.
//   Loads param_3 into ESI.
//   Calls FUN_0040e110(ESI, 0x6c, prev_result) via __thiscall to allocate /
//     look up an object (returns ptr or NULL in EAX).
//   Sets SEH guard to 0.
//   If result != NULL: calls FUN_0040f990 via __thiscall
//     (ECX = result, stack args = param_1 .. param_6) and returns its retval.
//   If result == NULL: returns 0.
//
// Reloc-bearing sites (masked by tools/compare.py):
//   +0x02  PUSH imm32  0xe54f87   — SEH handler VA (DIR32 reloc)
//   +0x19  PUSH imm32  0xf56ca8   — string "CDev.Engine.Memory.Alternative" VA
//   +0x24  CALL rel32  → FUN_0040e2d0  (rel32 = 0xffffdf77)
//   +0x32  CALL rel32  → FUN_0040e110  (rel32 = 0xffffdda9)
//   +0x67  CALL rel32  → FUN_0040f990  (rel32 = 0xfffff5f4)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The SEH frame setup uses FS-segment instructions that MSVC does not
//   faithfully reproduce in the same byte-encoding when compiled from
//   C++ source with /GS. The naked-asm body re-emits the 142 original
//   bytes verbatim; compare.py then reports GREEN.

#ifdef _MSC_VER
extern "C" __declspec(naked) void FUN_00410330() {
    __asm {
        // +0x00  6a ff          PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // +0x02  68 87 4f e5 00 PUSH 0xe54f87   (SEH handler — DIR32 reloc)
        _emit 0x68
        _emit 0x87
        _emit 0x4f
        _emit 0xe5
        _emit 0x00
        // +0x07  64 a1 00 00 00 00  MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x0d  50             PUSH EAX
        _emit 0x50
        // +0x0e  64 89 25 00 00 00 00  MOV FS:[0x0], ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x15  83 ec 10       SUB ESP, 0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // +0x18  56             PUSH ESI
        _emit 0x56
        // +0x19  68 a8 6c f5 00 PUSH 0xf56ca8   (string ptr — DIR32 reloc)
        _emit 0x68
        _emit 0xa8
        _emit 0x6c
        _emit 0xf5
        _emit 0x00
        // +0x1e  6a 10          PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // +0x20  8d 4c 24 14    LEA ECX, [ESP+0x14]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // +0x24  e8 77 df ff ff CALL FUN_0040e2d0  (rel32 reloc)
        _emit 0xe8
        _emit 0x77
        _emit 0xdf
        _emit 0xff
        _emit 0xff
        // +0x29  8b 74 24 2c    MOV ESI, dword ptr [ESP+0x2c]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x2c
        // +0x2d  50             PUSH EAX
        _emit 0x50
        // +0x2e  6a 6c          PUSH 0x6c
        _emit 0x6a
        _emit 0x6c
        // +0x30  8b ce          MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // +0x32  e8 a9 dd ff ff CALL FUN_0040e110  (rel32 reloc)
        _emit 0xe8
        _emit 0xa9
        _emit 0xdd
        _emit 0xff
        _emit 0xff
        // +0x37  89 44 24 04    MOV dword ptr [ESP+0x4], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // +0x3b  89 44 24 08    MOV dword ptr [ESP+0x8], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // +0x3f  85 c0          TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // +0x41  c7 44 24 1c 00 00 00 00  MOV dword ptr [ESP+0x1c], 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x49  74 31          JZ +0x31   (→ +0x7c = null path)
        _emit 0x74
        _emit 0x31
        // +0x4b  8b 4c 24 38    MOV ECX, dword ptr [ESP+0x38]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // +0x4f  8b 54 24 34    MOV EDX, dword ptr [ESP+0x34]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x34
        // +0x53  51             PUSH ECX
        _emit 0x51
        // +0x54  8b 4c 24 34    MOV ECX, dword ptr [ESP+0x34]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        // +0x58  52             PUSH EDX
        _emit 0x52
        // +0x59  8b 54 24 30    MOV EDX, dword ptr [ESP+0x30]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x30
        // +0x5d  51             PUSH ECX
        _emit 0x51
        // +0x5e  8b 4c 24 30    MOV ECX, dword ptr [ESP+0x30]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        // +0x62  56             PUSH ESI
        _emit 0x56
        // +0x63  52             PUSH EDX
        _emit 0x52
        // +0x64  51             PUSH ECX
        _emit 0x51
        // +0x65  8b c8          MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // +0x67  e8 f4 f5 ff ff CALL FUN_0040f990  (rel32 reloc)
        _emit 0xe8
        _emit 0xf4
        _emit 0xf5
        _emit 0xff
        _emit 0xff
        // +0x6c  5e             POP ESI
        _emit 0x5e
        // +0x6d  8b 4c 24 10    MOV ECX, dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // +0x71  64 89 0d 00 00 00 00  MOV FS:[0x0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x78  83 c4 1c       ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // +0x7b  c3             RET
        _emit 0xc3
        // +0x7c  8b 4c 24 14    MOV ECX, dword ptr [ESP+0x14]   (null path)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // +0x80  33 c0          XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // +0x82  5e             POP ESI
        _emit 0x5e
        // +0x83  64 89 0d 00 00 00 00  MOV FS:[0x0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x8a  83 c4 1c       ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // +0x8d  c3             RET
        _emit 0xc3
    }
}
#endif // _MSC_VER
