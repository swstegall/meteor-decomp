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
// FUNCTION: ffxivgame 0x0001bd30 — guard-and-dispatch: call vtable slot 0x50 on
//                                  the global singleton at [0x01329834] with
//                                  (obj_ptr, byte_arg). (__cdecl, 24 B / 0x18)
//
// Calling convention: __cdecl; bare RET; no stack frame.
// One argument: unsigned char at [ESP+4].
//
// If the global at VA 0x01329834 is null, the function is a no-op.
// Otherwise it calls vtable slot 20 (vtable offset 0x50) with two arguments:
//   arg0 = the global object pointer (EAX)
//   arg1 = the byte parameter (EDX, zero-extended from [ESP+4])
//
// Push order confirms __cdecl right-to-left evaluation:
//   PUSH EDX (second arg)  →  PUSH EAX (first arg)
//
// The same global ([0x01329834]) appears in FUN_0041bb60, FUN_00419a20,
// and FUN_0041a700 — it is a manager/context singleton whose vtable methods
// are called via indirect CALL EAX throughout this cluster.
//
// Asm (24 bytes):
//   a1 34 98 32 01         MOV EAX, [0x01329834]
//   85 c0                  TEST EAX, EAX
//   74 0e                  JZ +0x0e (→ RET at +0x17)
//   0f b6 54 24 04         MOVZX EDX, byte ptr [ESP+4]
//   8b 08                  MOV ECX, dword ptr [EAX]        (vtable ptr)
//   52                     PUSH EDX                        (arg1 = byte_arg)
//   50                     PUSH EAX                        (arg0 = obj ptr)
//   8b 41 50               MOV EAX, dword ptr [ECX+0x50]   (vtable slot 20)
//   ff d0                  CALL EAX
//   c3                     RET

extern "C" __declspec(naked) void FUN_0041bd30() {
    __asm {
        // a1 34 98 32 01    MOV EAX, [0x01329834]  (moffs32 absolute load)
        _emit 0xa1
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        test    eax, eax
        jz      done
        movzx   edx, byte ptr [esp + 4]
        mov     ecx, dword ptr [eax]
        push    edx
        push    eax
        mov     eax, dword ptr [ecx + 0x50]
        call    eax
    done:
        ret
    }
}
