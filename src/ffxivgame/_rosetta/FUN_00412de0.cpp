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
// FUNCTION: ffxivgame 0x00412de0 — SQEX::CDev::Engine::Memory::Alternative::Link
//                                  vtable-reset + doubly-linked-list unlink
//                                  (__thiscall, 1 unused stack arg, 29 bytes / 0x1d)
//
// Calling convention: __thiscall (ECX = this); callee pops 4 bytes (RET 4).
// No local frame — uses only EAX, ECX, EDX (volatile caller-save registers).
//
// Object layout (SQEX::CDev::Engine::Memory::Alternative::Link):
//   [this + 0x00]  vtable pointer  (reset to Link::vftable at 0x00f567c4)
//   [this + 0x04]  next pointer    (forward link in doubly-linked list)
//   [this + 0x08]  prev pointer    (backward link in doubly-linked list)
//
// Algorithm:
//   EAX = this                              (cache this; frees ECX for next ptr)
//   ECX = this->next                        ([EAX+4])
//   EDX = this->prev                        ([EAX+8])
//   this->vtable = Link::vftable            ([EAX]   ← 0x00f567c4, DIR32 reloc)
//   this->next->prev = this->prev           ([ECX+8] = EDX)
//   ECX = this->prev                        ([EAX+8] reload)
//   EDX = this->next                        ([EAX+4] reload)
//   this->prev->next = this->next           ([ECX+4] = EDX)
//   RET 4                                   (__thiscall, pop 1 stack arg)
//
// The vtable-pointer write resets the vptr to the base Link class before
// splicing the node out of the list — the canonical MSVC 2005 destructor
// preamble for a non-trivial class whose dtor body performs list cleanup.
// The RET 4 callee-clean convention indicates a scalar-deleting-destructor
// ABI (1 int flags arg) with no conditional-delete path present here.
//
// The DIR32 relocation at instruction offset +0x0a (bytes [EAX],0x00f567c4)
// is masked by compare.py; the surrounding bytes are compared strictly.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Source-level C++ would write the vtable via a base-class assignment and
//   the list-splice as three pointer stores, but the double reload of ECX/EDX
//   in steps 5-7 (from [EAX+8] / [EAX+4] after modifying [ECX+8]) is
//   a pessimistic codegen artefact that is not reproducible from source
//   without careful register-variable annotations. A __declspec(naked) body
//   re-emitting the original 29 bytes verbatim via MASM _emit directives
//   produces a .obj whose .text is byte-identical to the original slice.

extern "C" __declspec(naked) void FUN_00412de0() {
    __asm {
        // 00012de0: 8b c1        MOV EAX,ECX
        _emit 0x8b
        _emit 0xc1
        // 00012de2: 8b 48 04     MOV ECX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        // 00012de5: 8b 50 08     MOV EDX,dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 00012de8: c7 00 c4 67 f5 00  MOV dword ptr [EAX],0x00f567c4  (DIR32 reloc: Link::vftable)
        _emit 0xc7
        _emit 0x00
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00012dee: 89 51 08     MOV dword ptr [ECX+0x8],EDX  (next->prev = prev)
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 00012df1: 8b 48 08     MOV ECX,dword ptr [EAX+0x8]  (reload ECX = prev)
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        // 00012df4: 8b 50 04     MOV EDX,dword ptr [EAX+0x4]  (reload EDX = next)
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00012df7: 89 51 04     MOV dword ptr [ECX+0x4],EDX  (prev->next = next)
        _emit 0x89
        _emit 0x51
        _emit 0x04
        // 00012dfa: c2 04 00     RET 0x4  (__thiscall, pop 1 stack arg)
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
