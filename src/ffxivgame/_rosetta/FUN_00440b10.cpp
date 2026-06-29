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
// FUNCTION: ffxivgame 0x00040b10 — basic_string::assign(const char*) thunk
//                                  (__thiscall, 1 stack arg, 36 B / 0x24)
//
// Computes strlen of the C-string argument and delegates to
// basic_string::assign(const char*, size_type) at FUN_00404120.
// `this` lives in ECX throughout and is forwarded unchanged to the callee.
//
// Calling convention: __thiscall. ECX = this (basic_string*).
//                     Stack: [ESP+4] = const char* str.
//                     Callee cleans 4 bytes → RET 4.
//
// Register allocation (MSVC 2005 /O2 /Oy):
//   ESI = str       (callee-saved; holds original pointer for the call)
//   EAX = scanner   (advances past null; starts as a copy of ESI)
//   EDI = str+1     (precomputed denominator: len = EAX_after - EDI)
//   DL  = byte temp (volatile; the current byte being tested)
//   ECX = this      (not modified; forwarded implicitly to FUN_00404120)
//
// Loop alignment: MSVC 2005 /O2 pads the loop head to a 16-byte boundary
// via `JMP +3` (eb 03) followed by a 3-byte LEA-ECX nop (8d 49 00).
// The body therefore starts at an offset that is a multiple of 16 from
// the start of the function (0x40b20 mod 0x10 == 0), which matches the
// same npad form observed in FUN_00440130.cpp.
//
// Asm (36 bytes in compare window @ orig RVA 0x00040b10):
//   56              PUSH ESI
//   8b 74 24 08     MOV ESI, dword ptr [ESP+0x8]
//   8b c6           MOV EAX, ESI
//   57              PUSH EDI
//   8d 78 01        LEA EDI, [EAX+0x1]
//   eb 03           JMP +3  (skip npad to loop body)
//   8d 49 00        LEA ECX, [ECX+0]  (3-byte loop-alignment nop)
//   8a 10           MOV DL, byte ptr [EAX]   ; loop body
//   83 c0 01        ADD EAX, 0x1
//   84 d2           TEST DL, DL
//   75 f7           JNZ loop_body
//   2b c7           SUB EAX, EDI
//   50              PUSH EAX        (count)
//   56              PUSH ESI        (str)
//   e8 RR RR RR RR  CALL FUN_00404120   (reloc)
//   5f              POP EDI
//   5e              POP ESI
//   [c2 04 00]      RET 4  (outside 36-byte Ghidra compare window)

extern "C" void FUN_00404120();   // basic_string::assign(const char*, size_type)

extern "C" __declspec(naked) void __cdecl FUN_00440b10() {
    __asm {
        // 00040b10: 56
        push esi
        // 00040b11: 8b 74 24 08
        mov esi, dword ptr [esp+8]
        // 00040b15: 8b c6
        mov eax, esi
        // 00040b17: 57
        push edi
        // 00040b18: 8d 78 01
        lea edi, dword ptr [eax+1]
        // 00040b1b: eb 03  (JMP to loop body, skipping 3-byte npad)
        _emit 0xeb
        _emit 0x03
        // 00040b1d: 8d 49 00  (LEA ECX,[ECX+0] — loop-alignment nop)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // 00040b20: loop body (16-byte aligned)
    loop_body:
        // 00040b20: 8a 10
        mov dl, byte ptr [eax]
        // 00040b22: 83 c0 01
        add eax, 1
        // 00040b25: 84 d2
        test dl, dl
        // 00040b27: 75 f7
        jnz loop_body
        // 00040b29: 2b c7
        sub eax, edi
        // 00040b2b: 50
        push eax
        // 00040b2c: 56
        push esi
        // 00040b2d: e8 RR RR RR RR  (CALL FUN_00404120 — reloc)
        call FUN_00404120
        // 00040b32: 5f
        pop edi
        // 00040b33: 5e
        pop esi
        // 00040b34: c2 04 00  (outside Ghidra compare window)
        retn 4
    }
}
