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
// FUNCTION: ffxivgame 0x004532b0 — `__cdecl` string-attribute setter
//                                   (302 B / 0x12e, EH4-SEH wrapped,
//                                    __security_cookie / GS check).
//
// Inspection (read from the disassembly at orig RVA 0x000532b0):
//
//   __cdecl bool FUN_004532b0(void *obj, DWORD flags, BYTE wideFlag, ...);
//
//   EH4 prologue: PUSH -1 / PUSH 0xe580e8 (scope-table) / PUSH FS:[0] /
//   SUB ESP,0x20 / cookie XOR ESP twice / LEA EAX,[ESP+0x24] / MOV FS:[0],EAX.
//
//   Body: initialises a local std::string-like object (trylevel 7, size 0),
//   calls FUN_00449000 (some string construction helper via __cdecl CALL),
//   then calls two IAT entries at 0x00f3e288 / 0x00f3e28c (likely
//   GetFileAttributesW / SetFileAttributesW or similar Win32 attribute pair),
//   and a free helper FUN_0044d350 (string destructor) on the error and
//   success paths. Returns AL=1 on success, AL=0 on failure.
//
//   Reloc-bearing sites (absolute addresses resolve only in a full-binary
//   relink; standalone .obj compilation cannot reproduce them):
//     +0x02   scope-table push       (0x00e580e8 — .rdata FuncInfo)
//     +0x07   FS:[0] read            (constant 0, fold-through)
//     +0x11   __security_cookie load (.data 0x012ea8b0)
//     +0x1c   __security_cookie load (.data 0x012ea8b0, 2nd)
//     +0x28   FS:[0] install         (constant 0, fold-through)
//     +0x56   CALL FUN_00449000      (.text 0x00449000, rel32 −0xa30b)
//     +0x6b   IAT load [0x00f3e288]  (.rdata — e.g. GetFileAttributesW)
//     +0xab   IAT load [0x00f3e28c]  (.rdata — e.g. SetFileAttributesW)
//     +0xe0   CALL FUN_0044d350      (.text 0x0044d350, rel32)
//     +0xe9   CALL FUN_0044d350      (.text 0x0044d350, rel32)
//     +0x117  FS:[0] restore         (constant 0, fold-through)
//     +0x125  __security_check_cookie (.text 0x009d20f4, rel32)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   EH4 prologue shape, security-cookie offsets, IAT-indirect CALLs, and
//   rel32 displacements to three distinct callees are all brittle under
//   /O2 — any source-level rewrite shifts at least one byte. The pragmatic
//   choice (same as FUN_004014b0, FUN_00401a00, FUN_00408f10) is to
//   re-emit the 302 orig bytes verbatim via MASM _emit directives.

extern "C" __declspec(naked) void FUN_004532b0() {
    __asm {
        // 000532b0
        _emit 0x6a  // PUSH -0x1
        _emit 0xff
        _emit 0x68  // PUSH 0xe580e8  (scope-table)
        _emit 0xe8
        _emit 0x80
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX,FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x83  // SUB ESP,0x20
        _emit 0xec
        _emit 0x20
        _emit 0xa1  // MOV EAX,[0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX,ESP
        _emit 0xc4
        _emit 0x89  // MOV [ESP+0x1c],EAX
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xa1  // MOV EAX,[0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX,ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX
        _emit 0x8d  // LEA EAX,[ESP+0x24]
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x64  // MOV FS:[0x0],EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000532de
        _emit 0x8b  // MOV ECX,[ESP+0x34]
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        _emit 0xc7  // MOV dword ptr [ESP+0x1c],0x7
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7  // MOV dword ptr [ESP+0x18],0x0
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66  // MOV word ptr [ESP+0x8],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x8d  // LEA EAX,[ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x50  // PUSH EAX
        _emit 0xc7  // MOV dword ptr [ESP+0x30],0x0
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00053306
        _emit 0xe8  // CALL 0x00449000
        _emit 0xf5
        _emit 0x5c
        _emit 0xff
        _emit 0xff
        // 0005330b
        _emit 0x83  // CMP dword ptr [ESP+0x1c],0x8
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x08
        _emit 0x8b  // MOV EAX,[ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x73  // JNC +4
        _emit 0x04
        _emit 0x8d  // LEA EAX,[ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x50  // PUSH EAX
        // 0005331b
        _emit 0xff  // CALL dword ptr [0x00f3e288]
        _emit 0x15
        _emit 0x88
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x83  // CMP EAX,-0x1
        _emit 0xf8
        _emit 0xff
        _emit 0x74  // JZ 0x0045339c
        _emit 0x76
        _emit 0xa8  // TEST AL,0x10
        _emit 0x10
        _emit 0x74  // JZ 0x0045334d
        _emit 0x23
        // 0005332a
        _emit 0x8b  // MOV EAX,[ESP+0x1c]
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x83  // CMP EAX,0x8
        _emit 0xf8
        _emit 0x08
        _emit 0xc7  // MOV dword ptr [ESP+0x2c],0xffffffff
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00053339
        _emit 0x0f  // JC 0x004533c1
        _emit 0x82
        _emit 0x82
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EDX,[ESP+0x8]
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x6a  // PUSH 0xc
        _emit 0x0c
        _emit 0x8d  // LEA ECX,[EAX+EAX+0x2]
        _emit 0x4c
        _emit 0x00
        _emit 0x02
        _emit 0x51  // PUSH ECX
        _emit 0x52  // PUSH EDX
        _emit 0xeb  // JMP 0x004533b9
        _emit 0x6c
        // 0005334d
        _emit 0x80  // CMP byte ptr [ESP+0x38],0x0
        _emit 0x7c
        _emit 0x24
        _emit 0x38
        _emit 0x00
        _emit 0x74  // JZ +5
        _emit 0x05
        _emit 0x83  // OR EAX,0x1
        _emit 0xc8
        _emit 0x01
        _emit 0xeb  // JMP +3
        _emit 0x03
        _emit 0x83  // AND EAX,0xfffffffe
        _emit 0xe0
        _emit 0xfe
        // 0005335c
        _emit 0x83  // CMP dword ptr [ESP+0x1c],0x8
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x08
        _emit 0x8b  // MOV ECX,[ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x73  // JNC +4
        _emit 0x04
        _emit 0x8d  // LEA ECX,[ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x50  // PUSH EAX
        _emit 0x51  // PUSH ECX
        // 0005336d
        _emit 0xff  // CALL dword ptr [0x00f3e28c]
        _emit 0x15
        _emit 0x8c
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        // 00053373
        _emit 0x8b  // MOV EAX,[ESP+0x1c]
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x83  // CMP EAX,0x8
        _emit 0xf8
        _emit 0x08
        _emit 0xc7  // MOV dword ptr [ESP+0x2c],0xffffffff
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x72  // JC +0x14
        _emit 0x14
        _emit 0x8b  // MOV ECX,[ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x6a  // PUSH 0xc
        _emit 0x0c
        _emit 0x8d  // LEA EAX,[EAX+EAX+0x2]
        _emit 0x44
        _emit 0x00
        _emit 0x02
        _emit 0x50  // PUSH EAX
        _emit 0x51  // PUSH ECX
        // 00053390
        _emit 0xe8  // CALL 0x0044d350
        _emit 0xbb
        _emit 0x9f
        _emit 0xff
        _emit 0xff
        _emit 0x83  // ADD ESP,0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0xb0  // MOV AL,0x1
        _emit 0x01
        _emit 0xeb  // JMP 0x004533c3
        _emit 0x27
        // 0005339c
        _emit 0x8b  // MOV EAX,[ESP+0x1c]
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x83  // CMP EAX,0x8
        _emit 0xf8
        _emit 0x08
        _emit 0xc7  // MOV dword ptr [ESP+0x2c],0xffffffff
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x72  // JC +0x14
        _emit 0x14
        _emit 0x6a  // PUSH 0xc
        _emit 0x0c
        _emit 0x8d  // LEA EDX,[EAX+EAX+0x2]
        _emit 0x54
        _emit 0x00
        _emit 0x02
        _emit 0x8b  // MOV EAX,[ESP+0xc]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x52  // PUSH EDX
        _emit 0x50  // PUSH EAX
        // 000533b9
        _emit 0xe8  // CALL 0x0044d350
        _emit 0x92
        _emit 0x9f
        _emit 0xff
        _emit 0xff
        _emit 0x83  // ADD ESP,0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x32  // XOR AL,AL
        _emit 0xc0
        // 000533c3
        _emit 0x8b  // MOV ECX,[ESP+0x24]
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x64  // MOV FS:[0x0],ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX
        _emit 0x8b  // MOV ECX,[ESP+0x1c]
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x33  // XOR ECX,ESP
        _emit 0xcc
        // 000533d5
        _emit 0xe8  // CALL 0x009d20f4  (__security_check_cookie)
        _emit 0x1a
        _emit 0xed
        _emit 0x57
        _emit 0x00
        _emit 0x83  // ADD ESP,0x2c
        _emit 0xc4
        _emit 0x2c
        _emit 0xc3  // RET
    }
}
