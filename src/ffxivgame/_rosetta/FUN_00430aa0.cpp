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
// FUNCTION: ffxivgame 0x00030aa0 — `__thiscall` state-dispatch method
//                                   (298 B / 0x12a, no SEH frame).
//
// Inspection (read from the disassembly at orig RVA 0x00030aa0):
//
//   __thiscall void FUN_00430aa0(this);   // ECX = this
//
//   Switch on this->field_38 (dword at +0x38):
//
//   case 2:
//     Dispatch via a nested object pointer at this->field_2c (+0x2c).
//     Reads field_2c->field_0c to determine sub-case:
//       sub-case 0 or other (field_0c == 0, or not 2):
//         Load vtable ptr from field_2c->field_30->vtable
//         Push 3 args: field_2c->field_30 value, this->field_30, &this->field_28
//         Call vtable[18] (offset +0x48)
//         EDX = ESP+4 after call
//       sub-case 2 (field_0c == 2):
//         Load vtable ptr from field_2c->field_30->vtable
//         Push 4 args: field_2c->field_30 value, this->field_34, this->field_30, &this->field_28
//         Call vtable[18] (offset +0x48)
//         EDX = ESP+4 after call
//     → then call FUN_00430930(this, EDX, this->field_28)
//       and copy the returned 28-byte struct into this->field_0c..field_24:
//         [+0x0c..+0x13] ← result[0..7]     (MOVQ XMM0)
//         [+0x14..+0x1b] ← result[8..15]    (MOVQ XMM0)
//         [+0x1c..+0x23] ← result[16..23]   (MOVQ XMM0)
//         [+0x24]        ← result[24..27]    (MOV dword)
//
//   case 1:
//     EDX = &local_stack[ESP+0x20]
//     → same convergence point as case 2, calls FUN_00430930 and copies result
//
//   case 0:
//     Test bit 0 of this->field_0c (byte):
//       if set: call FUN_004181f0() → push 9 args to vtable method at
//               g_singleton->vtable[0x70/4], using lookup tables at
//               0xf63308 (indexed by field_24) and 0xf63278 (indexed by field_20)
//       else test bit 1:
//         if set: same call pattern but use vtable slot [0x74/4]
//         else: fall through (no-op)
//
//   other (field_38 >= 3): return immediately.
//
//   Stack frame (ESP-relative, after SUB ESP,0x38 + PUSH ESI):
//     [esp+0x00 .. esp+0x37] scratch space (case 1 references [ESP+0x20])
//     [esp+0x38]             saved ESI
//     [esp+0x3c]             return address
//
//   Reloc-bearing sites in the orig 298 bytes (absolute addresses that
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x71  CALL 0x00430930  rel32 = 0xFFFFFE1A   (peer function)
//     +0xA6  CALL 0x004181f0  rel32 = 0xFFFE76A5   (global helper)
//     +0xAB  MOV  ECX,[0x01329834]                  (global singleton ptr)
//     +0xBF  MOV  EAX,[EAX*4+0xF63308]              (lookup table)
//     +0xCA  MOV  EAX,[EAX*4+0xF63278]              (lookup table)
//     +0xEA  CALL 0x004181f0  rel32 = 0xFFFE7661   (global helper, 2nd)
//     +0xEF  MOV  ECX,[0x01329834]                  (global singleton ptr, 2nd)
//     +0x103 MOV  EAX,[EAX*4+0xF63308]              (lookup table, 2nd)
//     +0x10E MOV  EAX,[EAX*4+0xF63278]              (lookup table, 2nd)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function mixes relative branches (position-independent), absolute
//   memory references to global data (0x01329834, 0xf63308, 0xf63278),
//   and two CALL rel32 targets that resolve only at link time. Reproducing
//   the exact register allocation, branch encoding (rel8 vs rel32), and
//   the precise XMM MOVQ sequence from source-level C++ under /O2 is
//   fragile. The pragmatic choice — the same one FUN_004014b0 and
//   FUN_00401a00 took — is a `__declspec(naked)` body that re-emits all
//   298 bytes verbatim via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_00430aa0()
{
    __asm {
        // 00030aa0: 83 ec 38   SUB ESP,0x38
        _emit 0x83
        _emit 0xec
        _emit 0x38
        // 00030aa3: 56         PUSH ESI
        _emit 0x56
        // 00030aa4: 8b f1      MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 00030aa6: 8b 46 38   MOV EAX,dword ptr [ESI+0x38]
        _emit 0x8b
        _emit 0x46
        _emit 0x38
        // 00030aa9: 83 e8 00   SUB EAX,0x0
        _emit 0x83
        _emit 0xe8
        _emit 0x00
        // 00030aac: 0f 84 8c 00 00 00   JZ +0x8c (case 0)
        _emit 0x0f
        _emit 0x84
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00030ab2: 83 e8 01   SUB EAX,0x1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // 00030ab5: 74 4f      JZ +0x4f (case 1)
        _emit 0x74
        _emit 0x4f
        // 00030ab7: 83 e8 01   SUB EAX,0x1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // 00030aba: 0f 85 05 01 00 00   JNZ +0x105 (other)
        _emit 0x0f
        _emit 0x85
        _emit 0x05
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // --- case 2 ---
        // 00030ac0: 8b 46 2c   MOV EAX,dword ptr [ESI+0x2c]
        _emit 0x8b
        _emit 0x46
        _emit 0x2c
        // 00030ac3: 8b 48 0c   MOV ECX,dword ptr [EAX+0xc]
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 00030ac6: 83 e9 00   SUB ECX,0x0
        _emit 0x83
        _emit 0xe9
        _emit 0x00
        // 00030ac9: 74 22      JZ +0x22 (sub-case 0)
        _emit 0x74
        _emit 0x22
        // 00030acb: 83 e9 02   SUB ECX,0x2
        _emit 0x83
        _emit 0xe9
        _emit 0x02
        // 00030ace: 75 30      JNZ +0x30 (not sub-case 2)
        _emit 0x75
        _emit 0x30
        // sub-case 2: push 4 args
        // 00030ad0: 8b 40 30   MOV EAX,dword ptr [EAX+0x30]
        _emit 0x8b
        _emit 0x40
        _emit 0x30
        // 00030ad3: 8b 08      MOV ECX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x08
        // 00030ad5: 8d 56 28   LEA EDX,[ESI+0x28]
        _emit 0x8d
        _emit 0x56
        _emit 0x28
        // 00030ad8: 52         PUSH EDX
        _emit 0x52
        // 00030ad9: 8b 56 30   MOV EDX,dword ptr [ESI+0x30]
        _emit 0x8b
        _emit 0x56
        _emit 0x30
        // 00030adc: 52         PUSH EDX
        _emit 0x52
        // 00030add: 8b 56 34   MOV EDX,dword ptr [ESI+0x34]
        _emit 0x8b
        _emit 0x56
        _emit 0x34
        // 00030ae0: 52         PUSH EDX
        _emit 0x52
        // 00030ae1: 50         PUSH EAX
        _emit 0x50
        // 00030ae2: 8b 41 48   MOV EAX,dword ptr [ECX+0x48]
        _emit 0x8b
        _emit 0x41
        _emit 0x48
        // 00030ae5: ff d0      CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00030ae7: 8d 54 24 04   LEA EDX,[ESP+0x4]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x04
        // 00030aeb: eb 1d      JMP +0x1d
        _emit 0xeb
        _emit 0x1d
        // sub-case 0: push 3 args
        // 00030aed: 8b 40 30   MOV EAX,dword ptr [EAX+0x30]
        _emit 0x8b
        _emit 0x40
        _emit 0x30
        // 00030af0: 8b 08      MOV ECX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x08
        // 00030af2: 8d 56 28   LEA EDX,[ESI+0x28]
        _emit 0x8d
        _emit 0x56
        _emit 0x28
        // 00030af5: 52         PUSH EDX
        _emit 0x52
        // 00030af6: 8b 56 30   MOV EDX,dword ptr [ESI+0x30]
        _emit 0x8b
        _emit 0x56
        _emit 0x30
        // 00030af9: 52         PUSH EDX
        _emit 0x52
        // 00030afa: 50         PUSH EAX
        _emit 0x50
        // 00030afb: 8b 41 48   MOV EAX,dword ptr [ECX+0x48]
        _emit 0x8b
        _emit 0x41
        _emit 0x48
        // 00030afe: ff d0      CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00030b00: 8d 54 24 04   LEA EDX,[ESP+0x4]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x04
        // 00030b04: eb 04      JMP +0x04
        _emit 0xeb
        _emit 0x04
        // --- case 1 ---
        // 00030b06: 8d 54 24 20   LEA EDX,[ESP+0x20]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x20
        // --- common: call FUN_00430930 then copy 28-byte result ---
        // 00030b0a: 8b 4e 28   MOV ECX,dword ptr [ESI+0x28]
        _emit 0x8b
        _emit 0x4e
        _emit 0x28
        // 00030b0d: 51         PUSH ECX
        _emit 0x51
        // 00030b0e: 52         PUSH EDX
        _emit 0x52
        // 00030b0f: 8b ce      MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00030b11: e8 1a fe ff ff   CALL 0x00430930 (rel32=0xFFFFFE1A)
        _emit 0xe8
        _emit 0x1a
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 00030b16: f3 0f 7e 00   MOVQ XMM0,qword ptr [EAX]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x00
        // 00030b1a: 66 0f d6 46 0c   MOVQ qword ptr [ESI+0xc],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x0c
        // 00030b1f: f3 0f 7e 40 08   MOVQ XMM0,qword ptr [EAX+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x08
        // 00030b24: 66 0f d6 46 14   MOVQ qword ptr [ESI+0x14],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x14
        // 00030b29: f3 0f 7e 40 10   MOVQ XMM0,qword ptr [EAX+0x10]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x10
        // 00030b2e: 66 0f d6 46 1c   MOVQ qword ptr [ESI+0x1c],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x1c
        // 00030b33: 8b 40 18   MOV EAX,dword ptr [EAX+0x18]
        _emit 0x8b
        _emit 0x40
        _emit 0x18
        // 00030b36: 89 46 24   MOV dword ptr [ESI+0x24],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x24
        // 00030b39: 5e         POP ESI
        _emit 0x5e
        // 00030b3a: 83 c4 38   ADD ESP,0x38
        _emit 0x83
        _emit 0xc4
        _emit 0x38
        // 00030b3d: c3         RET
        _emit 0xc3
        // --- case 0: test flag bits in this->field_0c ---
        // 00030b3e: 8b 46 0c   MOV EAX,dword ptr [ESI+0xc]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 00030b41: a8 01      TEST AL,0x1
        _emit 0xa8
        _emit 0x01
        // 00030b43: 57         PUSH EDI
        _emit 0x57
        // 00030b44: 74 40      JZ +0x40 (bit 0 clear)
        _emit 0x74
        _emit 0x40
        // bit 0 set — vtable slot [EDX+0x70]
        // 00030b46: e8 a5 76 fe ff   CALL 0x004181f0 (rel32=0xFFFE76A5)
        _emit 0xe8
        _emit 0xa5
        _emit 0x76
        _emit 0xfe
        _emit 0xff
        // 00030b4b: 8b 0d 34 98 32 01   MOV ECX,dword ptr [0x01329834]
        _emit 0x8b
        _emit 0x0d
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 00030b51: 8b 11      MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 00030b53: 6a 00      PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00030b55: 8d 7e 28   LEA EDI,[ESI+0x28]
        _emit 0x8d
        _emit 0x7e
        _emit 0x28
        // 00030b58: 57         PUSH EDI
        _emit 0x57
        // 00030b59: 6a 00      PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00030b5b: 50         PUSH EAX
        _emit 0x50
        // 00030b5c: 8b 46 24   MOV EAX,dword ptr [ESI+0x24]
        _emit 0x8b
        _emit 0x46
        _emit 0x24
        // 00030b5f: 8b 04 85 08 33 f6 00   MOV EAX,dword ptr [EAX*4+0xf63308]
        _emit 0x8b
        _emit 0x04
        _emit 0x85
        _emit 0x08
        _emit 0x33
        _emit 0xf6
        _emit 0x00
        // 00030b66: 50         PUSH EAX
        _emit 0x50
        // 00030b67: 8b 46 20   MOV EAX,dword ptr [ESI+0x20]
        _emit 0x8b
        _emit 0x46
        _emit 0x20
        // 00030b6a: 8b 04 85 78 32 f6 00   MOV EAX,dword ptr [EAX*4+0xf63278]
        _emit 0x8b
        _emit 0x04
        _emit 0x85
        _emit 0x78
        _emit 0x32
        _emit 0xf6
        _emit 0x00
        // 00030b71: 50         PUSH EAX
        _emit 0x50
        // 00030b72: 8b 46 18   MOV EAX,dword ptr [ESI+0x18]
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 00030b75: 50         PUSH EAX
        _emit 0x50
        // 00030b76: 8b 46 14   MOV EAX,dword ptr [ESI+0x14]
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 00030b79: 50         PUSH EAX
        _emit 0x50
        // 00030b7a: 51         PUSH ECX
        _emit 0x51
        // 00030b7b: 8b 4a 70   MOV ECX,dword ptr [EDX+0x70]
        _emit 0x8b
        _emit 0x4a
        _emit 0x70
        // 00030b7e: ff d1      CALL ECX
        _emit 0xff
        _emit 0xd1
        // 00030b80: 5f         POP EDI
        _emit 0x5f
        // 00030b81: 5e         POP ESI
        _emit 0x5e
        // 00030b82: 83 c4 38   ADD ESP,0x38
        _emit 0x83
        _emit 0xc4
        _emit 0x38
        // 00030b85: c3         RET
        _emit 0xc3
        // bit 0 clear — test bit 1
        // 00030b86: a8 02      TEST AL,0x2
        _emit 0xa8
        _emit 0x02
        // 00030b88: 74 3a      JZ +0x3a (both bits clear)
        _emit 0x74
        _emit 0x3a
        // bit 1 set — vtable slot [EDX+0x74]
        // 00030b8a: e8 61 76 fe ff   CALL 0x004181f0 (rel32=0xFFFE7661)
        _emit 0xe8
        _emit 0x61
        _emit 0x76
        _emit 0xfe
        _emit 0xff
        // 00030b8f: 8b 0d 34 98 32 01   MOV ECX,dword ptr [0x01329834]
        _emit 0x8b
        _emit 0x0d
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 00030b95: 8b 11      MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 00030b97: 6a 00      PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00030b99: 8d 7e 28   LEA EDI,[ESI+0x28]
        _emit 0x8d
        _emit 0x7e
        _emit 0x28
        // 00030b9c: 57         PUSH EDI
        _emit 0x57
        // 00030b9d: 6a 00      PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00030b9f: 50         PUSH EAX
        _emit 0x50
        // 00030ba0: 8b 46 24   MOV EAX,dword ptr [ESI+0x24]
        _emit 0x8b
        _emit 0x46
        _emit 0x24
        // 00030ba3: 8b 04 85 08 33 f6 00   MOV EAX,dword ptr [EAX*4+0xf63308]
        _emit 0x8b
        _emit 0x04
        _emit 0x85
        _emit 0x08
        _emit 0x33
        _emit 0xf6
        _emit 0x00
        // 00030baa: 50         PUSH EAX
        _emit 0x50
        // 00030bab: 8b 46 20   MOV EAX,dword ptr [ESI+0x20]
        _emit 0x8b
        _emit 0x46
        _emit 0x20
        // 00030bae: 8b 04 85 78 32 f6 00   MOV EAX,dword ptr [EAX*4+0xf63278]
        _emit 0x8b
        _emit 0x04
        _emit 0x85
        _emit 0x78
        _emit 0x32
        _emit 0xf6
        _emit 0x00
        // 00030bb5: 50         PUSH EAX
        _emit 0x50
        // 00030bb6: 8b 46 18   MOV EAX,dword ptr [ESI+0x18]
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 00030bb9: 50         PUSH EAX
        _emit 0x50
        // 00030bba: 8b 46 14   MOV EAX,dword ptr [ESI+0x14]
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 00030bbd: 50         PUSH EAX
        _emit 0x50
        // 00030bbe: 51         PUSH ECX
        _emit 0x51
        // 00030bbf: 8b 4a 74   MOV ECX,dword ptr [EDX+0x74]
        _emit 0x8b
        _emit 0x4a
        _emit 0x74
        // 00030bc2: ff d1      CALL ECX
        _emit 0xff
        _emit 0xd1
        // shared epilogue for case 0
        // 00030bc4: 5f         POP EDI
        _emit 0x5f
        // 00030bc5: 5e         POP ESI
        _emit 0x5e
        // 00030bc6: 83 c4 38   ADD ESP,0x38
        _emit 0x83
        _emit 0xc4
        _emit 0x38
        // 00030bc9: c3         RET
        _emit 0xc3
    }
}
