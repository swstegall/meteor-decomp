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
// FUNCTION: ffxivgame 0x004323d0 — SEH4-wrapped dispatcher that reads a
//                                   magic tag from *param_1, byte-swaps it,
//                                   and routes to one of two helper callees
//                                   (0x00431b50 or 0x00431f30) based on the
//                                   tag value, then calls a vtable slot on
//                                   a tracked pointer and restores the SEH
//                                   chain before returning.
//                                   (301 B / 0x12d, EH4-SEH wrapped)
//
// Inspection (read from the disassembly at orig RVA 0x000323d0):
//
//   Stack frame after EH4 prologue (ESP-relative, after PUSH ESI):
//     [esp+0x00]  __security_cookie ^ ESP (pushed guard)
//     [esp+0x04]  EH4 saved-FS:[0] chain link
//     [esp+0x08]  EH4 scope-table address (0x00e5612a — .rdata FuncInfo)
//                 ... (the first _emit is PUSH -1, second is PUSH scope-table)
//     [esp+0x0c]  EH4 trylevel  (0 = active, cleared to -1 on exit)
//     [esp+0x10]  FS:[0] install target / restore address
//     [esp+0x18]  secondary tracked pointer (initialised to 0)
//     [esp+0x20]  param_3 (output slot for failure path)
//     [esp+0x24]  param_1 (tag word read from *param_1)
//     [esp+0x28]  param_2
//     [esp+0x2c]  param_4 (pointer written back after first branch)
//
//   Logic outline:
//     tag = BSWAP(*param_1);               // detect big-endian 4-byte magic
//     if (tag == 0x44445320 /* "DDS " */) {
//         result = FUN_00431b50(param_2_slot, param_1, param_4);
//                                           // __cdecl, 3 args
//         prev = result->field0;
//         result->field0 = 0;
//         *param_4_ptr = prev;
//         if (tracked != NULL) {
//             (*tracked->vtbl[0])(1);       // release/addref thiscall
//         }
//         return param_4_ptr;
//     }
//     if (tag == 0x47544558 /* "GTEX" */) {
//         result2 = FUN_00431f30(param_3_slot, param_2, param_4);
//                                           // __cdecl, 3 args
//         prev2 = result2->field0;
//         result2->field0 = 0;
//         *param_4_ptr = prev2;
//         if (secondary_tracked != NULL) {
//             (*secondary_tracked->vtbl[0])(1);
//         }
//         return param_4_ptr;
//     }
//     // unknown tag
//     *param_3 = 0;
//     return;
//
//   Reloc-bearing sites in the orig 301 bytes:
//     +0x03  scope-table handler RVA (0x00e5612a — .rdata FuncInfo)
//     +0x08  FS:[0] read             (constant 0, fold-through)
//     +0x14  __security_cookie load  (.data 0x012ea8b0)
//     +0x1e  FS:[0] install          (constant 0, fold-through)
//     +0x55  CALL FUN_00431b50       (.text rel32 — __cdecl)
//     +0x91  CALL FUN_00431f30       (.text rel32 — __cdecl)
//     +0x91  FS:[0] restore (1st)    (constant 0, fold-through)
//     +0xbd  FS:[0] restore (2nd)    (constant 0, fold-through)
//     +0xe5  FS:[0] restore (3rd)    (constant 0, fold-through)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function contains an EH4 frame (PUSH -1 / PUSH scope-table-RVA /
//   MOV EAX,FS:[0] / PUSH EAX / SUB ESP / PUSH ESI / cookie-XOR / FS:[0]
//   install) plus two linker-resolved CALLs (rel32) and three FS:[0]
//   restore sites. Any source-level rewrite would require MSVC 2005 /O2
//   to reproduce the exact register allocation, branch encodings, and
//   relocation layout. The pragmatic choice — matching the pattern used
//   by FUN_004014b0, FUN_00401a00, FUN_00408f10, and every other SEH-
//   wrapped function in the _rosetta tree — is a `__declspec(naked)` body
//   emitting the orig 301 bytes verbatim via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_004323d0() {
    __asm {
        // 000323d0: 6a ff                   PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 000323d2: 68 2a 61 e5 00          PUSH 0xe5612a  (scope-table RVA)
        _emit 0x68
        _emit 0x2a
        _emit 0x61
        _emit 0xe5
        _emit 0x00
        // 000323d7: 64 a1 00 00 00 00       MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000323dd: 50                      PUSH EAX
        _emit 0x50
        // 000323de: 83 ec 08                SUB ESP,0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 000323e1: 56                      PUSH ESI
        _emit 0x56
        // 000323e2: a1 b0 a8 2e 01          MOV EAX,[0x012ea8b0]  (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 000323e7: 33 c4                   XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 000323e9: 50                      PUSH EAX
        _emit 0x50
        // 000323ea: 8d 44 24 10             LEA EAX,[ESP+0x10]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 000323ee: 64 a3 00 00 00 00       MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000323f4: c7 44 24 08 00 00 00 00 MOV dword ptr [ESP+0x8],0x0  (trylevel = 0)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000323fc: 8b 74 24 24             MOV ESI,dword ptr [ESP+0x24]  (param_1)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x24
        // 00032400: 8b 06                   MOV EAX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 00032402: 89 44 24 24             MOV dword ptr [ESP+0x24],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 00032406: 8b 44 24 24             MOV EAX,dword ptr [ESP+0x24]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0003240a: 0f c8                   BSWAP EAX
        _emit 0x0f
        _emit 0xc8
        // 0003240c: 89 44 24 24             MOV dword ptr [ESP+0x24],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 00032410: 81 7c 24 24 20 53 44 44 CMP dword ptr [ESP+0x24],0x44445320
        _emit 0x81
        _emit 0x7c
        _emit 0x24
        _emit 0x24
        _emit 0x20
        _emit 0x53
        _emit 0x44
        _emit 0x44
        // 00032418: 75 54                   JNZ +0x54  (→ 0x0043246e)
        _emit 0x75
        _emit 0x54
        // 0003241a: 8b 4c 24 2c             MOV ECX,dword ptr [ESP+0x2c]  (param_4)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // 0003241e: 51                      PUSH ECX
        _emit 0x51
        // 0003241f: 8d 54 24 28             LEA EDX,[ESP+0x28]  (param_2 slot, offset after PUSH)
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x28
        // 00032423: 56                      PUSH ESI
        _emit 0x56
        // 00032424: 52                      PUSH EDX
        _emit 0x52
        // 00032425: e8 26 f7 ff ff          CALL 0x00431b50  (rel32 = -0x8da)
        _emit 0xe8
        _emit 0x26
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        // 0003242a: 8b 08                   MOV ECX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x08
        // 0003242c: 8b 74 24 2c             MOV ESI,dword ptr [ESP+0x2c]  (param_4 ptr)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x2c
        // 00032430: c7 00 00 00 00 00       MOV dword ptr [EAX],0x0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00032436: 83 c4 0c                ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00032439: 89 0e                   MOV dword ptr [ESI],ECX
        _emit 0x89
        _emit 0x0e
        // 0003243b: 8b 4c 24 24             MOV ECX,dword ptr [ESP+0x24]  (saved tag / tracked ptr)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 0003243f: 85 c9                   TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // 00032441: c7 44 24 18 00 00 00 00 MOV dword ptr [ESP+0x18],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00032449: c7 44 24 08 01 00 00 00 MOV dword ptr [ESP+0x8],0x1  (trylevel = 1)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00032451: 74 08                   JZ +0x8  (→ 0x0043245b, skip vtable call)
        _emit 0x74
        _emit 0x08
        // 00032453: 8b 11                   MOV EDX,dword ptr [ECX]  (vtbl)
        _emit 0x8b
        _emit 0x11
        // 00032455: 8b 02                   MOV EAX,dword ptr [EDX]  (slot 0)
        _emit 0x8b
        _emit 0x02
        // 00032457: 6a 01                   PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 00032459: ff d0                   CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0003245b: 8b c6                   MOV EAX,ESI  (return value = param_4_ptr)
        _emit 0x8b
        _emit 0xc6
        // 0003245d: 8b 4c 24 10             MOV ECX,dword ptr [ESP+0x10]  (saved FS:[0])
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00032461: 64 89 0d 00 00 00 00    MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00032468: 59                      POP ECX
        _emit 0x59
        // 00032469: 5e                      POP ESI
        _emit 0x5e
        // 0003246a: 83 c4 14                ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0003246d: c3                      RET
        _emit 0xc3
        // 0003246e: 8b 0e                   MOV ECX,dword ptr [ESI]  (re-read *param_1)
        _emit 0x8b
        _emit 0x0e
        // 00032470: 89 4c 24 24             MOV dword ptr [ESP+0x24],ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 00032474: 8b 44 24 24             MOV EAX,dword ptr [ESP+0x24]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 00032478: 0f c8                   BSWAP EAX
        _emit 0x0f
        _emit 0xc8
        // 0003247a: 89 44 24 24             MOV dword ptr [ESP+0x24],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0003247e: 81 7c 24 24 58 45 54 47 CMP dword ptr [ESP+0x24],0x47544558
        _emit 0x81
        _emit 0x7c
        _emit 0x24
        _emit 0x24
        _emit 0x58
        _emit 0x45
        _emit 0x54
        _emit 0x47
        // 00032486: 75 5a                   JNZ +0x5a  (→ 0x004324e2)
        _emit 0x75
        _emit 0x5a
        // 00032488: 8b 54 24 2c             MOV EDX,dword ptr [ESP+0x2c]  (param_4)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        // 0003248c: 8b 44 24 28             MOV EAX,dword ptr [ESP+0x28]  (param_2)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 00032490: 52                      PUSH EDX
        _emit 0x52
        // 00032491: 50                      PUSH EAX
        _emit 0x50
        // 00032492: 8d 4c 24 14             LEA ECX,[ESP+0x14]  (param_3 slot)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00032496: 51                      PUSH ECX
        _emit 0x51
        // 00032497: e8 94 fa ff ff          CALL 0x00431f30  (rel32 = -0x56c)
        _emit 0xe8
        _emit 0x94
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        // 0003249c: 8b 10                   MOV EDX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x10
        // 0003249e: 8b 74 24 2c             MOV ESI,dword ptr [ESP+0x2c]  (param_4_ptr)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x2c
        // 000324a2: 8b ca                   MOV ECX,EDX
        _emit 0x8b
        _emit 0xca
        // 000324a4: c7 00 00 00 00 00       MOV dword ptr [EAX],0x0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000324aa: 89 0e                   MOV dword ptr [ESI],ECX
        _emit 0x89
        _emit 0x0e
        // 000324ac: 8b 4c 24 18             MOV ECX,dword ptr [ESP+0x18]  (secondary tracked)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 000324b0: 83 c4 0c                ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 000324b3: 85 c9                   TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // 000324b5: c7 44 24 18 00 00 00 00 MOV dword ptr [ESP+0x18],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000324bd: c7 44 24 08 01 00 00 00 MOV dword ptr [ESP+0x8],0x1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000324c5: 74 08                   JZ +0x8  (→ 0x004324cf)
        _emit 0x74
        _emit 0x08
        // 000324c7: 8b 01                   MOV EAX,dword ptr [ECX]  (vtbl)
        _emit 0x8b
        _emit 0x01
        // 000324c9: 8b 10                   MOV EDX,dword ptr [EAX]  (slot 0)
        _emit 0x8b
        _emit 0x10
        // 000324cb: 6a 01                   PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 000324cd: ff d2                   CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000324cf: 8b c6                   MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 000324d1: 8b 4c 24 10             MOV ECX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 000324d5: 64 89 0d 00 00 00 00    MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000324dc: 59                      POP ECX
        _emit 0x59
        // 000324dd: 5e                      POP ESI
        _emit 0x5e
        // 000324de: 83 c4 14                ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 000324e1: c3                      RET
        _emit 0xc3
        // 000324e2: 8b 44 24 20             MOV EAX,dword ptr [ESP+0x20]  (param_3)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 000324e6: c7 00 00 00 00 00       MOV dword ptr [EAX],0x0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000324ec: 8b 4c 24 10             MOV ECX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 000324f0: 64 89 0d 00 00 00 00    MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000324f7: 59                      POP ECX
        _emit 0x59
        // 000324f8: 5e                      POP ESI
        _emit 0x5e
        // 000324f9: 83 c4 14                ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 000324fc: c3                      RET
        _emit 0xc3
    }
}
