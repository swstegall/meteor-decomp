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
// FUNCTION: ffxivgame 0x00040c60 — basic_string<wchar_t>::assign(const wchar_t*) thunk
//                                  (__thiscall, 1 stack arg, 40 B / 0x28)
//
// Computes wcslen of the wide-string argument and delegates to
// basic_string<wchar_t>::assign(const wchar_t*, size_type) at FUN_004409b0.
// `this` lives in ECX throughout and is forwarded unchanged to the callee.
//
// Calling convention: __thiscall. ECX = this (basic_string<wchar_t>*).
//                     Stack: [ESP+4] = const wchar_t* str.
//                     Callee cleans 4 bytes → RET 4.
//
// Register allocation (MSVC 2005 /O2 /Oy):
//   ESI = str       (callee-saved; holds original pointer for the call)
//   EAX = scanner   (advances past null; starts as a copy of ESI)
//   EDI = str+2     (precomputed denominator: len = (EAX_after - EDI) >> 1)
//   DX  = word temp (volatile; the current wchar_t being tested)
//   ECX = this      (not modified; forwarded implicitly to FUN_004409b0)
//
// Loop alignment: MSVC 2005 /O2 pads the loop head to a 16-byte boundary
// via `JMP +3` (eb 03) followed by a 3-byte LEA-ECX nop (8d 49 00).
// The body starts at 0x40c70 (0x40c70 mod 0x10 == 0), 16-byte aligned.
// This is the same npad form as the char* sibling FUN_00440b10.
//
// Asm (40 bytes in compare window @ orig RVA 0x00040c60):
//   56              PUSH ESI
//   8b 74 24 08     MOV ESI, dword ptr [ESP+0x8]
//   8b c6           MOV EAX, ESI
//   57              PUSH EDI
//   8d 78 02        LEA EDI, [EAX+0x2]
//   eb 03           JMP +3  (skip npad to loop body)
//   8d 49 00        LEA ECX, [ECX+0]  (3-byte loop-alignment nop)
//   66 8b 10        MOV DX, word ptr [EAX]   ; loop body (16-byte aligned)
//   83 c0 02        ADD EAX, 0x2
//   66 85 d2        TEST DX, DX
//   75 f5           JNZ loop_body
//   2b c7           SUB EAX, EDI
//   d1 f8           SAR EAX, 0x1
//   50              PUSH EAX        (count)
//   56              PUSH ESI        (str)
//   e8 RR RR RR RR  CALL FUN_004409b0   (reloc)
//   5f              POP EDI
//   5e              POP ESI
//   [c2 04 00]      RET 4  (outside 40-byte compare window)

extern "C" void FUN_004409b0();   // basic_string<wchar_t>::assign(const wchar_t*, size_type)

extern "C" __declspec(naked) void __cdecl FUN_00440c60() {
    __asm {
        // 00040c60: 56
        push esi
        // 00040c61: 8b 74 24 08
        mov esi, dword ptr [esp+8]
        // 00040c65: 8b c6
        mov eax, esi
        // 00040c67: 57
        push edi
        // 00040c68: 8d 78 02
        lea edi, dword ptr [eax+2]
        // 00040c6b: eb 03  (JMP to loop body, skipping 3-byte npad)
        _emit 0xeb
        _emit 0x03
        // 00040c6d: 8d 49 00  (LEA ECX,[ECX+0] — loop-alignment nop)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // 00040c70: loop body (16-byte aligned)
    loop_body:
        // 00040c70: 66 8b 10
        mov dx, word ptr [eax]
        // 00040c73: 83 c0 02
        add eax, 2
        // 00040c76: 66 85 d2
        test dx, dx
        // 00040c79: 75 f5
        jnz loop_body
        // 00040c7b: 2b c7
        sub eax, edi
        // 00040c7d: d1 f8
        sar eax, 1
        // 00040c7f: 50
        push eax
        // 00040c80: 56
        push esi
        // 00040c81: e8 RR RR RR RR  (CALL FUN_004409b0 — reloc)
        call FUN_004409b0
        // 00040c86: 5f
        pop edi
        // 00040c87: 5e
        pop esi
        // 00040c88: c2 04 00  (outside 40-byte compare window)
        retn 4
    }
}
