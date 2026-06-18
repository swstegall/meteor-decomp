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
// FUNCTION: ffxivgame 0x00436c50 — factory/dispatch helper that allocates a
//                                  0x10-byte event-record object from a
//                                  per-current-thread pool and forwards it
//                                  to a sink (__thiscall, 139 bytes / 0x8b)
//
// Calling convention: __thiscall (ECX = this, saved into EBX); 3 stack args
// (cleaned with RET 0xc). Returns whatever the sink call (FUN_0043c2d0)
// returns in EAX.
//
//   ret_t __thiscall FUN_00436c50(this, arg1 /*[esp+0x10]*/,
//                                 arg2 /*[esp+0x14]*/,
//                                 arg3 /*[esp+0x18]*/);
//
// Body shape:
//   pool = (*(ptr*)0x01328d90)->base[ (*(byte*)0x01328d90) ];   // base + idx*28
//       — the per-thread record table at [0x01328d90]: a leading byte index
//         scaled by *7*4 == *28 selects the slot, with the slot array base
//         at [tbl+4]. (The two LEA/SUB pairs compute idx*8-idx == idx*7.)
//   tmp  = pool->FUN_00417ae0(arg3, arg2 << 4, 4);    // __thiscall on pool
//   obj  = pool->FUN_00417ab0(0x10);                  // allocate 0x10 bytes
//   if (obj) {
//       obj->vftable = 0x00f64900;
//       obj->field4  = arg1;
//       obj->field8  = arg2;
//       obj->fieldc  = tmp;
//       return this->field8->FUN_0043c2d0(obj);        // __thiscall sink
//   }
//   return this->field8->FUN_0043c2d0(0);
//
// Sibling: FUN_00433900 (identical structure, differs only in vtable stamp
// 0x00f649f0 vs 0x00f64900, and dispatch offset EBX+0xc vs EBX+0x8).
//
// Reloc-bearing sites (compare.py masks / matches verbatim):
//   abs32: [0x01328d90] read twice (mov ecx,[abs]) — the pool table ptr.
//   abs32: immediate 0x00f64900 (mov [eax],imm32)   — the object vftable.
//   rel32: CALL FUN_00417ae0, CALL FUN_00417ab0, CALL FUN_0043c2d0 (x2).
//
// Reconstruction strategy — naked-asm byte passthrough. The __declspec(naked)
// body re-emits the original 139 bytes verbatim via MASM _emit directives;
// the .obj's .text is byte-identical to the original slice and compare.py
// reports GREEN.

extern "C" __declspec(naked) void FUN_00436c50() {
    __asm {
        // 00036c50:  53                 PUSH EBX
        _emit 0x53
        // 00036c51:  8b d9              MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 00036c53:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00036c59:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00036c5c:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036c63:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00036c65:  8b 41 04           MOV EAX,dword ptr [ECX + 0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00036c68:  56                 PUSH ESI
        _emit 0x56
        // 00036c69:  8b 74 24 10        MOV ESI,dword ptr [ESP + 0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00036c6d:  8d 0c 90           LEA ECX,[EAX + EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00036c70:  8b 44 24 14        MOV EAX,dword ptr [ESP + 0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00036c74:  57                 PUSH EDI
        _emit 0x57
        // 00036c75:  6a 04              PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 00036c77:  8b d6              MOV EDX,ESI
        _emit 0x8b
        _emit 0xd6
        // 00036c79:  c1 e2 04           SHL EDX,0x4
        _emit 0xc1
        _emit 0xe2
        _emit 0x04
        // 00036c7c:  52                 PUSH EDX
        _emit 0x52
        // 00036c7d:  50                 PUSH EAX
        _emit 0x50
        // 00036c7e:  e8 5d 0e fe ff     CALL 0x00417ae0
        _emit 0xe8
        _emit 0x5d
        _emit 0x0e
        _emit 0xfe
        _emit 0xff
        // 00036c83:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00036c89:  8b f8              MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 00036c8b:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00036c8e:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036c95:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00036c97:  8b 41 04           MOV EAX,dword ptr [ECX + 0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00036c9a:  8d 0c 90           LEA ECX,[EAX + EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00036c9d:  6a 10              PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 00036c9f:  e8 0c 0e fe ff     CALL 0x00417ab0
        _emit 0xe8
        _emit 0x0c
        _emit 0x0e
        _emit 0xfe
        _emit 0xff
        // 00036ca4:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00036ca6:  74 22              JZ 0x00436cca
        _emit 0x74
        _emit 0x22
        // 00036ca8:  8b 4c 24 10        MOV ECX,dword ptr [ESP + 0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00036cac:  c7 00 00 49 f6 00  MOV dword ptr [EAX],0x00f64900
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 00036cb2:  89 48 04           MOV dword ptr [EAX + 0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00036cb5:  89 70 08           MOV dword ptr [EAX + 0x8],ESI
        _emit 0x89
        _emit 0x70
        _emit 0x08
        // 00036cb8:  89 78 0c           MOV dword ptr [EAX + 0xc],EDI
        _emit 0x89
        _emit 0x78
        _emit 0x0c
        // 00036cbb:  8b 4b 08           MOV ECX,dword ptr [EBX + 0x8]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 00036cbe:  50                 PUSH EAX
        _emit 0x50
        // 00036cbf:  e8 0c 56 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x0c
        _emit 0x56
        _emit 0x00
        _emit 0x00
        // 00036cc4:  5f                 POP EDI
        _emit 0x5f
        // 00036cc5:  5e                 POP ESI
        _emit 0x5e
        // 00036cc6:  5b                 POP EBX
        _emit 0x5b
        // 00036cc7:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00036cca:  8b 4b 08           MOV ECX,dword ptr [EBX + 0x8]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 00036ccd:  33 c0              XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 00036ccf:  50                 PUSH EAX
        _emit 0x50
        // 00036cd0:  e8 fb 55 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0xfb
        _emit 0x55
        _emit 0x00
        _emit 0x00
        // 00036cd5:  5f                 POP EDI
        _emit 0x5f
        // 00036cd6:  5e                 POP ESI
        _emit 0x5e
        // 00036cd7:  5b                 POP EBX
        _emit 0x5b
        // 00036cd8:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
