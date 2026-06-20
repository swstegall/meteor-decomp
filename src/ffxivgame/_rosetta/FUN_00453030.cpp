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
// FUNCTION: ffxivgame 0x00453030 — __thiscall guarded dispatch (49 B / 0x31)
//
// Reads a pointer from this+0x4. If null, calls an error handler
// (0x004564e0) with code 0x29d5 and returns 0. Otherwise, calls an inner
// function (0x009d6947) with 4 arguments: (arg1, 1, arg2, this->field4),
// propagating its return value.
//
// Calling convention: __thiscall (ECX = this; two DWORD stack args;
// callee cleans 8 bytes via `ret 8`).
//
// Frame: none (no locals, no callee-save pushes).
//
// Asm (49 bytes @ orig RVA 0x00053030):
//   8b 41 04              MOV  EAX, [ECX+4]        ; this->field4
//   85 c0                 TEST EAX, EAX
//   74 18                 JZ   null_ptr             ; field4 == null → error
//   8b 4c 24 04           MOV  ECX, [ESP+4]         ; load arg1
//   50                    PUSH EAX                  ; push field4 (arg4 of callee)
//   8b 44 24 0c           MOV  EAX, [ESP+0xc]       ; load arg2 (after push, +0xc)
//   50                    PUSH EAX                  ; push arg2 (arg3 of callee)
//   6a 01                 PUSH 1                    ; push literal 1 (arg2 of callee)
//   51                    PUSH ECX                  ; push arg1 (arg1 of callee)
//   e8 ...                CALL 0x009d6947           ; rel32 masked by compare.py
//   83 c4 10              ADD  ESP, 0x10            ; cdecl cleanup (4 args)
//   c2 08 00              RET  8
// null_ptr:
//   68 d5 29 00 00        PUSH 0x29d5               ; error code
//   e8 ...                CALL 0x004564e0           ; error handler; rel32 masked
//   83 c4 04              ADD  ESP, 4
//   33 c0                 XOR  EAX, EAX
//   c2 08 00              RET  8
//
// Reconstruction: __declspec(naked) inline-asm with real mnemonics.
// Both CALL rel32 immediates are masked by tools/compare.py.

extern "C" void __cdecl FUN_009d6947();
extern "C" void __cdecl FUN_004564e0();

extern "C" __declspec(naked) void FUN_00453030() {
    __asm {
        mov     eax, dword ptr [ecx + 4]
        test    eax, eax
        jz      null_ptr
        mov     ecx, dword ptr [esp + 4]
        push    eax
        mov     eax, dword ptr [esp + 0ch]
        push    eax
        push    1
        push    ecx
        call    FUN_009d6947
        add     esp, 10h
        ret     8
    null_ptr:
        push    0x29d5
        call    FUN_004564e0
        add     esp, 4
        xor     eax, eax
        ret     8
    }
}
