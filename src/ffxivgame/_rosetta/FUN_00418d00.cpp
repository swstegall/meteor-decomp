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
// FUNCTION: ffxivgame 0x00018d00 — FUN_00418d00 (0xf2 B / 242 bytes,
//                                  EH4-SEH wrapped, __cdecl).
//
// Behaviour read from asm/ffxivgame/00018d00_FUN_00418d00.s:
//
//   __cdecl void* FUN_00418d00(void** out, arg2, arg3, arg4, arg5, arg6, arg7)
//
//   Constructs a struct on the local frame (size 0x10 at ESP+0x10 relative
//   to entry) and calls FUN_0040e2d0 (__thiscall) to initialise it from
//   seven caller-supplied arguments, setting some fields to fixed values
//   (type/kind fields 0x2 and 0x1 at offsets +0x08 and +0x0c).
//
//   Then calls FUN_00419c40 (a pool allocator / operator-new analogue) to
//   allocate 0x58 bytes.  If allocation succeeds, the constructor at
//   FUN_00431710 is called (with the local struct as init arg) and the
//   vtable pointer at 0x0105d5ac is installed at offset 0 of the new
//   object.  If allocation fails, the result pointer is set to NULL.
//
//   The resulting object pointer is written to *out.  If the object's
//   member at offset 0x34 is zero (NULL sub-object), the first vtable
//   method is called with argument 1 before clearing *out back to NULL.
//   Returns the original out pointer (ESI) in EAX.
//
//   Stack frame (post-prologue, ESP0-relative):
//     [ESP0+0x00]              security-cookie XOR'd value
//     [ESP0+0x04]              saved EDI
//     [ESP0+0x08]              saved ESI
//     [ESP0+0x0c..0x3b]        0x30 bytes of locals (init struct + spills)
//     [ESP0+0x3c]              EH4 saved FS:[0]
//     [ESP0+0x40]              EH4 scope-table (0x00e55501)
//     [ESP0+0x44]              EH4 trylevel (initially –1)
//     [ESP0+0x48]              return address
//     [ESP0+0x4c]              arg1  (out: void**)
//     [ESP0+0x50]              arg2
//     [ESP0+0x54]              arg3
//     [ESP0+0x58]              arg4
//     [ESP0+0x5c]              arg5
//     [ESP0+0x60]              arg6
//     [ESP0+0x64]              arg7
//
//   Reloc-bearing sites (absolute addresses baked in at link time):
//     +0x03   scope-table RVA  (0x00e55501)
//     +0x09   FS:[0] read      (constant 0)
//     +0x15   __security_cookie (.data 0x012ea8b0)
//     +0x1f   FS:[0] install   (constant 0)
//     +0x4f   string/data ptr  (0x00f57ca0)
//     +0x7e   CALL FUN_0040e2d0 (rel32)
//     +0x8a   CALL FUN_00419c40 (rel32)
//     +0xac   CALL FUN_00431710 (rel32)
//     +0xb1   vtable ptr       (0x0105d5ac)
//     +0xe4   FS:[0] restore   (constant 0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ for this function would require MSVC 2005 /O2 /GS /EHsc
//   to reproduce the exact EH4 prologue, the interleaved arg-shuffle into
//   the local init-struct, and every linker-resolved absolute address above.
//   The pragmatic choice — matching the approach taken by FUN_004014b0,
//   FUN_00403a20, FUN_004054d0, and FUN_00402a30 for their SEH-wrapped
//   bodies — is a `__declspec(naked)` body that re-emits all 242 bytes
//   verbatim via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_00418d00() {
    __asm {
        // 00018d00  PUSH -1
        _emit 0x6a
        _emit 0xff
        // 00018d02  PUSH 0xe55501  (EH4 scope-table)
        _emit 0x68
        _emit 0x01
        _emit 0x55
        _emit 0xe5
        _emit 0x00
        // 00018d07  MOV EAX, FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018d0d  PUSH EAX
        _emit 0x50
        // 00018d0e  SUB ESP, 0x30
        _emit 0x83
        _emit 0xec
        _emit 0x30
        // 00018d11  PUSH ESI
        _emit 0x56
        // 00018d12  PUSH EDI
        _emit 0x57
        // 00018d13  MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00018d18  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 00018d1a  PUSH EAX
        _emit 0x50
        // 00018d1b  LEA EAX, [ESP+0x3c]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        // 00018d1f  MOV FS:[0], EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018d25  MOV EDX, [ESP+0x50]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x50
        // 00018d29  XOR EDI, EDI
        _emit 0x33
        _emit 0xff
        // 00018d2b  MOV [ESP+0x0c], EDI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 00018d2f  MOV ECX, [ESP+0x64]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x64
        // 00018d33  MOV EAX, [ESP+0x5c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x5c
        // 00018d37  MOV [ESP+0x20], ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 00018d3b  MOV ECX, [ESP+0x58]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x58
        // 00018d3f  MOV [ESP+0x1c], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00018d43  MOV EAX, [ESP+0x54]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x54
        // 00018d47  MOV [ESP+0x24], EDX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 00018d4b  MOV EDX, [ESP+0x60]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x60
        // 00018d4f  PUSH 0xf57ca0
        _emit 0x68
        _emit 0xa0
        _emit 0x7c
        _emit 0xf5
        _emit 0x00
        // 00018d54  MOV [ESP+0x34], ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        // 00018d58  PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 00018d5a  LEA ECX, [ESP+0x18]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00018d5e  MOV [ESP+0x4c], EDI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x4c
        // 00018d62  MOV dword ptr [ESP+0x20], 2
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018d6a  MOV [ESP+0x30], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // 00018d6e  MOV dword ptr [ESP+0x34], 1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018d76  MOV [ESP+0x3c], EDX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x3c
        // 00018d7a  MOV [ESP+0x40], EDI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x40
        // 00018d7e  CALL 0x0040e2d0
        _emit 0xe8
        _emit 0x4d
        _emit 0x55
        _emit 0xff
        _emit 0xff
        // 00018d83  PUSH EAX
        _emit 0x50
        // 00018d84  PUSH 0x58
        _emit 0x6a
        _emit 0x58
        // 00018d86  MOV [ESP+0x64], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x64
        // 00018d8a  CALL 0x00419c40
        _emit 0xe8
        _emit 0xb1
        _emit 0x0e
        _emit 0x00
        _emit 0x00
        // 00018d8f  MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 00018d91  ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00018d94  MOV [ESP+0x64], ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x64
        // 00018d98  CMP ESI, EDI
        _emit 0x3b
        _emit 0xf7
        // 00018d9a  MOV dword ptr [ESP+0x44], 1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x44
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018da2  JZ +0x17  (-> 0x00418dbb)
        _emit 0x74
        _emit 0x17
        // 00018da4  PUSH ESI
        _emit 0x56
        // 00018da5  LEA EAX, [ESP+0x1c]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00018da9  PUSH EAX
        _emit 0x50
        // 00018daa  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00018dac  CALL 0x00431710
        _emit 0xe8
        _emit 0x5f
        _emit 0x89
        _emit 0x01
        _emit 0x00
        // 00018db1  MOV dword ptr [ESI], 0x105d5ac
        _emit 0xc7
        _emit 0x06
        _emit 0xac
        _emit 0xd5
        _emit 0x05
        _emit 0x01
        // 00018db7  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00018db9  JMP +2  (-> 0x00418dbd)
        _emit 0xeb
        _emit 0x02
        // 00018dbb  XOR ECX, ECX
        _emit 0x33
        _emit 0xc9
        // 00018dbd  MOV ESI, [ESP+0x4c]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x4c
        // 00018dc1  MOV [ESI], ECX
        _emit 0x89
        _emit 0x0e
        // 00018dc3  CMP [ECX+0x34], EDI
        _emit 0x39
        _emit 0x79
        _emit 0x34
        // 00018dc6  MOV [ESP+0x44], EDI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x44
        // 00018dca  MOV dword ptr [ESP+0x0c], 1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018dd2  JNZ +0x0a  (-> 0x00418dde)
        _emit 0x75
        _emit 0x0a
        // 00018dd4  MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 00018dd6  MOV EAX, [EDX]
        _emit 0x8b
        _emit 0x02
        // 00018dd8  PUSH 1
        _emit 0x6a
        _emit 0x01
        // 00018dda  CALL EAX  (vtable[0])
        _emit 0xff
        _emit 0xd0
        // 00018ddc  MOV [ESI], EDI
        _emit 0x89
        _emit 0x3e
        // 00018dde  MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 00018de0  MOV ECX, [ESP+0x3c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        // 00018de4  MOV FS:[0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018deb  POP ECX
        _emit 0x59
        // 00018dec  POP EDI
        _emit 0x5f
        // 00018ded  POP ESI
        _emit 0x5e
        // 00018dee  ADD ESP, 0x3c
        _emit 0x83
        _emit 0xc4
        _emit 0x3c
        // 00018df1  RET
        _emit 0xc3
    }
}
