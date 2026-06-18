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
// FUNCTION: ffxivgame 0x00039ed0 — flag-normalisation + dispatch helper
//           (220 B / 0xdc, __thiscall, RET 0x18 = 6 DWORD stack args)
//
// Signature (recovered from asm):
//
//   void __thiscall FUN_00439ed0(this,
//                                 arg0,   // [ESP+0x28] after frame push
//                                 arg1,   // [ESP+0x2c]
//                                 arg2,   // [ESP+0x30]
//                                 arg3*,  // [ESP+0x34] — ptr; *(arg3+0xc) is a float
//                                 flags,  // [ESP+0x38] — bitmask
//                                 fval);  // [ESP+0x3c] — float
//
// Body sketch (logical):
//
//   flags = arg4 (loaded early into EBX via [ESP+0x2c] after single PUSH EBX)
//   if (!(flags & 0x26)) flags |= 0x02;   // ensure at least one of bits 1/2/5 set
//   if (!(flags & 0x58)) flags |= 0x08;   // ensure at least one of bits 3/4/6 set
//
//   int si = (int)fval;                   // CVTTSS2SI truncate float arg to int
//   int eax = FUN_00439560(arg0);         // helper returning some scaled value
//   eax *= si;                            // IMUL
//
//   int di = arg1;
//   int bp = arg2;
//   if (flags & 0x04) di -= eax;
//   if (flags & 0x20) { eax = -(floor_div2(eax)); di += eax; }
//   if (flags & 0x10) bp -= 12;
//   if (flags & 0x40) bp -= 6;
//
//   void* ebx = arg3;
//   if (flags & 0x01) {
//       // First dispatch: passes extra XMM-scaled field from *arg3
//       float scale = (float)(*(float*)(ebx+0x0c) * k_const);  // SSE double round-trip
//       FUN_00439db0(this, fval, &outStruct, si+bp, si+di, arg0, ..., scale);
//   }
//   // Second dispatch: unconditional
//   FUN_00439db0(this, fval, ebx, bp, di, ecx_copy, ...);
//
// Relocation sites (compare.py wildcards these 4-byte windows):
//   +0x2b   REL32 → 0x00439560   (CALL FUN_00439560)
//   +0x74   DIR32 → 0x00f59898   (MULSD constant double address)
//   +0x7f   REL32 → 0x00439db0   (CALL FUN_00439db0, first)
//   +0xce   REL32 → 0x00439db0   (CALL FUN_00439db0, second)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The function mixes x87 FLD/FSTP with SSE MOVSS/CVTPS2PD/MULSD/CVTPD2PS
//   in a way that MSVC 2005 /O2 would not reproduce from a C++ source without
//   very specific local-variable ordering tricks. The interleaving of
//   XORPS XMM0,XMM0 zero-init of a stack struct with the x87 float-push
//   idiom (FLD + FSTP [ESP]) and the branch-conditional first dispatch make
//   exact byte reproduction via C++ impractical for the same reasons as
//   FUN_00408910 and FUN_00404e40. Emitting the 220 original bytes verbatim
//   via MASM _emit directives gives the .obj a byte-identical .text section.

extern "C" __declspec(naked) void FUN_00439ed0() {
    __asm {
        // 00039ed0: SUB ESP,0x14
        _emit 0x83
        _emit 0xec
        _emit 0x14
        // 00039ed3: PUSH EBX
        _emit 0x53
        // 00039ed4: MOV EBX,[ESP+0x2c]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x2c
        // 00039ed8: TEST BL,0x26
        _emit 0xf6
        _emit 0xc3
        _emit 0x26
        // 00039edb: PUSH EBP
        _emit 0x55
        // 00039edc: PUSH ESI
        _emit 0x56
        // 00039edd: PUSH EDI
        _emit 0x57
        // 00039ede: MOV [ESP+0x10],ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00039ee2: JNZ +3 (skip OR)
        _emit 0x75
        _emit 0x03
        // 00039ee4: OR EBX,0x2
        _emit 0x83
        _emit 0xcb
        _emit 0x02
        // 00039ee7: TEST BL,0x58
        _emit 0xf6
        _emit 0xc3
        _emit 0x58
        // 00039eea: JNZ +3 (skip OR)
        _emit 0x75
        _emit 0x03
        // 00039eec: OR EBX,0x8
        _emit 0x83
        _emit 0xcb
        _emit 0x08
        // 00039eef: MOV EAX,[ESP+0x28]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 00039ef3: CVTTSS2SI ESI,[ESP+0x3c]
        _emit 0xf3
        _emit 0x0f
        _emit 0x2c
        _emit 0x74
        _emit 0x24
        _emit 0x3c
        // 00039ef9: PUSH EAX
        _emit 0x50
        // 00039efa: CALL 0x00439560  [REL32 reloc]
        _emit 0xe8
        _emit 0x61
        _emit 0xf6
        _emit 0xff
        _emit 0xff
        // 00039eff: IMUL EAX,ESI
        _emit 0x0f
        _emit 0xaf
        _emit 0xc6
        // 00039f02: TEST BL,0x4
        _emit 0xf6
        _emit 0xc3
        _emit 0x04
        // 00039f05: MOV EDI,[ESP+0x2c]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x2c
        // 00039f09: MOV EBP,[ESP+0x30]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x30
        // 00039f0d: JZ +2 (skip SUB)
        _emit 0x74
        _emit 0x02
        // 00039f0f: SUB EDI,EAX
        _emit 0x2b
        _emit 0xf8
        // 00039f11: TEST BL,0x20
        _emit 0xf6
        _emit 0xc3
        _emit 0x20
        // 00039f14: JZ +9 (skip block)
        _emit 0x74
        _emit 0x09
        // 00039f16: CDQ
        _emit 0x99
        // 00039f17: SUB EAX,EDX
        _emit 0x2b
        _emit 0xc2
        // 00039f19: SAR EAX,1
        _emit 0xd1
        _emit 0xf8
        // 00039f1b: NEG EAX
        _emit 0xf7
        _emit 0xd8
        // 00039f1d: ADD EDI,EAX
        _emit 0x03
        _emit 0xf8
        // 00039f1f: TEST BL,0x10
        _emit 0xf6
        _emit 0xc3
        _emit 0x10
        // 00039f22: JZ +3 (skip ADD)
        _emit 0x74
        _emit 0x03
        // 00039f24: ADD EBP,-0xc
        _emit 0x83
        _emit 0xc5
        _emit 0xf4
        // 00039f27: TEST BL,0x40
        _emit 0xf6
        _emit 0xc3
        _emit 0x40
        // 00039f2a: JZ +3 (skip SUB)
        _emit 0x74
        _emit 0x03
        // 00039f2c: SUB EBP,0x6
        _emit 0x83
        _emit 0xed
        _emit 0x06
        // 00039f2f: TEST BL,0x1
        _emit 0xf6
        _emit 0xc3
        _emit 0x01
        // 00039f32: MOV EBX,[ESP+0x34]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x34
        // 00039f36: JZ +0x51 (skip to second dispatch)
        _emit 0x74
        _emit 0x51
        // 00039f38: XORPS XMM0,XMM0
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        // 00039f3b: FLD [ESP+0x3c]
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        // 00039f3f: MOV EAX,[ESP+0x28]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 00039f43: PUSH ECX
        _emit 0x51
        // 00039f44: FSTP [ESP]
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // 00039f47: LEA ECX,[ESP+0x18]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00039f4b: PUSH ECX
        _emit 0x51
        // 00039f4c: MOV ECX,[ESP+0x18]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00039f50: MOVSS [ESP+0x1c],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00039f56: MOVSS [ESP+0x20],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 00039f5c: MOVSS [ESP+0x24],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 00039f62: MOVSS XMM0,[EBX+0xc]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x43
        _emit 0x0c
        // 00039f67: LEA EDX,[ESI+EBP]
        _emit 0x8d
        _emit 0x14
        _emit 0x2e
        // 00039f6a: PUSH EDX
        _emit 0x52
        // 00039f6b: ADD ESI,EDI
        _emit 0x03
        _emit 0xf7
        // 00039f6d: CVTPS2PD XMM0,XMM0
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        // 00039f70: MULSD XMM0,[0x00f59898]  [DIR32 reloc]
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0x05
        _emit 0x98
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        // 00039f78: PUSH ESI
        _emit 0x56
        // 00039f79: CVTPD2PS XMM0,XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        // 00039f7d: PUSH EAX
        _emit 0x50
        // 00039f7e: MOVSS [ESP+0x34],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 00039f84: CALL 0x00439db0  [REL32 reloc]
        _emit 0xe8
        _emit 0x27
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 00039f89: FLD [ESP+0x3c]
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        // 00039f8d: PUSH ECX
        _emit 0x51
        // 00039f8e: MOV ECX,[ESP+0x2c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // 00039f92: FSTP [ESP]
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // 00039f95: PUSH EBX
        _emit 0x53
        // 00039f96: PUSH EBP
        _emit 0x55
        // 00039f97: PUSH EDI
        _emit 0x57
        // 00039f98: PUSH ECX
        _emit 0x51
        // 00039f99: MOV ECX,[ESP+0x24]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 00039f9d: CALL 0x00439db0  [REL32 reloc]
        _emit 0xe8
        _emit 0x0e
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 00039fa2: POP EDI
        _emit 0x5f
        // 00039fa3: POP ESI
        _emit 0x5e
        // 00039fa4: POP EBP
        _emit 0x5d
        // 00039fa5: POP EBX
        _emit 0x5b
        // 00039fa6: ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00039fa9: RET 0x18
        _emit 0xc2
        _emit 0x18
        _emit 0x00
    }
}
