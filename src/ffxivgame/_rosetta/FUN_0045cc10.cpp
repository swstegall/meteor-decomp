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
// FUNCTION: ffxivgame 0x0005cc10 — guarded indirect dispatch (40 B / 0x28)
//
// int FUN_0045cc10(Obj *param_1, int param_2)
//
// Two-level null guard before calling a function pointer stored in a
// nested struct field.  param_1->field_0xc is an inner-object pointer;
// field_0x54 of that inner object is a __cdecl function pointer.
// If either is null the function returns -2 (0xFFFFFFFE) immediately.
// Otherwise it calls fn(param_1, 3, 0, param_2) and returns the result.
//
// Calling convention: __cdecl — two args, int return, plain RET.
// Frame: none (/Oy — function has no locals, no EBP frame).
//
// Asm (40 bytes @ orig RVA 0x0005cc10):
//   8b 4c 24 04           MOV ECX, [ESP+4]         ; param_1
//   8b 41 0c              MOV EAX, [ECX+0xc]       ; inner = param_1->field_0xc
//   85 c0                 TEST EAX, EAX
//   74 17                 JZ  +0x17 → return -2
//   8b 40 54              MOV EAX, [EAX+0x54]      ; fn = inner->field_0x54
//   85 c0                 TEST EAX, EAX
//   74 10                 JZ  +0x10 → return -2
//   8b 54 24 08           MOV EDX, [ESP+8]          ; param_2
//   52                    PUSH EDX
//   6a 00                 PUSH 0
//   6a 03                 PUSH 3
//   51                    PUSH ECX                  ; param_1
//   ff d0                 CALL EAX
//   83 c4 10              ADD ESP, 0x10
//   c3                    RET
//   b8 fe ff ff ff        MOV EAX, 0xfffffffe       ; -2
//   c3                    RET

struct Obj5cc10;

struct ObjInner5cc10 {
    char _pad[0x54];
    int (__cdecl *call)(Obj5cc10 *, int, int, int);
};

struct Obj5cc10 {
    char _pad[0xc];
    ObjInner5cc10 *inner;
};

int __cdecl FUN_0045cc10(Obj5cc10 *param_1, int param_2)
{
    ObjInner5cc10 *inner = param_1->inner;
    if (inner != 0)
    {
        int (__cdecl *fn)(Obj5cc10 *, int, int, int) = inner->call;
        if (fn != 0)
        {
            return fn(param_1, 3, 0, param_2);
        }
    }
    return -2;
}
