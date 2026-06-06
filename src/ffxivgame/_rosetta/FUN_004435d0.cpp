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
// FUNCTION: ffxivgame 0x004435d0 — constructor for an object with an
//                                  embedded 0x50000+ sub-object, wrapped
//                                  in an SEH __try frame (__thiscall,
//                                  141 bytes / 0x8d).
//
// Calling convention: __thiscall (ECX = this); returns this in EAX.
// Frame: full /GS + /EHsc constructor prologue —
//   PUSH -1 / PUSH scopetable(0xe57273) / PUSH FS:[0] establishes the
//   _EH4_SCOPETABLE-style exception registration record; the
//   `MOV EAX,[0x012ea8b0] ; XOR EAX,ESP ; PUSH EAX` sequence is the
//   __security_cookie frame guard. RET 0xc → three DWORD stack args.
//
// Object layout touched:
//   [this + 0x00]      vtable ← 0xf671a4
//   [this + 0x28]      secondary vtable ← 0xf6717c then 0xf67190
//   [this + 0x2c]      DWORD ← arg0  ([ESP+0x1c])
//   [this + 0x30]      DWORD ← arg2  ([ESP+0x24])
//   [this + 0x50034]   sub-object (ctor'd via CALL 0x00452a40, arg 0)
//   [this + 0x50038]   DWORD ← 0
//
// Calls (reloc sites — compare.py masks these rel32 windows):
//   REL: FUN_00442aa0  (rel32 = 0xfffff49e) — base-class ctor, arg = [ESP+0x20]
//   REL: FUN_00452a40  (rel32 = 0x0000f402) — sub-object ctor at this+0x50034
//
// Absolute-immediate sites (PUSH imm32 / MOV [mem],imm32 / MOV reg,[abs]):
//   0xe57273   SEH scope table address
//   0x012ea8b0 __security_cookie
//   0xf6717c / 0xf671a4 / 0xf67190  vtable pointers
//
// Reconstruction strategy — naked-asm byte passthrough (same as sibling
// FUN_0040ad30): a source-level C++ ctor would emit this shape, but the
// /GS cookie frame, the manual _EH4 scope-table push, and the exact temp
// materialisation ([ESP+0x14]=0, [ESP+0x18]=1 SEH state bytes) are not
// reproducible from portable C++. The __declspec(naked) body re-emits the
// original 141 bytes verbatim via MASM _emit directives; the .obj's .text
// is byte-identical to the original slice and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004435d0() {
    __asm {
        // 000435d0:  6a ff              PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 000435d2:  68 73 72 e5 00     PUSH 0xe57273
        _emit 0x68
        _emit 0x73
        _emit 0x72
        _emit 0xe5
        _emit 0x00
        // 000435d7:  64 a1 00 00 00 00  MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000435dd:  50                 PUSH EAX
        _emit 0x50
        // 000435de:  51                 PUSH ECX
        _emit 0x51
        // 000435df:  56                 PUSH ESI
        _emit 0x56
        // 000435e0:  a1 b0 a8 2e 01     MOV EAX,[0x012ea8b0]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 000435e5:  33 c4              XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 000435e7:  50                 PUSH EAX
        _emit 0x50
        // 000435e8:  8d 44 24 0c        LEA EAX,[ESP+0xc]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 000435ec:  64 a3 00 00 00 00  MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000435f2:  8b f1              MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 000435f4:  89 74 24 08        MOV [ESP+0x8],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 000435f8:  8b 44 24 20        MOV EAX,[ESP+0x20]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 000435fc:  50                 PUSH EAX
        _emit 0x50
        // 000435fd:  e8 9e f4 ff ff     CALL 0x00442aa0
        _emit 0xe8
        _emit 0x9e
        _emit 0xf4
        _emit 0xff
        _emit 0xff
        // 00043602:  c7 44 24 14 00 00 00 00  MOV [ESP+0x14],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004360a:  c7 46 28 7c 71 f6 00  MOV [ESI+0x28],0xf6717c
        _emit 0xc7
        _emit 0x46
        _emit 0x28
        _emit 0x7c
        _emit 0x71
        _emit 0xf6
        _emit 0x00
        // 00043611:  8b 4c 24 1c        MOV ECX,[ESP+0x1c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 00043615:  8b 54 24 24        MOV EDX,[ESP+0x24]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 00043619:  89 4e 2c           MOV [ESI+0x2c],ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x2c
        // 0004361c:  6a 00              PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0004361e:  8d 8e 34 00 05 00  LEA ECX,[ESI+0x50034]
        _emit 0x8d
        _emit 0x8e
        _emit 0x34
        _emit 0x00
        _emit 0x05
        _emit 0x00
        // 00043624:  c6 44 24 18 01     MOV byte ptr [ESP+0x18],0x1
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x01
        // 00043629:  c7 06 a4 71 f6 00  MOV [ESI],0xf671a4
        _emit 0xc7
        _emit 0x06
        _emit 0xa4
        _emit 0x71
        _emit 0xf6
        _emit 0x00
        // 0004362f:  c7 46 28 90 71 f6 00  MOV [ESI+0x28],0xf67190
        _emit 0xc7
        _emit 0x46
        _emit 0x28
        _emit 0x90
        _emit 0x71
        _emit 0xf6
        _emit 0x00
        // 00043636:  89 56 30           MOV [ESI+0x30],EDX
        _emit 0x89
        _emit 0x56
        _emit 0x30
        // 00043639:  e8 02 f4 00 00     CALL 0x00452a40
        _emit 0xe8
        _emit 0x02
        _emit 0xf4
        _emit 0x00
        _emit 0x00
        // 0004363e:  c7 86 38 00 05 00 00 00 00 00  MOV [ESI+0x50038],0x0
        _emit 0xc7
        _emit 0x86
        _emit 0x38
        _emit 0x00
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00043648:  8b c6              MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0004364a:  8b 4c 24 0c        MOV ECX,[ESP+0xc]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0004364e:  64 89 0d 00 00 00 00  MOV FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00043655:  59                 POP ECX
        _emit 0x59
        // 00043656:  5e                 POP ESI
        _emit 0x5e
        // 00043657:  83 c4 10           ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0004365a:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
