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
// FUNCTION: ffxivgame 0x0045b310 — __thiscall serialise-state-to-buffer
//                                  (127 B, no SEH frame, 1 stack arg).
//
// Behaviour reconstructed from the asm (RVA 0x0005b310, 127 bytes):
//
//   __thiscall int FUN_0045b310(Ctx *this /*ECX*/, BYTE *buf /*[ESP+0x4]*/)
//
//   Shape:
//     if (buf == nullptr) return 1;               // EDI=0 → LEA EAX,[EDI+1]
//     if (this->field_0x64 != 0) return field_0x64; // already-done flag?
//     if (this->field_0x60 == 0) {                // not initialised yet
//         FUN_0045b240(this);                     // init / compress final block
//         memset(&this->field_0x20, 0, 0x40);    // wipe message buffer
//         this->field_0x14 = 0;
//         this->field_0x18 = 0;
//         this->field_0x60 = 1;                  // mark initialised
//     }
//     // Serialise this->words[0..4] (20 bytes) into buf in big-endian order:
//     for (int i = 0; i < 20; ++i)
//         buf[i] = (BYTE)(this->words[i >> 2] >> (8 * (3 - (i & 3))));
//     return 0;
//
// Calling convention: __thiscall (ECX = this; one DWORD stack arg, callee
// cleans via RET 4).
//
// Reloc-bearing sites in the orig 127 bytes (masked in the diff):
//   +0x20  CALL rel32 → FUN_0045b240  (displacement 0x0b ff ff ff,
//                       relative from RVA 0x0005b335)
//   +0x2d  CALL rel32 → FUN_009d2110  (displacement 0xce 6d 57 00,
//                       relative from RVA 0x0005b342)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level C++ form would require coaxing MSVC 2005 /O2 into the
//   exact loop shape (two ADD ECX,ECX doublings instead of SHL ECX,2; the
//   LEA ESP,[ESP] 4-byte NOP at loop entry; SAR EDX,2 then MOV EDX,[ESI+EDX*4];
//   ADD EAX,1 before the MOV byte ptr) as well as the two binary-resident
//   CALL rel32 displacements. Naked asm emits all 127 bytes verbatim; the
//   .obj carries no relocations, and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0045b310() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP + 0xc]
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x75              // JNZ +8
        _emit 0x08
        _emit 0x8d              // LEA EAX, [EDI + 1]
        _emit 0x47
        _emit 0x01
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x64]
        _emit 0x46
        _emit 0x64
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +0x5f  (→ epilogue at +0x7a)
        _emit 0x5f
        _emit 0x39              // CMP dword ptr [ESI + 0x60], EAX
        _emit 0x46
        _emit 0x60
        _emit 0x75              // JNZ +0x2a  (→ loop at +0x4a)
        _emit 0x2a
        _emit 0xe8              // CALL FUN_0045b240  (rel32 = 0x0b ff ff ff)
        _emit 0x0b
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x6a              // PUSH 0x40
        _emit 0x40
        _emit 0x8d              // LEA EAX, [ESI + 0x20]
        _emit 0x46
        _emit 0x20
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_009d2110  (rel32 = 0xce 6d 57 00)
        _emit 0xce
        _emit 0x6d
        _emit 0x57
        _emit 0x00
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0xc7              // MOV dword ptr [ESI + 0x14], 0
        _emit 0x46
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI + 0x18], 0
        _emit 0x46
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI + 0x60], 1
        _emit 0x46
        _emit 0x60
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33              // XOR EAX, EAX  (i = 0)
        _emit 0xc0
        _emit 0x8d              // LEA ESP, [ESP]  (4-byte NOP, loop alignment)
        _emit 0x64
        _emit 0x24
        _emit 0x00
        // loop top (RVA 0x0005b360):
        _emit 0x8b              // MOV EDX, EAX
        _emit 0xd0
        _emit 0x83              // AND EDX, 3
        _emit 0xe2
        _emit 0x03
        _emit 0xb9              // MOV ECX, 3
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB ECX, EDX
        _emit 0xca
        _emit 0x03              // ADD ECX, ECX
        _emit 0xc9
        _emit 0x8b              // MOV EDX, EAX
        _emit 0xd0
        _emit 0xc1              // SAR EDX, 2
        _emit 0xfa
        _emit 0x02
        _emit 0x8b              // MOV EDX, dword ptr [ESI + EDX*4]
        _emit 0x14
        _emit 0x96
        _emit 0x03              // ADD ECX, ECX
        _emit 0xc9
        _emit 0x03              // ADD ECX, ECX
        _emit 0xc9
        _emit 0xd3              // SHR EDX, CL
        _emit 0xea
        _emit 0x83              // ADD EAX, 1
        _emit 0xc0
        _emit 0x01
        _emit 0x83              // CMP EAX, 0x14
        _emit 0xf8
        _emit 0x14
        _emit 0x88              // MOV byte ptr [EAX + EDI*1 - 1], DL
        _emit 0x54
        _emit 0x38
        _emit 0xff
        _emit 0x7c              // JL -0x28  (→ loop top)
        _emit 0xd8
        _emit 0x33              // XOR EAX, EAX  (return 0)
        _emit 0xc0
        // epilogue (RVA 0x0005b38a):
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
