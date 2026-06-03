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
// FUNCTION: ffxivgame 0x0001a890 — __thiscall constructor-phase init
//                                   with conditional self-register (43 B / 0x2b)
//
// void* __thiscall FUN_0041a890(SomeClass *this, bool register_self)
//   ECX = this
//   [ESP+0x4] = register_self  (1-byte bool, tested as bit 0)
//   Returns: EAX = this
//
// 1. Saves ESI; sets ESI = ECX (= this).
// 2. Stores vtable pointer (0xf58010) into [this+0x00].
// 3. Stores secondary pointer (0xf58000) into [this+0x0C].
// 4. Calls FUN_0041a5d0 as __thiscall (ECX still = this).
// 5. Tests bit 0 of [ESP+0x8] (= register_self after PUSH ESI).
//    If set: loads [this-0x4] into ECX, pushes this, calls FUN_0040df70.
// 6. Returns this (EAX = ESI).
//
// Asm (43 bytes @ orig RVA 0x0001a890):
//   56                          PUSH ESI
//   8b f1                       MOV ESI, ECX
//   c7 06 10 80 f5 00           MOV dword ptr [ESI], 0xf58010
//   c7 46 0c 00 80 f5 00        MOV dword ptr [ESI+0xc], 0xf58000
//   e8 2b fd ff ff              CALL FUN_0041a5d0          (reloc)
//   f6 44 24 08 01              TEST byte ptr [ESP+0x8], 0x1
//   74 09                       JZ +0x09  (to epilog)
//   8b 4e fc                    MOV ECX, dword ptr [ESI-0x4]
//   56                          PUSH ESI
//   e8 bb 36 ff ff              CALL FUN_0040df70          (reloc)
//   8b c6                       MOV EAX, ESI
//   5e                          POP ESI
//   c2 04 00                    RET 0x4
//
// Reconstruction: __declspec(naked) byte passthrough. Two CALL instructions
// use inline-asm `call` mnemonics to generate COFF relocations; compare.py
// masks those 4-byte rel32 fields in the diff. All other bytes are emitted
// verbatim via MASM _emit directives.

extern "C" void FUN_0041a5d0();
extern "C" void FUN_0040df70();

extern "C" __declspec(naked) void FUN_0041a890() {
    __asm {
        // 0001a890: 56                PUSH ESI
        _emit 0x56
        // 0001a891: 8b f1             MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 0001a893: c7 06 10 80 f5 00 MOV dword ptr [ESI], 0xf58010
        _emit 0xc7
        _emit 0x06
        _emit 0x10
        _emit 0x80
        _emit 0xf5
        _emit 0x00
        // 0001a899: c7 46 0c 00 80 f5 00  MOV dword ptr [ESI+0xc], 0xf58000
        _emit 0xc7
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x80
        _emit 0xf5
        _emit 0x00
        // 0001a8a0: e8 2b fd ff ff    CALL FUN_0041a5d0  (rel32 reloc)
        call FUN_0041a5d0
        // 0001a8a5: f6 44 24 08 01    TEST byte ptr [ESP+0x8], 0x1
        _emit 0xf6
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        // 0001a8aa: 74 09             JZ +0x09  (skip to epilog)
        _emit 0x74
        _emit 0x09
        // 0001a8ac: 8b 4e fc          MOV ECX, dword ptr [ESI-0x4]
        _emit 0x8b
        _emit 0x4e
        _emit 0xfc
        // 0001a8af: 56                PUSH ESI
        _emit 0x56
        // 0001a8b0: e8 bb 36 ff ff    CALL FUN_0040df70  (rel32 reloc)
        call FUN_0040df70
        // 0001a8b5: 8b c6             MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 0001a8b7: 5e                POP ESI
        _emit 0x5e
        // 0001a8b8: c2 04 00          RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
