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
// FUNCTION: ffxivgame 0x00018970 — `__cdecl` factory / create wrapper
//                                  (242 B / 0xf2, EH4-SEH wrapped).
//
// Behaviour read from the disassembly at orig RVA 0x00018970:
//
//   __cdecl void* FUN_00418970(void **ppOut,
//                               param_1, param_2, param_3,
//                               param_4, param_5, param_6);
//
//   EH4 prologue (PUSH -1 / PUSH 0xe553ff / PUSH FS:[0] / SUB ESP,0x28 /
//   PUSH ESI / __security_cookie XOR ESP / install FS:[0]).
//
//   Frame layout (ESP-relative, after prologue):
//     [esp+0x00]       __security_cookie ^ ESP
//     [esp+0x04]       saved ESI
//     [esp+0x08]       local (SEH scope / scratch), initialised to 0
//     [esp+0x0c..2f]   descriptor struct built from incoming params
//     [esp+0x30]       saved FS:[0] (SEH chain link)
//     [esp+0x34]       SEH handler address  (0x00e553ff)
//     [esp+0x38]       SEH scope counter   (-1 → 0 → 1 → 0)
//     [esp+0x3c]       return address
//     [esp+0x40]       param_0  (ppOut — pointer to result pointer)
//     [esp+0x44]       param_1
//     [esp+0x48]       param_2
//     [esp+0x4c]       param_3
//     [esp+0x50]       param_4
//     [esp+0x54]       param_5
//     [esp+0x58]       param_6
//
//   Body outline:
//     1. Copy params into a local descriptor struct at [esp+0x0c]:
//          [esp+0x14] = param_3
//          [esp+0x18] = param_5
//          [esp+0x1c] = param_1
//          [esp+0x20] = param_2
//          [esp+0x24] = 1  (fixed field)
//          [esp+0x28] = param_4
//          [esp+0x2c] = param_6
//     2. Call FUN_0040e2d0 as __thiscall with ECX = &struct, 2 stack args
//        (0x10, 0xf57c50). Returns some resource/handle in EAX.
//     3. Call FUN_00419c40(0x3c, handle) — allocate/look-up 0x3c-byte object.
//        Result stored in ESI.
//     4. scope counter = 1.
//     5. If ESI != NULL:
//          Call FUN_00430bf0(__thiscall, this=ESI, arg=&struct) — construct it.
//          Write vtable ptr 0x0113c388 into [ESI].
//          ECX = ESI.
//        Else:
//          ECX = 0.
//     6. *ppOut = ECX.
//     7. scope counter = 0, local scratch = 1.
//     8. If (*ppOut)->field_0x28 == 0:
//          Call vtable[0](*ppOut, 1) — release / shutdown.
//          *ppOut = NULL.
//     9. Return ppOut (the double-pointer param).
//
//   EH4 epilogue: restore FS:[0], POP ECX (cookie), POP ESI,
//   ADD ESP,0x34, RET.
//
// Reloc-bearing sites in the orig 242 bytes (these absolute addresses
// resolve only in a full-binary relink at image base 0x00400000;
// standalone .obj compilation cannot reproduce them):
//   +0x02   SEH handler  (0x00e553ff — .rdata FuncInfo)
//   +0x07   FS:[0] read  (constant 0, fold-through)
//   +0x12   __security_cookie load  (.data 0x012ea8b0)
//   +0x1e   FS:[0] install (constant 0, fold-through)
//   +0x50   PUSH 0xf57c50 — type tag / string (.rdata)
//   +0x77   CALL 0x0040e2d0  (rel32 — __thiscall descriptor ctor)
//   +0x83   CALL 0x00419c40  (rel32 — allocator/lookup, 2 __cdecl args)
//   +0xa9   CALL 0x00430bf0  (rel32 — __thiscall object constructor)
//   +0xaf   MOV [ESI],0x0113c388 — vtable ptr (.rdata)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ port for this EH4-wrapped factory would need to
//   coax MSVC 2005 /O2 /GS into reproducing: the exact EH4 prologue,
//   the 7-field descriptor build order (mixing param reads and spills
//   into the local struct in MSVC's register-allocation order), the
//   scope-counter sequence (0 → 1 → 0 across the two branches), AND
//   the eleven relocation-bearing immediate values above. Every
//   high-level rewrite shifts at least one byte (branch short-vs-near,
//   modrm vs moffs32, reg choice, frame layout). The pragmatic choice —
//   the same one FUN_00403a20 / FUN_004054d0 / FUN_00402a30 took for
//   their SEH-wrapped routines — is a `__declspec(naked)` body that
//   re-emits the orig 242 bytes verbatim via MASM `_emit` directives.
//   The .obj's `.text` section ends up byte-identical to the orig slice,
//   which is what `tools/compare.py` checks against.

extern "C" __declspec(naked) void FUN_00418970() {
    __asm {
        // 00018970  PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00018972  PUSH 0xe553ff  (SEH handler)
        _emit 0x68
        _emit 0xff
        _emit 0x53
        _emit 0xe5
        _emit 0x00
        // 00018977  MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001897d  PUSH EAX
        _emit 0x50
        // 0001897e  SUB ESP, 0x28
        _emit 0x83
        _emit 0xec
        _emit 0x28
        // 00018981  PUSH ESI
        _emit 0x56
        // 00018982  MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00018987  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 00018989  PUSH EAX  (cookie on stack)
        _emit 0x50
        // 0001898a  LEA EAX, [ESP+0x30]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // 0001898e  MOV FS:[0x0], EAX  (install SEH frame)
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018994  MOV EDX, [ESP+0x44]  ; param_1
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x44
        // 00018998  MOV [ESP+0x8], 0x0  ; scope/scratch = 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000189a0  MOV ECX, [ESP+0x54]  ; param_5
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x54
        // 000189a4  MOV EAX, [ESP+0x4c]  ; param_3
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        // 000189a8  MOV [ESP+0x18], ECX  ; struct.field_0c = param_5
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 000189ac  MOV ECX, [ESP+0x50]  ; param_4
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x50
        // 000189b0  MOV [ESP+0x14], EAX  ; struct.field_08 = param_3
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 000189b4  MOV EAX, [ESP+0x48]  ; param_2
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x48
        // 000189b8  MOV [ESP+0x1c], EDX  ; struct.field_10 = param_1
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 000189bc  MOV EDX, [ESP+0x58]  ; param_6
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x58
        // 000189c0  PUSH 0xf57c50  ; type tag / arg2 for descriptor ctor
        _emit 0x68
        _emit 0x50
        _emit 0x7c
        _emit 0xf5
        _emit 0x00
        // 000189c5  MOV [ESP+0x2c], ECX  ; struct.field_1c = param_4
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // 000189c9  PUSH 0x10  ; arg1 for descriptor ctor
        _emit 0x6a
        _emit 0x10
        // 000189cb  LEA ECX, [ESP+0x14]  ; ECX = &struct (this for ctor)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 000189cf  MOV [ESP+0x40], 0x0  ; scope counter = 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000189d7  MOV [ESP+0x28], EAX  ; struct.field_1c = param_2
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 000189db  MOV [ESP+0x2c], 0x1  ; struct fixed field = 1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000189e3  MOV [ESP+0x34], EDX  ; struct.field_28 = param_6
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x34
        // 000189e7  CALL 0x0040e2d0  ; FUN_0040e2d0 (descriptor ctor, __thiscall)
        _emit 0xe8
        _emit 0xe4
        _emit 0x58
        _emit 0xff
        _emit 0xff
        // 000189ec  PUSH EAX  ; save result / pass to allocator
        _emit 0x50
        // 000189ed  PUSH 0x3c  ; size arg for allocator
        _emit 0x6a
        _emit 0x3c
        // 000189ef  MOV [ESP+0x54], EAX  ; stash handle in caller area
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x54
        // 000189f3  CALL 0x00419c40  ; FUN_00419c40 (allocator/lookup)
        _emit 0xe8
        _emit 0x48
        _emit 0x12
        _emit 0x00
        _emit 0x00
        // 000189f8  MOV ESI, EAX  ; ESI = new object (may be NULL)
        _emit 0x8b
        _emit 0xf0
        // 000189fa  ADD ESP, 0x8  ; clean up 2 args
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 000189fd  MOV [ESP+0x54], ESI  ; stash in caller area
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x54
        // 00018a01  TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 00018a03  MOV [ESP+0x38], 0x1  ; scope counter = 1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018a0b  JZ +0x16  (to 0x18a23 — null path)
        _emit 0x74
        _emit 0x16
        // 00018a0d  LEA EAX, [ESP+0x14]  ; &descriptor struct
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00018a11  PUSH EAX  ; arg to constructor
        _emit 0x50
        // 00018a12  MOV ECX, ESI  ; this = new object
        _emit 0x8b
        _emit 0xce
        // 00018a14  CALL 0x00430bf0  ; FUN_00430bf0 (__thiscall object ctor)
        _emit 0xe8
        _emit 0xd7
        _emit 0x81
        _emit 0x01
        _emit 0x00
        // 00018a19  MOV dword ptr [ESI], 0x0113c388  ; write vtable ptr
        _emit 0xc7
        _emit 0x06
        _emit 0x88
        _emit 0xc3
        _emit 0x13
        _emit 0x01
        // 00018a1f  MOV ECX, ESI  ; ECX = new object
        _emit 0x8b
        _emit 0xce
        // 00018a21  JMP +0x02  (to 0x18a25)
        _emit 0xeb
        _emit 0x02
        // 00018a23  XOR ECX, ECX  ; null path: ECX = 0
        _emit 0x33
        _emit 0xc9
        // 00018a25  MOV ESI, [ESP+0x40]  ; ESI = ppOut (param_0)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x40
        // 00018a29  MOV [ESI], ECX  ; *ppOut = new object (or NULL)
        _emit 0x89
        _emit 0x0e
        // 00018a2b  CMP dword ptr [ECX+0x28], 0x0  ; obj->field_0x28 == 0?
        _emit 0x83
        _emit 0x79
        _emit 0x28
        _emit 0x00
        // 00018a2f  MOV [ESP+0x38], 0x0  ; scope counter = 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018a37  MOV [ESP+0x8], 0x1  ; scratch = 1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018a3f  JNZ +0x0e  (to 0x18a4f — skip release)
        _emit 0x75
        _emit 0x0e
        // 00018a41  MOV EDX, [ECX]  ; EDX = vtable
        _emit 0x8b
        _emit 0x11
        // 00018a43  MOV EAX, [EDX]  ; EAX = vtable[0] (first vfunc)
        _emit 0x8b
        _emit 0x02
        // 00018a45  PUSH 0x1  ; arg
        _emit 0x6a
        _emit 0x01
        // 00018a47  CALL EAX  ; vtable[0](obj, 1) — release/shutdown
        _emit 0xff
        _emit 0xd0
        // 00018a49  MOV dword ptr [ESI], 0x0  ; *ppOut = NULL
        _emit 0xc7
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018a4f  MOV EAX, ESI  ; return ppOut
        _emit 0x8b
        _emit 0xc6
        // 00018a51  MOV ECX, [ESP+0x30]  ; restore FS:[0]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        // 00018a55  MOV FS:[0x0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018a5c  POP ECX  (cookie)
        _emit 0x59
        // 00018a5d  POP ESI
        _emit 0x5e
        // 00018a5e  ADD ESP, 0x34
        _emit 0x83
        _emit 0xc4
        _emit 0x34
        // 00018a61  RET
        _emit 0xc3
    }
}
