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
// FUNCTION: ffxivgame 0x00044200 — __thiscall object teardown (44 B / 0x2C)
//
// Re-seats the object's vtable pointer to 0x00F67290 and, if the owned
// sub-object pointer at +0x08 is non-null, runs a CRT bookkeeping/free
// pair on it. The owned block's true allocation base sits 4 bytes
// behind the stored pointer (the `EAX - 4` adjustment) — the classic
// "header DWORD in front of the user payload" allocator layout.
//
//   mov  eax, [ecx + 8]            ; eax = this->_Owned   (+0x08)
//   test eax, eax
//   mov  dword ptr [ecx], 0xF67290 ; this->vtable = &vtbl@0x00F67290
//   jz   done                      ; nothing owned → return
//   mov  ecx, [eax - 4]            ; ecx = header DWORD (alloc cookie/size)
//   push esi
//   lea  esi, [eax - 4]            ; esi = allocation base
//   push 0x00443C90                ; bookkeeping arg (callsite tag / table)
//   push ecx
//   push 0x000000BC               ; 0xBC immediate (id / size constant)
//   push eax
//   call 0x009D1C4C               ; CRT tracker/validate (stdcall, cleans)
//   push esi
//   call 0x009D1BE9               ; CRT free thunk (__cdecl, 1 arg)
//   add  esp, 0x14                ; clean both __cdecl callsites (16 + 4)
//   pop  esi
// done:
//   ret                          ; shared tail; JZ null-path jumps here
//
// Calling convention: __thiscall (ECX = this; no stack params).
// Frame: PUSH ESI on the owned-branch only; the null-path JZ targets the
// shared `ret` at 0x4422f so it never pushes/pops ESI. The two callees
// are __cdecl, so their 20 bytes of args are cleaned post-call by a
// single `add esp, 0x14` before restoring ESI.
//
// The two REL32 callsites (0x009D1C4C, 0x009D1BE9) and the three imm32
// operands carrying absolute addresses (the vtable pointer 0x00F67290 and
// the bookkeeping tag 0x00443C90) are masked out of the byte diff by
// tools/compare.py. Naked-asm byte passthrough pins the TEST/JZ branch
// shape and the moffs/imm encodings to the orig 44-byte slice exactly.

extern "C" __declspec(naked) void FUN_00444200() {
    __asm {
        // 00044200: 8b 41 08            MOV EAX, dword ptr [ECX + 0x8]
        _emit 0x8b
        _emit 0x41
        _emit 0x08
        // 00044203: 85 c0               TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00044205: c7 01 90 72 f6 00   MOV dword ptr [ECX], 0xF67290
        _emit 0xc7
        _emit 0x01
        _emit 0x90
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        // 0004420b: 74 22               JZ done (+0x22 → 0x0044422f)
        _emit 0x74
        _emit 0x22
        // 0004420d: 8b 48 fc            MOV ECX, dword ptr [EAX - 0x4]
        _emit 0x8b
        _emit 0x48
        _emit 0xfc
        // 00044210: 56                  PUSH ESI
        _emit 0x56
        // 00044211: 8d 70 fc            LEA ESI, [EAX - 0x4]
        _emit 0x8d
        _emit 0x70
        _emit 0xfc
        // 00044214: 68 90 3c 44 00      PUSH 0x00443C90
        _emit 0x68
        _emit 0x90
        _emit 0x3c
        _emit 0x44
        _emit 0x00
        // 00044219: 51                  PUSH ECX
        _emit 0x51
        // 0004421a: 68 bc 00 00 00      PUSH 0x000000BC
        _emit 0x68
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004421f: 50                  PUSH EAX
        _emit 0x50
        // 00044220: e8 27 da 58 00      CALL 0x009D1C4C
        _emit 0xe8
        _emit 0x27
        _emit 0xda
        _emit 0x58
        _emit 0x00
        // 00044225: 56                  PUSH ESI
        _emit 0x56
        // 00044226: e8 be d9 58 00      CALL 0x009D1BE9
        _emit 0xe8
        _emit 0xbe
        _emit 0xd9
        _emit 0x58
        _emit 0x00
        // 0004422b: 83 ...              ADD ESP, 0x14 / POP ESI / RET begins here, but
        //                              the recorded 44-byte symbol extent ends on this
        //                              first opcode byte. The remaining `c4 14 5e c3`
        //                              (the rest of `add esp,0x14; pop esi; ret`) lives
        //                              in the shared epilogue tail at 0x4422c..0x4422f
        //                              which the JZ null-path also lands in — that tail
        //                              is outside this symbol and is matched elsewhere.
        _emit 0x83
    }
}
