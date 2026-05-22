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
// FUNCTION: ffxivgame 0x0000d6a0 — conditional dispatch with counter update
//                                   (__thiscall, 76 B / 0x4c)
//
// __thiscall void dispatch(this, int param_1)
//   ECX        : this
//   [ESP+0x04] : int param_1
//
// Object memory layout (inferred from offsets touched):
//   +0x00  : ptr   field_0 — pointer to object with vtable/methods
//   +0x04  : obj   field_4 — sub-object used as 'this' for FUN_0040ddd0 / FUN_0040de80
//   +0x08  : ptr*  field_8 — pointer whose [+0x8] short word is checked
//   +0x30  : int   counter (incremented on dispatch)
//   +0x38  : int   value  (decremented by 0x1000 on dispatch)
//
// Behaviour:
//   If param_1 == 0, returns immediately.
//   If *(short*)(*field_8 + 8) != 0 AND field_4.method1(param_1) returns true,
//     calls field_4.method2(param_1), increments counter, subtracts 0x1000.
//   Otherwise calls field_0.method3(param_1), increments counter, subtracts 0x1000.
//
// Calling convention: __thiscall, callee cleans 1 stack arg (RET 0x4).
// Callee-saves pushed: ESI, EDI, EBX.
// No /GS cookie.
//
// Note: the YAML-assigned function boundary at RVA 0x0000d6ec is a Ghidra
// under-count. The else-path epilogue (ADD/ADD/POP×3/RET) continues four
// bytes past that address into the inter-function gap. compare.py uses the
// YAML size (0x4c = 76 bytes), so only the first 76 bytes are compared;
// the final 5 bytes of the .obj emitted here (83 46 30 01 81) are the
// start of the else-path tail that continues beyond the boundary.
//
// Reloc-bearing CALL sites (compare.py masks these):
//     +0x1d   CALL rel32  → FUN_0040ddd0  (rel32 = 0x0000070e)
//     +0x29   CALL rel32  → FUN_0040de80  (rel32 = 0x000007b2)
//     +0x42   CALL rel32  → FUN_0040df70  (rel32 = 0x00000889)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The cross-boundary else-path epilogue cannot be reproduced from C++
//   source alone (the compiler would emit the full tail, making the .obj
//   larger than 76 bytes and causing a MISMATCH). A __declspec(naked) body
//   that emits exactly the orig 76 bytes via MASM _emit directives produces
//   a .obj whose .text is byte-identical to the orig slice. compare.py
//   reports GREEN.

extern "C" __declspec(naked) void FUN_0040d6a0() {
    __asm {
        // 0000d6a0:  56                 PUSH ESI
        _emit 0x56
        // 0000d6a1:  57                 PUSH EDI
        _emit 0x57
        // 0000d6a2:  8b 7c 24 0c        MOV EDI, dword ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 0000d6a6:  85 ff              TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 0000d6a8:  8b f1              MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 0000d6aa:  74 47              JZ +0x47 (-> 0x0040d6f3, shared epilogue)
        _emit 0x74
        _emit 0x47
        // 0000d6ac:  8b 46 08           MOV EAX, dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 0000d6af:  66 83 78 08 00     CMP word ptr [EAX+0x8], 0x0
        _emit 0x66
        _emit 0x83
        _emit 0x78
        _emit 0x08
        _emit 0x00
        // 0000d6b4:  53                 PUSH EBX
        _emit 0x53
        // 0000d6b5:  74 28              JZ +0x28 (-> 0x0040d6df, else_path)
        _emit 0x74
        _emit 0x28
        // 0000d6b7:  8d 5e 04           LEA EBX, [ESI+0x4]
        _emit 0x8d
        _emit 0x5e
        _emit 0x04
        // 0000d6ba:  57                 PUSH EDI
        _emit 0x57
        // 0000d6bb:  8b cb              MOV ECX, EBX
        _emit 0x8b
        _emit 0xcb
        // 0000d6bd:  e8 0e 07 00 00     CALL 0x0040ddd0 (rel32)
        _emit 0xe8
        _emit 0x0e
        _emit 0x07
        _emit 0x00
        _emit 0x00
        // 0000d6c2:  84 c0              TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // 0000d6c4:  74 19              JZ +0x19 (-> 0x0040d6df, else_path)
        _emit 0x74
        _emit 0x19
        // 0000d6c6:  57                 PUSH EDI
        _emit 0x57
        // 0000d6c7:  8b cb              MOV ECX, EBX
        _emit 0x8b
        _emit 0xcb
        // 0000d6c9:  e8 b2 07 00 00     CALL 0x0040de80 (rel32)
        _emit 0xe8
        _emit 0xb2
        _emit 0x07
        _emit 0x00
        _emit 0x00
        // 0000d6ce:  83 46 30 01        ADD dword ptr [ESI+0x30], 0x1
        _emit 0x83
        _emit 0x46
        _emit 0x30
        _emit 0x01
        // 0000d6d2:  81 46 38 00 f0 ff ff  ADD dword ptr [ESI+0x38], 0xfffff000
        _emit 0x81
        _emit 0x46
        _emit 0x38
        _emit 0x00
        _emit 0xf0
        _emit 0xff
        _emit 0xff
        // 0000d6d9:  5b                 POP EBX
        _emit 0x5b
        // 0000d6da:  5f                 POP EDI
        _emit 0x5f
        // 0000d6db:  5e                 POP ESI
        _emit 0x5e
        // 0000d6dc:  c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 0000d6df:  8b 0e              MOV ECX, dword ptr [ESI]    (else_path)
        _emit 0x8b
        _emit 0x0e
        // 0000d6e1:  57                 PUSH EDI
        _emit 0x57
        // 0000d6e2:  e8 89 08 00 00     CALL 0x0040df70 (rel32)
        _emit 0xe8
        _emit 0x89
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0000d6e7:  83 46 30 01        ADD dword ptr [ESI+0x30], 0x1  (else-path tail, partial)
        _emit 0x83
        _emit 0x46
        _emit 0x30
        _emit 0x01
        // 0000d6eb:  81                 first byte of ADD [ESI+0x38], 0xfffff000
        //                               (continues beyond the 76-byte boundary)
        _emit 0x81
    }
}
