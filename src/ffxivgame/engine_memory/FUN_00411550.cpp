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
// FUNCTION: ffxivgame 0x00011550 — __thiscall: guarded virtual call through
//                                   inner list head, store result in field_0xc,
//                                   return &field_0x8 or NULL (41 bytes / 0x29)
//
// Asm (41 bytes @ orig RVA 0x00011550):
//   56          PUSH ESI
//   8b f1       MOV ESI, ECX              ; cache this
//   8b 56 04    MOV EDX, [ESI + 0x4]     ; EDX = this->inner
//   8b 4a 54    MOV ECX, [EDX + 0x54]    ; ECX = inner->field_0x54 (next of sentinel)
//   83 c2 4c    ADD EDX, 0x4c             ; EDX = &inner->field_0x4c (sentinel node)
//   33 c0       XOR EAX, EAX              ; result = 0
//   3b ca       CMP ECX, EDX              ; node == sentinel? (list empty?)
//   74 07       JZ  skip                  ; yes → skip call, result stays 0
//   8b 01       MOV EAX, [ECX]            ; EAX = node->vtable
//   8b 50 04    MOV EDX, [EAX + 0x4]     ; EDX = vtable[1] (method1)
//   ff d2       CALL EDX                  ; node->method1() — ECX=node (__thiscall)
// skip:
//   85 c0       TEST EAX, EAX
//   89 46 0c    MOV [ESI + 0xc], EAX      ; this->fieldC = result
//   74 05       JZ  ret_null              ; if 0, return NULL
//   8d 46 08    LEA EAX, [ESI + 0x8]     ; EAX = &this->field8
//   5e          POP ESI
//   c3          RET
// ret_null:
//   33 c0       XOR EAX, EAX
//   5e          POP ESI
//   c3          RET
//
// The Inner object contains an intrusive list whose sentinel node is
// embedded at inner+0x4c. The sentinel's next-pointer lives at inner+0x54
// (= sentinel + 0x8, after two 4-byte fields in the sentinel). If the list
// is non-empty the head node's virtual slot 1 is called; the result is
// stored in this->fieldC. Returns &this->field8 when the result is
// non-null, NULL otherwise.
//
// Calling convention: __thiscall, no stack args (plain RET).
// Callee-saves used: ESI only.

struct FUN_00411550_Node {
    virtual int method0();
    virtual int method1();
};

// The sentinel node embedded in Inner at offset 0x4c.
// Its first two fields occupy 8 bytes, putting the next-pointer at +8
// (i.e., at inner+0x54). The sentinel itself is also treated as a Node
// for comparison purposes (&sentinel is the sentinel address used in the
// list-empty guard).
struct FUN_00411550_Sentinel {
    int _field0;                  // at sentinel+0x0 (inner+0x4c)
    int _field4;                  // at sentinel+0x4 (inner+0x50)
    FUN_00411550_Node *next;      // at sentinel+0x8 (inner+0x54)
};

struct FUN_00411550_Inner {
    char _pad[0x4c];
    FUN_00411550_Sentinel sentinel; // embedded at 0x4c; next at 0x54
};

struct FUN_00411550_C {
    char _pad0[0x4];
    FUN_00411550_Inner *inner; // at 0x4
    int  field8;               // at 0x8
    int  fieldC;               // at 0xc

    int *FUN_00411550();
};

int *FUN_00411550_C::FUN_00411550()
{
    FUN_00411550_Inner    *_inner   = inner;
    FUN_00411550_Node     *sentinel = (FUN_00411550_Node *)&_inner->sentinel;
    FUN_00411550_Node     *node     = _inner->sentinel.next;
    int result = 0;
    if (node != sentinel)
        result = node->method1();
    fieldC = result;
    if (result)
        return &field8;
    return 0;
}
