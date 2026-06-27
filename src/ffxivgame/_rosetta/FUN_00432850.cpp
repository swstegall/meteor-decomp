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
// FUNCTION: ffxivgame 0x00032850 — singleton constructor / sub-object init
//                                   (__thiscall, 41 B / 0x29)
//
// Receives `this` in ECX (__thiscall).  Sets the vtable pointer at [this]
// to 0x00f6399c, then calls FUN_00432660 (with ECX = this+4) to initialise
// the sub-object starting at offset 4.  Stores the return value into
// [this+8] and zeros [this+12].  Writes `this` to the global singleton
// slot at 0x0132c8ac, then returns `this` in EAX.
//
// Calling convention: __thiscall — `this` in ECX, plain RET (no callee
// stack cleanup); caller restores nothing.
// Callee-saves: ESI (= this), EDI (= this+4).
// Frame: none (/Oy — no EBP frame).
//
// Asm (41 bytes @ orig RVA 0x00032850):
//   56                    PUSH ESI
//   8b f1                 MOV ESI, ECX                    ; save this
//   57                    PUSH EDI
//   8d 7e 04              LEA EDI, [ESI+0x4]              ; EDI = this+4
//   8b cf                 MOV ECX, EDI                    ; this for sub-call
//   c7 06 9c 39 f6 00     MOV dword ptr [ESI], 0xf6399c   ; set vtable
//   e8 fc fd ff ff        CALL 0x00432660                  ; → FUN_00432660 (reloc)
//   89 47 04              MOV dword ptr [EDI+0x4], EAX    ; [this+8] = result
//   c7 47 08 00 00 00 00  MOV dword ptr [EDI+0x8], 0x0    ; [this+12] = 0
//   5f                    POP EDI
//   89 35 ac c8 32 01     MOV dword ptr [0x0132c8ac], ESI  ; global = this
//   8b c6                 MOV EAX, ESI                    ; return this
//   5e                    POP ESI
//   c3                    RET
//
// Reloc-bearing sites (masked by compare.py):
//   +0x09  MOV imm32  → vtable ptr  0x00f6399c  (.rdata)
//   +0x0f  CALL rel32 → FUN_00432660 (RVA 0x00032660)
//   +0x18  MOV moffs32 → global     0x0132c8ac  (.data singleton slot)

extern "C" void FUN_00432660();

extern "C" __declspec(naked) void FUN_00432850() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA EDI, [ESI+0x4]
        _emit 0x7e
        _emit 0x04
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xc7              // MOV dword ptr [ESI], 0xf6399c
        _emit 0x06
        _emit 0x9c
        _emit 0x39
        _emit 0xf6
        _emit 0x00
        call FUN_00432660       // CALL rel32 → FUN_00432660  (5-byte reloc)
        _emit 0x89              // MOV dword ptr [EDI+0x4], EAX
        _emit 0x47
        _emit 0x04
        _emit 0xc7              // MOV dword ptr [EDI+0x8], 0x0
        _emit 0x47
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x89              // MOV dword ptr [0x0132c8ac], ESI
        _emit 0x35
        _emit 0xac
        _emit 0xc8
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
