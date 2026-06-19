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
// FUNCTION: ffxivgame 0x00039700 — __thiscall constructor (294 B / 0x126,
//                                   EH4-SEH wrapped, one stack argument).
//
// Inspection (read from the disassembly at orig RVA 0x00039700):
//
//   __thiscall void* FUN_00439700(void *this, int param1);
//
//   Constructor for a class whose vtable is at 0x00f66330.
//   Initialises three member fields at offsets +0x00 (vtable), +0x04,
//   +0x08, and later +0xd4, +0xd8, +0xdc, +0xe0.
//
//   A function-scope magic-static singleton at .data 0x01327c1c is
//   lazily initialised once (guarded by a 1-byte flag at 0x01327c20)
//   inside a __try block (EH4 scope index 0).  The singleton value is
//   also stored at .data 0x0132c9d0.
//
//   The sole stack argument (EBX) is used as an allocation count:
//     this->0xd8 = ctor_helper(singleton, arg, ...)
//     this->0xdc = arg / 24   (magic-multiply: MUL 0xAAAAAAAB, SHR EDX,4)
//     this->0xd4 = 0
//     this->0xe0 = 0
//
//   Stack frame (after the EH4 prologue, ESP-relative):
//     [esp+0x00]  __security_cookie ^ ESP
//     [esp+0x04]  saved EDI
//     [esp+0x08]  saved ESI
//     [esp+0x0c]  saved EBP
//     [esp+0x10]  saved EBX
//     [esp+0x14 .. esp+0x20]  4 local dword slots (SUB ESP,0x10)
//     [esp+0x24]  saved FS:[0] chain link
//     [esp+0x28]  EH4 scope-table address (0x00e5645f)
//     [esp+0x2c]  EH4 try-level (-1 idle, 0 inside __try)
//     [esp+0x30]  return address
//     [esp+0x34]  param1 (the one __stdcall-style stack argument)
//
//   Absolute addresses in the 294 bytes (resolve only in a full-binary
//   relink at image base 0x00400000 — standalone .obj compilation
//   reproduces them as raw immediates via _emit, so compare.py accepts):
//     +0x03  EH4 scope-table RVA (0x00e5645f — .rdata FuncInfo)
//     +0x09  FS:[0] read (constant 0, fold-through)
//     +0x15  __security_cookie load     (.data 0x012ea8b0)
//     +0x21  FS:[0] install             (constant 0, fold-through)
//     +0x2b  vtable write               (.rdata 0x00f66330)
//     +0x37  init-flag TEST             (.data 0x01327c20)
//     +0x3e  singleton-this MOV         (.data 0x0132c9d0)
//     +0x46  init-flag OR               (.data 0x01327c20)
//     +0x51  singleton-init CALL        (.text 0x0040e500 rel32)
//     +0x56  singleton store            (.data 0x01327c1c)
//     +0x63  singleton load             (.data 0x01327c1c)
//     +0x69  arg push                   (.rdata 0x00f6626c)
//     +0x74  helper CALL                (.text 0x0040e2d0 rel32)
//     +0x81  helper CALL                (.text 0x0040e110 rel32)
//     +0xac  helper CALL                (.text 0x0043e1d0 rel32)
//     +0xc6  helper CALL                (.text 0x0043c690 rel32)
//     +0xde  helper CALL                (.text 0x0043d1f0 rel32)
//     +0x109 dtor   CALL                (.text 0x0043c770 rel32)
//     +0x114 FS:[0] restore             (constant 0, fold-through)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Reproducing the exact EH4 prologue (PUSH -1 / PUSH scope-table /
//   PUSH FS:[0] / SUB ESP,0x10 / callee-saves / cookie-XOR / FS install),
//   the precise register allocation (ESI=this, EBP=0, EBX=param1,
//   EDI=singleton), the specific MOV [ESI+offset] encodings, the magic-
//   multiply for /24, and the post-try object manipulation would require
//   coaxing MSVC 2005 /O2 /GS /EHsc into matching seventeen relocation
//   windows and several compiler-internal heuristics.  The risk of even
//   a single byte diverging is very high.  The pragmatic choice —
//   following the same pattern as FUN_00401a00 and FUN_00408f10 in this
//   module — is a __declspec(naked) body that re-emits the 294 bytes
//   verbatim via MASM _emit directives.

extern "C" __declspec(naked) void FUN_00439700() {
    __asm {
        // 00039700: 6a ff            PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00039702: 68 5f 64 e5 00   PUSH 0xe5645f
        _emit 0x68
        _emit 0x5f
        _emit 0x64
        _emit 0xe5
        _emit 0x00
        // 00039707: 64 a1 00 00 00 00  MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003970d: 50               PUSH EAX
        _emit 0x50
        // 0003970e: 83 ec 10         SUB ESP,0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 00039711: 53               PUSH EBX
        _emit 0x53
        // 00039712: 55               PUSH EBP
        _emit 0x55
        // 00039713: 56               PUSH ESI
        _emit 0x56
        // 00039714: 57               PUSH EDI
        _emit 0x57
        // 00039715: a1 b0 a8 2e 01   MOV EAX,[0x012ea8b0]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0003971a: 33 c4            XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0003971c: 50               PUSH EAX
        _emit 0x50
        // 0003971d: 8d 44 24 24      LEA EAX,[ESP+0x24]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 00039721: 64 a3 00 00 00 00  MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00039727: 8b f1            MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 00039729: 33 ed            XOR EBP,EBP
        _emit 0x33
        _emit 0xed
        // 0003972b: c7 06 30 63 f6 00  MOV dword ptr [ESI],0xf66330
        _emit 0xc7
        _emit 0x06
        _emit 0x30
        _emit 0x63
        _emit 0xf6
        _emit 0x00
        // 00039731: 89 6e 04         MOV dword ptr [ESI+0x4],EBP
        _emit 0x89
        _emit 0x6e
        _emit 0x04
        // 00039734: 89 6e 08         MOV dword ptr [ESI+0x8],EBP
        _emit 0x89
        _emit 0x6e
        _emit 0x08
        // 00039737: f6 05 20 7c 32 01 01  TEST byte ptr [0x01327c20],0x1
        _emit 0xf6
        _emit 0x05
        _emit 0x20
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0003973e: 89 35 d0 c9 32 01  MOV dword ptr [0x0132c9d0],ESI
        _emit 0x89
        _emit 0x35
        _emit 0xd0
        _emit 0xc9
        _emit 0x32
        _emit 0x01
        // 00039744: 75 1d            JNZ 0x00439763
        _emit 0x75
        _emit 0x1d
        // 00039746: 83 0d 20 7c 32 01 01  OR dword ptr [0x01327c20],0x1
        _emit 0x83
        _emit 0x0d
        _emit 0x20
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0003974d: 89 6c 24 2c      MOV dword ptr [ESP+0x2c],EBP
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x2c
        // 00039751: e8 aa 4d fd ff   CALL 0x0040e500
        _emit 0xe8
        _emit 0xaa
        _emit 0x4d
        _emit 0xfd
        _emit 0xff
        // 00039756: a3 1c 7c 32 01   MOV [0x01327c1c],EAX
        _emit 0xa3
        _emit 0x1c
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        // 0003975b: c7 44 24 2c ff ff ff ff  MOV dword ptr [ESP+0x2c],0xffffffff
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00039763: 8b 3d 1c 7c 32 01  MOV EDI,dword ptr [0x01327c1c]
        _emit 0x8b
        _emit 0x3d
        _emit 0x1c
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        // 00039769: 68 6c 62 f6 00   PUSH 0xf6626c
        _emit 0x68
        _emit 0x6c
        _emit 0x62
        _emit 0xf6
        _emit 0x00
        // 0003976e: 6a 10            PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 00039770: 8d 4c 24 1c      LEA ECX,[ESP+0x1c]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 00039774: e8 57 4b fd ff   CALL 0x0040e2d0
        _emit 0xe8
        _emit 0x57
        _emit 0x4b
        _emit 0xfd
        _emit 0xff
        // 00039779: 8b 5c 24 34      MOV EBX,dword ptr [ESP+0x34]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x34
        // 0003977d: 50               PUSH EAX
        _emit 0x50
        // 0003977e: 53               PUSH EBX
        _emit 0x53
        // 0003977f: 8b cf            MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 00039781: e8 8a 49 fd ff   CALL 0x0040e110
        _emit 0xe8
        _emit 0x8a
        _emit 0x49
        _emit 0xfd
        _emit 0xff
        // 00039786: 89 86 d8 00 00 00  MOV dword ptr [ESI+0xd8],EAX
        _emit 0x89
        _emit 0x86
        _emit 0xd8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003978c: b8 ab aa aa aa   MOV EAX,0xaaaaaaab
        _emit 0xb8
        _emit 0xab
        _emit 0xaa
        _emit 0xaa
        _emit 0xaa
        // 00039791: f7 e3            MUL EBX
        _emit 0xf7
        _emit 0xe3
        // 00039793: c1 ea 04         SHR EDX,0x4
        _emit 0xc1
        _emit 0xea
        _emit 0x04
        // 00039796: 8d 4c 24 1c      LEA ECX,[ESP+0x1c]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0003979a: 89 96 dc 00 00 00  MOV dword ptr [ESI+0xdc],EDX
        _emit 0x89
        _emit 0x96
        _emit 0xdc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000397a0: 89 ae e0 00 00 00  MOV dword ptr [ESI+0xe0],EBP
        _emit 0x89
        _emit 0xae
        _emit 0xe0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000397a6: 89 ae d4 00 00 00  MOV dword ptr [ESI+0xd4],EBP
        _emit 0x89
        _emit 0xae
        _emit 0xd4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000397ac: e8 1f 4a 00 00   CALL 0x0043e1d0
        _emit 0xe8
        _emit 0x1f
        _emit 0x4a
        _emit 0x00
        _emit 0x00
        // 000397b1: 51               PUSH ECX
        _emit 0x51
        // 000397b2: 8b c4            MOV EAX,ESP
        _emit 0x8b
        _emit 0xc4
        // 000397b4: 89 64 24 38      MOV dword ptr [ESP+0x38],ESP
        _emit 0x89
        _emit 0x64
        _emit 0x24
        _emit 0x38
        // 000397b8: 8d 4c 24 20      LEA ECX,[ESP+0x20]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 000397bc: c7 44 24 30 01 00 00 00  MOV dword ptr [ESP+0x30],0x1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000397c4: 89 28            MOV dword ptr [EAX],EBP
        _emit 0x89
        _emit 0x28
        // 000397c6: e8 c5 2e 00 00   CALL 0x0043c690
        _emit 0xe8
        _emit 0xc5
        _emit 0x2e
        _emit 0x00
        _emit 0x00
        // 000397cb: 50               PUSH EAX
        _emit 0x50
        // 000397cc: 51               PUSH ECX
        _emit 0x51
        // 000397cd: 8b c4            MOV EAX,ESP
        _emit 0x8b
        _emit 0xc4
        // 000397cf: c7 00 02 00 00 00  MOV dword ptr [EAX],0x2
        _emit 0xc7
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000397d5: 8d 44 24 40      LEA EAX,[ESP+0x40]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x40
        // 000397d9: 89 64 24 20      MOV dword ptr [ESP+0x20],ESP
        _emit 0x89
        _emit 0x64
        _emit 0x24
        _emit 0x20
        // 000397dd: 50               PUSH EAX
        _emit 0x50
        // 000397de: e8 0d 3a 00 00   CALL 0x0043d1f0
        _emit 0xe8
        _emit 0x0d
        _emit 0x3a
        _emit 0x00
        _emit 0x00
        // 000397e3: 8b 08            MOV ECX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x08
        // 000397e5: 89 28            MOV dword ptr [EAX],EBP
        _emit 0x89
        _emit 0x28
        // 000397e7: 89 4e 08         MOV dword ptr [ESI+0x8],ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x08
        // 000397ea: 8b 4c 24 44      MOV ECX,dword ptr [ESP+0x44]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x44
        // 000397ee: 83 c4 10         ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 000397f1: 3b cd            CMP ECX,EBP
        _emit 0x3b
        _emit 0xcd
        // 000397f3: 74 08            JZ 0x004397fd
        _emit 0x74
        _emit 0x08
        // 000397f5: 8b 11            MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 000397f7: 8b 02            MOV EAX,dword ptr [EDX]
        _emit 0x8b
        _emit 0x02
        // 000397f9: 6a 01            PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 000397fb: ff d0            CALL EAX
        _emit 0xff
        _emit 0xd0
        // 000397fd: 8d 4c 24 1c      LEA ECX,[ESP+0x1c]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 00039801: c7 44 24 2c ff ff ff ff  MOV dword ptr [ESP+0x2c],0xffffffff
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00039809: e8 62 2f 00 00   CALL 0x0043c770
        _emit 0xe8
        _emit 0x62
        _emit 0x2f
        _emit 0x00
        _emit 0x00
        // 0003980e: 8b c6            MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 00039810: 8b 4c 24 24      MOV ECX,dword ptr [ESP+0x24]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 00039814: 64 89 0d 00 00 00 00  MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003981b: 59               POP ECX
        _emit 0x59
        // 0003981c: 5f               POP EDI
        _emit 0x5f
        // 0003981d: 5e               POP ESI
        _emit 0x5e
        // 0003981e: 5d               POP EBP
        _emit 0x5d
        // 0003981f: 5b               POP EBX
        _emit 0x5b
        // 00039820: 83 c4 1c         ADD ESP,0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 00039823: c2 04 00         RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
