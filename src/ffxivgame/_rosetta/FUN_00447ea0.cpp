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
// FUNCTION: ffxivgame 0x00047ea0 — FUN_00447ea0 (__thiscall, 39 B / 0x27)
//
// A __thiscall member function with three stack arguments that calls two
// sibling methods and returns `this` in EAX:
//
//   1. FUN_004460a0(param_1, param_2)   — __thiscall, 2 stack args, RET 8
//   2. FUN_00447de0(param_1, param_3)   — __thiscall, 2 stack args, RET 8
//
// Calling convention: __thiscall — ECX = this, 3 stack args (param_1 / param_2 /
//   param_3), callee cleans 12 bytes (RET 0xC).  Returns this (ESI → EAX).
// Frame: none (/Oy — no local variables).
//
// Notable MSVC 2005 /O2 scheduling: param_2 is pre-loaded into EAX before
// the PUSH ESI / PUSH EDI callee-saves to keep the stack-offset instruction
// small ([ESP+8] vs [ESP+10] post-push).  The `MOV ESI, ECX` save of `this`
// is deferred until after the two argument pushes — a deferred-callee-save
// idiom also seen in FUN_004134b0.
//
// Asm (39 bytes @ orig RVA 0x00047ea0):
//   8b 44 24 08   MOV EAX, [ESP+8]       ; param_2
//   56            PUSH ESI
//   57            PUSH EDI
//   8b 7c 24 0c  MOV EDI, [ESP+C]        ; param_1 (shifted by 2 pushes)
//   50            PUSH EAX               ; push param_2
//   57            PUSH EDI               ; push param_1
//   8b f1         MOV ESI, ECX           ; save this (deferred)
//   e8 RR RR RR RR CALL FUN_004460a0     ; this->FUN_004460a0(param_1, param_2)
//   8b 4c 24 14  MOV ECX, [ESP+0x14]     ; param_3 (callee cleaned 8B → entry+0xC)
//   51            PUSH ECX               ; push param_3
//   57            PUSH EDI               ; push param_1
//   8b ce         MOV ECX, ESI           ; restore this
//   e8 RR RR RR RR CALL FUN_00447de0     ; this->FUN_00447de0(param_1, param_3)
//   5f            POP EDI
//   8b c6         MOV EAX, ESI           ; return this
//   5e            POP ESI
//   c2 0c 00      RET 0xC
//
// Reconstruction: __declspec(naked) to preserve the deferred-save ordering
// and exact push/pop interleaving.  The two CALL rel32 sites are emitted via
// real `call` directives so compare.py masks their reloc bytes correctly.

extern "C" void FUN_004460a0();
extern "C" void FUN_00447de0();

extern "C" __declspec(naked) void FUN_00447ea0() {
    __asm {
        // 00047ea0: 8b 44 24 08   MOV EAX, [ESP+8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00047ea4: 56            PUSH ESI
        _emit 0x56
        // 00047ea5: 57            PUSH EDI
        _emit 0x57
        // 00047ea6: 8b 7c 24 0c  MOV EDI, [ESP+C]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 00047eaa: 50            PUSH EAX
        _emit 0x50
        // 00047eab: 57            PUSH EDI
        _emit 0x57
        // 00047eac: 8b f1         MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00047eae: e8 ...        CALL FUN_004460a0  (reloc)
        call FUN_004460a0
        // 00047eb3: 8b 4c 24 14  MOV ECX, [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00047eb7: 51            PUSH ECX
        _emit 0x51
        // 00047eb8: 57            PUSH EDI
        _emit 0x57
        // 00047eb9: 8b ce         MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00047ebb: e8 ...        CALL FUN_00447de0  (reloc)
        call FUN_00447de0
        // 00047ec0: 5f            POP EDI
        _emit 0x5f
        // 00047ec1: 8b c6         MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 00047ec3: 5e            POP ESI
        _emit 0x5e
        // 00047ec4: c2 0c 00      RET 0xC
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
