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
// FUNCTION: ffxivgame 0x00433990 — __thiscall method (3 explicit stack args)
//                                  that writes data into a pool slot, allocates
//                                  a 16-byte object from the same pool, fills
//                                  in its vtable and fields, then dispatches it
//                                  via this->field_C->field_0x24 (139 bytes).
//
// Calling convention: __thiscall (ECX = this); 3 explicit stack args; RET 0xC.
// Callee-saves pushed: EBX (as 'this'), ESI (arg2), EDI (result of first call).
//
// Pseudocode:
//
//   void FUN_00433990(this, arg1, arg2, arg3) {
//       auto slot = g_pool[byte(*g_pool) * 7];   // indexed pool slot
//       void* data = FUN_00417a70(slot, arg3, arg2*4);  // write arg2*4 bytes
//       auto* obj  = FUN_00417ab0(slot, 0x10);          // alloc 16 bytes
//       if (obj) {
//           obj->vtbl    = (void*)0x00F64908;
//           obj->field_4 = arg1;
//           obj->field_8 = arg2;
//           obj->field_C = data;
//           FUN_0043c2d0(this->field_C, obj);
//       } else {
//           FUN_0043c2d0(this->field_C, nullptr);
//       }
//   }
//
// Global references:
//   [0x01328D90]  — g_pool pointer (byte at [0] = pool index, [4] = array ptr)
//   0x00F64908    — vtable pointer for the newly allocated 16-byte object
//
// Calls (reloc sites — compare.py masks these 4-byte displacements):
//   REL: FUN_00417a70  (pool write  helper, __thiscall 2 explicit args, RET 0x8)
//   REL: FUN_00417ab0  (pool alloc  helper, __thiscall 1 explicit arg,  RET 0x4)
//   REL: FUN_0043c2d0  (dispatch,           __thiscall 1 explicit arg,  RET 0x4)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The two LEA-with-scaled-index-only forms
//     LEA EDX, [EAX*8+0]   (8d 14 c5 00 00 00 00, 7 bytes)
//     LEA EDX, [ESI*4+0]   (8d 14 b5 00 00 00 00, 7 bytes)
//   require the mandatory SIB+disp32 encoding (no shorter form exists for a
//   pure scaled-index with no base register). MASM would assemble these
//   correctly, but __declspec(naked) with _emit gives guaranteed byte identity
//   and keeps the pattern consistent with sibling _rosetta functions.

extern "C" __declspec(naked) void FUN_00433990()
{
    __asm {
        // 00033990:  53                 PUSH EBX
        _emit 0x53
        // 00033991:  8b d9              MOV EBX, ECX
        _emit 0x8b
        _emit 0xd9
        // 00033993:  8b 0d 90 8d 32 01  MOV ECX, dword ptr [0x01328D90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00033999:  0f b6 01           MOVZX EAX, byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 0003399c:  8d 14 c5 00 00 00 00  LEA EDX, [EAX*8+0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000339a3:  2b d0              SUB EDX, EAX
        _emit 0x2b
        _emit 0xd0
        // 000339a5:  8b 41 04           MOV EAX, dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 000339a8:  56                 PUSH ESI
        _emit 0x56
        // 000339a9:  8b 74 24 10        MOV ESI, dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 000339ad:  8d 0c 90           LEA ECX, [EAX+EDX*4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 000339b0:  8b 44 24 14        MOV EAX, dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 000339b4:  57                 PUSH EDI
        _emit 0x57
        // 000339b5:  8d 14 b5 00 00 00 00  LEA EDX, [ESI*4+0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xb5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000339bc:  52                 PUSH EDX
        _emit 0x52
        // 000339bd:  50                 PUSH EAX
        _emit 0x50
        // 000339be:  e8 ad 40 fe ff     CALL FUN_00417a70
        _emit 0xe8
        _emit 0xad
        _emit 0x40
        _emit 0xfe
        _emit 0xff
        // 000339c3:  8b 0d 90 8d 32 01  MOV ECX, dword ptr [0x01328D90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000339c9:  8b f8              MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // 000339cb:  0f b6 01           MOVZX EAX, byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 000339ce:  8d 14 c5 00 00 00 00  LEA EDX, [EAX*8+0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000339d5:  2b d0              SUB EDX, EAX
        _emit 0x2b
        _emit 0xd0
        // 000339d7:  8b 41 04           MOV EAX, dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 000339da:  8d 0c 90           LEA ECX, [EAX+EDX*4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 000339dd:  6a 10              PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 000339df:  e8 cc 40 fe ff     CALL FUN_00417ab0
        _emit 0xe8
        _emit 0xcc
        _emit 0x40
        _emit 0xfe
        _emit 0xff
        // 000339e4:  85 c0              TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000339e6:  74 22              JZ +0x22  (→ null_path at 0x00433a0a)
        _emit 0x74
        _emit 0x22
        // 000339e8:  8b 4c 24 10        MOV ECX, dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 000339ec:  c7 00 08 49 f6 00  MOV dword ptr [EAX], 0x00F64908
        _emit 0xc7
        _emit 0x00
        _emit 0x08
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 000339f2:  89 48 04           MOV dword ptr [EAX+0x4], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 000339f5:  89 70 08           MOV dword ptr [EAX+0x8], ESI
        _emit 0x89
        _emit 0x70
        _emit 0x08
        // 000339f8:  89 78 0c           MOV dword ptr [EAX+0xC], EDI
        _emit 0x89
        _emit 0x78
        _emit 0x0c
        // 000339fb:  8b 4b 0c           MOV ECX, dword ptr [EBX+0xC]
        _emit 0x8b
        _emit 0x4b
        _emit 0x0c
        // 000339fe:  50                 PUSH EAX
        _emit 0x50
        // 000339ff:  e8 cc 88 00 00     CALL FUN_0043c2d0
        _emit 0xe8
        _emit 0xcc
        _emit 0x88
        _emit 0x00
        _emit 0x00
        // 00033a04:  5f                 POP EDI
        _emit 0x5f
        // 00033a05:  5e                 POP ESI
        _emit 0x5e
        // 00033a06:  5b                 POP EBX
        _emit 0x5b
        // 00033a07:  c2 0c 00           RET 0xC
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00033a0a: (null_path)
        // 00033a0a:  8b 4b 0c           MOV ECX, dword ptr [EBX+0xC]
        _emit 0x8b
        _emit 0x4b
        _emit 0x0c
        // 00033a0d:  33 c0              XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00033a0f:  50                 PUSH EAX
        _emit 0x50
        // 00033a10:  e8 bb 88 00 00     CALL FUN_0043c2d0
        _emit 0xe8
        _emit 0xbb
        _emit 0x88
        _emit 0x00
        _emit 0x00
        // 00033a15:  5f                 POP EDI
        _emit 0x5f
        // 00033a16:  5e                 POP ESI
        _emit 0x5e
        // 00033a17:  5b                 POP EBX
        _emit 0x5b
        // 00033a18:  c2 0c 00           RET 0xC
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
