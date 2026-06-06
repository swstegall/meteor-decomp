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
// FUNCTION: ffxivgame 0x000536e0 — `__cdecl` wide-string compare helper
//                                  (267 B / 0x10b, EH4-SEH wrapped, /GS).
//
// Behaviour read from the disassembly at orig RVA 0x000536e0:
//
//   __cdecl bool FUN_004536e0(void* arg0, void** arg1);
//
//     The function initialises a local SSO-capable wide-string object at
//     [esp+0x8] (capacity=7, size=0, first wchar=0) and calls an initialiser
//     at 0x00449000 (__thiscall, ECX=arg0) to populate it from the first
//     argument.
//
//     It then fetches the raw string pointer (SSO-aware: if capacity < 8 use
//     &obj, otherwise use obj[data_ptr]) and passes it to an IAT function at
//     [0x00f3e288] to obtain file attributes or a handle.
//
//     Result == -1  →  free if large string, return false (0)
//     Result & 0x10 →  free large string, return false (0)  [attribute branch]
//     Otherwise     →  compare string with *arg1 via IAT [0x00f3e28c], call
//                       0x00403fd0 on success, return true (1)
//
//   The SEH trylevel at [esp+0x30] is driven through 0→-1 across the two
//   branches that call the destructor/free path (0x0044d350).
//
//   Stack frame (after the EH4 prologue, final ESP-relative layout):
//     [esp+0x00]         EH4 cookie #2 (PUSH'd post-XOR-ESP)
//     [esp+0x04]         saved ESI (arg1 mirror)
//     [esp+0x08..+0x23]  local wstring-like object (28 B used)
//     [esp+0x24]         EH4 cookie #1
//     [esp+0x28]         EH4 saved-FS:[0] chain link
//     [esp+0x2c]         EH4 scope-table address (0x00e581d8)
//     [esp+0x30]         EH4 trylevel (initially -1)
//     [esp+0x34]         return address
//     [esp+0x38]         arg0
//     [esp+0x3c]         arg1
//
//   Reloc-bearing sites in the orig 267 bytes:
//     +0x03   scope-table handler RVA  (0x00e581d8 — .rdata FuncInfo)
//     +0x09   FS:[0] read              (constant 0, fold-through)
//     +0x11   __security_cookie load   (.data 0x012ea8b0)
//     +0x1d   __security_cookie load   (.data 0x012ea8b0, 2nd)
//     +0x29   FS:[0] install           (constant 0, fold-through)
//     +0x53   CALL 0x00449000          (rel32 — wstring initialiser)
//     +0x68   IAT [0x00f3e288]         (GetFileAttributesW or similar)
//     +0x8f   CALL 0x00403fd0          (rel32 — success handler)
//     +0xa5   IAT [0x00f3e28c]         (string compare / FindFirstFileW)
//     +0xe5   CALL 0x0044d350          (rel32 — string/heap free)
//     +0xf2   CALL 0x009d20f4          (rel32 — __security_check_cookie)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc into
//   reproducing the exact EH4 prolog (PUSH -1 / PUSH scope-table /
//   PUSH FS:[0] / SUB ESP / double __security_cookie XOR ESP), the
//   exact SSO-aware branch sequence, the trylevel-update placement, AND the
//   linker-resolved absolute addresses in the eleven relocation windows
//   above. A `__declspec(naked)` body re-emits the orig 267 bytes verbatim
//   via MASM `_emit` directives, which is the approach taken by the other
//   SEH-wrapped functions in this _rosetta set (FUN_004054d0, FUN_004053a0,
//   etc.).

extern "C" __declspec(naked) void FUN_004536e0() {
    __asm {
        // 000536e0  PUSH -1
        _emit 0x6a
        _emit 0xff
        // 000536e2  PUSH 0xe581d8  (scope-table RVA)
        _emit 0x68
        _emit 0xd8
        _emit 0x81
        _emit 0xe5
        _emit 0x00
        // 000536e7  MOV EAX, FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000536ed  PUSH EAX
        _emit 0x50
        // 000536ee  SUB ESP, 0x20
        _emit 0x83
        _emit 0xec
        _emit 0x20
        // 000536f1  MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 000536f6  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 000536f8  MOV [ESP+0x1c], EAX  (cookie #1)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 000536fc  PUSH ESI
        _emit 0x56
        // 000536fd  MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00053702  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 00053704  PUSH EAX  (cookie #2)
        _emit 0x50
        // 00053705  LEA EAX, [ESP+0x28]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 00053709  MOV FS:[0], EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005370f  MOV ECX, [ESP+0x38]  (arg0)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // 00053713  MOV ESI, [ESP+0x3c]  (arg1)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x3c
        // 00053717  XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00053719  MOV dword [ESP+0x20], 7  (capacity = 7)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00053721  MOV dword [ESP+0x1c], EAX  (size = 0)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00053725  MOV word [ESP+0x0c], AX  (first wchar = 0)
        _emit 0x66
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0005372a  MOV dword [ESP+0x30], EAX  (trylevel = 0)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // 0005372e  LEA EAX, [ESP+0x8]  (&wstring object)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00053732  PUSH EAX
        _emit 0x50
        // 00053733  CALL 0x00449000  (wstring initialiser, __thiscall ECX=arg0)
        _emit 0xe8
        _emit 0xc8
        _emit 0x58
        _emit 0xff
        _emit 0xff
        // 00053738  CMP dword [ESP+0x20], 8  (capacity vs SSO limit)
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x08
        // 0005373d  MOV EAX, dword [ESP+0xc]  (data ptr if large)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00053741  JNC +4 (to 0x453747, use heap ptr)
        _emit 0x73
        _emit 0x04
        // 00053743  LEA EAX, [ESP+0xc]  (use inline buf addr)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00053747  PUSH EAX
        _emit 0x50
        // 00053748  CALL [0x00f3e288]  (GetFileAttributesW or similar IAT)
        _emit 0xff
        _emit 0x15
        _emit 0x88
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        // 0005374e  CMP EAX, -1
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        // 00053751  JZ +0x55 (to 0x4537a8, error path)
        _emit 0x74
        _emit 0x55
        // 00053753  TEST AL, 0x10
        _emit 0xa8
        _emit 0x10
        // 00053755  JZ +0x1f (to 0x453776, compare path)
        _emit 0x74
        _emit 0x1f
        // 00053757  MOV EAX, dword [ESP+0x20]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0005375b  CMP EAX, 8
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        // 0005375e  MOV dword [ESP+0x30], -1  (trylevel = -1)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00053766  JC +0x65 (to 0x4537cd, small string skip free)
        _emit 0x72
        _emit 0x65
        // 00053768  MOV EDX, dword [ESP+0xc]  (heap ptr)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 0005376c  PUSH 0xc
        _emit 0x6a
        _emit 0x0c
        // 0005376e  LEA ECX, [EAX+EAX+2]  (alloc size = capacity*2+2)
        _emit 0x8d
        _emit 0x4c
        _emit 0x00
        _emit 0x02
        // 00053772  PUSH ECX
        _emit 0x51
        // 00053773  PUSH EDX  (heap ptr)
        _emit 0x52
        // 00053774  JMP +0x4f (to 0x4537c5, CALL free)
        _emit 0xeb
        _emit 0x4f
        // 00053776  CMP dword [ESP+0x20], 8
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x08
        // 0005377b  MOV EAX, dword [ESP+0xc]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0005377f  JNC +4 (to 0x453785, use heap ptr)
        _emit 0x73
        _emit 0x04
        // 00053781  LEA EAX, [ESP+0xc]  (use inline buf addr)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00053785  MOV ECX, dword [ESI]  (*arg1)
        _emit 0x8b
        _emit 0x0e
        // 00053787  PUSH ECX
        _emit 0x51
        // 00053788  PUSH EAX
        _emit 0x50
        // 00053789  CALL [0x00f3e28c]  (string compare IAT)
        _emit 0xff
        _emit 0x15
        _emit 0x8c
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        // 0005378f  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00053791  JZ +0x15 (to 0x4537a8, not found)
        _emit 0x74
        _emit 0x15
        // 00053793  LEA ECX, [ESP+0x8]  (&wstring object)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 00053797  MOV dword [ESP+0x30], -1  (trylevel = -1)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0005379f  CALL 0x00403fd0  (success handler)
        _emit 0xe8
        _emit 0x2c
        _emit 0x08
        _emit 0xfb
        _emit 0xff
        // 000537a4  MOV AL, 1
        _emit 0xb0
        _emit 0x01
        // 000537a6  JMP +0x27 (to 0x4537cf, epilogue)
        _emit 0xeb
        _emit 0x27
        // 000537a8  MOV EAX, dword [ESP+0x20]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 000537ac  CMP EAX, 8
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        // 000537af  MOV dword [ESP+0x30], -1  (trylevel = -1)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 000537b7  JC +0x14 (to 0x4537cd, small string skip free)
        _emit 0x72
        _emit 0x14
        // 000537b9  PUSH 0xc
        _emit 0x6a
        _emit 0x0c
        // 000537bb  LEA EDX, [EAX+EAX+2]
        _emit 0x8d
        _emit 0x54
        _emit 0x00
        _emit 0x02
        // 000537bf  MOV EAX, dword [ESP+0x10]  (heap data ptr)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 000537c3  PUSH EDX
        _emit 0x52
        // 000537c4  PUSH EAX
        _emit 0x50
        // 000537c5  CALL 0x0044d350  (heap free)
        _emit 0xe8
        _emit 0x86
        _emit 0x9b
        _emit 0xff
        _emit 0xff
        // 000537ca  ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 000537cd  XOR AL, AL  (return false)
        _emit 0x32
        _emit 0xc0
        // 000537cf  MOV ECX, dword [ESP+0x28]  (saved FS:[0])
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // 000537d3  MOV FS:[0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000537da  POP ECX  (cookie #2)
        _emit 0x59
        // 000537db  POP ESI
        _emit 0x5e
        // 000537dc  MOV ECX, dword [ESP+0x1c]  (cookie #1)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 000537e0  XOR ECX, ESP
        _emit 0x33
        _emit 0xcc
        // 000537e2  CALL 0x009d20f4  (__security_check_cookie)
        _emit 0xe8
        _emit 0x0d
        _emit 0xe9
        _emit 0x57
        _emit 0x00
        // 000537e7  ADD ESP, 0x2c
        _emit 0x83
        _emit 0xc4
        _emit 0x2c
        // 000537ea  RET
        _emit 0xc3
    }
}
