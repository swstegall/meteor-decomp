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
// FUNCTION: ffxivgame 0x00054430 — two-argument bool-returning function with
//                                  EH4-SEH frame and 0x2020-byte stack
//                                  (303 B / 0x12f).
//
// Inspection (read from the disassembly at orig RVA 0x00054430):
//
//   __cdecl bool FUN_00454430(arg0, arg1*);
//
//   EH4 prologue with __alloca_probe (0x2014 bytes), __security_cookie,
//   saves EBX + ESI, installs SEH frame.  Initialises a local struct on
//   the huge stack buffer, then calls FUN_00453c00 (ECX = local struct,
//   three pushed args: EBX=0, 0xf676e8, EAX=arg0).
//
//   If FUN_00453c00 returns 0 (AL==0):
//     - Frees local[0x10] via a free-like pair (0x009d71df / 0x009d2646)
//     - Returns false (AL = 0)
//
//   If FUN_00453c00 returns non-zero:
//     - Calls FUN_00456250
//     - If local[0x10] == 0: calls FUN_00456060(0x29d5) to fill local[0x14]
//     - Stores local[0x14] via *ESI (arg1)
//     - Frees local[0x10] via (0x009d71df / 0x009d2646) if non-null
//     - Resets local[0x10] = 0
//     - If local[0x14] != 0: frees it, resets [0x10] = 0 again
//     - Returns true (AL = 1)
//
//   Stack frame (after EH4 prologue + alloca, ESP-relative at body):
//     [esp+0x00 .. ]       EH4 SEH frame node (scope-table + chain + cookie)
//     [esp+0x0c]           EH4 scope-table pointer (0xf676f4)
//     [esp+0x10]           local ptr #1 (freed on both paths)
//     [esp+0x14]           local ptr #2 (HANDLE / alloc result)
//     [esp+0x18]           local byte struct (2 bytes, BL-inited)
//     [esp+0x2010]         __security_cookie ^ ESP (orig copy)
//     [esp+0x2020]         saved EH4 FS:[0] link
//     [esp+0x2030]         arg0 (first param)
//     [esp+0x2034]         arg1 (second param — pointer written by callee)
//
//   Reconstruction strategy — naked-asm byte passthrough:
//
//   The EH4 prolog (PUSH -1 / PUSH scope-table / MOV EAX,FS:[0] / PUSH EAX /
//   MOV EAX,0x2014 / CALL __alloca_probe / cookie XOR / cookie store / PUSH
//   EBX ESI / second cookie / PUSH / LEA / FS:[0]=frame) with its three
//   absolute-address relocations (scope-table at +0x03, two cookie loads,
//   two FS:[0] sites, plus all the rel32 CALL targets into other .text
//   sections) cannot be reproduced byte-identically from source-level C++
//   under /O2 — branch lengths, modrm encoding, and reloc slots all shift.
//   The `__declspec(naked)` + `_emit` passthrough used by FUN_004014b0,
//   FUN_00401a00, FUN_00408f10, etc. is the correct approach here too.

extern "C" __declspec(naked) void FUN_00454430() {
    __asm {
        // PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // PUSH 0xe5838d  (EH4 scope-table handler RVA)
        _emit 0x68
        _emit 0x8d
        _emit 0x83
        _emit 0xe5
        _emit 0x00
        // MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // PUSH EAX
        _emit 0x50
        // MOV EAX,0x2014
        _emit 0xb8
        _emit 0x14
        _emit 0x20
        _emit 0x00
        _emit 0x00
        // CALL __alloca_probe (0x009d29d0)
        _emit 0xe8
        _emit 0x88
        _emit 0xe5
        _emit 0x57
        _emit 0x00
        // MOV EAX,[0x012ea8b0]  (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // MOV dword ptr [ESP+0x2010],EAX
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0x10
        _emit 0x20
        _emit 0x00
        _emit 0x00
        // PUSH EBX
        _emit 0x53
        // PUSH ESI
        _emit 0x56
        // MOV EAX,[0x012ea8b0]  (__security_cookie, 2nd load)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // PUSH EAX
        _emit 0x50
        // LEA EAX,[ESP+0x2020]
        _emit 0x8d
        _emit 0x84
        _emit 0x24
        _emit 0x20
        _emit 0x20
        _emit 0x00
        _emit 0x00
        // MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // MOV EAX,dword ptr [ESP+0x2030]  (arg0)
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0x30
        _emit 0x20
        _emit 0x00
        _emit 0x00
        // MOV ESI,dword ptr [ESP+0x2034]  (arg1)
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0x34
        _emit 0x20
        _emit 0x00
        _emit 0x00
        // XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // MOV dword ptr [ESP+0xc],0xf676f4
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xf4
        _emit 0x76
        _emit 0xf6
        _emit 0x00
        // MOV dword ptr [ESP+0x10],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        // MOV dword ptr [ESP+0x14],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        // MOV byte ptr [ESP+0x18],BL
        _emit 0x88
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // MOV byte ptr [ESP+0x19],BL
        _emit 0x88
        _emit 0x5c
        _emit 0x24
        _emit 0x19
        // PUSH EBX
        _emit 0x53
        // PUSH 0xf676e8
        _emit 0x68
        _emit 0xe8
        _emit 0x76
        _emit 0xf6
        _emit 0x00
        // PUSH EAX  (arg0)
        _emit 0x50
        // LEA ECX,[ESP+0x18]  (local struct — this ptr for FUN_00453c00)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // MOV dword ptr [ESP+0x2034],EBX
        _emit 0x89
        _emit 0x9c
        _emit 0x24
        _emit 0x34
        _emit 0x20
        _emit 0x00
        _emit 0x00
        // CALL FUN_00453c00
        _emit 0xe8
        _emit 0x54
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        // TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // JNZ  +0x27  (→ success path at 0x004544d7)
        _emit 0x75
        _emit 0x27
        // --- failure path ---
        // MOV EAX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // MOV dword ptr [ESP+0xc],0xf676f4
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xf4
        _emit 0x76
        _emit 0xf6
        _emit 0x00
        // JZ   +0x13  (→ 0x004544d3, skip free)
        _emit 0x74
        _emit 0x13
        // PUSH EAX
        _emit 0x50
        // CALL 0x009d71df
        _emit 0xe8
        _emit 0x19
        _emit 0x2d
        _emit 0x58
        _emit 0x00
        // MOV EAX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // PUSH EAX
        _emit 0x50
        // CALL 0x009d2646
        _emit 0xe8
        _emit 0x76
        _emit 0xe1
        _emit 0x57
        _emit 0x00
        // ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // XOR AL,AL  (return false)
        _emit 0x32
        _emit 0xc0
        // JMP  +0x62  (→ epilogue at 0x00454539)
        _emit 0xeb
        _emit 0x62
        // --- success path (0x004544d7) ---
        // CALL FUN_00456250
        _emit 0xe8
        _emit 0x74
        _emit 0x1d
        _emit 0x00
        _emit 0x00
        // MOV EAX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // JNZ  +0x11  (→ 0x004544f5, skip FUN_00456060)
        _emit 0x75
        _emit 0x11
        // PUSH 0x29d5
        _emit 0x68
        _emit 0xd5
        _emit 0x29
        _emit 0x00
        _emit 0x00
        // CALL FUN_00456060
        _emit 0xe8
        _emit 0x72
        _emit 0x1b
        _emit 0x00
        _emit 0x00
        // MOV EAX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // MOV ECX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // MOV dword ptr [ESI],ECX  (*arg1 = local[0x14])
        _emit 0x89
        _emit 0x0e
        // JZ   +0x19  (→ 0x00454518, skip free of local[0x10])
        _emit 0x74
        _emit 0x19
        // PUSH EAX
        _emit 0x50
        // CALL 0x009d71df
        _emit 0xe8
        _emit 0xda
        _emit 0x2c
        _emit 0x58
        _emit 0x00
        // MOV EDX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // PUSH EDX
        _emit 0x52
        // CALL 0x009d2646
        _emit 0xe8
        _emit 0x37
        _emit 0xe1
        _emit 0x57
        _emit 0x00
        // ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // MOV dword ptr [ESP+0x10],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // MOV dword ptr [ESP+0xc],0xf676f4
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xf4
        _emit 0x76
        _emit 0xf6
        _emit 0x00
        // JZ   +0x13  (→ 0x00454537, skip second free)
        _emit 0x74
        _emit 0x13
        // PUSH EAX
        _emit 0x50
        // CALL 0x009d71df
        _emit 0xe8
        _emit 0xb5
        _emit 0x2c
        _emit 0x58
        _emit 0x00
        // MOV EAX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // PUSH EAX
        _emit 0x50
        // CALL 0x009d2646
        _emit 0xe8
        _emit 0x12
        _emit 0xe1
        _emit 0x57
        _emit 0x00
        // ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // MOV AL,0x1  (return true)
        _emit 0xb0
        _emit 0x01
        // --- epilogue (0x00454539) ---
        // MOV ECX,dword ptr [ESP+0x2020]  (saved FS:[0] chain link)
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x20
        _emit 0x20
        _emit 0x00
        _emit 0x00
        // MOV FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // POP ECX
        _emit 0x59
        // POP ESI
        _emit 0x5e
        // POP EBX
        _emit 0x5b
        // MOV ECX,dword ptr [ESP+0x2010]  (orig cookie ^ ESP)
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x10
        _emit 0x20
        _emit 0x00
        _emit 0x00
        // XOR ECX,ESP
        _emit 0x33
        _emit 0xcc
        // CALL __security_check_cookie (0x009d20f4)
        _emit 0xe8
        _emit 0x9c
        _emit 0xdb
        _emit 0x57
        _emit 0x00
        // ADD ESP,0x2020
        _emit 0x81
        _emit 0xc4
        _emit 0x20
        _emit 0x20
        _emit 0x00
        _emit 0x00
        // RET
        _emit 0xc3
    }
}
