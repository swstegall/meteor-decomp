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
// FUNCTION: ffxivgame 0x00011580 — __thiscall member: conditional-advance
//                                  iterator with vtable[1] dispatch
//
// Asm (55 bytes @ orig RVA 0x00011580):
//   56              PUSH ESI
//   8b f1           MOV ESI, ECX                ; cache this
//   8b 4e 0c        MOV ECX, [ESI + 0xc]        ; cursor = this->_c
//   33 c0           XOR EAX, EAX                ; result = 0 (default)
//   85 c9           TEST ECX, ECX               ; cursor non-null?
//   74 05           JZ  null_path
//   83 c1 08        ADD ECX, 0x8                ; cursor_adj = cursor + 8
//   eb 02           JMP continue
//   33 c9           XOR ECX, ECX                ; null_path: cursor_adj = 0
//   8b 56 04        MOV EDX, [ESI + 0x4]        ; base = this->_4
//   8b 49 08        MOV ECX, [ECX + 0x8]        ; next = *(cursor_adj + 8)
//   83 c2 4c        ADD EDX, 0x4c               ; base += 0x4c (sentinel)
//   3b ca           CMP ECX, EDX                ; next == sentinel?
//   74 07           JZ  skip_call
//   8b 01           MOV EAX, [ECX]              ; vtable = *next
//   8b 50 04        MOV EDX, [EAX + 0x4]        ; fn = vtable[1]
//   ff d2           CALL EDX                    ; result = next->_vf1()
//   85 c0           TEST EAX, EAX
//   89 46 0c        MOV [ESI + 0xc], EAX        ; this->_c = result
//   74 05           JZ  return_null
//   8d 46 08        LEA EAX, [ESI + 0x8]        ; return &this->_8
//   5e              POP ESI
//   c3              RET
//   33 c0           XOR EAX, EAX                ; return_null: return 0
//   5e              POP ESI
//   c3              RET
//
// Class has no virtual destructor so vtable[0] = _vf0, vtable[1] = _vf1.
// All fields are stored as int (raw 32-bit value) enabling plain integer
// arithmetic to produce ADD ECX/EDX immediates rather than scaled
// pointer arithmetic.

class FUN_00411580_Node {
public:
    virtual void _vf0();
    virtual int _vf1();
};

struct FUN_00411580_C {
    int _0;   // [+0x0]
    int _4;   // [+0x4] container base (sentinel = _4 + 0x4c)
    int _8;   // [+0x8] result buffer (address returned on success)
    int _c;   // [+0xc] cursor (node pointer stored as raw int)

    int* FUN_00411580();
};

int* FUN_00411580_C::FUN_00411580() {
    int cursor = _c;
    int result = 0;
    int ecx;
    if (cursor) {
        ecx = cursor + 8;
    } else {
        ecx = 0;
    }
    int base = _4;
    int next = *(int*)(ecx + 8);
    base += 0x4c;
    if (next != base) {
        result = ((FUN_00411580_Node*)next)->_vf1();
    }
    _c = result;
    if (result) {
        return &_8;
    }
    return 0;
}
