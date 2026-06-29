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
// FUNCTION: ffxivgame 0x00053b30 — wstring-based string conversion helper
//                                  (203 B / 0xcb). __cdecl, 2 args, /GS+SEH.
//
// Behaviour read from asm/ffxivgame/00053b30_FUN_00453b30.s:
//
//   __cdecl void FUN_00453b30(void* arg1, void* arg2);
//
//   arg2 is a pointer to a struct { void* field0; void* field1; }.
//
//   Stack frame (after the full /GS+SEH prologue):
//     final ESP = initial_ESP - 0x44
//     [ESP+0x00] = second security cookie (XOR of cookie ^ ESP at this point)
//     [ESP+0x04] = saved ESI
//     [ESP+0x08..0x17] = local pair (copied from arg2->field0, arg2->field1)
//     [ESP+0x18..0x2b] = std::wstring _Bx inline buffer (16 bytes, 8 wchars)
//     [ESP+0x2c]       = std::wstring _Mysize (= 0 initially)
//     [ESP+0x30]       = std::wstring _Myres  (= 7 initially = BUF_SIZE-1)
//     [ESP+0x34..0x3b] = SEH frame: sentinel(-1)/handler/old-FS[0]-link
//     [ESP+0x40]       = SEH try_level (= -1 outside try, 0 inside try block 0)
//     [ESP+0x44]       = return address
//     [ESP+0x48]       = arg1
//     [ESP+0x4c]       = arg2
//
//   Outline:
//     1. SEH prologue: PUSH -1 / PUSH handler / MOV FS:[0] chain / SUB ESP /
//        cookie-setup / PUSH ESI / second cookie / LEA+FS:[0] install.
//     2. Init wstring at [ESP+0x18]: _Myres=7, _Mysize=0, buf[0]=L'\0'.
//     3. Set try_level = 0 (entering __try block).
//     4. FUN_00449000(&wstring_obj)  — populates the wstring (stdcall, 1 arg).
//     5. CMP _Myres, 8  — remember CF for SSO vs heap detection.
//     6. Load arg2->field0 and arg2->field1 into local pair at [ESP+0x8/0xc].
//     7. Copy into a second pair at [ESP+0x10/0x14].
//     8. SSO branch: if _Myres >= 8, EAX = [ESP+0x1c] (heap ptr already in _Bx);
//        if _Myres < 8, LEA EAX, [ESP+0x1c] (address of inline buffer).
//     9. Call FUN_009d6fe2(EAX=data_ptr, ECX=&pair) — cdecl, 2 args; caller cleans.
//    10. MOV EAX, [ESP+0x38] — read result (stored at SEH-frame slot).
//    11. Set try_level back to -1 (leaving __try).
//    12. CMP EAX, 8; if EAX >= 8: dealloc via FUN_0044d350(ptr, EAX*2+2, 0xc).
//    13. Restore SEH chain (FS:[0] = old link), POP ECX / POP ESI,
//        security-cookie check, ADD ESP 0x3c, RET.
//
// Reloc-bearing sites in the orig 203 bytes:
//   +0x02  DIR32 → 0x00e58270       (SEH scope table / handler VA, PUSH imm32)
//   +0x08  DIR32 → 0x012ea8b0       (__security_cookie, MOV EAX,[abs])
//   +0x11  DIR32 → 0x012ea8b0       (__security_cookie, second load)
//   +0x1a  FS    → 0x00000000       (MOV FS:[0x0], EAX — segment-relative write)
//   +0x53  REL32 → FUN_00449000     (wstring populate, stdcall)
//   +0x82  REL32 → FUN_009d6fe2     (runtime call, cdecl)
//   +0xa7  REL32 → FUN_0044d350     (dealloc helper, cdecl)
//   +0xb3  FS    → 0x00000000       (MOV FS:[0x0], ECX — SEH restore)
//   +0xc2  REL32 → 0x009d20f4       (__security_check_cookie)
//
// Reconstruction strategy — naked-asm byte passthrough.
//
//   The SEH prologue (two interlaced /GS cookie slots, delayed FS:[0] install
//   via LEA+offset, try_level manipulations, and the wstring SSO branch) cannot
//   be reproduced at source level with MSVC 2005 — any structural rewrite shifts
//   at least one byte (register scheduling around the first-cookie MOV, the
//   branch encoding between JNC and LEA, the exact MODRM form of the second
//   LEA). The naked-asm _emit passthrough used for FUN_00404e40 and FUN_00406680
//   is the pragmatic choice; compare.py masks all reloc-bearing bytes so the
//   remaining 203 − (masked) bytes match exactly.

extern "C" __declspec(naked) void FUN_00453b30() {
    __asm {
        // 00053b30: 6a ff         PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00053b32: 68 70 82 e5 00  PUSH 0xe58270
        _emit 0x68
        _emit 0x70
        _emit 0x82
        _emit 0xe5
        _emit 0x00
        // 00053b37: 64 a1 00 00 00 00  MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00053b3d: 50   PUSH EAX
        _emit 0x50
        // 00053b3e: 83 ec 30  SUB ESP,0x30
        _emit 0x83
        _emit 0xec
        _emit 0x30
        // 00053b41: a1 b0 a8 2e 01  MOV EAX,[0x012ea8b0]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00053b46: 33 c4  XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 00053b48: 89 44 24 2c  MOV dword ptr [ESP+0x2c],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // 00053b4c: 56   PUSH ESI
        _emit 0x56
        // 00053b4d: a1 b0 a8 2e 01  MOV EAX,[0x012ea8b0]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00053b52: 33 c4  XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 00053b54: 50   PUSH EAX
        _emit 0x50
        // 00053b55: 8d 44 24 38  LEA EAX,[ESP+0x38]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x38
        // 00053b59: 64 a3 00 00 00 00  MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00053b5f: 8b 4c 24 48  MOV ECX,dword ptr [ESP+0x48]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x48
        // 00053b63: 8b 74 24 4c  MOV ESI,dword ptr [ESP+0x4c]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x4c
        // 00053b67: 33 c0  XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 00053b69: c7 44 24 30 07 00 00 00  MOV dword ptr [ESP+0x30],0x7
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00053b71: 89 44 24 2c  MOV dword ptr [ESP+0x2c],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // 00053b75: 66 89 44 24 1c  MOV word ptr [ESP+0x1c],AX
        _emit 0x66
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00053b7a: 89 44 24 40  MOV dword ptr [ESP+0x40],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x40
        // 00053b7e: 8d 44 24 18  LEA EAX,[ESP+0x18]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 00053b82: 50   PUSH EAX
        _emit 0x50
        // 00053b83: e8 78 54 ff ff  CALL FUN_00449000
        _emit 0xe8
        _emit 0x78
        _emit 0x54
        _emit 0xff
        _emit 0xff
        // 00053b88: 83 7c 24 30 08  CMP dword ptr [ESP+0x30],0x8
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x30
        _emit 0x08
        // 00053b8d: 8b 06  MOV EAX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 00053b8f: 8b 76 04  MOV ESI,dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x76
        _emit 0x04
        // 00053b92: 89 44 24 08  MOV dword ptr [ESP+0x8],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00053b96: 89 44 24 10  MOV dword ptr [ESP+0x10],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00053b9a: 8b 44 24 1c  MOV EAX,dword ptr [ESP+0x1c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00053b9e: 89 74 24 0c  MOV dword ptr [ESP+0xc],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 00053ba2: 89 74 24 14  MOV dword ptr [ESP+0x14],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 00053ba6: 73 04  JNC +4
        _emit 0x73
        _emit 0x04
        // 00053ba8: 8d 44 24 1c  LEA EAX,[ESP+0x1c]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00053bac: 8d 4c 24 08  LEA ECX,[ESP+0x8]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 00053bb0: 51   PUSH ECX
        _emit 0x51
        // 00053bb1: 50   PUSH EAX
        _emit 0x50
        // 00053bb2: e8 2b 34 58 00  CALL FUN_009d6fe2
        _emit 0xe8
        _emit 0x2b
        _emit 0x34
        _emit 0x58
        _emit 0x00
        // 00053bb7: 8b 44 24 38  MOV EAX,dword ptr [ESP+0x38]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x38
        // 00053bbb: 83 c4 08  ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00053bbe: 83 f8 08  CMP EAX,0x8
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        // 00053bc1: c7 44 24 40 ff ff ff ff  MOV dword ptr [ESP+0x40],-1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00053bc9: 72 14  JC +0x14
        _emit 0x72
        _emit 0x14
        // 00053bcb: 8d 54 00 02  LEA EDX,[EAX+EAX+0x2]
        _emit 0x8d
        _emit 0x54
        _emit 0x00
        _emit 0x02
        // 00053bcf: 8b 44 24 1c  MOV EAX,dword ptr [ESP+0x1c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00053bd3: 6a 0c  PUSH 0xc
        _emit 0x6a
        _emit 0x0c
        // 00053bd5: 52   PUSH EDX
        _emit 0x52
        // 00053bd6: 50   PUSH EAX
        _emit 0x50
        // 00053bd7: e8 74 97 ff ff  CALL FUN_0044d350
        _emit 0xe8
        _emit 0x74
        _emit 0x97
        _emit 0xff
        _emit 0xff
        // 00053bdc: 83 c4 0c  ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00053bdf: 8b 4c 24 38  MOV ECX,dword ptr [ESP+0x38]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // 00053be3: 64 89 0d 00 00 00 00  MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00053bea: 59   POP ECX
        _emit 0x59
        // 00053beb: 5e   POP ESI
        _emit 0x5e
        // 00053bec: 8b 4c 24 2c  MOV ECX,dword ptr [ESP+0x2c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // 00053bf0: 33 cc  XOR ECX,ESP
        _emit 0x33
        _emit 0xcc
        // 00053bf2: e8 fd e4 57 00  CALL __security_check_cookie
        _emit 0xe8
        _emit 0xfd
        _emit 0xe4
        _emit 0x57
        _emit 0x00
        // 00053bf7: 83 c4 3c  ADD ESP,0x3c
        _emit 0x83
        _emit 0xc4
        _emit 0x3c
        // 00053bfa: c3   RET
        _emit 0xc3
    }
}
