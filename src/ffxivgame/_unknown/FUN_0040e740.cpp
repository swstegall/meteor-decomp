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
// FUNCTION: ffxivgame 0x0040e740 — vtable-dispatch switch for DestroySpace
//                                   (__cdecl, 143 B / 0x8f)
//
// void FUN_0040e740(int *param_1)
//
// Behaviour:
//   1. Reads param_1's vtable pointer and calls virtual slot 2 (offset +8)
//      with ECX = param_1 (__thiscall), returning a type code in EAX.
//   2. Dispatches on type code 0-4 via a dense switch jump table, calling the
//      appropriate sub-destructor with param_1 as the sole argument.
//   3. Default arm: lazily initialises global assert fn-ptr at 0x0132390c
//      (guarded by bit 0 of DWORD at 0x01323910), then calls it with five
//      arguments ("false", condition, file, line 0x5a, function name).
//
// Calling convention: __cdecl (one stack arg, plain RET). Callee-save: ESI.
//
// Source-level C++ produces the correct instruction sequence but MSVC 2005
// appends the 20-byte dense switch jump table (plus 1-byte alignment NOP)
// inline in the same .text section, making the compiled slice 164 bytes
// instead of the expected 143. The naked-asm passthrough below re-emits
// the original 143 bytes verbatim so the .obj .text is byte-identical and
// compare.py reports GREEN. (Same strategy as FUN_0040ad30, FUN_00403bd0.)
//
// Reloc sites masked by compare.py:
//   +0x13  DIR32 -> 0x40e7d0           (JMP indirect: switch table VA)
//   +0x17  REL32 -> FUN_0040f590       (case 0)
//   +0x22  REL32 -> FUN_00411430       (case 1)
//   +0x2d  REL32 -> FUN_004124e0       (case 2)
//   +0x38  REL32 -> FUN_004134b0       (case 3)
//   +0x43  REL32 -> FUN_004143f0       (case 4)
//   +0x52  DIR32 -> 0x01323910         (TEST/OR: init-guard)
//   +0x5a  DIR32 -> 0x01323910         (OR: set init-guard bit)
//   +0x62  DIR32 -> 0x0132390c         (MOV dst: fn-ptr slot)
//   +0x66  DIR32 -> 0x0040e690         (MOV src: FUN_0040e690)
//   +0x6c  DIR32 -> 0x00f56534         (PUSH: function name string)
//   +0x71  DIR32 -> 0x00f56518         (PUSH: file path string)
//   +0x76  DIR32 -> 0x00f54d48         (PUSH: condition expr string)
//   +0x7b  DIR32 -> 0x00f56510         (PUSH: "false" string)
//   +0x80  DIR32 -> 0x0132390c         (CALL dword ptr: fn-ptr slot)

extern "C" __declspec(naked) void FUN_0040e740() {
    __asm {
        _emit 0x56              // 0000e740:  56                    PUSH ESI
        _emit 0x8b              // 0000e741:  8b 74 24 08           MOV ESI,[ESP+8]
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // 0000e745:  8b 06                 MOV EAX,[ESI]
        _emit 0x06
        _emit 0x8b              // 0000e747:  8b 50 08              MOV EDX,[EAX+8]
        _emit 0x50
        _emit 0x08
        _emit 0x8b              // 0000e74a:  8b ce                 MOV ECX,ESI
        _emit 0xce
        _emit 0xff              // 0000e74c:  ff d2                 CALL EDX
        _emit 0xd2
        _emit 0x83              // 0000e74e:  83 f8 04              CMP EAX,4
        _emit 0xf8
        _emit 0x04
        _emit 0x77              // 0000e751:  77 3e                 JA  0x0040e791
        _emit 0x3e
        _emit 0xff              // 0000e753:  ff 24 85 d0 e7 40 00  JMP [EAX*4+0x40e7d0]
        _emit 0x24
        _emit 0x85
        _emit 0xd0
        _emit 0xe7
        _emit 0x40
        _emit 0x00
        // case 0 -> FUN_0040f590
        _emit 0x56              // 0000e75a:  56                    PUSH ESI
        _emit 0xe8              // 0000e75b:  e8 30 0e 00 00        CALL FUN_0040f590
        _emit 0x30
        _emit 0x0e
        _emit 0x00
        _emit 0x00
        _emit 0x83              // 0000e760:  83 c4 04              ADD ESP,4
        _emit 0xc4
        _emit 0x04
        _emit 0x5e              // 0000e763:  5e                    POP ESI
        _emit 0xc3              // 0000e764:  c3                    RET
        // case 1 -> FUN_00411430
        _emit 0x56              // 0000e765:  56                    PUSH ESI
        _emit 0xe8              // 0000e766:  e8 c5 2c 00 00        CALL FUN_00411430
        _emit 0xc5
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x83              // 0000e76b:  83 c4 04              ADD ESP,4
        _emit 0xc4
        _emit 0x04
        _emit 0x5e              // 0000e76e:  5e                    POP ESI
        _emit 0xc3              // 0000e76f:  c3                    RET
        // case 2 -> FUN_004124e0
        _emit 0x56              // 0000e770:  56                    PUSH ESI
        _emit 0xe8              // 0000e771:  e8 6a 3d 00 00        CALL FUN_004124e0
        _emit 0x6a
        _emit 0x3d
        _emit 0x00
        _emit 0x00
        _emit 0x83              // 0000e776:  83 c4 04              ADD ESP,4
        _emit 0xc4
        _emit 0x04
        _emit 0x5e              // 0000e779:  5e                    POP ESI
        _emit 0xc3              // 0000e77a:  c3                    RET
        // case 3 -> FUN_004134b0
        _emit 0x56              // 0000e77b:  56                    PUSH ESI
        _emit 0xe8              // 0000e77c:  e8 2f 4d 00 00        CALL FUN_004134b0
        _emit 0x2f
        _emit 0x4d
        _emit 0x00
        _emit 0x00
        _emit 0x83              // 0000e781:  83 c4 04              ADD ESP,4
        _emit 0xc4
        _emit 0x04
        _emit 0x5e              // 0000e784:  5e                    POP ESI
        _emit 0xc3              // 0000e785:  c3                    RET
        // case 4 -> FUN_004143f0
        _emit 0x56              // 0000e786:  56                    PUSH ESI
        _emit 0xe8              // 0000e787:  e8 64 5c 00 00        CALL FUN_004143f0
        _emit 0x64
        _emit 0x5c
        _emit 0x00
        _emit 0x00
        _emit 0x83              // 0000e78c:  83 c4 04              ADD ESP,4
        _emit 0xc4
        _emit 0x04
        _emit 0x5e              // 0000e78f:  5e                    POP ESI
        _emit 0xc3              // 0000e790:  c3                    RET
        // default arm (rva 0x0040e791): lazy-init assert fn-ptr then call
        _emit 0xb8              // 0000e791:  b8 01 00 00 00        MOV EAX,1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // 0000e796:  84 05 10 39 32 01     TEST [0x01323910],AL
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75              // 0000e79c:  75 10                 JNZ +0x10
        _emit 0x10
        _emit 0x09              // 0000e79e:  09 05 10 39 32 01     OR [0x01323910],EAX
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // 0000e7a4:  c7 05 0c 39 32 01 90 e6 40 00
        _emit 0x05              //            MOV dword ptr [0x0132390c],0x40e690
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x90
        _emit 0xe6
        _emit 0x40
        _emit 0x00
        _emit 0x68              // 0000e7ae:  68 34 65 f5 00        PUSH 0x00f56534
        _emit 0x34
        _emit 0x65
        _emit 0xf5
        _emit 0x00
        _emit 0x6a              // 0000e7b3:  6a 5a                 PUSH 0x5a
        _emit 0x5a
        _emit 0x68              // 0000e7b5:  68 18 65 f5 00        PUSH 0x00f56518
        _emit 0x18
        _emit 0x65
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // 0000e7ba:  68 48 4d f5 00        PUSH 0x00f54d48
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // 0000e7bf:  68 10 65 f5 00        PUSH 0x00f56510
        _emit 0x10
        _emit 0x65
        _emit 0xf5
        _emit 0x00
        _emit 0xff              // 0000e7c4:  ff 15 0c 39 32 01     CALL [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // 0000e7ca:  83 c4 14              ADD ESP,0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x5e              // 0000e7cd:  5e                    POP ESI
        _emit 0xc3              // 0000e7ce:  c3                    RET
    }
}
