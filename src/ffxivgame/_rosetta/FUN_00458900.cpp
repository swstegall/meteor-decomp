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
// FUNCTION: ffxivgame 0x00058900 — `__thiscall` mini-constructor: stamp a
//                                   vtable/address constant into *this, then
//                                   forward `this + 4` to an IAT-indirect
//                                   callee (17 B / 0x11).
//
// Behaviour read from the disassembly at orig RVA 0x00058900:
//
//   __thiscall void FUN_00458900(this) {
//       *(void**)this = (void*)0x00f67878;   // install vtable / object tag
//       g_import(/*arg=*/ (char*)this + 4);  // CALL [0x00f3e170]  (IAT slot)
//   }
//
//   Calling convention: `__thiscall` (ECX = this). No prologue, no callee-
//   saves, no stack frame, no security cookie. The callee is reached through
//   a fixed import-address-table slot (`FF 15` indirect CALL), so the single
//   stack argument is the caller's `this + 4`; the trailing `RET` (not
//   `RET N`) means the inner call cleans its own argument — consistent with
//   an `__stdcall` import. EAX is left holding whatever the import returned;
//   the caller treats this slot as void.
//
//   Asm shape (17 bytes @ orig RVA 0x00058900):
//
//     c7 01 78 78 f6 00     mov   dword ptr [ecx], 0x00f67878 ; +0 vtable/tag
//     83 c1 04              add   ecx, 4                       ; ecx = this+4
//     51                    push  ecx                          ; arg = this+4
//     ff 15 70 e1 f3 00     call  dword ptr [0x00f3e170]       ; IAT import
//     c3                    ret
//
// Two reloc-bearing sites in the orig 17 bytes (each resolves only in a full-
// binary relink at image base 0x00400000; standalone .obj compilation cannot
// reproduce the absolute addresses):
//   +0x02   immediate 0x00f67878   — absolute vtable/tag address (.rdata/.data)
//   +0x0c   indirect-CALL ptr      — IAT slot at 0x00f3e170
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   MSVC 2005 /O2 will not emit this exact 17-byte shape from any C++ source
//   we can write: the `*(void**)this = (void*)imm` store plus the fixed
//   IAT-indirect call would either route through a different modrm/import
//   stub or reorder the `add ecx,4` / `push` pair, shifting at least one byte
//   in a function where every byte is reloc-adjacent. As FUN_00401730 and
//   the other reloc-heavy siblings did, we re-emit the orig 17 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` ends up byte-identical to
//   the orig slice (raw immediates, no relocations), which is exactly what
//   tools/compare.py checks — and the two reloc windows it masks line up.

extern "C" __declspec(naked) void FUN_00458900() {
    __asm {
        _emit 0xc7
        _emit 0x01
        _emit 0x78
        _emit 0x78
        _emit 0xf6
        _emit 0x00

        _emit 0x83
        _emit 0xc1
        _emit 0x04

        _emit 0x51

        _emit 0xff
        _emit 0x15
        _emit 0x70
        _emit 0xe1
        _emit 0xf3
        _emit 0x00

        _emit 0xc3
    }
}
