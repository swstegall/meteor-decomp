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
// FUNCTION: ffxivgame 0x005d18c9 — __thiscall init: EAX=this; AND-clear
//                                   [this+4] and [this+8]; store vtable ptr
//                                   at [this+0]; ret (17 B / 0x11).
//
// Context: sandwiched between CRT memmove_s and exception constructors in
//   the .text section. The vtable written at [this] (RVA 0x00c85cc8 in
//   .rdata) contains two entries: FUN_009d1a63 (virtual dtor) and
//   FUN_009d19ae (exception::what()), identifying this as the MSVC 2005
//   CRT exception class default-initialisation helper.
//
// Calling convention: __thiscall (ECX = this).  Returns this in EAX.
//   No prologue; no callee-saves; no stack args.
//
// Asm (17 bytes @ orig RVA 0x005d18c9):
//   8b c1                MOV  EAX, ECX
//   83 60 04 00          AND  DWORD PTR [EAX+4], 0      ; clear _Ptr field
//   83 60 08 00          AND  DWORD PTR [EAX+8], 0      ; clear _Data field
//   c7 00 [reloc32]      MOV  DWORD PTR [EAX], <vtable> ; install vtable ptr
//   c3                   RET
//
// The MOV at byte-offset +10 carries a DIR32 relocation pointing to the
// exception class vtable at .rdata:0x00c85cc8.  tools/compare.py masks
// the 4-byte imm payload, so the symbol name below is only a placeholder
// to generate the COFF relocation record.
//
// Reloc-bearing position (masked by tools/compare.py):
//   off 0x0c  IMAGE_REL_I386_DIR32  → exception_vtable

extern "C" int exception_vtable;   // placeholder — resolves the reloc at +12

extern "C" __declspec(naked) void FUN_009d18c9() {
    __asm {
        mov  eax, ecx
        and  dword ptr [eax + 4], 0
        and  dword ptr [eax + 8], 0
        mov  dword ptr [eax], OFFSET exception_vtable
        ret
    }
}
