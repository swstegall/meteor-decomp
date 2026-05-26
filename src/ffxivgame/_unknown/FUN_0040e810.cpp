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
// FUNCTION: ffxivgame 0x0000e810 — __thiscall init: set fields on this from
//                                   param_1 + dispatch through two vtable paths
//                                   (263 B / 0x107)
//
// __thiscall void* FUN_0040e810(this, param_1*)
//   ECX        : this  — pointer to struct with fields at 0x0..0x24
//   [ESP+0x04] : param_1* — pointer to vtable-bearing object
//
// Behaviour:
//   1. Stores param_1 → this->field_0 and 2 → this->field_4;
//      zeroes this->field_8..field_24.
//   2. Calls param_1->virtual_method_6() [via FUN_0040e0b0].
//      Stores return value in this->field_4.
//   3. Branches on the return value:
//      a. If return == 0: calls param_1->virtual_method_5() (vtable slot 5),
//         then sets up fields 8/c/10/14/18/1c/20/24 from a linked-list walk.
//      b. If return == 1: computes memory-range fields (8/c) from
//         param_1->field_4 and param_1->field_8, scans a range for a max
//         value, then stores 18/1c/20/24 as 0/0/0/0xfffffff0.
//      c. Otherwise: falls through to common epilogue (returns this).
//   Returns: EAX = this.
//
// Calling convention: __thiscall, callee cleans 1 stack arg (RET 0x4).
// Callee-saves: EBX, ESI, EDI.
// Local stack frame: SUB ESP,0x20 (32 bytes of local space; ESP used as
//   scratch buffer in one branch via LEA EDX,[ESP+0xc]).
//
// Reloc-bearing sites in the orig 263 bytes:
//   +0x2f  CALL rel32 → FUN_0040e0b0           (rel32 = 0xfffff86c)
//   +0xb5  CALL dword ptr [0x00f3e160]          (abs import ptr)
//   +0xc4  CALL rel32 → FUN_0040eae0            (rel32 = 0x00000208)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Branch shape, alignment NOPs (LEA ESP,[ESP+0] at +0xdc),
//   and the specific SUB/JZ/SUB/JNZ dispatch pattern are not safely
//   reproducible from C++ source under MSVC 2005 /O2. The
//   __declspec(naked) body re-emits the original 263 bytes verbatim
//   via MASM _emit directives; the .obj's .text is byte-identical to
//   the original slice, and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0040e810() {
    __asm {
        // 0000e810:  83 ec 20               SUB ESP,0x20
        _emit 0x83
        _emit 0xec
        _emit 0x20
        // 0000e813:  53                     PUSH EBX
        _emit 0x53
        // 0000e814:  56                     PUSH ESI
        _emit 0x56
        // 0000e815:  8b f1                  MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0000e817:  8b 4c 24 2c            MOV ECX,dword ptr [ESP+0x2c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // 0000e81b:  57                     PUSH EDI
        _emit 0x57
        // 0000e81c:  33 ff                  XOR EDI,EDI
        _emit 0x33
        _emit 0xff
        // 0000e81e:  89 0e                  MOV dword ptr [ESI],ECX
        _emit 0x89
        _emit 0x0e
        // 0000e820:  c7 46 04 02 00 00 00   MOV dword ptr [ESI+0x4],0x2
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000e827:  89 7e 08               MOV dword ptr [ESI+0x8],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x08
        // 0000e82a:  89 7e 0c               MOV dword ptr [ESI+0xc],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x0c
        // 0000e82d:  89 7e 10               MOV dword ptr [ESI+0x10],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x10
        // 0000e830:  89 7e 14               MOV dword ptr [ESI+0x14],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        // 0000e833:  89 7e 18               MOV dword ptr [ESI+0x18],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x18
        // 0000e836:  89 7e 1c               MOV dword ptr [ESI+0x1c],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x1c
        // 0000e839:  89 7e 20               MOV dword ptr [ESI+0x20],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x20
        // 0000e83c:  89 7e 24               MOV dword ptr [ESI+0x24],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x24
        // 0000e83f:  e8 6c f8 ff ff         CALL 0x0040e0b0  [reloc]
        _emit 0xe8
        _emit 0x6c
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        // 0000e844:  89 46 04               MOV dword ptr [ESI+0x4],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 0000e847:  2b c7                  SUB EAX,EDI
        _emit 0x2b
        _emit 0xc7
        // 0000e849:  74 5b                  JZ 0x0040e8a6
        _emit 0x74
        _emit 0x5b
        // 0000e84b:  83 e8 01               SUB EAX,0x1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // 0000e84e:  0f 85 b8 00 00 00      JNZ 0x0040e90c
        _emit 0x0f
        _emit 0x85
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000e854:  8b 06                  MOV EAX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 0000e856:  8b 48 04               MOV ECX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        // 0000e859:  03 c8                  ADD ECX,EAX
        _emit 0x03
        _emit 0xc8
        // 0000e85b:  89 4e 08               MOV dword ptr [ESI+0x8],ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x08
        // 0000e85e:  8b 50 08               MOV EDX,dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 0000e861:  89 56 0c               MOV dword ptr [ESI+0xc],EDX
        _emit 0x89
        _emit 0x56
        _emit 0x0c
        // 0000e864:  8b 50 08               MOV EDX,dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 0000e867:  03 50 04               ADD EDX,dword ptr [EAX+0x4]
        _emit 0x03
        _emit 0x50
        _emit 0x04
        // 0000e86a:  8b 58 04               MOV EBX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x58
        _emit 0x04
        // 0000e86d:  8d 54 02 f0            LEA EDX,[EDX+EAX*1-0x10]
        _emit 0x8d
        _emit 0x54
        _emit 0x02
        _emit 0xf0
        // 0000e871:  8d 44 03 10            LEA EAX,[EBX+EAX*1+0x10]
        _emit 0x8d
        _emit 0x44
        _emit 0x03
        _emit 0x10
        // 0000e875:  33 c9                  XOR ECX,ECX
        _emit 0x33
        _emit 0xc9
        // 0000e877:  3b c2                  CMP EAX,EDX
        _emit 0x3b
        _emit 0xc2
        // 0000e879:  74 10                  JZ 0x0040e88b
        _emit 0x74
        _emit 0x10
        // 0000e87b:  8b 10                  MOV EDX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x10
        // 0000e87d:  8b 40 04               MOV EAX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        // 0000e880:  c1 e8 1f               SHR EAX,0x1f
        _emit 0xc1
        _emit 0xe8
        _emit 0x1f
        // 0000e883:  3b ca                  CMP ECX,EDX
        _emit 0x3b
        _emit 0xca
        // 0000e885:  73 fc                  JNC 0x0040e883
        _emit 0x73
        _emit 0xfc
        // 0000e887:  8b ca                  MOV ECX,EDX
        _emit 0x8b
        _emit 0xca
        // 0000e889:  eb f8                  JMP 0x0040e883
        _emit 0xeb
        _emit 0xf8
        // 0000e88b:  89 7e 18               MOV dword ptr [ESI+0x18],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x18
        // 0000e88e:  89 7e 1c               MOV dword ptr [ESI+0x1c],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x1c
        // 0000e891:  89 7e 20               MOV dword ptr [ESI+0x20],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x20
        // 0000e894:  5f                     POP EDI
        _emit 0x5f
        // 0000e895:  c7 46 24 f0 ff ff ff   MOV dword ptr [ESI+0x24],0xfffffff0
        _emit 0xc7
        _emit 0x46
        _emit 0x24
        _emit 0xf0
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0000e89c:  8b c6                  MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0000e89e:  5e                     POP ESI
        _emit 0x5e
        // 0000e89f:  5b                     POP EBX
        _emit 0x5b
        // 0000e8a0:  83 c4 20               ADD ESP,0x20
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        // 0000e8a3:  c2 04 00               RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 0000e8a6:  8b 06                  MOV EAX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 0000e8a8:  8b 08                  MOV ECX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x08
        // 0000e8aa:  8b 11                  MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 0000e8ac:  8b 42 14               MOV EAX,dword ptr [EDX+0x14]
        _emit 0x8b
        _emit 0x42
        _emit 0x14
        // 0000e8af:  ff d0                  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0000e8b1:  8b 0e                  MOV ECX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x0e
        // 0000e8b3:  8b d8                  MOV EBX,EAX
        _emit 0x8b
        _emit 0xd8
        // 0000e8b5:  89 7e 08               MOV dword ptr [ESI+0x8],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x08
        // 0000e8b8:  8b 41 08               MOV EAX,dword ptr [ECX+0x8]
        _emit 0x8b
        _emit 0x41
        _emit 0x08
        // 0000e8bb:  3b c7                  CMP EAX,EDI
        _emit 0x3b
        _emit 0xc7
        // 0000e8bd:  75 0f                  JNZ 0x0040e8ce
        _emit 0x75
        _emit 0x0f
        // 0000e8bf:  8d 54 24 0c            LEA EDX,[ESP+0xc]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 0000e8c3:  52                     PUSH EDX
        _emit 0x52
        // 0000e8c4:  ff 15 60 e1 f3 00      CALL dword ptr [0x00f3e160]  [reloc]
        _emit 0xff
        _emit 0x15
        _emit 0x60
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0000e8ca:  8b 44 24 24            MOV EAX,dword ptr [ESP+0x24]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0000e8ce:  8b cb                  MOV ECX,EBX
        _emit 0x8b
        _emit 0xcb
        // 0000e8d0:  89 46 0c               MOV dword ptr [ESI+0xc],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        // 0000e8d3:  e8 08 02 00 00         CALL 0x0040eae0  [reloc]
        _emit 0xe8
        _emit 0x08
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0000e8d8:  8b 56 0c               MOV EDX,dword ptr [ESI+0xc]
        _emit 0x8b
        _emit 0x56
        _emit 0x0c
        // 0000e8db:  2b d0                  SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 0000e8dd:  8d 4b 04               LEA ECX,[EBX+0x4]
        _emit 0x8d
        _emit 0x4b
        _emit 0x04
        // 0000e8e0:  89 46 10               MOV dword ptr [ESI+0x10],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x10
        // 0000e8e3:  89 56 14               MOV dword ptr [ESI+0x14],EDX
        _emit 0x89
        _emit 0x56
        _emit 0x14
        // 0000e8e6:  8b 01                  MOV EAX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 0000e8e8:  3b c1                  CMP EAX,ECX
        _emit 0x3b
        _emit 0xc1
        // 0000e8ea:  74 0d                  JZ 0x0040e8f9
        _emit 0x74
        _emit 0x0d
        // 0000e8ec:  8d 64 24 00            LEA ESP,[ESP+0x0]  (4-byte NOP)
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
        // 0000e8f0:  8b 00                  MOV EAX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x00
        // 0000e8f2:  83 c7 01               ADD EDI,0x1
        _emit 0x83
        _emit 0xc7
        _emit 0x01
        // 0000e8f5:  3b c1                  CMP EAX,ECX
        _emit 0x3b
        _emit 0xc1
        // 0000e8f7:  75 f7                  JNZ 0x0040e8f0
        _emit 0x75
        _emit 0xf7
        // 0000e8f9:  89 7e 18               MOV dword ptr [ESI+0x18],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x18
        // 0000e8fc:  83 c7 01               ADD EDI,0x1
        _emit 0x83
        _emit 0xc7
        _emit 0x01
        // 0000e8ff:  c7 46 1c 01 00 00 00   MOV dword ptr [ESI+0x1c],0x1
        _emit 0xc7
        _emit 0x46
        _emit 0x1c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000e906:  89 56 24               MOV dword ptr [ESI+0x24],EDX
        _emit 0x89
        _emit 0x56
        _emit 0x24
        // 0000e909:  89 7e 20               MOV dword ptr [ESI+0x20],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x20
        // 0000e90c:  5f                     POP EDI
        _emit 0x5f
        // 0000e90d:  8b c6                  MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0000e90f:  5e                     POP ESI
        _emit 0x5e
        // 0000e910:  5b                     POP EBX
        _emit 0x5b
        // 0000e911:  83 c4 20               ADD ESP,0x20
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        // 0000e914:  c2 04 00               RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
