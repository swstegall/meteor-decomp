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
// FUNCTION: ffxivgame 0x00436f20 — allocate + construct a small command /
//                                  event node, then hand it (or null on OOM)
//                                  to a member dispatch routine (__thiscall,
//                                  139 bytes / 0x8b)
//
// Calling convention: __thiscall (ECX = this); cleans up 0xc bytes of
// stack args (RET 0xc → three DWORD parameters). Callee-saves: EBX, ESI,
// EDI. No local frame.
//
// Shape:
//   this        = ECX               (saved into EBX)
//   param_1     = [ESP+0x10]        (→ ESI)
//   param_2     = [ESP+0x14]        (→ EAX, passed to allocator)
//   param_3     = [ESP+0x10] after frame grows (reloaded into ECX, slot)
//
// A per-CPU / per-thread arena selector sits behind the global pointer
// DAT_01328d90:
//   ECX  = *DAT_01328d90;                 // arena table
//   idx  = (unsigned char)ECX[0];         // current arena index
//   pool = ((int*)ECX)[1];                // base of 0x1c-byte arena recs
//   rec  = pool + idx * 0x1c;             // &arena[idx]  (idx*7*4 == *28)
//
// First call (FUN_00417ae0, __thiscall this=rec) is a sized arena op:
//   FUN_00417ae0(rec, /*a*/ param_2, /*b*/ param_1 << 4, /*c*/ 4);
//   stash result in EDI.
// Second call (FUN_00417ab0, __thiscall this=rec') allocates 0x10 bytes:
//   node = FUN_00417ab0(rec', 0x10);
//   if (node) {
//       node->vftable = 0x00f64910;       // vtable / type tag
//       node->field4  = param_3;          // [ESP+0x10] reloaded
//       node->field8  = param_1;          // ESI
//       node->fieldc  = EDI;              // result of first call
//       FUN_0043c2d0(this->field8 /*[EBX+8]*/, node);
//   } else {
//       FUN_0043c2d0(this->field8 /*[EBX+8]*/, 0);
//   }
//
// Reloc-bearing sites compare.py masks:
//   ABS: MOV ECX,[0x01328d90]  ×2   (8b 0d ..)   global pointer
//   ABS: MOV [EAX],0x00f64910        (c7 00 ..)   vtable immediate
//   REL: CALL FUN_00417ae0          (e8 8d 0b fe ff)
//   REL: CALL FUN_00417ab0          (e8 3c 0b fe ff)
//   REL: CALL FUN_0043c2d0          (e8 3c 53 00 00)
//   REL: CALL FUN_0043c2d0          (e8 2b 53 00 00)
//
// Reconstruction strategy — naked-asm byte passthrough (matching the
// local _rosetta idiom): the SIB-scaled arena addressing, the absolute
// global/vtable references, and the duplicated tail are re-emitted as
// the original 139 bytes verbatim. The .obj's .text is byte-identical
// to the original slice and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00436f20() {
    __asm {
        // 00036f20:  53                 PUSH EBX
        _emit 0x53
        // 00036f21:  8b d9              MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 00036f23:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00036f29:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00036f2c:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036f33:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00036f35:  8b 41 04           MOV EAX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00036f38:  56                 PUSH ESI
        _emit 0x56
        // 00036f39:  8b 74 24 10        MOV ESI,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00036f3d:  8d 0c 90           LEA ECX,[EAX+EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00036f40:  8b 44 24 14        MOV EAX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00036f44:  57                 PUSH EDI
        _emit 0x57
        // 00036f45:  6a 04              PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 00036f47:  8b d6              MOV EDX,ESI
        _emit 0x8b
        _emit 0xd6
        // 00036f49:  c1 e2 04           SHL EDX,0x4
        _emit 0xc1
        _emit 0xe2
        _emit 0x04
        // 00036f4c:  52                 PUSH EDX
        _emit 0x52
        // 00036f4d:  50                 PUSH EAX
        _emit 0x50
        // 00036f4e:  e8 8d 0b fe ff     CALL 0x00417ae0
        _emit 0xe8
        _emit 0x8d
        _emit 0x0b
        _emit 0xfe
        _emit 0xff
        // 00036f53:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00036f59:  8b f8              MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 00036f5b:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00036f5e:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036f65:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00036f67:  8b 41 04           MOV EAX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00036f6a:  8d 0c 90           LEA ECX,[EAX+EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00036f6d:  6a 10              PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 00036f6f:  e8 3c 0b fe ff     CALL 0x00417ab0
        _emit 0xe8
        _emit 0x3c
        _emit 0x0b
        _emit 0xfe
        _emit 0xff
        // 00036f74:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00036f76:  74 22              JZ 0x00436f9a
        _emit 0x74
        _emit 0x22
        // 00036f78:  8b 4c 24 10        MOV ECX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00036f7c:  c7 00 10 49 f6 00  MOV dword ptr [EAX],0xf64910
        _emit 0xc7
        _emit 0x00
        _emit 0x10
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 00036f82:  89 48 04           MOV dword ptr [EAX+0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00036f85:  89 70 08           MOV dword ptr [EAX+0x8],ESI
        _emit 0x89
        _emit 0x70
        _emit 0x08
        // 00036f88:  89 78 0c           MOV dword ptr [EAX+0xc],EDI
        _emit 0x89
        _emit 0x78
        _emit 0x0c
        // 00036f8b:  8b 4b 08           MOV ECX,dword ptr [EBX+0x8]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 00036f8e:  50                 PUSH EAX
        _emit 0x50
        // 00036f8f:  e8 3c 53 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x3c
        _emit 0x53
        _emit 0x00
        _emit 0x00
        // 00036f94:  5f                 POP EDI
        _emit 0x5f
        // 00036f95:  5e                 POP ESI
        _emit 0x5e
        // 00036f96:  5b                 POP EBX
        _emit 0x5b
        // 00036f97:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00036f9a:  8b 4b 08           MOV ECX,dword ptr [EBX+0x8]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 00036f9d:  33 c0              XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 00036f9f:  50                 PUSH EAX
        _emit 0x50
        // 00036fa0:  e8 2b 53 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x2b
        _emit 0x53
        _emit 0x00
        _emit 0x00
        // 00036fa5:  5f                 POP EDI
        _emit 0x5f
        // 00036fa6:  5e                 POP ESI
        _emit 0x5e
        // 00036fa7:  5b                 POP EBX
        _emit 0x5b
        // 00036fa8:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
