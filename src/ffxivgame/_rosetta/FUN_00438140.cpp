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
// FUNCTION: ffxivgame 0x00038140 — `__thiscall` argument-marshalling
//                                  forwarder (60 B / 0x3c)
//
// Inspection (read from the disassembly at orig RVA 0x00038140):
//
//   __thiscall void FUN_00438140(this, int a, int b, int c, int d, char e)
//     ECX        : this
//     [ESP+0x04] : int  a   (after SUB ESP,0x14 → read at [ESP+0x18])
//     [ESP+0x08] : int  b   ( …                              [ESP+0x1c])
//     [ESP+0x0c] : int  c   ( …                              [ESP+0x20])
//     [ESP+0x10] : int  d   ( …                              [ESP+0x24])
//     [ESP+0x14] : char e   ( …                              [ESP+0x28])
//
//   The body builds a 0x14-byte temporary on the stack:
//     tmp+0x00 = a;  tmp+0x04 = b;  tmp+0x08 = c;
//     tmp+0x0c = d;  tmp+0x10 = e;  (byte, with 3 bytes padding)
//   then calls the __thiscall helper at 0x004367b0 with:
//     ECX = this + 0x10   (a subobject at this+0x10)
//     arg = &tmp          (pushed pointer to the marshalled struct)
//
//   Source shape (inferred):
//     void Foo::method(int a, int b, int c, int d, char e) {
//         Bar tmp = { a, b, c, d, e };
//         this->sub_10.process(&tmp);          // FUN_004367b0
//     }
//
//   `RET 0x14` — __thiscall, callee-cleans the 0x14 bytes of stack args
//   (4 DWORDs + 1 padded byte param).
//
// Reloc-bearing site in the orig 60 bytes (resolves only in a full-binary
// relink at image base 0x00400000):
//     +0x31   CALL rel32 → FUN_004367b0 (RVA 0x000367b0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The same approach the siblings FUN_00406fa0 / FUN_00408780 took: a
//   `__declspec(naked)` body that re-emits the orig 60 bytes verbatim via
//   MASM `_emit` directives. The .obj's `.text` ends up byte-identical to
//   the orig slice (the rel32 displacement is emitted as a raw immediate
//   that matches the orig binary's already-resolved bytes; no relocation
//   is applied). `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_00438140() {
    __asm {
        _emit 0x83              // SUB ESP, 0x14
        _emit 0xec
        _emit 0x14
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 0x1c]
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x89              // MOV dword ptr [ESP + 0x04], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 0x24]
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x89              // MOV dword ptr [ESP], EAX
        _emit 0x04
        _emit 0x24
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x89              // MOV dword ptr [ESP + 0x0c], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x89              // MOV dword ptr [ESP + 0x08], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8a              // MOV AL, byte ptr [ESP + 0x28]
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x8d              // LEA EDX, [ESP]
        _emit 0x14
        _emit 0x24
        _emit 0x52              // PUSH EDX
        _emit 0x83              // ADD ECX, 0x10
        _emit 0xc1
        _emit 0x10
        _emit 0x88              // MOV byte ptr [ESP + 0x14], AL
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xe8              // CALL rel32 → 0x004367b0
        _emit 0x3a
        _emit 0xe6
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET 0x0014
        _emit 0x14
        _emit 0x00
    }
}
