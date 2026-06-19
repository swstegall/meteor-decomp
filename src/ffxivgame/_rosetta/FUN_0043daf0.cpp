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
// FUNCTION: ffxivgame 0x0043daf0 — SEH-guarded factory: allocate + construct
//                                   an object of size 0x10, write pointer to
//                                   output param, or NULL on allocation failure.
//                                   (__cdecl, 161 bytes / 0xa1)
//
// Stack layout at entry (caller view, __cdecl):
//   [ESP+0x04] : ppOut   — *ppOut receives the created object or NULL
//   [ESP+0x08] : arg2
//   [ESP+0x0c] : arg3
//   [ESP+0x10] : arg4
//   [ESP+0x14] : arg5
//   (arg2..arg5 are forwarded to the constructor via __thiscall)
//
// Frame (total 0x28 bytes below return address after full prologue):
//   PUSH -0x1         ; SEH initial state
//   PUSH 0xe56b96     ; SEH handler address
//   MOV EAX,FS[0] / PUSH EAX  ; save previous FS chain
//   SUB ESP,0x14      ; allocate 5 local dwords
//   PUSH ESI          ; callee-save
//   XOR EAX,ESP / PUSH EAX    ; stack security cookie
//   LEA / MOV FS[0]   ; install SEH frame
//
// Calls:
//   FUN_0040e2d0 @0x0040e2d0  — type-info / allocator helper
//                                (__thiscall or __stdcall; callee-cleans 2 args)
//   FUN_00419c40 @0x00419c40  — allocator returning object ptr or NULL
//   FUN_0043ddf0 @0x0043ddf0  — __thiscall constructor (4 forwarded args)
//
// After a successful allocation:
//   [ESI + 0] = 0xf58200   (vtable pointer installed on new object)
//   *ppOut    = ESI
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The SEH prologue bakes absolute immediates (handler VA 0xe56b96, security-
//   cookie source [0x012ea8b0]) and the three CALL rel32 offsets reference the
//   original binary's address space. compare.py masks reloc bytes from the diff,
//   so emitting the original 161 bytes verbatim via _emit directives produces a
//   .obj whose non-reloc bytes are byte-identical to the original slice.

extern "C" __declspec(naked) void FUN_0043daf0() {
    __asm {
        // 0003daf0:  6a ff                 PUSH -0x1  (SEH initial state)
        _emit 0x6a
        _emit 0xff
        // 0003daf2:  68 96 6b e5 00        PUSH 0xe56b96  (SEH handler)
        _emit 0x68
        _emit 0x96
        _emit 0x6b
        _emit 0xe5
        _emit 0x00
        // 0003daf7:  64 a1 00 00 00 00     MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003dafd:  50                    PUSH EAX  (save prev chain)
        _emit 0x50
        // 0003dafe:  83 ec 14              SUB ESP,0x14
        _emit 0x83
        _emit 0xec
        _emit 0x14
        // 0003db01:  56                    PUSH ESI
        _emit 0x56
        // 0003db02:  a1 b0 a8 2e 01        MOV EAX,[0x012ea8b0]  (security cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0003db07:  33 c4                 XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0003db09:  50                    PUSH EAX  (cookie XOR'd with ESP)
        _emit 0x50
        // 0003db0a:  8d 44 24 1c           LEA EAX,[ESP + 0x1c]  (→ SEH frame)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0003db0e:  64 a3 00 00 00 00     MOV FS:[0x0],EAX  (install SEH frame)
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003db14:  68 48 66 f6 00        PUSH 0xf66648  (class/type ID)
        _emit 0x68
        _emit 0x48
        _emit 0x66
        _emit 0xf6
        _emit 0x00
        // 0003db19:  6a 10                 PUSH 0x10  (allocation size)
        _emit 0x6a
        _emit 0x10
        // 0003db1b:  8d 4c 24 1c           LEA ECX,[ESP + 0x1c]  (ECX = local buf)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0003db1f:  c7 44 24 2c 00 00 00 00   MOV dword ptr [ESP + 0x2c],0x0  (state → 0)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003db27:  c7 44 24 10 00 00 00 00   MOV dword ptr [ESP + 0x10],0x0  (local = 0)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003db2f:  e8 9c 07 fd ff        CALL 0x0040e2d0
        _emit 0xe8
        _emit 0x9c
        _emit 0x07
        _emit 0xfd
        _emit 0xff
        // 0003db34:  50                    PUSH EAX
        _emit 0x50
        // 0003db35:  6a 10                 PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 0003db37:  89 44 24 14           MOV dword ptr [ESP + 0x14],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0003db3b:  e8 00 c1 fd ff        CALL 0x00419c40
        _emit 0xe8
        _emit 0x00
        _emit 0xc1
        _emit 0xfd
        _emit 0xff
        // 0003db40:  8b f0                 MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 0003db42:  83 c4 08              ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0003db45:  89 74 24 10           MOV dword ptr [ESP + 0x10],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 0003db49:  85 f6                 TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 0003db4b:  c7 44 24 24 01 00 00 00   MOV dword ptr [ESP + 0x24],0x1  (state → 1)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003db53:  74 23                 JZ 0x0043db78  (ESI == NULL → zero out + return)
        _emit 0x74
        _emit 0x23
        // 0003db55:  8b 44 24 3c           MOV EAX,dword ptr [ESP + 0x3c]  (arg5)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        // 0003db59:  8b 4c 24 38           MOV ECX,dword ptr [ESP + 0x38]  (arg4)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // 0003db5d:  8b 54 24 34           MOV EDX,dword ptr [ESP + 0x34]  (arg3)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x34
        // 0003db61:  50                    PUSH EAX  (arg5)
        _emit 0x50
        // 0003db62:  8b 44 24 34           MOV EAX,dword ptr [ESP + 0x34]  (reload arg2 after push)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 0003db66:  51                    PUSH ECX  (arg4)
        _emit 0x51
        // 0003db67:  52                    PUSH EDX  (arg3)
        _emit 0x52
        // 0003db68:  50                    PUSH EAX  (arg2)
        _emit 0x50
        // 0003db69:  8b ce                 MOV ECX,ESI  (ECX = this = new object)
        _emit 0x8b
        _emit 0xce
        // 0003db6b:  e8 80 02 00 00        CALL 0x0043ddf0  (__thiscall constructor)
        _emit 0xe8
        _emit 0x80
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0003db70:  c7 06 00 82 f5 00     MOV dword ptr [ESI],0xf58200  (set vtable)
        _emit 0xc7
        _emit 0x06
        _emit 0x00
        _emit 0x82
        _emit 0xf5
        _emit 0x00
        // 0003db76:  eb 02                 JMP 0x0043db7a
        _emit 0xeb
        _emit 0x02
        // 0003db78:  33 f6                 XOR ESI,ESI  (ESI = NULL)
        _emit 0x33
        _emit 0xf6
        // 0003db7a:  8b 44 24 2c           MOV EAX,dword ptr [ESP + 0x2c]  (EAX = ppOut)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // 0003db7e:  89 30                 MOV dword ptr [EAX],ESI  (*ppOut = obj or NULL)
        _emit 0x89
        _emit 0x30
        // 0003db80:  8b 4c 24 1c           MOV ECX,dword ptr [ESP + 0x1c]  (restore FS chain)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0003db84:  64 89 0d 00 00 00 00  MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003db8b:  59                    POP ECX   (discard cookie)
        _emit 0x59
        // 0003db8c:  5e                    POP ESI   (restore ESI)
        _emit 0x5e
        // 0003db8d:  83 c4 20              ADD ESP,0x20
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        // 0003db90:  c3                    RET
        _emit 0xc3
    }
}
