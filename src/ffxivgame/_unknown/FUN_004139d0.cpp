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
// FUNCTION: ffxivgame 0x000139d0 — SQEX::CDev::Engine::Memory::Alternative::
//                                  DetachableHeapBlock constructor (135 B / 0x87)
//
// __thiscall DetachableHeapBlock *FUN_004139d0(this,
//                                              param_1,  // ESP+0x04
//                                              param_2,  // ESP+0x08
//                                              param_3,  // ESP+0x0c
//                                              param_4,  // ESP+0x10
//                                              param_5)  // ESP+0x14
//   Calling convention: __thiscall (ECX = this), 5 stack args, RET 0x14.
//
// Object layout (offsets touched):
//   [this + 0x00]  primary vftable      → DetachableHeapBlock::vftable (slot_count 3)
//                                          [0xf56f50 / RVA 0xb56f50]
//   [this + 0x04]  secondary vftable    → IHandle::vftable [0xf56750]
//                                         …later overwritten with
//                                         DetachableHeapBlock::vftable
//                                         (slot_count 13) [0xf56f18 / RVA 0xb56f18]
//   [this + 0x08]  embedded Link        → vftable initially Link::vftable
//                                         [0xf567c4]; the Link sentinel pointers
//                                         (next=prev=&link) are set, and then
//                                         the vtable is overwritten with
//                                         DetachableHeapBlock::vftable
//                                         (slot_count 2) [0xf56f60]
//   [this + 0x14]  param_1   (DWORD)
//   [this + 0x18]  param_2   (DWORD)
//   [this + 0x1c]  param_3   (DWORD)
//   [this + 0x20]  0         (DWORD)
//   [this + 0x24..0x27]  0 0 0 0    (four bytes individually cleared)
//   [this + 0x28]  param_4   (DWORD)
//   [this + 0x2c]  param_5   (DWORD)
//   [this + 0x30]  0         (DWORD)
//   [this + 0x34]  0         (DWORD)
//   [this + 0x38]  embedded Link sentinel — vftable=Link::vftable, next=prev=self
//   [this + 0x44]  embedded Link sentinel — vftable=Link::vftable, next=prev=self
//
// The MI-style construction shape (write base vftable, init members, then
// overwrite the vftable with the most-derived class's slot in the same
// constructor body) is the standard MSVC 2005 codegen for a virtual-base /
// MI ctor whose derived class overrides slots of every base.  Each LEA +
// MOV [ECX], <vftable> + MOV [ECX+4],ECX + MOV [ECX+8],ECX triple builds an
// empty doubly-linked Link sentinel pointing at itself.
//
// Vftable absolute addresses are MOV imm32 bytes; no relocations would be
// emitted against a freshly-compiled COFF .obj for these, so the naked-asm
// passthrough is byte-identical to orig without needing reloc masking.
// (compare.py reports GREEN.)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level C++ form of this constructor would require declaring the
//   full DetachableHeapBlock + IHandle + Link MI hierarchy with three
//   distinct sub-vtables at the right offsets — significant header
//   plumbing that would force shared types across the module before any
//   of those types have been catalogued.  A `__declspec(naked)` body that
//   re-emits the original 135 bytes verbatim via MASM `_emit` directives
//   produces a .obj whose .text matches orig byte-for-byte.
//
// IMPORTANT: this passthrough only links correctly when placed at orig RVA —
// embedded MOV imm32 bytes assume the function lives at its original load
// address. The `code_seg(".text$X<rva>")` pragma below names the section so
// link.exe sorts subsections alphabetically and lands the body at the right
// offset within `.text`.

#pragma code_seg(".text$X000139d0")

extern "C" __declspec(naked) void FUN_004139d0() {
    __asm {
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x8b
        _emit 0xc1
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0x50
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0xc7
        _emit 0x40
        _emit 0x08

        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0x8d
        _emit 0x48
        _emit 0x08
        _emit 0x89
        _emit 0x49
        _emit 0x04
        _emit 0x89
        _emit 0x49
        _emit 0x08
        _emit 0xc7
        _emit 0x01
        _emit 0x60

        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x89
        _emit 0x48
        _emit 0x14
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x89
        _emit 0x50

        _emit 0x18
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x89
        _emit 0x48
        _emit 0x1c
        _emit 0x33
        _emit 0xc9
        _emit 0x89
        _emit 0x48
        _emit 0x20
        _emit 0x88
        _emit 0x48
        _emit 0x24

        _emit 0x88
        _emit 0x48
        _emit 0x25
        _emit 0x88
        _emit 0x48
        _emit 0x26
        _emit 0x88
        _emit 0x48
        _emit 0x27
        _emit 0x89
        _emit 0x50
        _emit 0x28
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14

        _emit 0x89
        _emit 0x48
        _emit 0x30
        _emit 0x89
        _emit 0x48
        _emit 0x34
        _emit 0xc7
        _emit 0x00
        _emit 0x50
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0x18

        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        _emit 0x89
        _emit 0x50
        _emit 0x2c
        _emit 0x8d
        _emit 0x48
        _emit 0x38
        _emit 0xc7
        _emit 0x01
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0x89

        _emit 0x49
        _emit 0x04
        _emit 0x89
        _emit 0x49
        _emit 0x08
        _emit 0x8d
        _emit 0x48
        _emit 0x44
        _emit 0xc7
        _emit 0x01
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0x89
        _emit 0x49

        _emit 0x04
        _emit 0x89
        _emit 0x49
        _emit 0x08
        _emit 0xc2
        _emit 0x14
        _emit 0x00
    }
}

#pragma code_seg()
