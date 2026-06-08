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
// FUNCTION: ffxivgame 0x000388f0 — __thiscall wrapper that builds a 2-DWORD
//                                  local checker struct and fires FUN_00435970
//                                  (38 B / 0x26, RET 4).
//
// Calling convention: __thiscall — ECX = this (the outer object), one 4-byte
//   stack argument (arg1).  Callee cleans arg1 via RET 4.
//
// Frame: no EBP frame (/Oy).  SUB ESP, 8 on entry reserves two DWORDs for
//   a local checker-context struct {typeId, data}.
//
// What it does:
//   1. Loads this->field_4 into ECX   (MOV ECX,[ECX+4])
//   2. Loads arg1 into EAX            (MOV EAX,[ESP+0xc] — after SUB ESP,8)
//   3. PUSHes this->field_4 onto the stack (the single stack arg for the
//      upcoming __thiscall to FUN_00435970)
//   4. Points ECX at the local 8-byte buffer  (LEA ECX,[ESP+4])
//   5. Initialises the buffer:
//        [ESP+4] = 0xf64968   (typeId / assertion-context tag)
//        [ESP+8] = arg1       (data carried into FUN_00435970)
//   6. CALL FUN_00435970 (__thiscall; ECX = &buffer, [ESP+0] = this->field_4)
//   7. ADD ESP, 8            (undo the SUB; PUSH will be cleaned by RET 4)
//   8. RET 4                 (return and pop the one stack arg)
//
// FUN_00435970 (already matched, 89 B) is a virtual-dispatch guard: it calls
// vtable[0x170] on the passed object (this->field_4 of the outer caller) with
// buffer.data (arg1) as the second argument, and if the predicate returns
// non-zero it fires a lazily-bound assertion reporter.
//
// Reconstruction: naked-asm byte passthrough with a single linker-resolved
//   CALL relocation to FUN_00435970.  compare.py masks the 4-byte rel32
//   displacement at +0x1b so the .obj matches byte-for-byte.
//
// Asm (38 bytes @ orig RVA 0x000388f0):
//   83 ec 08                          SUB  ESP, 0x8
//   8b 49 04                          MOV  ECX, [ECX+0x4]
//   8b 44 24 0c                       MOV  EAX, [ESP+0xc]
//   51                                PUSH ECX
//   8d 4c 24 04                       LEA  ECX, [ESP+0x4]
//   c7 44 24 04 68 49 f6 00           MOV  dword ptr [ESP+0x4], 0xf64968
//   89 44 24 08                       MOV  dword ptr [ESP+0x8], EAX
//   e8 60 d0 ff ff                    CALL FUN_00435970   (rel32 reloc)
//   83 c4 08                          ADD  ESP, 0x8
//   c2 04 00                          RET  0x4

extern "C" void FUN_00435970();

extern "C" __declspec(naked) void __cdecl FUN_004388f0() {
    __asm {
        sub  esp, 8
        mov  ecx, dword ptr [ecx + 4]
        mov  eax, dword ptr [esp + 0xc]
        push ecx
        lea  ecx, [esp + 4]
        mov  dword ptr [esp + 4], 0xf64968
        mov  dword ptr [esp + 8], eax
        call FUN_00435970
        add  esp, 8
        ret  4
    }
}
