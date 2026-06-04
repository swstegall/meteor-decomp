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
// FUNCTION: ffxivgame 0x00434650 — __thiscall accessor that validates a
//                                  container index, drives a callee, then
//                                  forwards through a vtable slot. 149 B / 0x95,
//                                  ret 4.
//
// __thiscall int FUN_00434650(this):
//   ECX = this. One implicit stack arg is consumed by the embedded
//   callback (`RET 4`).
//
//   Flow summary (read from the disassembly at orig RVA 0x00034650):
//     1. Compute element count from a pointer pair: if this->begin
//        (+0x1c) != 0, count = (this->end(+0x20) - begin) / 5  via the
//        0x66666667 magic-number signed divide-by-5 idiom; else 0.
//     2. if (count > 0) {  // unsigned (JBE)
//            one-time init of the assert-handler function pointer at
//            [0x0132390c] (guarded by the bit-flag at [0x01323910]),
//            then push 5 args and call it (assert/report site, line 0x1c2).
//        }
//     3. Call FUN_0043bf50 (__thiscall, ECX = this->field_0xc).
//     4. Load an object from the incoming stack arg, dispatch through its
//        vtable slot +0xc (5 zero args + the object) via CALL [EDX+0xc].
//     5. Push that result and forward through this->vtable(+0)[+0xa0]
//        (__thiscall, ECX = this), then return this->field_0x2c.
//
//   Reloc-bearing sites resolve only in a full-binary relink at image
//   base 0x00400000; here the moffs32 / imm32 globals are the literal
//   preferred-base values and the single rel32 CALL (FUN_0043bf50) is
//   the literal linker-computed displacement, so the standalone .obj's
//   .text bytes match orig exactly.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The one-time guarded function-pointer init + indirect assert-call
//   pattern (shared with sibling FUN_009fe782, which touches the very
//   same [0x01323910]/[0x0132390c] globals), the signed magic divide,
//   and the mixed direct/indirect/vtable call sequence are MSVC 2005
//   /O2 artefacts that source-level C++ will not reliably reproduce.
//   Per the ffxivgame rosetta convention, emit the verified 0x95 bytes
//   directly.

extern "C" __declspec(naked) void FUN_00434650() {
    __asm {
        _emit 0x56  // PUSH ESI
        _emit 0x8b  // MOV ESI,ECX
        _emit 0xf1
        _emit 0x8b  // MOV EAX,dword ptr [ESI+0x1c]
        _emit 0x46
        _emit 0x1c
        _emit 0x85  // TEST EAX,EAX
        _emit 0xc0
        _emit 0x74  // JZ +0x16
        _emit 0x16
        _emit 0x8b  // MOV ECX,dword ptr [ESI+0x20]
        _emit 0x4e
        _emit 0x20
        _emit 0x2b  // SUB ECX,EAX
        _emit 0xc8
        _emit 0xb8  // MOV EAX,0x66666667
        _emit 0x67
        _emit 0x66
        _emit 0x66
        _emit 0x66
        _emit 0xf7  // IMUL ECX
        _emit 0xe9
        _emit 0xc1  // SAR EDX,0x3
        _emit 0xfa
        _emit 0x03
        _emit 0x8b  // MOV EAX,EDX
        _emit 0xc2
        _emit 0xc1  // SHR EAX,0x1f
        _emit 0xe8
        _emit 0x1f
        _emit 0x03  // ADD EAX,EDX
        _emit 0xc2
        _emit 0x85  // TEST EAX,EAX
        _emit 0xc0
        _emit 0x76  // JBE +0x3f
        _emit 0x3f
        _emit 0xb8  // MOV EAX,0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84  // TEST byte ptr [0x01323910],AL
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75  // JNZ +0x10
        _emit 0x10
        _emit 0x09  // OR dword ptr [0x01323910],EAX
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7  // MOV dword ptr [0x0132390c],0x433720
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00
        _emit 0x68  // PUSH 0xf64778
        _emit 0x78
        _emit 0x47
        _emit 0xf6
        _emit 0x00
        _emit 0x68  // PUSH 0x1c2
        _emit 0xc2
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68  // PUSH 0xf647d8
        _emit 0xd8
        _emit 0x47
        _emit 0xf6
        _emit 0x00
        _emit 0x68  // PUSH 0xf643af
        _emit 0xaf
        _emit 0x43
        _emit 0xf6
        _emit 0x00
        _emit 0x68  // PUSH 0xf64840
        _emit 0x40
        _emit 0x48
        _emit 0xf6
        _emit 0x00
        _emit 0xff  // CALL dword ptr [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83  // ADD ESP,0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x8b  // MOV ECX,dword ptr [ESI+0xc]
        _emit 0x4e
        _emit 0x0c
        _emit 0xe8  // CALL 0x0043bf50
        _emit 0x95
        _emit 0x78
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EAX,dword ptr [ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b  // MOV ECX,dword ptr [EAX]
        _emit 0x08
        _emit 0x8b  // MOV EDX,dword ptr [ECX+0xc]
        _emit 0x51
        _emit 0x0c
        _emit 0x6a  // PUSH 0x0
        _emit 0x00
        _emit 0x6a  // PUSH 0x0
        _emit 0x00
        _emit 0x6a  // PUSH 0x0
        _emit 0x00
        _emit 0x6a  // PUSH 0x0
        _emit 0x00
        _emit 0x6a  // PUSH 0x0
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0xff  // CALL EDX
        _emit 0xd2
        _emit 0x8b  // MOV EDX,dword ptr [ESI]
        _emit 0x16
        _emit 0x50  // PUSH EAX
        _emit 0x8b  // MOV EAX,dword ptr [EDX+0xa0]
        _emit 0x82
        _emit 0xa0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV ECX,ESI
        _emit 0xce
        _emit 0xff  // CALL EAX
        _emit 0xd0
        _emit 0x8b  // MOV EAX,dword ptr [ESI+0x2c]
        _emit 0x46
        _emit 0x2c
        _emit 0x5e  // POP ESI
        _emit 0xc2  // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
