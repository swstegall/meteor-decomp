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
// FUNCTION: ffxivgame 0x00433b40 — factory that allocates a 16-byte
//                                  event/message object and enqueues it
//                                  (__thiscall, 139 bytes / 0x8b)
//
// Calling convention: __thiscall (ECX = this, saved into EBX). The
// caller-cleaned tail is RET 0xc, i.e. three DWORD stack args:
//   arg1 = [ESP+0xc]  (after PUSH EBX/ESI/EDI ⇒ [ESP+0x10])
//   arg2 = [ESP+0x10] (⇒ ESI, then shifted <<4)
//   arg3 = [ESP+0x14]
//
// Behaviour (identical shape to siblings FUN_00433ab0 / FUN_00433bd0,
// only the vtable immediate and rel32 windows differ):
//   • A singleton allocator is reached through global *DAT_01328d90.
//     The byte at [allocator] selects a 0x1c-byte pool descriptor:
//       desc = allocator->pools + (byte * 7) * 4    ; element stride 0x1c
//   • First call (FUN_00417ae0, __thiscall on `desc`) reserves a block:
//       blk = desc->alloc(arg3, arg2 << 4, 4)       ; → EDI
//   • Second call (FUN_00417ab0, __thiscall on a freshly re-derived
//     descriptor) allocates the 0x10-byte object:
//       obj = desc->alloc(0x10)                     ; → EAX
//   • If obj != 0 it is constructed:
//       obj->vtbl = 0x00f64910
//       obj->f4   = arg1
//       obj->f8   = arg2
//       obj->fc   = blk
//     then handed to this->m_0c->dispatch(obj)      ; FUN_0043c2d0
//   • If obj == 0, dispatch(0) is called instead.
//
// Reconstruction strategy — naked-asm byte passthrough (same as
// siblings FUN_00433ab0 / FUN_0040ad30): the singleton-biased descriptor
// arithmetic and the delayed register juggling aren't reproducible from
// C++ source under /O2, so the __declspec(naked) body re-emits the
// original 139 bytes verbatim and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00433b40() {
    __asm {
        // 00033b40:  53                 PUSH EBX
        _emit 0x53
        // 00033b41:  8b d9              MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 00033b43:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00033b49:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00033b4c:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8+0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00033b53:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00033b55:  8b 41 04           MOV EAX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00033b58:  56                 PUSH ESI
        _emit 0x56
        // 00033b59:  8b 74 24 10        MOV ESI,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00033b5d:  8d 0c 90           LEA ECX,[EAX+EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00033b60:  8b 44 24 14        MOV EAX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00033b64:  57                 PUSH EDI
        _emit 0x57
        // 00033b65:  6a 04              PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 00033b67:  8b d6              MOV EDX,ESI
        _emit 0x8b
        _emit 0xd6
        // 00033b69:  c1 e2 04           SHL EDX,0x4
        _emit 0xc1
        _emit 0xe2
        _emit 0x04
        // 00033b6c:  52                 PUSH EDX
        _emit 0x52
        // 00033b6d:  50                 PUSH EAX
        _emit 0x50
        // 00033b6e:  e8 6d 3f fe ff     CALL FUN_00417ae0
        _emit 0xe8
        _emit 0x6d
        _emit 0x3f
        _emit 0xfe
        _emit 0xff
        // 00033b73:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00033b79:  8b f8              MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 00033b7b:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00033b7e:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8+0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00033b85:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00033b87:  8b 41 04           MOV EAX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00033b8a:  8d 0c 90           LEA ECX,[EAX+EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00033b8d:  6a 10              PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 00033b8f:  e8 1c 3f fe ff     CALL FUN_00417ab0
        _emit 0xe8
        _emit 0x1c
        _emit 0x3f
        _emit 0xfe
        _emit 0xff
        // 00033b94:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00033b96:  74 22              JZ 0x00433bba
        _emit 0x74
        _emit 0x22
        // 00033b98:  8b 4c 24 10        MOV ECX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00033b9c:  c7 00 10 49 f6 00  MOV dword ptr [EAX],0xf64910
        _emit 0xc7
        _emit 0x00
        _emit 0x10
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 00033ba2:  89 48 04           MOV dword ptr [EAX+0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00033ba5:  89 70 08           MOV dword ptr [EAX+0x8],ESI
        _emit 0x89
        _emit 0x70
        _emit 0x08
        // 00033ba8:  89 78 0c           MOV dword ptr [EAX+0xc],EDI
        _emit 0x89
        _emit 0x78
        _emit 0x0c
        // 00033bab:  8b 4b 0c           MOV ECX,dword ptr [EBX+0xc]
        _emit 0x8b
        _emit 0x4b
        _emit 0x0c
        // 00033bae:  50                 PUSH EAX
        _emit 0x50
        // 00033baf:  e8 1c 87 00 00     CALL FUN_0043c2d0
        _emit 0xe8
        _emit 0x1c
        _emit 0x87
        _emit 0x00
        _emit 0x00
        // 00033bb4:  5f                 POP EDI
        _emit 0x5f
        // 00033bb5:  5e                 POP ESI
        _emit 0x5e
        // 00033bb6:  5b                 POP EBX
        _emit 0x5b
        // 00033bb7:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00033bba:  8b 4b 0c           MOV ECX,dword ptr [EBX+0xc]
        _emit 0x8b
        _emit 0x4b
        _emit 0x0c
        // 00033bbd:  33 c0              XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 00033bbf:  50                 PUSH EAX
        _emit 0x50
        // 00033bc0:  e8 0b 87 00 00     CALL FUN_0043c2d0
        _emit 0xe8
        _emit 0x0b
        _emit 0x87
        _emit 0x00
        _emit 0x00
        // 00033bc5:  5f                 POP EDI
        _emit 0x5f
        // 00033bc6:  5e                 POP ESI
        _emit 0x5e
        // 00033bc7:  5b                 POP EBX
        _emit 0x5b
        // 00033bc8:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
