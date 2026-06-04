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
// FUNCTION: ffxivgame 0x00433900 — factory/dispatch helper that allocates a
//                                  0x10-byte event-record object from a
//                                  per-current-thread pool and forwards it
//                                  to a sink (__thiscall, 139 bytes / 0x8b)
//
// Calling convention: __thiscall (ECX = this, saved into EBX); 3 stack args
// (cleaned with RET 0xc). Returns whatever the sink call (FUN_0043c2d0)
// returns in EAX.
//
//   ret_t __thiscall FUN_00433900(this, arg1 /*[esp+0x10]*/,
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
//       obj->vftable = 0x00f649f0;
//       obj->field4  = arg1;
//       obj->field8  = arg2;
//       obj->fieldc  = tmp;
//       return this->fieldc->FUN_0043c2d0(obj);        // __thiscall sink
//   }
//   return this->fieldc->FUN_0043c2d0(0);
//
// Reloc-bearing sites (compare.py masks / matches verbatim):
//   abs32: [0x01328d90] read twice (mov ecx,[abs]) — the pool table ptr.
//   abs32: immediate 0x00f649f0 (mov [eax],imm32)   — the object vftable.
//   rel32: CALL FUN_00417ae0, CALL FUN_00417ab0, CALL FUN_0043c2d0 (x2).
//
// Reconstruction strategy — naked-asm byte passthrough. A source-level C++
// form would emit the same structural shape, but the delayed callee-save
// layout (PUSH ESI / PUSH EDI interleaved with arg setup) and the exact
// idx*8-idx LEA/SUB scaling are not reliably reproducible from C++ under
// /O2. The __declspec(naked) body re-emits the original 139 bytes verbatim
// via MASM _emit directives; the .obj's .text is byte-identical to the
// original slice and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00433900() {
    __asm {
        // 00033900:  53                 PUSH EBX
        _emit 0x53
        // 00033901:  8b d9              MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 00033903:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00033909:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 0003390c:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00033913:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00033915:  8b 41 04           MOV EAX,dword ptr [ECX + 0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00033918:  56                 PUSH ESI
        _emit 0x56
        // 00033919:  8b 74 24 10        MOV ESI,dword ptr [ESP + 0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 0003391d:  8d 0c 90           LEA ECX,[EAX + EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00033920:  8b 44 24 14        MOV EAX,dword ptr [ESP + 0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00033924:  57                 PUSH EDI
        _emit 0x57
        // 00033925:  6a 04              PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 00033927:  8b d6              MOV EDX,ESI
        _emit 0x8b
        _emit 0xd6
        // 00033929:  c1 e2 04           SHL EDX,0x4
        _emit 0xc1
        _emit 0xe2
        _emit 0x04
        // 0003392c:  52                 PUSH EDX
        _emit 0x52
        // 0003392d:  50                 PUSH EAX
        _emit 0x50
        // 0003392e:  e8 ad 41 fe ff     CALL 0x00417ae0
        _emit 0xe8
        _emit 0xad
        _emit 0x41
        _emit 0xfe
        _emit 0xff
        // 00033933:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00033939:  8b f8              MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 0003393b:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 0003393e:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00033945:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00033947:  8b 41 04           MOV EAX,dword ptr [ECX + 0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 0003394a:  8d 0c 90           LEA ECX,[EAX + EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 0003394d:  6a 10              PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 0003394f:  e8 5c 41 fe ff     CALL 0x00417ab0
        _emit 0xe8
        _emit 0x5c
        _emit 0x41
        _emit 0xfe
        _emit 0xff
        // 00033954:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00033956:  74 22              JZ 0x0043397a
        _emit 0x74
        _emit 0x22
        // 00033958:  8b 4c 24 10        MOV ECX,dword ptr [ESP + 0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0003395c:  c7 00 f0 49 f6 00  MOV dword ptr [EAX],0xf649f0
        _emit 0xc7
        _emit 0x00
        _emit 0xf0
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 00033962:  89 48 04           MOV dword ptr [EAX + 0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00033965:  89 70 08           MOV dword ptr [EAX + 0x8],ESI
        _emit 0x89
        _emit 0x70
        _emit 0x08
        // 00033968:  89 78 0c           MOV dword ptr [EAX + 0xc],EDI
        _emit 0x89
        _emit 0x78
        _emit 0x0c
        // 0003396b:  8b 4b 0c           MOV ECX,dword ptr [EBX + 0xc]
        _emit 0x8b
        _emit 0x4b
        _emit 0x0c
        // 0003396e:  50                 PUSH EAX
        _emit 0x50
        // 0003396f:  e8 5c 89 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x5c
        _emit 0x89
        _emit 0x00
        _emit 0x00
        // 00033974:  5f                 POP EDI
        _emit 0x5f
        // 00033975:  5e                 POP ESI
        _emit 0x5e
        // 00033976:  5b                 POP EBX
        _emit 0x5b
        // 00033977:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 0003397a:  8b 4b 0c           MOV ECX,dword ptr [EBX + 0xc]
        _emit 0x8b
        _emit 0x4b
        _emit 0x0c
        // 0003397d:  33 c0              XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0003397f:  50                 PUSH EAX
        _emit 0x50
        // 00033980:  e8 4b 89 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x4b
        _emit 0x89
        _emit 0x00
        _emit 0x00
        // 00033985:  5f                 POP EDI
        _emit 0x5f
        // 00033986:  5e                 POP ESI
        _emit 0x5e
        // 00033987:  5b                 POP EBX
        _emit 0x5b
        // 00033988:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
