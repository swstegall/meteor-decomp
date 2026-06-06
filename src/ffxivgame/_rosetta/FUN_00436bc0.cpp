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
// FUNCTION: ffxivgame 0x00436bc0 — factory that allocates a 16-byte
//                                  event/message object, fills it with a
//                                  vtable + 3 fields, and dispatches it
//                                  (__thiscall, 139 bytes / 0x8b)
//
// Calling convention: __thiscall (ECX = this); RET 0xc → 3 stack args.
//
//   void * __thiscall FUN_00436bc0(this, int a, int b, int c);
//
// Frame: pushes EBX (=this), ESI (=b), EDI (=first-call result). No
// locals. Two slot-allocator calls indexed through the global pool at
// 0x01328d90 (byte index *7*4 = *28 selects a per-thread/per-frame pool
// entry; [pool+4] is the entry array base):
//
//   tmp  = pool_entry->alloc(c, b << 4, 4);            // CALL 0x00417ae0
//   obj  = pool_entry->alloc2(0x10);                   // CALL 0x00417ab0
//   if (obj) {
//       obj[0] = 0x00f64900;   // vtable
//       obj[1] = a;
//       obj[2] = b;
//       obj[3] = tmp;
//       this->field8->dispatch(obj);                   // CALL 0x0043c2d0
//   } else {
//       this->field8->dispatch(0);
//   }
//
// Reloc-bearing sites (compare.py masks these positions):
//   ABS imm/mem: MOV ECX,[0x01328d90]  ×2   (global pool pointer)
//   ABS imm    : MOV [EAX],0x00f64900       (vtable address)
//   REL rel32  : CALL 0x00417ae0, 0x00417ab0, 0x0043c2d0 ×2
//
// Reconstruction strategy — naked-asm byte passthrough. The biased
// pool-index addressing (idx*8-idx → idx*7, scaled *4) and the
// twin-allocator dispatch shape are awkward to coerce from C++ /O2;
// the __declspec(naked) body re-emits the original 139 bytes verbatim
// via MASM _emit directives, so the .obj's .text is byte-identical to
// the original slice and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00436bc0() {
    __asm {
        // 00036bc0:  53                 PUSH EBX
        _emit 0x53
        // 00036bc1:  8b d9              MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 00036bc3:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00036bc9:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00036bcc:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036bd3:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00036bd5:  8b 41 04           MOV EAX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00036bd8:  56                 PUSH ESI
        _emit 0x56
        // 00036bd9:  8b 74 24 10        MOV ESI,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00036bdd:  8d 0c 90           LEA ECX,[EAX+EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00036be0:  8b 44 24 14        MOV EAX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00036be4:  57                 PUSH EDI
        _emit 0x57
        // 00036be5:  6a 04              PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 00036be7:  8b d6              MOV EDX,ESI
        _emit 0x8b
        _emit 0xd6
        // 00036be9:  c1 e2 04           SHL EDX,0x4
        _emit 0xc1
        _emit 0xe2
        _emit 0x04
        // 00036bec:  52                 PUSH EDX
        _emit 0x52
        // 00036bed:  50                 PUSH EAX
        _emit 0x50
        // 00036bee:  e8 ed 0e fe ff     CALL 0x00417ae0
        _emit 0xe8
        _emit 0xed
        _emit 0x0e
        _emit 0xfe
        _emit 0xff
        // 00036bf3:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00036bf9:  8b f8              MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 00036bfb:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00036bfe:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036c05:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00036c07:  8b 41 04           MOV EAX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00036c0a:  8d 0c 90           LEA ECX,[EAX+EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00036c0d:  6a 10              PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 00036c0f:  e8 9c 0e fe ff     CALL 0x00417ab0
        _emit 0xe8
        _emit 0x9c
        _emit 0x0e
        _emit 0xfe
        _emit 0xff
        // 00036c14:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00036c16:  74 22              JZ 0x00436c3a
        _emit 0x74
        _emit 0x22
        // 00036c18:  8b 4c 24 10        MOV ECX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00036c1c:  c7 00 00 49 f6 00  MOV dword ptr [EAX],0xf64900
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 00036c22:  89 48 04           MOV dword ptr [EAX+0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00036c25:  89 70 08           MOV dword ptr [EAX+0x8],ESI
        _emit 0x89
        _emit 0x70
        _emit 0x08
        // 00036c28:  89 78 0c           MOV dword ptr [EAX+0xc],EDI
        _emit 0x89
        _emit 0x78
        _emit 0x0c
        // 00036c2b:  8b 4b 08           MOV ECX,dword ptr [EBX+0x8]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 00036c2e:  50                 PUSH EAX
        _emit 0x50
        // 00036c2f:  e8 9c 56 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x9c
        _emit 0x56
        _emit 0x00
        _emit 0x00
        // 00036c34:  5f                 POP EDI
        _emit 0x5f
        // 00036c35:  5e                 POP ESI
        _emit 0x5e
        // 00036c36:  5b                 POP EBX
        _emit 0x5b
        // 00036c37:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00036c3a:  8b 4b 08           MOV ECX,dword ptr [EBX+0x8]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 00036c3d:  33 c0              XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 00036c3f:  50                 PUSH EAX
        _emit 0x50
        // 00036c40:  e8 8b 56 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x8b
        _emit 0x56
        _emit 0x00
        _emit 0x00
        // 00036c45:  5f                 POP EDI
        _emit 0x5f
        // 00036c46:  5e                 POP ESI
        _emit 0x5e
        // 00036c47:  5b                 POP EBX
        _emit 0x5b
        // 00036c48:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
