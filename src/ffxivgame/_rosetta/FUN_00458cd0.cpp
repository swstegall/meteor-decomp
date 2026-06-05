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
// FUNCTION: ffxivgame 0x00058cd0 — pre-increment-and-return of a 32-bit
//                                  field at +0x8 of a pointer argument
//                                  (14 B / 0xe).
//
// Behaviour read from the disassembly at orig RVA 0x00058cd0:
//
//   8b 44 24 04        mov  eax, [esp+4]        ; load pointer arg
//   83 40 08 01        add  dword ptr [eax+8], 1 ; ++p->field_8
//   8b 40 08           mov  eax, [eax+8]        ; return the new value
//   c2 04 00           ret  4                    ; stdcall, 1 stack arg
//
//   No prologue, no frame, no callee-saves, no security cookie, and no
//   relocations — every byte is fixed. This is the classic
//   `__stdcall int f(T *p) { return ++p->counter; }` shape: increment the
//   dword at offset 0x8 in-place, then materialise the post-increment
//   value into EAX for the return.
//
// Reconstruction strategy — naked-asm byte passthrough (the local idiom
// for tiny reloc-free leaves): emit the orig 14 bytes verbatim so the
// .obj `.text` slice is byte-identical to the orig (what
// tools/compare.py checks). Promote to a real source-level
// `__stdcall` match once the owning struct is catalogued under
// decomp-notes/types/.

extern "C" __declspec(naked) void FUN_00458cd0() {
    __asm {
        mov eax, [esp + 4]
        add dword ptr [eax + 8], 1
        mov eax, [eax + 8]
        ret 4
    }
}
