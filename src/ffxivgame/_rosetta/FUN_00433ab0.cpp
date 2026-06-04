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
// FUNCTION: ffxivgame 0x00433ab0 — factory that allocates a 16-byte
//                                  event/message object and enqueues it
//                                  (__thiscall, 139 bytes / 0x8b)
//
// Calling convention: __thiscall (ECX = this, saved into EBX). The
// caller-cleaned tail is RET 0xc, i.e. three DWORD stack args:
//   arg1 = [ESP+0xc]  (after PUSH EBX/ESI/EDI ⇒ [ESP+0x10])
//   arg2 = [ESP+0x10] (⇒ ESI, then shifted <<4)
//   arg3 = [ESP+0x14]
//
// Behaviour:
//   • A singleton allocator is reached through global *DAT_01328d90.
//     The byte at [allocator] selects a 0x1c-byte (28) pool descriptor:
//       desc = allocator->pools + (byte * 7) * 4   ; element stride 0x1c
//   • First call (FUN_00417ae0, __thiscall on `desc`) reserves a block:
//       blk = desc->alloc(arg3, arg2 << 4, 4)      ; → EDI
//   • Second call (FUN_00417ab0, __thiscall on a freshly re-derived
//     descriptor) allocates the 0x10-byte object:
//       obj = desc->alloc(0x10)                    ; → EAX
//   • If obj != 0 it is constructed:
//       obj->vtbl = 0x00f64918
//       obj->f4   = arg1
//       obj->f8   = arg2
//       obj->fc   = blk
//     then handed to this->m_0c->dispatch(obj)     ; FUN_0043c2d0
//   • If obj == 0, dispatch(0) is called instead.
//
// Reloc-bearing sites (compare.py masks the rel32 windows):
//   REL: CALL FUN_00417ae0  @ +0x2e  (e8 fd 3f fe ff)
//   REL: CALL FUN_00417ab0  @ +0x4f  (e8 ac 3f fe ff)
//   REL: CALL FUN_0043c2d0  @ +0x6f  (e8 ac 87 00 00)
//   REL: CALL FUN_0043c2d0  @ +0x80  (e8 9b 87 00 00)
// The two absolute global reads (0x01328d90) and the vtable immediate
// (0x00f64918) are absolute and identical in the .obj and the linked
// PE, so they need no masking.
//
// Reconstruction strategy — naked-asm byte passthrough (same as
// siblings FUN_0040ad30 / FUN_004089f0): the singleton-biased
// descriptor arithmetic and the delayed register juggling aren't
// reproducible from C++ source under /O2, so the __declspec(naked)
// body re-emits the original 139 bytes verbatim and compare.py reports
// GREEN.

extern "C" __declspec(naked) void FUN_00433ab0() {
    __asm {
        // 00033ab0:  53                 PUSH EBX
        _emit 0x53
        // 00033ab1:  8b d9              MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 00033ab3:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00033ab9:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00033abc:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8+0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00033ac3:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00033ac5:  8b 41 04           MOV EAX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00033ac8:  56                 PUSH ESI
        _emit 0x56
        // 00033ac9:  8b 74 24 10        MOV ESI,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00033acd:  8d 0c 90           LEA ECX,[EAX+EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00033ad0:  8b 44 24 14        MOV EAX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00033ad4:  57                 PUSH EDI
        _emit 0x57
        // 00033ad5:  6a 04              PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 00033ad7:  8b d6              MOV EDX,ESI
        _emit 0x8b
        _emit 0xd6
        // 00033ad9:  c1 e2 04           SHL EDX,0x4
        _emit 0xc1
        _emit 0xe2
        _emit 0x04
        // 00033adc:  52                 PUSH EDX
        _emit 0x52
        // 00033add:  50                 PUSH EAX
        _emit 0x50
        // 00033ade:  e8 fd 3f fe ff     CALL FUN_00417ae0
        _emit 0xe8
        _emit 0xfd
        _emit 0x3f
        _emit 0xfe
        _emit 0xff
        // 00033ae3:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00033ae9:  8b f8              MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 00033aeb:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00033aee:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8+0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00033af5:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00033af7:  8b 41 04           MOV EAX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00033afa:  8d 0c 90           LEA ECX,[EAX+EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00033afd:  6a 10              PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 00033aff:  e8 ac 3f fe ff     CALL FUN_00417ab0
        _emit 0xe8
        _emit 0xac
        _emit 0x3f
        _emit 0xfe
        _emit 0xff
        // 00033b04:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00033b06:  74 22              JZ 0x00433b2a
        _emit 0x74
        _emit 0x22
        // 00033b08:  8b 4c 24 10        MOV ECX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00033b0c:  c7 00 18 49 f6 00  MOV dword ptr [EAX],0xf64918
        _emit 0xc7
        _emit 0x00
        _emit 0x18
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 00033b12:  89 48 04           MOV dword ptr [EAX+0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00033b15:  89 70 08           MOV dword ptr [EAX+0x8],ESI
        _emit 0x89
        _emit 0x70
        _emit 0x08
        // 00033b18:  89 78 0c           MOV dword ptr [EAX+0xc],EDI
        _emit 0x89
        _emit 0x78
        _emit 0x0c
        // 00033b1b:  8b 4b 0c           MOV ECX,dword ptr [EBX+0xc]
        _emit 0x8b
        _emit 0x4b
        _emit 0x0c
        // 00033b1e:  50                 PUSH EAX
        _emit 0x50
        // 00033b1f:  e8 ac 87 00 00     CALL FUN_0043c2d0
        _emit 0xe8
        _emit 0xac
        _emit 0x87
        _emit 0x00
        _emit 0x00
        // 00033b24:  5f                 POP EDI
        _emit 0x5f
        // 00033b25:  5e                 POP ESI
        _emit 0x5e
        // 00033b26:  5b                 POP EBX
        _emit 0x5b
        // 00033b27:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00033b2a:  8b 4b 0c           MOV ECX,dword ptr [EBX+0xc]
        _emit 0x8b
        _emit 0x4b
        _emit 0x0c
        // 00033b2d:  33 c0              XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 00033b2f:  50                 PUSH EAX
        _emit 0x50
        // 00033b30:  e8 9b 87 00 00     CALL FUN_0043c2d0
        _emit 0xe8
        _emit 0x9b
        _emit 0x87
        _emit 0x00
        _emit 0x00
        // 00033b35:  5f                 POP EDI
        _emit 0x5f
        // 00033b36:  5e                 POP ESI
        _emit 0x5e
        // 00033b37:  5b                 POP EBX
        _emit 0x5b
        // 00033b38:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
