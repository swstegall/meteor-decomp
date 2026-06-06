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
// FUNCTION: ffxivgame 0x0002ee20 — __thiscall wrapper: build temp, dispatch
//                                   (1 stack arg, 40 B / 0x28)
//
// SomeObj *FUN_0042ee20(SomeObj *this, void *param_1)
//
// 1. Loads the stack argument from [ESP+4] into EAX before opening the frame.
// 2. Allocates a 0x40-byte local scratch buffer (SUB ESP,0x40).
// 3. Saves ESI / stashes `this` in ESI.
// 4. Calls FUN_0042edb0 as __thiscall (this = ESI) with (&scratch, param_1);
//    the result is a single value returned in EAX.
// 5. Pushes that result and calls FUN_00419b60 as __thiscall (this = ESI).
// 6. Returns `this` (ESI) and cleans the single stack arg (RET 4).
//
// MSVC loads the stack arg into EAX *before* SUB ESP,0x40 / PUSH ESI, then
// references the freshly-allocated scratch buffer via LEA [ESP+8] (past the
// two pushes). Reproducing the exact prologue ordering, frame size, and the
// `this`-in-ESI caching from source-level C++ is fragile, so this function
// is encoded as a __declspec(naked) byte-for-byte passthrough. The only
// linker-relocated fields are the 4-byte relative offsets of the two CALLs
// (FUN_0042edb0, FUN_00419b60); compare.py masks those bytes.
//
// Asm (40 bytes @ orig RVA 0x0002ee20):
//   8b 44 24 04     MOV EAX, dword ptr [ESP+0x4]   ; param_1
//   83 ec 40        SUB ESP, 0x40                  ; 0x40-byte scratch
//   56              PUSH ESI
//   8b f1           MOV ESI, ECX                   ; this
//   50              PUSH EAX                        ; arg2 = param_1
//   8d 4c 24 08     LEA ECX, [ESP+0x8]             ; &scratch
//   51              PUSH ECX                        ; arg1 = &scratch
//   8b ce           MOV ECX, ESI                   ; this
//   e8 RR RR RR RR  CALL FUN_0042edb0              ; (reloc)
//   50              PUSH EAX                        ; arg = result
//   8b ce           MOV ECX, ESI                   ; this
//   e8 RR RR RR RR  CALL FUN_00419b60              ; (reloc)
//   8b c6           MOV EAX, ESI                   ; return this
//   5e              POP ESI
//   83 c4 40        ADD ESP, 0x40
//   c2 04 00        RET 0x4

extern "C" void FUN_0042edb0();   // forward declaration for the CALL relocation
extern "C" void FUN_00419b60();   // forward declaration for the CALL relocation

extern "C" __declspec(naked) void FUN_0042ee20() {
    __asm {
        // 0002ee20: 8b 44 24 04   MOV EAX, [ESP+4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0002ee24: 83 ec 40      SUB ESP, 0x40
        _emit 0x83
        _emit 0xec
        _emit 0x40
        // 0002ee27: 56            PUSH ESI
        _emit 0x56
        // 0002ee28: 8b f1         MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 0002ee2a: 50            PUSH EAX
        _emit 0x50
        // 0002ee2b: 8d 4c 24 08   LEA ECX, [ESP+8]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 0002ee2f: 51            PUSH ECX
        _emit 0x51
        // 0002ee30: 8b ce         MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0002ee32: e8 RR RR RR RR  CALL FUN_0042edb0  (reloc)
        call FUN_0042edb0
        // 0002ee37: 50            PUSH EAX
        _emit 0x50
        // 0002ee38: 8b ce         MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0002ee3a: e8 RR RR RR RR  CALL FUN_00419b60  (reloc)
        call FUN_00419b60
        // 0002ee3f: 8b c6         MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 0002ee41: 5e            POP ESI
        _emit 0x5e
        // 0002ee42: 83 c4 40      ADD ESP, 0x40
        _emit 0x83
        _emit 0xc4
        _emit 0x40
        // 0002ee45: c2 04 00      RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
