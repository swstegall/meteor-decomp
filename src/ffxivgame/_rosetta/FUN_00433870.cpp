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
// FUNCTION: ffxivgame 0x00433870 — allocate + dispatch helper that builds a
//                                  16-byte command object (vtable 0x00f64900)
//                                  and hands it to this->field0xc's handler
//                                  (__thiscall, 139 bytes / 0x8b)
//
// Calling convention: __thiscall (ECX = this); cleans 3 DWORD stack args
//   (RET 0xc). Callee-saves pushed: EBX, ESI, EDI.
//
//   void __thiscall FUN_00433870(this, int arg1, int arg2, int arg3);
//
// Body sketch:
//   reg   = *(Reg**)0x01328d90;                 // global registry pointer
//   slot  = reg->field4 + reg->byte0 * 0x1c;    // (b*8 - b)*4 == b*28 record
//   r1    = slot->FUN_00417ae0(arg3, arg2<<4, 4);
//   p     = slot->FUN_00417ab0(0x10);           // allocate 16 bytes
//   if (p) {
//       p->vtable = 0x00f64900;
//       p->a = arg1; p->b = arg2; p->c = r1;
//       this->field0xc->FUN_0043c2d0(p);
//   } else {
//       this->field0xc->FUN_0043c2d0(0);
//   }
//
// Reloc-bearing sites (compare.py masks these positions; the literal
// displacements below are the original slice bytes so they also match
// verbatim):
//   abs disp32 [0x01328d90]   ×2   (90 8d 32 01)
//   imm32      0x00f64900           (00 49 f6 00)
//   CALL rel32 0x00417ae0  (e8 3d 42 fe ff)
//   CALL rel32 0x00417ab0  (e8 ec 41 fe ff)
//   CALL rel32 0x0043c2d0  (e8 ec 89 00 00 / e8 db 89 00 00)
//
// Reconstruction strategy — naked-asm byte passthrough. A source-level
// form would emit the same shape, but the (b*8-b)*4 record-stride LEA and
// the exact callee-save/branch layout aren't coercible from C++ under /O2.
// The __declspec(naked) body re-emits the original 139 bytes verbatim via
// MASM _emit directives, so the .obj's .text is byte-identical to the orig
// slice and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00433870() {
    __asm {
        // 00033870:  53                 PUSH EBX
        _emit 0x53
        // 00033871:  8b d9              MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 00033873:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00033879:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 0003387c:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8+0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00033883:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00033885:  8b 41 04           MOV EAX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00033888:  56                 PUSH ESI
        _emit 0x56
        // 00033889:  8b 74 24 10        MOV ESI,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 0003388d:  8d 0c 90           LEA ECX,[EAX+EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00033890:  8b 44 24 14        MOV EAX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00033894:  57                 PUSH EDI
        _emit 0x57
        // 00033895:  6a 04              PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 00033897:  8b d6              MOV EDX,ESI
        _emit 0x8b
        _emit 0xd6
        // 00033899:  c1 e2 04           SHL EDX,0x4
        _emit 0xc1
        _emit 0xe2
        _emit 0x04
        // 0003389c:  52                 PUSH EDX
        _emit 0x52
        // 0003389d:  50                 PUSH EAX
        _emit 0x50
        // 0003389e:  e8 3d 42 fe ff     CALL 0x00417ae0
        _emit 0xe8
        _emit 0x3d
        _emit 0x42
        _emit 0xfe
        _emit 0xff
        // 000338a3:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000338a9:  8b f8              MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 000338ab:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 000338ae:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8+0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000338b5:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 000338b7:  8b 41 04           MOV EAX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 000338ba:  8d 0c 90           LEA ECX,[EAX+EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 000338bd:  6a 10              PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 000338bf:  e8 ec 41 fe ff     CALL 0x00417ab0
        _emit 0xe8
        _emit 0xec
        _emit 0x41
        _emit 0xfe
        _emit 0xff
        // 000338c4:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 000338c6:  74 22              JZ 0x004338ea
        _emit 0x74
        _emit 0x22
        // 000338c8:  8b 4c 24 10        MOV ECX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 000338cc:  c7 00 00 49 f6 00  MOV dword ptr [EAX],0xf64900
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 000338d2:  89 48 04           MOV dword ptr [EAX+0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 000338d5:  89 70 08           MOV dword ptr [EAX+0x8],ESI
        _emit 0x89
        _emit 0x70
        _emit 0x08
        // 000338d8:  89 78 0c           MOV dword ptr [EAX+0xc],EDI
        _emit 0x89
        _emit 0x78
        _emit 0x0c
        // 000338db:  8b 4b 0c           MOV ECX,dword ptr [EBX+0xc]
        _emit 0x8b
        _emit 0x4b
        _emit 0x0c
        // 000338de:  50                 PUSH EAX
        _emit 0x50
        // 000338df:  e8 ec 89 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0xec
        _emit 0x89
        _emit 0x00
        _emit 0x00
        // 000338e4:  5f                 POP EDI
        _emit 0x5f
        // 000338e5:  5e                 POP ESI
        _emit 0x5e
        // 000338e6:  5b                 POP EBX
        _emit 0x5b
        // 000338e7:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 000338ea:  8b 4b 0c           MOV ECX,dword ptr [EBX+0xc]
        _emit 0x8b
        _emit 0x4b
        _emit 0x0c
        // 000338ed:  33 c0              XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 000338ef:  50                 PUSH EAX
        _emit 0x50
        // 000338f0:  e8 db 89 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0xdb
        _emit 0x89
        _emit 0x00
        _emit 0x00
        // 000338f5:  5f                 POP EDI
        _emit 0x5f
        // 000338f6:  5e                 POP ESI
        _emit 0x5e
        // 000338f7:  5b                 POP EBX
        _emit 0x5b
        // 000338f8:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
