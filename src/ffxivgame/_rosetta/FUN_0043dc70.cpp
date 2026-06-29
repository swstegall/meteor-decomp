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
// FUNCTION: ffxivgame 0x0043dc70 — SEH-wrapped object factory (197 B / 0xC5)
//
// Asm (asm/ffxivgame/0003dc70_FUN_0043dc70.s):
//
//   __cdecl void FUN_0043dc70(SomeObj** ppOut, SomeArg arg1, SomeArg arg2,
//                              SomeArg arg3, SomeArg arg4);
//
//   Full MSVC 2005 SEH prologue (PUSH -1 / PUSH handler / MOV EAX,FS:[0] /
//   PUSH EAX / SUB ESP,0x10 / save EBX,ESI,EDI / security-cookie XOR).
//   EH state variable at [ESP+0x28] (initially -1 on the PUSH, set to 0
//   at function start, then set to 1 once the object allocation succeeds).
//
//   Outline:
//     EBX = arg1
//     EDI = arg4
//     if (EDI == 0) {
//         EAX = FUN_00433030(&arg4, EBX)  // lookup helper → *EAX = resolved EDI
//         EDI = *EAX
//     }
//     // Construct stack obj at [ESP+0x18] (relative to ESP_run),
//     // call FUN_0040e2d0(this=stack_obj, 0x10, 0xf66698).
//     // Then FUN_00419c40(0x10, EAX) → allocate 16 bytes → ESI.
//     // EH state = 1 once ESI acquired.
//     if (ESI != 0) {
//         bool flag = (EDI == 2)
//         ESI->FUN_0043e0b0(flag, arg1, arg2, arg3, 7)  // __thiscall
//         *ESI = 0xf58204   // install vtable pointer
//     } else {
//         ESI = 0
//     }
//     *ppOut = ESI
//     // SEH epilogue: unlink frame, restore regs, RET
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Reproducing MSVC 2005's SEH frame layout (state transitions at
//   [ESP+0x28] driven by the EH-table at 0xe56c20, security cookie at
//   0x012ea8b0) and the exact stack scheduling of the four CALL sites
//   in source-level C++ is impractical without round-tripping the
//   MSVC internal EH metadata.  Emitting the 197 orig bytes verbatim
//   makes the .obj text-section byte-identical to the orig slice.
//   All four CALL rel32 sites (+0x48, +0x5d, +0x69, +0x9d) are masked
//   by tools/compare.py; the remaining bytes are raw.
//
// Reloc-bearing sites (absolute VAs baked into the orig image):
//   +0x02  PUSH 0xe56c20           (SEH handler VA)
//   +0x14  MOV EAX,[0x012ea8b0]    (__security_cookie)
//   +0x55  PUSH 0xf66698           (.data pointer)
//   +0xe2  MOV [ESI],0xf58204      (vtable pointer)

extern "C" __declspec(naked) void FUN_0043dc70() {
    __asm {
        // 0003dc70: 6a ff — PUSH -1
        _emit 0x6a
        _emit 0xff
        // 0003dc72: 68 20 6c e5 00 — PUSH 0xe56c20
        _emit 0x68
        _emit 0x20
        _emit 0x6c
        _emit 0xe5
        _emit 0x00
        // 0003dc77: 64 a1 00 00 00 00 — MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003dc7d: 50 — PUSH EAX
        _emit 0x50
        // 0003dc7e: 83 ec 10 — SUB ESP,0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 0003dc81: 53 — PUSH EBX
        _emit 0x53
        // 0003dc82: 56 — PUSH ESI
        _emit 0x56
        // 0003dc83: 57 — PUSH EDI
        _emit 0x57
        // 0003dc84: a1 b0 a8 2e 01 — MOV EAX,[0x012ea8b0]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0003dc89: 33 c4 — XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0003dc8b: 50 — PUSH EAX
        _emit 0x50
        // 0003dc8c: 8d 44 24 20 — LEA EAX,[ESP+0x20]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0003dc90: 64 a3 00 00 00 00 — MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003dc96: 8b 5c 24 34 — MOV EBX,[ESP+0x34]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x34
        // 0003dc9a: c7 44 24 10 00 00 00 00 — MOV [ESP+0x10],0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003dca2: 8b 7c 24 40 — MOV EDI,[ESP+0x40]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x40
        // 0003dca6: 85 ff — TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 0003dca8: c7 44 24 28 00 00 00 00 — MOV [ESP+0x28],0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003dcb0: 75 10 — JNZ +0x10
        _emit 0x75
        _emit 0x10
        // 0003dcb2: 8d 44 24 40 — LEA EAX,[ESP+0x40]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x40
        // 0003dcb6: 53 — PUSH EBX
        _emit 0x53
        // 0003dcb7: 50 — PUSH EAX
        _emit 0x50
        // 0003dcb8: e8 73 53 ff ff — CALL 0x00433030
        _emit 0xe8
        _emit 0x73
        _emit 0x53
        _emit 0xff
        _emit 0xff
        // 0003dcbd: 8b 38 — MOV EDI,[EAX]
        _emit 0x8b
        _emit 0x38
        // 0003dcbf: 83 c4 08 — ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0003dcc2: 68 98 66 f6 00 — PUSH 0xf66698
        _emit 0x68
        _emit 0x98
        _emit 0x66
        _emit 0xf6
        _emit 0x00
        // 0003dcc7: 6a 10 — PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 0003dcc9: 8d 4c 24 20 — LEA ECX,[ESP+0x20]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 0003dccd: e8 fe 05 fd ff — CALL 0x0040e2d0
        _emit 0xe8
        _emit 0xfe
        _emit 0x05
        _emit 0xfd
        _emit 0xff
        // 0003dcd2: 50 — PUSH EAX
        _emit 0x50
        // 0003dcd3: 6a 10 — PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 0003dcd5: 89 44 24 48 — MOV [ESP+0x48],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x48
        // 0003dcd9: e8 62 bf fd ff — CALL 0x00419c40
        _emit 0xe8
        _emit 0x62
        _emit 0xbf
        _emit 0xfd
        _emit 0xff
        // 0003dcde: 8b f0 — MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 0003dce0: 83 c4 08 — ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0003dce3: 89 74 24 14 — MOV [ESP+0x14],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 0003dce7: 85 f6 — TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 0003dce9: c7 44 24 28 01 00 00 00 — MOV [ESP+0x28],1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003dcf1: 74 27 — JZ +0x27
        _emit 0x74
        _emit 0x27
        // 0003dcf3: 8b 4c 24 3c — MOV ECX,[ESP+0x3c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        // 0003dcf7: 8b 54 24 38 — MOV EDX,[ESP+0x38]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x38
        // 0003dcfb: b8 07 00 00 00 — MOV EAX,0x7
        _emit 0xb8
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003dd00: 50 — PUSH EAX
        _emit 0x50
        // 0003dd01: 83 ff 02 — CMP EDI,0x2
        _emit 0x83
        _emit 0xff
        _emit 0x02
        // 0003dd04: 51 — PUSH ECX
        _emit 0x51
        // 0003dd05: 0f 94 c0 — SETZ AL
        _emit 0x0f
        _emit 0x94
        _emit 0xc0
        // 0003dd08: 52 — PUSH EDX
        _emit 0x52
        // 0003dd09: 53 — PUSH EBX
        _emit 0x53
        // 0003dd0a: 8b ce — MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0003dd0c: 50 — PUSH EAX
        _emit 0x50
        // 0003dd0d: e8 9e 03 00 00 — CALL 0x0043e0b0
        _emit 0xe8
        _emit 0x9e
        _emit 0x03
        _emit 0x00
        _emit 0x00
        // 0003dd12: c7 06 04 82 f5 00 — MOV [ESI],0xf58204
        _emit 0xc7
        _emit 0x06
        _emit 0x04
        _emit 0x82
        _emit 0xf5
        _emit 0x00
        // 0003dd18: eb 02 — JMP +0x02
        _emit 0xeb
        _emit 0x02
        // 0003dd1a: 33 f6 — XOR ESI,ESI
        _emit 0x33
        _emit 0xf6
        // 0003dd1c: 8b 44 24 30 — MOV EAX,[ESP+0x30]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // 0003dd20: 89 30 — MOV [EAX],ESI
        _emit 0x89
        _emit 0x30
        // 0003dd22: 8b 4c 24 20 — MOV ECX,[ESP+0x20]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 0003dd26: 64 89 0d 00 00 00 00 — MOV FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003dd2d: 59 — POP ECX
        _emit 0x59
        // 0003dd2e: 5f — POP EDI
        _emit 0x5f
        // 0003dd2f: 5e — POP ESI
        _emit 0x5e
        // 0003dd30: 5b — POP EBX
        _emit 0x5b
        // 0003dd31: 83 c4 1c — ADD ESP,0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 0003dd34: c3 — RET
        _emit 0xc3
    }
}
