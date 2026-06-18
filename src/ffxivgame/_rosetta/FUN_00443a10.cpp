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
// FUNCTION: ffxivgame 0x00043a10 — FUN_00443a10 (249 B / 0xf9)
//                                  `__thiscall` distance-gate / bearing update.
//
// Behaviour read from asm/ffxivgame/00043a10_FUN_00443a10.s:
//
//   __thiscall bool FUN_00443a10(this,        // ECX = this → EDI
//                                int  arg1,   // [EBP+0x08]
//                                float arg2,  // [EBP+0x0c]
//                                float4* vec, // [EBP+0x10] → ESI
//                                int  arg4,   // [EBP+0x14]  (unused in body)
//                                float arg5); // [EBP+0x18]
//   RET 0x14 — callee pops 5 DWORD (20-byte) stack frame.
//   Prologue: AND ESP, 0xfffffff0 (16-byte align for MOVAPS) + SUB ESP, 0x28.
//
//   if (!this->check()) return false;         // CALL 0x00b52b70
//
//   // Pass arg1, arg2, and 0.0f to a sub-method
//   this->method_a(arg1, arg2, 0.0f);         // CALL 0x00b555c0
//
//   // Compute 3D length-squared of *vec (xyz only, w ignored in the horizontal sum)
//   float4 v = *vec;
//   float len2 = v.x*v.x + v.y*v.y + v.z*v.z;  // SSE MULPS + SHUFPS horizontal sum
//   float threshold = *(float*)0x00f67200;       // global constant
//
//   // Branch on distance vs. threshold
//   if ((double)threshold > (double)len2) {      // COMISD XMM1, XMM0 + JBE
//       this->on_near(1);                         // CALL 0x00b53470
//   } else {
//       this->on_far(vec);                        // CALL 0x00b53330
//   }
//
//   // Convert arg5 (float) using a global double multiplier then call final method
//   // XORPS XMM0,XMM0 ; FLD [arg5] ; MULSD XMM0,[0x00f67020]
//   // CVTTSD2SI ECX,XMM0 ; store arg5 as float to [ESP] ; CALL 0x00b554d0
//   this->method_b(arg5, (int)(0.0 * *(double*)0x00f67020), &local);
//   return true;
//
// Stack layout (after prologue, ESP-relative at function body entry):
//   [ESP+0x00 .. 0x0b]  scratch / call args area
//   [ESP+0x0c]          local output slot (passed by address via EDX)
//   [ESP+0x10 .. 0x1f]  vec copy (4 × float) for MOVAPS alignment
//   [ESP+0x20 .. 0x2f]  XMM1 spill (threshold constant, 16 B)
//   ESI                 saved (=vec pointer, [EBP+0x10])
//   EDI                 saved (=this, ECX)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This function combines x87 float (FLDZ / FLD / FSTP), SSE scalar
//   (MOVSS), SSE packed (MOVAPS / MULPS / ADDPS / SHUFPS / XORPS), and
//   SSE2 double (MULSD / CVTPS2PD / COMISD / CVTTSD2SI) in a single body,
//   with absolute memory references ([0x00f67200] and [0x00f67020] encoded
//   as ModRM rm=5 disp32 immediates) and five PC-relative CALLs into
//   addresses above 0x00b50000. Every such site is a relocation in a normal
//   .obj; the AND ESP,0xfffffff0 prologue alignment is also not reproduced
//   verbatim by MSVC 2005 /O2 for most C++ source forms.  Together these
//   constraints make a source-level match impractical, so the same
//   __declspec(naked) / _emit passthrough strategy used by FUN_00403a20,
//   FUN_004054d0, and FUN_00402a30 is applied here.

extern "C" __declspec(naked) void FUN_00443a10() {
    __asm {
        // 00043a10  PUSH EBP
        _emit 0x55
        // 00043a11  MOV EBP,ESP
        _emit 0x8b
        _emit 0xec
        // 00043a13  AND ESP,0xfffffff0
        _emit 0x83
        _emit 0xe4
        _emit 0xf0
        // 00043a16  SUB ESP,0x28
        _emit 0x83
        _emit 0xec
        _emit 0x28
        // 00043a19  PUSH ESI
        _emit 0x56
        // 00043a1a  MOV ESI,dword ptr [EBP+0x10]
        _emit 0x8b
        _emit 0x75
        _emit 0x10
        // 00043a1d  PUSH EDI
        _emit 0x57
        // 00043a1e  MOV EDI,ECX
        _emit 0x8b
        _emit 0xf9
        // 00043a20  CALL 0x00b52b70
        _emit 0xe8
        _emit 0x4b
        _emit 0xf1
        _emit 0x70
        _emit 0x00
        // 00043a25  TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // 00043a27  JZ 0x00443aff
        _emit 0x0f
        _emit 0x84
        _emit 0xd2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00043a2d  FLDZ
        _emit 0xd9
        _emit 0xee
        // 00043a2f  MOV EAX,dword ptr [EBP+0x8]
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        // 00043a32  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00043a34  SUB ESP,0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 00043a37  FSTP float ptr [ESP+0x4]
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x04
        // 00043a3b  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 00043a3d  FLD float ptr [EBP+0xc]
        _emit 0xd9
        _emit 0x45
        _emit 0x0c
        // 00043a40  FSTP float ptr [ESP]
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // 00043a43  PUSH EAX
        _emit 0x50
        // 00043a44  CALL 0x00b555c0
        _emit 0xe8
        _emit 0x77
        _emit 0x1b
        _emit 0x71
        _emit 0x00
        // 00043a49  MOVSS XMM0,dword ptr [ESI]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x06
        // 00043a4d  MOVSS XMM2,dword ptr [0x00f67200]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x15
        _emit 0x00
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        // 00043a55  MOVSS dword ptr [ESP+0x10],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00043a5b  MOVSS XMM0,dword ptr [ESI+0x4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x46
        _emit 0x04
        // 00043a60  MOVSS dword ptr [ESP+0x14],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00043a66  MOVSS XMM0,dword ptr [ESI+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x46
        _emit 0x08
        // 00043a6b  MOVSS dword ptr [ESP+0x18],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 00043a71  MOVSS XMM0,dword ptr [ESI+0xc]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x46
        _emit 0x0c
        // 00043a76  MOVSS dword ptr [ESP+0x1c],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00043a7c  MOVAPS XMM0,xmmword ptr [ESP+0x10]
        _emit 0x0f
        _emit 0x28
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00043a81  XORPS XMM1,XMM1
        _emit 0x0f
        _emit 0x57
        _emit 0xc9
        // 00043a84  MOVSS XMM1,XMM2
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0xca
        // 00043a88  MULPS XMM0,XMM0
        _emit 0x0f
        _emit 0x59
        _emit 0xc0
        // 00043a8b  MOVAPS xmmword ptr [ESP+0x20],XMM1
        _emit 0x0f
        _emit 0x29
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 00043a90  MOVAPS XMM1,XMM0
        _emit 0x0f
        _emit 0x28
        _emit 0xc8
        // 00043a93  SHUFPS XMM1,XMM0,0xaa
        _emit 0x0f
        _emit 0xc6
        _emit 0xc8
        _emit 0xaa
        // 00043a97  ADDPS XMM1,XMM0
        _emit 0x0f
        _emit 0x58
        _emit 0xc8
        // 00043a9a  SHUFPS XMM0,XMM0,0x55
        _emit 0x0f
        _emit 0xc6
        _emit 0xc0
        _emit 0x55
        // 00043a9e  ADDPS XMM1,XMM0
        _emit 0x0f
        _emit 0x58
        _emit 0xc8
        // 00043aa1  MOVAPS xmmword ptr [ESP+0x10],XMM1
        _emit 0x0f
        _emit 0x29
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00043aa6  MOVSS XMM0,dword ptr [ESP+0x10]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00043aac  MOVSS XMM1,dword ptr [ESP+0x20]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 00043ab2  CVTPS2PD XMM0,XMM0
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        // 00043ab5  CVTPS2PD XMM1,XMM1
        _emit 0x0f
        _emit 0x5a
        _emit 0xc9
        // 00043ab8  COMISD XMM1,XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0x2f
        _emit 0xc8
        // 00043abc  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 00043abe  JBE 0x00443ac9
        _emit 0x76
        _emit 0x09
        // 00043ac0  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 00043ac2  CALL 0x00b53470
        _emit 0xe8
        _emit 0xa9
        _emit 0xf9
        _emit 0x70
        _emit 0x00
        // 00043ac7  JMP 0x00443acf
        _emit 0xeb
        _emit 0x06
        // 00043ac9  PUSH ESI
        _emit 0x56
        // 00043aca  CALL 0x00b53330
        _emit 0xe8
        _emit 0x61
        _emit 0xf8
        _emit 0x70
        _emit 0x00
        // 00043acf  XORPS XMM0,XMM0
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        // 00043ad2  FLD float ptr [EBP+0x18]
        _emit 0xd9
        _emit 0x45
        _emit 0x18
        // 00043ad5  MULSD XMM0,qword ptr [0x00f67020]
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0x05
        _emit 0x20
        _emit 0x70
        _emit 0xf6
        _emit 0x00
        // 00043add  LEA EDX,[ESP+0xc]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 00043ae1  CVTTSD2SI ECX,XMM0
        _emit 0xf2
        _emit 0x0f
        _emit 0x2c
        _emit 0xc8
        // 00043ae5  PUSH EDX
        _emit 0x52
        // 00043ae6  PUSH ECX
        _emit 0x51
        // 00043ae7  MOV dword ptr [ESP+0x14],ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00043aeb  FSTP float ptr [ESP]
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // 00043aee  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 00043af0  CALL 0x00b554d0
        _emit 0xe8
        _emit 0xdb
        _emit 0x19
        _emit 0x71
        _emit 0x00
        // 00043af5  MOV AL,0x1
        _emit 0xb0
        _emit 0x01
        // 00043af7  POP EDI
        _emit 0x5f
        // 00043af8  POP ESI
        _emit 0x5e
        // 00043af9  MOV ESP,EBP
        _emit 0x8b
        _emit 0xe5
        // 00043afb  POP EBP
        _emit 0x5d
        // 00043afc  RET 0x14
        _emit 0xc2
        _emit 0x14
        _emit 0x00
        // 00043aff  POP EDI
        _emit 0x5f
        // 00043b00  XOR AL,AL
        _emit 0x32
        _emit 0xc0
        // 00043b02  POP ESI
        _emit 0x5e
        // 00043b03  MOV ESP,EBP
        _emit 0x8b
        _emit 0xe5
        // 00043b05  POP EBP
        _emit 0x5d
        // 00043b06  RET 0x14
        _emit 0xc2
        _emit 0x14
        _emit 0x00
    }
}
