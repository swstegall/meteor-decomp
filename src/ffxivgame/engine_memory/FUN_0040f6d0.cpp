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
// FUNCTION: ffxivgame 0x0000f6d0 — __thiscall: conditional vtable call on linked-list
//                                   node via fieldC; return &field8 or 0 (47 bytes)
//
// Asm (47 bytes @ orig RVA 0x0000f6d0):
//   56          PUSH ESI
//   8b f1       MOV ESI, ECX                   ; cache this
//   8b 4e 0c    MOV ECX, [ESI + 0xc]           ; ECX = this->fieldC
//   33 c0       XOR EAX, EAX                   ; EAX = 0 (default return)
//   85 c9       TEST ECX, ECX
//   74 05       JZ  +5                          ; fieldC == 0 → XOR ECX,ECX
//   83 c1 08    ADD ECX, 0x8                    ; ECX = fieldC + 8
//   eb 02       JMP +2
//   33 c9       XOR ECX, ECX                   ; ECX = 0
//   8b 56 04    MOV EDX, [ESI + 0x4]           ; EDX = this->field4
//   8b 49 08    MOV ECX, [ECX + 0x8]           ; ECX = *(ECX + 8) → node ptr
//   83 c2 44    ADD EDX, 0x44                  ; EDX = &inner->sentinel
//   3b ca       CMP ECX, EDX                   ; at sentinel?
//   74 0d       JZ  +13                        ; yes → skip, return 0
//   8b 01       MOV EAX, [ECX]                 ; EAX = node->vtable
//   8b 50 04    MOV EDX, [EAX + 0x4]           ; EDX = vtable[1]
//   ff d2       CALL EDX                        ; node->method1() — ECX = node
//   89 46 0c    MOV [ESI + 0xc], EAX           ; this->fieldC = result
//   8d 46 08    LEA EAX, [ESI + 0x8]           ; EAX = &this->field8
//   5e          POP ESI
//   c3          RET
//
// The node IS a vtable-backed object (vtable at offset 0). The sentinel is
// embedded at field4+0x44. When fieldC != NULL the code accesses *(fieldC+0x10);
// when fieldC == NULL it reads from address 8 (null-base sentinel probe).

struct FUN_0040f6d0_Node {
    virtual int method0();
    virtual int method1();
    char _pad[0x4];            // offset 0x4
    // offset 0x8 is accessed from outside as the "next" candidate field
};

struct FUN_0040f6d0_Inner {
    char _pad1[0x44];
    FUN_0040f6d0_Node sentinel; // offset 0x44
};

struct FUN_0040f6d0_C {
    char _pad0[0x4];
    FUN_0040f6d0_Inner *field4;    // offset 0x4
    int field8;                    // offset 0x8
    FUN_0040f6d0_Node *fieldC;     // offset 0xc

    int *FUN_0040f6d0();
};

int *FUN_0040f6d0_C::FUN_0040f6d0()
{
    int *eax = 0;
    int ecx = (int)fieldC;
    if (ecx != 0)
        ecx = ecx + 8;
    else
        ecx = 0;
    FUN_0040f6d0_Node *node = *(FUN_0040f6d0_Node **)(ecx + 8);
    FUN_0040f6d0_Node *sentinel = (FUN_0040f6d0_Node *)((char *)field4 + 0x44);
    if (node != sentinel) {
        fieldC = (FUN_0040f6d0_Node *)node->method1();
        eax = &field8;
    }
    return eax;
}
