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
// FUNCTION: ffxivgame 0x00018bf0 — VFX object factory (260 B / 0x104, EH4-SEH wrapped).
//
// Behaviour read from asm/ffxivgame/00018bf0_FUN_00418bf0.s:
//
//   __cdecl SomeObj** FUN_00418bf0(SomeObj** ppOut, int width, int height,
//                                  int arg4, void* arg5, int arg6, int arg7,
//                                  int arg8);
//
//   Clamps `width` and `height` to a minimum of 1, builds an initialiser
//   struct in the local frame (0x30 bytes of SUB-ESP space), then:
//
//     InitStruct s;
//     s.unk08  = 0;                        // EBX = 0
//     s.unk0c  = arg5;                     // [ESP+0x5C]
//     s.unk10  = arg8;                     // [ESP+0x68]
//     s.unk14  = max(width,  1);           // [ESP+0x50] clamped
//     s.unk18  = max(height, 1);           // [ESP+0x54] clamped
//     s.unk1c  = 1;
//     s.unk20  = arg4;                     // [ESP+0x58]
//     s.unk24  = arg6;                     // [ESP+0x60]
//     s.unk28  = arg7;                     // [ESP+0x64]
//
//     void* extra = FUN_0040e2d0(          // @ 0x0040e2d0 — some descriptor builder
//                       &s, 0x10,
//                       (void*)0x00f57c78  // type-tag string in .rdata
//                   );
//     SomeObj* obj = (SomeObj*)FUN_00419c40(0x58, extra);  // alloc 0x58 bytes
//     *ppOut = obj;
//     if (obj) {
//         FUN_00431710(obj, &s);           // constructor body
//         obj->vtbl = (void*)0x00f57e3c;   // stamp vtable
//     }
//     if ((*ppOut)->field_0x34 == 0) {
//         (*ppOut)->vtbl[0](1);            // virtual call if field not set
//         *ppOut = NULL;
//     }
//     return ppOut;
//
//   Stack frame (after the EH4 prologue, ESP-relative offsets):
//     [esp+0x00]             security cookie
//     [esp+0x04]             saved ESI (output ptr ptr mirror)
//     [esp+0x08]             saved EBX (= 0 throughout)
//     [esp+0x0c]             local flag (EH state tracker, set 0→1)
//     [esp+0x10 .. 0x3b]     InitStruct s (built in place)
//     [esp+0x3c]             SEH.next (old FS:[0])
//     [esp+0x40]             SEH.handler (0x00e554bc)
//     [esp+0x44]             SEH.state (−1 → 0 → 1 → 0 during execution)
//     [esp+0x48]             return address
//     [esp+0x4c]             arg1 = ppOut
//     [esp+0x50]             arg2 = width
//     [esp+0x54]             arg3 = height
//     [esp+0x58]             arg4
//     [esp+0x5c]             arg5
//     [esp+0x60]             arg6
//     [esp+0x64]             arg7
//     [esp+0x68]             arg8
//
//   Reloc-bearing sites in the orig 260 bytes:
//     +0x02   PUSH 0x00e554bc  — SEH handler address (.text)
//     +0x07   MOV EAX,FS:[0]  — constant 0
//     +0x13   MOV EAX,[0x012ea8b0] — __security_cookie (.data)
//     +0x1f   MOV FS:[0],EAX  — constant 0
//     +0x6d   PUSH 0x00f57c78 — type-tag string (.rdata)
//     +0x90   CALL 0x0040e2d0 — FUN_0040e2d0 (.text rel32)
//     +0x9c   CALL 0x00419c40 — FUN_00419c40 alloc (.text rel32)
//     +0xbe   CALL 0x00431710 — FUN_00431710 ctor (.text rel32)
//     +0xc3   MOV [ESI],0x00f57e3c — vtable pointer (.rdata)
//     +0xf6   MOV FS:[0],ECX  — constant 0
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The EH4-wrapped prologue (double security-cookie XOR, PUSH -1 /
//   PUSH scope-table / PUSH FS:[0]) combined with the interleaved SEH
//   state writes ([ESP+0x44] toggled 0→1→0 during the body) and the
//   seven linker-resolved absolute addresses above make a source-level
//   /O2 /GS /EHsc match extremely brittle — any reorder of the store
//   sequence or alternate register choice shifts bytes. The naked-asm
//   passthrough (same approach as FUN_00403a20, FUN_004054d0, etc.)
//   is the clean path to a byte-identical .obj.

extern "C" __declspec(naked) void FUN_00418bf0() {
    __asm {
        // 00018bf0  PUSH -1
        _emit 0x6a
        _emit 0xff
        // 00018bf2  PUSH 0xe554bc  (SEH handler)
        _emit 0x68
        _emit 0xbc
        _emit 0x54
        _emit 0xe5
        _emit 0x00
        // 00018bf7  MOV EAX,FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018bfd  PUSH EAX
        _emit 0x50
        // 00018bfe  SUB ESP,0x30
        _emit 0x83
        _emit 0xec
        _emit 0x30
        // 00018c01  PUSH EBX
        _emit 0x53
        // 00018c02  PUSH ESI
        _emit 0x56
        // 00018c03  MOV EAX,[__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00018c08  XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 00018c0a  PUSH EAX
        _emit 0x50
        // 00018c0b  LEA EAX,[ESP+0x3c]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        // 00018c0f  MOV FS:[0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018c15  XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // 00018c17  MOV [ESP+0xc],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        // 00018c1b  MOV ECX,[ESP+0x50]  ; arg2 = width
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x50
        // 00018c1f  CMP ECX,1
        _emit 0x83
        _emit 0xf9
        _emit 0x01
        // 00018c22  MOV [ESP+0x44],EBX  ; SEH state = 0
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x44
        // 00018c26  JNC +3  (skip LEA if width >= 1)
        _emit 0x73
        _emit 0x03
        // 00018c28  LEA ECX,[EBX+1]  ; ECX = 1
        _emit 0x8d
        _emit 0x4b
        _emit 0x01
        // 00018c2b  MOV EAX,[ESP+0x54]  ; arg3 = height
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x54
        // 00018c2f  CMP EAX,1
        _emit 0x83
        _emit 0xf8
        _emit 0x01
        // 00018c32  JNC +5  (skip MOV if height >= 1)
        _emit 0x73
        _emit 0x05
        // 00018c34  MOV EAX,1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018c39  MOV EDX,[ESP+0x5c]  ; arg5
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x5c
        // 00018c3d  MOV [ESP+0x1c],EDX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 00018c41  MOV EDX,[ESP+0x68]  ; arg8
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x68
        // 00018c45  MOV [ESP+0x24],ECX  ; clamped width
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 00018c49  MOV ECX,[ESP+0x60]  ; arg6
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        // 00018c4d  MOV [ESP+0x20],EDX  ; arg8
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x20
        // 00018c51  MOV EDX,[ESP+0x64]  ; arg7
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x64
        // 00018c55  MOV [ESP+0x28],EAX  ; clamped height
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 00018c59  MOV EAX,[ESP+0x58]  ; arg4
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x58
        // 00018c5d  PUSH 0xf57c78  (type-tag string)
        _emit 0x68
        _emit 0x78
        _emit 0x7c
        _emit 0xf5
        _emit 0x00
        // 00018c62  MOV [ESP+0x38],ECX  ; arg6 (ESP shifted by 1 PUSH)
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // 00018c66  PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 00018c68  LEA ECX,[ESP+0x18]  ; &local struct (ESP shifted by 2 PUSHes)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00018c6c  MOV [ESP+0x20],EBX  ; = 0
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        // 00018c70  MOV [ESP+0x34],1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018c78  MOV [ESP+0x38],EAX  ; arg4
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x38
        // 00018c7c  MOV [ESP+0x40],EDX  ; arg7
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x40
        // 00018c80  CALL 0x0040e2d0
        _emit 0xe8
        _emit 0x4b
        _emit 0x56
        _emit 0xff
        _emit 0xff
        // 00018c85  PUSH EAX
        _emit 0x50
        // 00018c86  PUSH 0x58
        _emit 0x6a
        _emit 0x58
        // 00018c88  MOV [ESP+0x58],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x58
        // 00018c8c  CALL 0x00419c40
        _emit 0xe8
        _emit 0xaf
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        // 00018c91  MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 00018c93  ADD ESP,8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00018c96  MOV [ESP+0x54],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x54
        // 00018c9a  CMP ESI,EBX
        _emit 0x3b
        _emit 0xf3
        // 00018c9c  MOV [ESP+0x44],1  ; SEH state = 1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x44
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018ca4  JZ +0x17  (null → skip constructor)
        _emit 0x74
        _emit 0x17
        // 00018ca6  PUSH ESI
        _emit 0x56
        // 00018ca7  LEA EAX,[ESP+0x1c]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00018cab  PUSH EAX
        _emit 0x50
        // 00018cac  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00018cae  CALL 0x00431710
        _emit 0xe8
        _emit 0x5d
        _emit 0x8a
        _emit 0x01
        _emit 0x00
        // 00018cb3  MOV [ESI],0xf57e3c  (vtable)
        _emit 0xc7
        _emit 0x06
        _emit 0x3c
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        // 00018cb9  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00018cbb  JMP +2  (over XOR ECX,ECX)
        _emit 0xeb
        _emit 0x02
        // 00018cbd  XOR ECX,ECX  (null path)
        _emit 0x33
        _emit 0xc9
        // 00018cbf  MOV ESI,[ESP+0x4c]  ; arg1 = ppOut
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x4c
        // 00018cc3  MOV [ESI],ECX  ; *ppOut = obj (or null)
        _emit 0x89
        _emit 0x0e
        // 00018cc5  CMP [ECX+0x34],EBX  ; check field_0x34
        _emit 0x39
        _emit 0x59
        _emit 0x34
        // 00018cc8  MOV [ESP+0x44],EBX  ; SEH state = 0
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x44
        // 00018ccc  MOV [ESP+0xc],1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018cd4  JNZ +0xa  (non-zero: skip vtable call)
        _emit 0x75
        _emit 0x0a
        // 00018cd6  MOV EDX,[ECX]  ; vtable ptr
        _emit 0x8b
        _emit 0x11
        // 00018cd8  MOV EAX,[EDX]  ; vtable[0]
        _emit 0x8b
        _emit 0x02
        // 00018cda  PUSH 1
        _emit 0x6a
        _emit 0x01
        // 00018cdc  CALL EAX  (virtual call)
        _emit 0xff
        _emit 0xd0
        // 00018cde  MOV [ESI],EBX  ; *ppOut = 0
        _emit 0x89
        _emit 0x1e
        // 00018ce0  MOV EAX,ESI  ; return ppOut
        _emit 0x8b
        _emit 0xc6
        // 00018ce2  MOV ECX,[ESP+0x3c]  ; restore SEH chain
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        // 00018ce6  MOV FS:[0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018ced  POP ECX
        _emit 0x59
        // 00018cee  POP ESI
        _emit 0x5e
        // 00018cef  POP EBX
        _emit 0x5b
        // 00018cf0  ADD ESP,0x3c
        _emit 0x83
        _emit 0xc4
        _emit 0x3c
        // 00018cf3  RET
        _emit 0xc3
    }
}
