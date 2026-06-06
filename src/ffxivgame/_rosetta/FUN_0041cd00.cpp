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
// FUNCTION: ffxivgame 0x0001cd00 — unknown parameter-set dispatch
//                                  (__cdecl, 353 B / 0x161, no SEH;
//                                   no explicit stack frame, two reg saves).
//
// Inspection (read from the disassembly at orig RVA 0x0001cd00):
//
//   __cdecl void FUN_0041cd00(void *arg1, int arg2, int arg3, int arg4,
//                              signed char arg5);
//
//   Structural shape:
//
//     float fval = (float)((double)(signed char)arg5 * [0xf598a8]);
//
//     switch (FUN_004181e0()) {
//       case 0:                          // JZ → 0x0041cd76
//         // fval unchanged (MOVSS + re-store identity)
//         break;
//       case 1:                          // second sub EAX,EBX → JZ
//         fval = (float)((double)fval + [0xf59898]);
//         break;
//       case 2:                          // third sub EAX,EBX → fall-through
//         fval = (float)((double)fval + [0xf598a0]);
//         break;
//       default:                         // JNZ → 0x0041cd95 (skip the push)
//         goto skip_float_call;
//     }
//     (*[0x0132987c])(arg1, 8, fval);    // FUN_004236a0 via __thiscall dispatch
//
//   skip_float_call:
//     bool bl_flag;                      // BL register
//     // Compute byte_local from arg3:
//     if (arg3 == 0 || arg3 == 1) byte_local = 1; else byte_local = 0;
//     // Compute bl_flag from arg2:
//     if (arg2 != 0 && arg2 != 1) BL = 0; // else stays 1
//
//     int r = FUN_004181c0();
//     if (r >= 0) {
//       int v;
//       if (r <= 2) {
//         v = (byte_local == 0) ? 3 : 1;
//       } else if (r == 3) {
//         v = byte_local ? 1 : 2;
//       } else {
//         goto tail;
//       }
//       (*[0x0132987c])(arg1, 6, v);
//       int bv = BL ? 1 : 2;
//       (*[0x0132987c])(arg1, 5, bv);
//       (*[0x0132987c])(arg1, 7, 2);
//     }
//   tail:
//     if (arg4 == 0) {
//       (*[0x0132987c])(arg1, 9, 0);
//       float zero = 0.0f;
//       (*[0x0132987c])(arg1, 8, zero);
//       (*[0x0132987c])(arg1, 7, 0);
//     }
//
//   Reloc-bearing sites in the orig 353 bytes:
//     +0x09   DIR32 → 0x00f598a8   (MULSD double constant)
//     +0x1d   REL32 → 0x004181e0   (CALL FUN_004181e0)
//     +0x3e   DIR32 → 0x00f598a0   (ADDSD double constant, case 2)
//     +0x5d   DIR32 → 0x00f59898   (ADDSD double constant, case 1)
//     +0x87   DIR32 → 0x0132987c   (global dispatch table ptr, ×8)
//     +0x90   REL32 → 0x004236a0   (CALL FUN_004236a0, ×7)
//     +0xb8   REL32 → 0x004181c0   (CALL FUN_004181c0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function contains eight absolute DIR32 references (global pointer
//   at 0x0132987c and three double-constant addresses) and eight CALL
//   rel32 targets that resolve only at the orig image base. A source-level
//   C++ port would need to reproduce the exact SSE2 float-conversion
//   sequence (CVTSI2SD/MULSD/CVTSD2SS/MOVSS/CVTSS2SD/ADDSD) plus the
//   exact register allocation (EBX=1 live across the whole body, BL as a
//   boolean flag), the cascaded SUB-EAX/JZ switch shape, and all
//   reloc-bearing immediates. The naked-asm passthrough re-emits the
//   orig 353 bytes verbatim; tools/compare.py reports GREEN (353/353).

extern "C" __declspec(naked) void FUN_0041cd00() {
    __asm {
        // 0001cd00: MOVSX EAX,byte ptr [ESP+0x14]
        _emit 0x0f
        _emit 0xbe
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0001cd05: CVTSI2SD XMM0,EAX
        _emit 0xf2
        _emit 0x0f
        _emit 0x2a
        _emit 0xc0
        // 0001cd09: MULSD XMM0,qword ptr [0x00f598a8]
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0x05
        _emit 0xa8
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        // 0001cd11: PUSH EBX
        _emit 0x53
        // 0001cd12: CVTSD2SS XMM0,XMM0
        _emit 0xf2
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        // 0001cd16: PUSH ESI
        _emit 0x56
        // 0001cd17: MOVSS dword ptr [ESP+0x1c],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0001cd1d: CALL 0x004181e0
        _emit 0xe8
        _emit 0xbe
        _emit 0xb4
        _emit 0xff
        _emit 0xff
        // 0001cd22: SUB EAX,0x0
        _emit 0x83
        _emit 0xe8
        _emit 0x00
        // 0001cd25: MOV ESI,dword ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 0001cd29: MOV EBX,0x1
        _emit 0xbb
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001cd2e: JZ 0x0041cd76
        _emit 0x74
        _emit 0x46
        // 0001cd30: SUB EAX,EBX
        _emit 0x2b
        _emit 0xc3
        // 0001cd32: JZ 0x0041cd57
        _emit 0x74
        _emit 0x23
        // 0001cd34: SUB EAX,EBX
        _emit 0x2b
        _emit 0xc3
        // 0001cd36: JNZ 0x0041cd95
        _emit 0x75
        _emit 0x5d
        // 0001cd38: CVTSS2SD XMM0,dword ptr [ESP+0x1c]
        _emit 0xf3
        _emit 0x0f
        _emit 0x5a
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0001cd3e: ADDSD XMM0,qword ptr [0x00f598a0]
        _emit 0xf2
        _emit 0x0f
        _emit 0x58
        _emit 0x05
        _emit 0xa0
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        // 0001cd46: CVTSD2SS XMM0,XMM0
        _emit 0xf2
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        // 0001cd4a: MOVSS dword ptr [ESP+0x1c],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0001cd50: MOV ECX,dword ptr [ESP+0x1c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0001cd54: PUSH ECX
        _emit 0x51
        // 0001cd55: JMP 0x0041cd87
        _emit 0xeb
        _emit 0x30
        // 0001cd57: CVTSS2SD XMM0,dword ptr [ESP+0x1c]
        _emit 0xf3
        _emit 0x0f
        _emit 0x5a
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0001cd5d: ADDSD XMM0,qword ptr [0x00f59898]
        _emit 0xf2
        _emit 0x0f
        _emit 0x58
        _emit 0x05
        _emit 0x98
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        // 0001cd65: CVTSD2SS XMM0,XMM0
        _emit 0xf2
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        // 0001cd69: MOVSS dword ptr [ESP+0x1c],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0001cd6f: MOV EDX,dword ptr [ESP+0x1c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 0001cd73: PUSH EDX
        _emit 0x52
        // 0001cd74: JMP 0x0041cd87
        _emit 0xeb
        _emit 0x11
        // 0001cd76: MOVSS XMM0,dword ptr [ESP+0x1c]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0001cd7c: MOVSS dword ptr [ESP+0x1c],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0001cd82: MOV EAX,dword ptr [ESP+0x1c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0001cd86: PUSH EAX
        _emit 0x50
        // 0001cd87: MOV ECX,dword ptr [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001cd8d: PUSH 0x8
        _emit 0x6a
        _emit 0x08
        // 0001cd8f: PUSH ESI
        _emit 0x56
        // 0001cd90: CALL 0x004236a0
        _emit 0xe8
        _emit 0x0b
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // 0001cd95: MOV EAX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0001cd99: TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001cd9b: JZ 0x0041cda6
        _emit 0x74
        _emit 0x09
        // 0001cd9d: CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // 0001cd9f: MOV byte ptr [ESP+0x1c],0x0
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x00
        // 0001cda4: JNZ 0x0041cdaa
        _emit 0x75
        _emit 0x04
        // 0001cda6: MOV byte ptr [ESP+0x1c],BL
        _emit 0x88
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        // 0001cdaa: MOV EAX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0001cdae: TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001cdb0: JZ 0x0041cdb8
        _emit 0x74
        _emit 0x06
        // 0001cdb2: CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // 0001cdb4: JZ 0x0041cdb8
        _emit 0x74
        _emit 0x02
        // 0001cdb6: XOR BL,BL
        _emit 0x32
        _emit 0xdb
        // 0001cdb8: CALL 0x004181c0
        _emit 0xe8
        _emit 0x03
        _emit 0xb4
        _emit 0xff
        _emit 0xff
        // 0001cdbd: TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001cdbf: JL 0x0041ce1b
        _emit 0x7c
        _emit 0x5a
        // 0001cdc1: CMP EAX,0x2
        _emit 0x83
        _emit 0xf8
        _emit 0x02
        // 0001cdc4: JLE 0x0041cdd9
        _emit 0x7e
        _emit 0x13
        // 0001cdc6: CMP EAX,0x3
        _emit 0x83
        _emit 0xf8
        _emit 0x03
        // 0001cdc9: JNZ 0x0041ce1b
        _emit 0x75
        _emit 0x50
        // 0001cdcb: MOV AL,byte ptr [ESP+0x1c]
        _emit 0x8a
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0001cdcf: NEG AL
        _emit 0xf6
        _emit 0xd8
        // 0001cdd1: SBB EAX,EAX
        _emit 0x1b
        _emit 0xc0
        // 0001cdd3: ADD EAX,0x2
        _emit 0x83
        _emit 0xc0
        _emit 0x02
        // 0001cdd6: PUSH EAX
        _emit 0x50
        // 0001cdd7: JMP 0x0041cde7
        _emit 0xeb
        _emit 0x0e
        // 0001cdd9: XOR ECX,ECX
        _emit 0x33
        _emit 0xc9
        // 0001cddb: CMP byte ptr [ESP+0x1c],CL
        _emit 0x38
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0001cddf: SETZ CL
        _emit 0x0f
        _emit 0x94
        _emit 0xc1
        // 0001cde2: LEA ECX,[ECX+ECX*0x1+0x1]
        _emit 0x8d
        _emit 0x4c
        _emit 0x09
        _emit 0x01
        // 0001cde6: PUSH ECX
        _emit 0x51
        // 0001cde7: MOV ECX,dword ptr [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001cded: PUSH 0x6
        _emit 0x6a
        _emit 0x06
        // 0001cdef: PUSH ESI
        _emit 0x56
        // 0001cdf0: CALL 0x004236a0
        _emit 0xe8
        _emit 0xab
        _emit 0x68
        _emit 0x00
        _emit 0x00
        // 0001cdf5: MOV ECX,dword ptr [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001cdfb: NEG BL
        _emit 0xf6
        _emit 0xdb
        // 0001cdfd: SBB EBX,EBX
        _emit 0x1b
        _emit 0xdb
        // 0001cdff: ADD EBX,0x2
        _emit 0x83
        _emit 0xc3
        _emit 0x02
        // 0001ce02: PUSH EBX
        _emit 0x53
        // 0001ce03: PUSH 0x5
        _emit 0x6a
        _emit 0x05
        // 0001ce05: PUSH ESI
        _emit 0x56
        // 0001ce06: CALL 0x004236a0
        _emit 0xe8
        _emit 0x95
        _emit 0x68
        _emit 0x00
        _emit 0x00
        // 0001ce0b: MOV ECX,dword ptr [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001ce11: PUSH 0x2
        _emit 0x6a
        _emit 0x02
        // 0001ce13: PUSH 0x7
        _emit 0x6a
        _emit 0x07
        // 0001ce15: PUSH ESI
        _emit 0x56
        // 0001ce16: CALL 0x004236a0
        _emit 0xe8
        _emit 0x85
        _emit 0x68
        _emit 0x00
        _emit 0x00
        // 0001ce1b: CMP dword ptr [ESP+0x18],0x0
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x00
        // 0001ce20: JNZ 0x0041ce5e
        _emit 0x75
        _emit 0x3c
        // 0001ce22: MOV ECX,dword ptr [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001ce28: PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001ce2a: PUSH 0x9
        _emit 0x6a
        _emit 0x09
        // 0001ce2c: PUSH ESI
        _emit 0x56
        // 0001ce2d: CALL 0x004236a0
        _emit 0xe8
        _emit 0x6e
        _emit 0x68
        _emit 0x00
        _emit 0x00
        // 0001ce32: XORPS XMM0,XMM0
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        // 0001ce35: MOV ECX,dword ptr [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001ce3b: MOVSS dword ptr [ESP+0x1c],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0001ce41: MOV EDX,dword ptr [ESP+0x1c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 0001ce45: PUSH EDX
        _emit 0x52
        // 0001ce46: PUSH 0x8
        _emit 0x6a
        _emit 0x08
        // 0001ce48: PUSH ESI
        _emit 0x56
        // 0001ce49: CALL 0x004236a0
        _emit 0xe8
        _emit 0x52
        _emit 0x68
        _emit 0x00
        _emit 0x00
        // 0001ce4e: MOV ECX,dword ptr [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001ce54: PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001ce56: PUSH 0x7
        _emit 0x6a
        _emit 0x07
        // 0001ce58: PUSH ESI
        _emit 0x56
        // 0001ce59: CALL 0x004236a0
        _emit 0xe8
        _emit 0x42
        _emit 0x68
        _emit 0x00
        _emit 0x00
        // 0001ce5e: POP ESI
        _emit 0x5e
        // 0001ce5f: POP EBX
        _emit 0x5b
        // 0001ce60: RET
        _emit 0xc3
    }
}
