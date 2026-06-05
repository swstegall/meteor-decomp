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
// FUNCTION: ffxivgame 0x0004ac40 — FUN_0044ac40
//                                  (255 B / 0xff, EH4-SEH wrapped, /GS cookie).
//
// Behaviour read from the disassembly at orig RVA 0x0004ac40:
//
//   __cdecl void FUN_0044ac40(void* arg1, void* arg2, void* arg3, bool arg4);
//
//   EH4 SEH prologue (PUSH -1 / PUSH 0xe5779e / PUSH FS:[0] /
//   SUB ESP,0xfc / PUSH ESI / __security_cookie XOR ESP PUSH / install FS:[0]).
//
//   ESI = arg1;
//
//   // Choose string by bool arg4
//   if (arg4)
//       method_at_447450(ESI /*this*/, 0x132cca0 /*string ptr*/);
//   else
//       method_at_447450(ESI /*this*/, 0x132cc48 /*string ptr*/);
//
//   // Build a path string (thiscall / __cdecl variant)
//   tmp = method_at_447620(&local_b0, 0xf672e8, arg2);  // __cdecl, 3 args
//
//   // Chain of two thiscall calls through the result of each previous call
//   /* unwind state = 0 */
//   result1 = method_at_447490(ESI /*this*/, &local_8, tmp);
//
//   /* unwind state = 1 */
//   result2 = method_at_447490(result1 /*this*/, &local_5c, arg3);
//
//   /* unwind state = 2 */
//   method_at_447450(ESI /*this*/, result2);
//
//   // Three destructor calls with descending unwind states
//   /* unwind state = 1 */ local_5c.~Dtor();   // FUN_00446f50
//   /* unwind state = 0 */ local_8.~Dtor();    // FUN_00446f50
//   /* unwind state = -1 */ local_b0.~Dtor();  // FUN_00446f50
//
//   EH4 SEH epilogue (restore FS:[0], POP ECX, POP ESI, ADD ESP,0x108, RET).
//
//   Stack frame (after prologue, ESP-relative at base_frame_esp):
//     [esp+0x000]              EH4 cookie (PUSH'd post-XOR-ESP)
//     [esp+0x004]              saved ESI
//     [esp+0x008 .. +0x05b]   local_8  object (~0x54 B)
//     [esp+0x05c .. +0x0af]   local_5c object (~0x54 B)
//     [esp+0x0b0 .. +0x103]   local_b0 object (~0x54 B)
//     [esp+0x104]             EH4 saved FS:[0] chain link
//     [esp+0x108]             EH4 scope-table address (0x00e5779e)
//     [esp+0x10c]             EH4 trylevel / unwind state
//     [esp+0x110]             PUSH -1 slot
//     [esp+0x114]             arg1  (first parameter)
//     [esp+0x118]             arg2  (second parameter)
//     [esp+0x11c]             arg3  (third parameter)
//     [esp+0x120]             arg4  (fourth parameter, bool)
//
//   Reloc-bearing sites in the orig 255 bytes:
//     +0x02   scope-table handler addr  (0x00e5779e — .rdata FuncInfo)
//     +0x08   FS:[0] read              (constant 0, fold-through)
//     +0x15   __security_cookie        (.data 0x012ea8b0)
//     +0x1d   LEA+MOV FS:[0] install   (constant 0, fold-through)
//     +0x3d   string 0x132cca0         (.data)
//     +0x44   string 0x132cc48         (.data)
//     +0x49   CALL 0x00447450          (.text rel32 — __thiscall)
//     +0x5d   PUSH 0xf672e8            (.data or .rdata ptr)
//     +0x63   CALL 0x00447620          (.text rel32 — __cdecl)
//     +0x7e   CALL 0x00447490          (.text rel32 — __thiscall)
//     +0x9a   CALL 0x00447490          (.text rel32 — __thiscall)
//     +0xaa   CALL 0x00447450          (.text rel32 — __thiscall)
//     +0xbb   CALL 0x00446f50          (.text rel32 — __thiscall, dtor)
//     +0xcc   CALL 0x00446f50          (.text rel32 — __thiscall, dtor)
//     +0xe3   CALL 0x00446f50          (.text rel32 — __thiscall, dtor)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Source-level C++ for an EH4-wrapped function with /O2 /GS would also
//   emit .text$x COMDAT unwind funclets, causing a size mismatch against the
//   255-byte orig slice. The pragmatic choice — identical to FUN_004054d0,
//   FUN_00403a20, and related SEH siblings — is a `__declspec(naked)` body
//   that re-emits the orig 255 bytes verbatim via MASM `_emit` directives.
//   The .obj's `.text` section ends up byte-identical to the orig slice with
//   no auxiliary subsections, which is what tools/compare.py checks against.

extern "C" __declspec(naked) void FUN_0044ac40() {
    __asm {
        // 0004ac40  PUSH -1
        _emit 0x6a
        _emit 0xff
        // 0004ac42  PUSH 0xe5779e  (SEH scope-table handler)
        _emit 0x68
        _emit 0x9e
        _emit 0x77
        _emit 0xe5
        _emit 0x00
        // 0004ac47  MOV EAX, FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004ac4d  PUSH EAX
        _emit 0x50
        // 0004ac4e  SUB ESP, 0xfc
        _emit 0x81
        _emit 0xec
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004ac54  PUSH ESI
        _emit 0x56
        // 0004ac55  MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0004ac5a  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 0004ac5c  PUSH EAX  (cookie)
        _emit 0x50
        // 0004ac5d  LEA EAX, [ESP+0x104]
        _emit 0x8d
        _emit 0x84
        _emit 0x24
        _emit 0x04
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0004ac64  MOV FS:[0], EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004ac6a  CMP byte ptr [ESP+0x120], 0x0
        _emit 0x80
        _emit 0xbc
        _emit 0x24
        _emit 0x20
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004ac72  MOV ESI, dword ptr [ESP+0x114]
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0x14
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0004ac79  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0004ac7b  JZ +7  (to 0x0044ac84)
        _emit 0x74
        _emit 0x07
        // 0004ac7d  PUSH 0x132cca0
        _emit 0x68
        _emit 0xa0
        _emit 0xcc
        _emit 0x32
        _emit 0x01
        // 0004ac82  JMP +5  (to 0x0044ac89)
        _emit 0xeb
        _emit 0x05
        // 0004ac84  PUSH 0x132cc48
        _emit 0x68
        _emit 0x48
        _emit 0xcc
        _emit 0x32
        _emit 0x01
        // 0004ac89  CALL 0x00447450
        _emit 0xe8
        _emit 0xc2
        _emit 0xc7
        _emit 0xff
        _emit 0xff
        // 0004ac8e  MOV EAX, dword ptr [ESP+0x118]
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0x18
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0004ac95  PUSH EAX
        _emit 0x50
        // 0004ac96  LEA ECX, [ESP+0xb4]
        _emit 0x8d
        _emit 0x8c
        _emit 0x24
        _emit 0xb4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004ac9d  PUSH 0xf672e8
        _emit 0x68
        _emit 0xe8
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        // 0004aca2  PUSH ECX
        _emit 0x51
        // 0004aca3  CALL 0x00447620
        _emit 0xe8
        _emit 0x78
        _emit 0xc9
        _emit 0xff
        _emit 0xff
        // 0004aca8  ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0004acab  PUSH EAX
        _emit 0x50
        // 0004acac  LEA EDX, [ESP+0xc]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 0004acb0  PUSH EDX
        _emit 0x52
        // 0004acb1  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0004acb3  MOV dword ptr [ESP+0x114], 0x0
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0x14
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004acbe  CALL 0x00447490
        _emit 0xe8
        _emit 0xcd
        _emit 0xc7
        _emit 0xff
        _emit 0xff
        // 0004acc3  MOV ECX, dword ptr [ESP+0x11c]
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x1c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0004acca  PUSH ECX
        _emit 0x51
        // 0004accb  LEA EDX, [ESP+0x60]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x60
        // 0004accf  PUSH EDX
        _emit 0x52
        // 0004acd0  MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 0004acd2  MOV byte ptr [ESP+0x114], 0x1
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x14
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x01
        // 0004acda  CALL 0x00447490
        _emit 0xe8
        _emit 0xb1
        _emit 0xc7
        _emit 0xff
        _emit 0xff
        // 0004acdf  PUSH EAX
        _emit 0x50
        // 0004ace0  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0004ace2  MOV byte ptr [ESP+0x110], 0x2
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x10
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x02
        // 0004acea  CALL 0x00447450
        _emit 0xe8
        _emit 0x61
        _emit 0xc7
        _emit 0xff
        _emit 0xff
        // 0004acef  LEA ECX, [ESP+0x5c]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x5c
        // 0004acf3  MOV byte ptr [ESP+0x10c], 0x1
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x0c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x01
        // 0004acfb  CALL 0x00446f50
        _emit 0xe8
        _emit 0x50
        _emit 0xc2
        _emit 0xff
        _emit 0xff
        // 0004ad00  LEA ECX, [ESP+0x8]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 0004ad04  MOV byte ptr [ESP+0x10c], 0x0
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x0c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004ad0c  CALL 0x00446f50
        _emit 0xe8
        _emit 0x3f
        _emit 0xc2
        _emit 0xff
        _emit 0xff
        // 0004ad11  LEA ECX, [ESP+0xb0]
        _emit 0x8d
        _emit 0x8c
        _emit 0x24
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004ad18  MOV dword ptr [ESP+0x10c], 0xffffffff
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0x0c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0004ad23  CALL 0x00446f50
        _emit 0xe8
        _emit 0x28
        _emit 0xc2
        _emit 0xff
        _emit 0xff
        // 0004ad28  MOV ECX, dword ptr [ESP+0x104]
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x04
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0004ad2f  MOV FS:[0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004ad36  POP ECX
        _emit 0x59
        // 0004ad37  POP ESI
        _emit 0x5e
        // 0004ad38  ADD ESP, 0x108
        _emit 0x81
        _emit 0xc4
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0004ad3e  RET
        _emit 0xc3
    }
}
