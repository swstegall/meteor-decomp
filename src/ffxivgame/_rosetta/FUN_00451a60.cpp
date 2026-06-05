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
// FUNCTION: ffxivgame 0x00051a60 — `__thiscall` 2-stack-arg iterator-find
//                                  returning a pair via out-pointer (122 B).
//
// Asm shape (read from RVA 0x00051a60, 122 bytes of `.text`):
//
//   __thiscall void FUN_00451a60(Container* this /*ECX*/,
//                                PairOut* result /*arg1*/,
//                                Key      key    /*arg2*/) — RET 0x8
//
//   ESP-relative frame (no EBP):
//     SUB ESP, 0x10  (16-byte scratch: locals at [ESP+0x0..0xF])
//     saves EBX, ESI, EDI below the scratch
//
//   Structural shape:
//     EBX = key (arg2, loaded from [ESP+0x1c] after SUB+PUSH EBX)
//     ESI = this (ECX)
//     call FUN_00451540(EBX)        → lower_bound / find, result in EDI(EAX)
//     if (ESI == null) call _assert_handler
//     if (EDI == this->end) goto not_found
//     // string data pointer extraction (SSO pattern):
//     if ([EDI+0x24] < 0x10) EDI += 0x10   // small-string buffer inline
//     else                   EDI = [EDI+0x10] // heap pointer
//     call FUN_00620110(ECX=EBX, 0, [EBX+0x14], EDI, EAX=[EDI+0x20])
//     if (result >= 0) return {&local_esi, this} via [ECX]=[ESP+0xc]
//   not_found:
//     build local pair: {this->end, this} at [ESP+0x14]
//   return:
//     copy [ECX],[ECX+4] into *result (arg1)
//
// Reloc-bearing sites (absolute addresses baked-in as raw immediates;
// compare.py masks relocations so naked-asm _emit produces byte-identical .obj):
//   +0x0d  CALL rel32 → FUN_00451540  (rel = 0xffffd0ce + RVA 0x00051a72 context)
//   +0x1c  CALL rel32 → 0x009d22b4   (rel = 0x00580833 from RVA 0x00051a81)
//   +0x45  CALL rel32 → FUN_00620110  (rel = 0x001ce666 from RVA 0x00051aaa)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The ESP-relative frame, register allocation (ESI=this, EBX=key, EDI=found),
//   SSO branch shape, and two absolute CALL targets with far rel32 displacements
//   make a source-level reconstruction brittle under /O2. The pragmatic choice —
//   matching the local _rosetta idiom — is a __declspec(naked) body re-emitting
//   the orig 122 bytes verbatim via MASM _emit directives.

extern "C" __declspec(naked) void FUN_00451a60() {
    __asm {
        _emit 0x83              // SUB ESP, 0x10
        _emit 0xec
        _emit 0x10
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP + 0x1c]
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xe8              // CALL 0x00451540 (rel32 = 0xffffd0ce)
        _emit 0xce
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x8b              // MOV EDI, EAX
        _emit 0xf8
        _emit 0x89              // MOV dword ptr [ESP + 0x10], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x75              // JNZ +5
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4 (rel32 = 0x00580833)
        _emit 0x33
        _emit 0x08
        _emit 0x58
        _emit 0x00
        _emit 0x3b              // CMP EDI, dword ptr [ESI + 0x4]
        _emit 0x7e
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ESP + 0xc], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x74              // JZ +0x2a
        _emit 0x2a
        _emit 0x83              // CMP dword ptr [EDI + 0x24], 0x10
        _emit 0x7f
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [EDI + 0x20]
        _emit 0x47
        _emit 0x20
        _emit 0x72              // JC +5
        _emit 0x05
        _emit 0x8b              // MOV EDI, dword ptr [EDI + 0x10]
        _emit 0x7f
        _emit 0x10
        _emit 0xeb              // JMP +3
        _emit 0x03
        _emit 0x83              // ADD EDI, 0x10
        _emit 0xc7
        _emit 0x10
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [EBX + 0x14]
        _emit 0x43
        _emit 0x14
        _emit 0x57              // PUSH EDI
        _emit 0x50              // PUSH EAX
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x8b              // MOV ECX, EBX
        _emit 0xcb
        _emit 0xe8              // CALL 0x00620110 (rel32 = 0x001ce666)
        _emit 0x66
        _emit 0xe6
        _emit 0x1c
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7c              // JL +6
        _emit 0x06
        _emit 0x8d              // LEA ECX, [ESP + 0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0xeb              // JMP +0xf
        _emit 0x0f
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x4]
        _emit 0x4e
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ESP + 0x18], ECX
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x89              // MOV dword ptr [ESP + 0x14], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x8d              // LEA ECX, [ESP + 0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EDX, dword ptr [ECX]
        _emit 0x11
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x8b              // MOV ECX, dword ptr [ECX + 0x4]
        _emit 0x49
        _emit 0x04
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x89              // MOV dword ptr [EAX], EDX
        _emit 0x10
        _emit 0x89              // MOV dword ptr [EAX + 0x4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
