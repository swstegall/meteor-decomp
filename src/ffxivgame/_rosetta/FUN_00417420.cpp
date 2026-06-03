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
// FUNCTION: ffxivgame 0x00017420 — constructor-like thiscall, sets vtable 0x00f5787c
//                                   (__thiscall, 2 stack args, RET 0x8, 40 B / 0x28)
//
// Pattern: saves ESI (= this), calls FUN_00417040 (base-class ctor style, 6 args
// via __thiscall / RET 0x18), then stamps the vtable pointer at [this+0x00], and
// returns this in EAX.  The MOV EAX,[ESP+8] before the PUSH ESI is the classic
// MSVC 2005 /O2 pre-push load: the compiler caches param_2 into a volatile
// register before adjusting ESP so it never needs to re-derive the offset.
//
// Calling convention: __thiscall — ECX = this, two stack args, RET 0x8.
// Frame: none (/Oy).
//
// Asm (40 bytes @ orig RVA 0x00017420):
//   8b 44 24 08           MOV EAX,[ESP+0x8]       ; cache param_2 before PUSH ESI
//   56                    PUSH ESI                  ; save callee-save
//   50                    PUSH EAX                  ; arg6 = param_2 (rightmost, pushed first)
//   6a 18                 PUSH 0x18                 ; arg5
//   6a 00                 PUSH 0x0                  ; arg4
//   6a 04                 PUSH 0x4                  ; arg3
//   8b f1                 MOV ESI,ECX               ; ESI = this
//   8b 4c 24 18           MOV ECX,[ESP+0x18]        ; reload param_1 (now at +5 dwords)
//   6a 04                 PUSH 0x4                  ; arg2
//   51                    PUSH ECX                  ; arg1 = param_1 (leftmost, pushed last)
//   8b ce                 MOV ECX,ESI               ; ECX = this (for __thiscall)
//   e8 04 fc ff ff        CALL FUN_00417040         ; base-class ctor (RET 0x18 cleans 6 args)
//   c7 06 7c 78 f5 00     MOV dword ptr [ESI],0x00f5787c  ; stamp vtable
//   8b c6                 MOV EAX,ESI               ; return this
//   5e                    POP ESI
//   c2 08 00              RET 0x8                   ; callee-cleans 2 stack args

extern "C" void FUN_00417040();   // forward declaration for the CALL relocation

extern "C" __declspec(naked) void FUN_00417420() {
    __asm {
        // 00017420: 8b 44 24 08   MOV EAX,[ESP+0x8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00017424: 56            PUSH ESI
        _emit 0x56
        // 00017425: 50            PUSH EAX
        _emit 0x50
        // 00017426: 6a 18         PUSH 0x18
        _emit 0x6a
        _emit 0x18
        // 00017428: 6a 00         PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001742a: 6a 04         PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 0001742c: 8b f1         MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0001742e: 8b 4c 24 18   MOV ECX,[ESP+0x18]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00017432: 6a 04         PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 00017434: 51            PUSH ECX
        _emit 0x51
        // 00017435: 8b ce         MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00017437: e8 04 fc ff ff  CALL FUN_00417040  (reloc — REL32)
        call FUN_00417040
        // 0001743c: c7 06 7c 78 f5 00  MOV dword ptr [ESI],0x00f5787c
        _emit 0xc7
        _emit 0x06
        _emit 0x7c
        _emit 0x78
        _emit 0xf5
        _emit 0x00
        // 00017442: 8b c6         MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 00017444: 5e            POP ESI
        _emit 0x5e
        // 00017445: c2 08 00      RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
