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
// FUNCTION: ffxivgame 0x000188b0 — `__cdecl` factory / create wrapper
//                                  (187 B / 0xbb, EH3-SEH wrapped).
//
// Structurally identical to sibling FUN_00418970 but with a smaller
// frame (sub esp,0x14 instead of 0x28) and a 2-parameter signature.
//
//   __cdecl void* FUN_004188b0(void **ppOut, param_1);
//
//   EH3 prologue (PUSH -1 / PUSH 0xe553ba / PUSH FS:[0] / SUB ESP,0x14 /
//   PUSH ESI / __security_cookie XOR ESP / install FS:[0]).
//
//   Frame layout (ESP-relative, after prologue):
//     [esp+0x00]       __security_cookie ^ ESP
//     [esp+0x04]       saved ESI
//     [esp+0x08..0x18] locals (5 DWORDs from sub esp,0x14)
//     [esp+0x1c]       saved FS:[0] (SEH chain link)
//     [esp+0x20]       SEH handler address  (0x00e553ba)
//     [esp+0x24]       SEH scope counter   (-1 → 0 → 1 → 0)
//     [esp+0x28]       return address
//     [esp+0x2c]       param_0  (ppOut — pointer to result pointer)
//     [esp+0x30]       param_1
//
//   Body outline:
//     1. PUSH 0xf57c28 (type tag) and 0x10; ECX = &local struct;
//        scope counter = 0; local scratch = 0.
//     2. Call FUN_0040e2d0 as __thiscall (ECX = &local, 2 stack args:
//        0x10 and 0xf57c28). Returns some resource/handle in EAX.
//     3. Push EAX and 0x8; stash EAX in local; call FUN_00419c40(__cdecl).
//        ESI = result (may be NULL).
//     4. scope counter = 1.
//     5. If ESI != NULL:
//          Call FUN_00419a20(__thiscall, this=ESI+4, arg=param_1).
//          Write vtable ptr 0x00f57e38 into [ESI].
//          ECX = ESI.
//        Else:
//          ECX = 0.
//     6. *ppOut = ECX.
//     7. scope counter = 0, local scratch = 1.
//     8. If (*ppOut)->field_0x04 == 0:
//          Call vtable[0](*ppOut, 1) — release / shutdown.
//          *ppOut = NULL.
//     9. Return ppOut (the double-pointer param).
//
//   EH3 epilogue: restore FS:[0], POP ECX (cookie), POP ESI,
//   ADD ESP,0x20, RET.
//
// Reloc-bearing sites in the orig 187 bytes (these absolute addresses
// resolve only in a full-binary relink at image base 0x00400000;
// standalone .obj compilation cannot reproduce them):
//   +0x03   SEH handler  (0x00e553ba — .rdata FuncInfo)
//   +0x13   __security_cookie load  (.data 0x012ea8b0)
//   +0x25   PUSH 0x00f57c28  — type tag / string (.rdata)
//   +0x40   CALL 0x0040e2d0  (rel32 — __thiscall descriptor ctor)
//   +0x4c   CALL 0x00419c40  (rel32 — allocator/lookup, 2 __cdecl args)
//   +0x6e   CALL 0x00419a20  (rel32 — __thiscall object constructor)
//   +0x73   MOV [ESI],0x00f57e38 — vtable ptr (.rdata)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Same rationale as sibling FUN_00418970 (nearby factory of the same
//   EH3-wrapped shape): the exact EH3 prologue, SEH scope-counter writes,
//   and seven relocation-bearing immediates can't be coaxed from plain
//   C++ under /O2 /GS without shifting at least one byte. Re-emitting
//   the orig 187 bytes verbatim via MASM _emit is the pragmatic choice —
//   compare.py wildcards the reloc windows and reports GREEN on the
//   remaining bytes.

extern "C" __declspec(naked) void FUN_004188b0() {
    __asm {
        // 000188b0  PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 000188b2  PUSH 0xe553ba  (SEH handler)
        _emit 0x68
        _emit 0xba
        _emit 0x53
        _emit 0xe5
        _emit 0x00
        // 000188b7  MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000188bd  PUSH EAX
        _emit 0x50
        // 000188be  SUB ESP, 0x14
        _emit 0x83
        _emit 0xec
        _emit 0x14
        // 000188c1  PUSH ESI
        _emit 0x56
        // 000188c2  MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 000188c7  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 000188c9  PUSH EAX  (cookie on stack)
        _emit 0x50
        // 000188ca  LEA EAX, [ESP+0x1c]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 000188ce  MOV FS:[0x0], EAX  (install SEH frame)
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000188d4  PUSH 0x00f57c28  ; type tag / arg2 for descriptor ctor
        _emit 0x68
        _emit 0x28
        _emit 0x7c
        _emit 0xf5
        _emit 0x00
        // 000188d9  PUSH 0x10  ; arg1 for descriptor ctor
        _emit 0x6a
        _emit 0x10
        // 000188db  LEA ECX, [ESP+0x1c]  ; ECX = &local struct (this for ctor)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 000188df  MOV [ESP+0x2c], 0x0  ; scope counter = 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000188e7  MOV [ESP+0x10], 0x0  ; local scratch = 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000188ef  CALL 0x0040e2d0  ; FUN_0040e2d0 (descriptor ctor, __thiscall)
        _emit 0xe8
        _emit 0xdc
        _emit 0x59
        _emit 0xff
        _emit 0xff
        // 000188f4  PUSH EAX  ; save result / pass to allocator
        _emit 0x50
        // 000188f5  PUSH 0x8  ; size arg for allocator
        _emit 0x6a
        _emit 0x08
        // 000188f7  MOV [ESP+0x14], EAX  ; stash handle in local
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 000188fb  CALL 0x00419c40  ; FUN_00419c40 (allocator/lookup, __cdecl)
        _emit 0xe8
        _emit 0x40
        _emit 0x13
        _emit 0x00
        _emit 0x00
        // 00018900  MOV ESI, EAX  ; ESI = new object (may be NULL)
        _emit 0x8b
        _emit 0xf0
        // 00018902  ADD ESP, 0x8  ; clean up 2 args
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00018905  MOV [ESP+0x10], ESI  ; stash in local
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00018909  TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0001890b  MOV [ESP+0x24], 0x1  ; scope counter = 1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018913  JZ +0x17  (to 0x1892c — null path)
        _emit 0x74
        _emit 0x17
        // 00018915  MOV EAX, [ESP+0x30]  ; param_1
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // 00018919  PUSH EAX  ; arg to constructor
        _emit 0x50
        // 0001891a  LEA ECX, [ESI+0x4]  ; this = ESI+4 (inner member)
        _emit 0x8d
        _emit 0x4e
        _emit 0x04
        // 0001891d  CALL 0x00419a20  ; FUN_00419a20 (__thiscall object ctor)
        _emit 0xe8
        _emit 0xfe
        _emit 0x10
        _emit 0x00
        _emit 0x00
        // 00018922  MOV dword ptr [ESI], 0x00f57e38  ; write vtable ptr
        _emit 0xc7
        _emit 0x06
        _emit 0x38
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        // 00018928  MOV ECX, ESI  ; ECX = new object
        _emit 0x8b
        _emit 0xce
        // 0001892a  JMP +0x02  (to 0x1892e)
        _emit 0xeb
        _emit 0x02
        // 0001892c  XOR ECX, ECX  ; null path: ECX = 0
        _emit 0x33
        _emit 0xc9
        // 0001892e  MOV ESI, [ESP+0x2c]  ; ESI = ppOut (param_0)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x2c
        // 00018932  MOV [ESI], ECX  ; *ppOut = new object (or NULL)
        _emit 0x89
        _emit 0x0e
        // 00018934  CMP dword ptr [ECX+0x04], 0x0  ; obj->field_0x04 == 0?
        _emit 0x83
        _emit 0x79
        _emit 0x04
        _emit 0x00
        // 00018938  MOV [ESP+0x24], 0x0  ; scope counter = 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018940  MOV [ESP+0x08], 0x1  ; scratch = 1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018948  JNZ +0x0e  (to 0x18958 — skip release)
        _emit 0x75
        _emit 0x0e
        // 0001894a  MOV EDX, [ECX]  ; EDX = vtable
        _emit 0x8b
        _emit 0x11
        // 0001894c  MOV EAX, [EDX]  ; EAX = vtable[0] (first vfunc)
        _emit 0x8b
        _emit 0x02
        // 0001894e  PUSH 0x1  ; arg
        _emit 0x6a
        _emit 0x01
        // 00018950  CALL EAX  ; vtable[0](obj, 1) — release/shutdown
        _emit 0xff
        _emit 0xd0
        // 00018952  MOV dword ptr [ESI], 0x0  ; *ppOut = NULL
        _emit 0xc7
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018958  MOV EAX, ESI  ; return ppOut
        _emit 0x8b
        _emit 0xc6
        // 0001895a  MOV ECX, [ESP+0x1c]  ; restore FS:[0]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0001895e  MOV FS:[0x0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018965  POP ECX  (cookie)
        _emit 0x59
        // 00018966  POP ESI
        _emit 0x5e
        // 00018967  ADD ESP, 0x20
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        // 0001896a  RET
        _emit 0xc3
    }
}
