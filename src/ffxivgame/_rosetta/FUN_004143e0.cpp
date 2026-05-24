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
// FUNCTION: ffxivgame 0x000143e0 — 7-byte `__thiscall` getter that initialises
// an embedded back-pointer at this+0x38 to `this`, then returns the address of
// the inline sub-object that begins at this+0x34. This is the canonical MSVC
// 2005 emission for a one-liner member like:
//
//     SubObject *Owner::GetSub() {
//         m_sub.parent = this;   // *(Owner**)(this+0x38) = this
//         return &m_sub;         // EAX = this+0x34
//     }
//
// where `m_sub` is laid out at offset 0x34 within `Owner` and carries an
// `Owner*` back-pointer at offset 4 within itself (= absolute offset 0x38).
// The pattern shows up across the SQEX::CDev::Engine::Memory::Alternative::
// family — see decomp-notes/types/ffxivgame/0x000139d0.md for the surrounding
// DetachableHeapBlock layout where embedded Link sub-sentinels live at +0x38
// and +0x44.
//
// Asm (7 bytes):
//   89 49 38        MOV   [ECX+0x38], ECX     ; sub.parent = this
//   8d 41 34        LEA   EAX, [ECX+0x34]     ; return &this->m_sub
//   c3              RET                        ; __thiscall, no stack args
//
// __thiscall member function, zero stack args, plain RET (not RET N).

extern "C" __declspec(naked) void FUN_004143e0() {
    __asm {
        mov dword ptr [ecx + 0x38], ecx
        lea eax, [ecx + 0x34]
        ret
    }
}
