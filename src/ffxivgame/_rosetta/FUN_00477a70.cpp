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
// FUNCTION: ffxivgame 0x00077a70 — conditional release of three nullable
//                                  pointer fields at offsets +0x40, +0x44,
//                                  +0x48 of an unknown struct (60 bytes).
//
// __cdecl int FUN_00477a70(some_struct *p)
//   stack layout (RET — caller-cleans):
//     [ESP+0x04]  some_struct *p  (arg1)
//
// Inspection (read from the orig bytes at RVA 0x00077a70, 60 bytes total):
//
//   push esi
//   mov  esi, [esp+0x8]          ; esi = p  (callee-saved copy)
//   mov  eax, [esi+0x40]
//   test eax, eax
//   jz   +9   →  skip_1
//   push eax
//   call FUN_004958b0            ; (rel32 – caller-cleans)
//   add  esp, 4
// skip_1:
//   mov  eax, [esi+0x44]
//   test eax, eax
//   jz   +9   →  skip_2
//   push eax
//   call FUN_004958b0
//   add  esp, 4
// skip_2:
//   mov  esi, [esi+0x48]         ; reuse ESI – last use of struct pointer
//   test esi, esi
//   jz   +9   →  skip_3
//   push esi
//   call FUN_004958b0
//   add  esp, 4
// skip_3:
//   mov  eax, 1
//   pop  esi
//   ret
//
// Register-allocation note: MSVC 2005 /O2 keeps `p` in ESI (callee-saved)
// throughout.  For the first two fields it scratches EAX.  For the third
// field it recycles ESI (the struct pointer's last live use), loading
// p->field_48 directly into ESI and pushing ESI.  The C++ source below
// reproduces this naturally — MSVC sees that `p` is dead after the last
// dereference and collapses the load into the same register.
//
// Calling convention: __cdecl (one pointer-sized arg on stack; callee
// preserves ESI/EDI/EBX; returns via `ret` with no immediate).

struct _FUN_00477a70_type {
    char _pad[0x40];
    int  field_40;
    int  field_44;
    int  field_48;
};

extern "C" void __cdecl FUN_004958b0(int);

extern "C" int __cdecl FUN_00477a70(_FUN_00477a70_type *p) {
    if (p->field_40)
        FUN_004958b0(p->field_40);
    if (p->field_44)
        FUN_004958b0(p->field_44);
    if (p->field_48)
        FUN_004958b0(p->field_48);
    return 1;
}
