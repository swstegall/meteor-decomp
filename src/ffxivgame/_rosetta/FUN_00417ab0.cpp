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
// FUNCTION: ffxivgame 0x00417ab0 — unaligned write-cursor advance in segmented
//                                  buffer (__thiscall, 1 stack arg, RET 0x4, 48 bytes)
//
// void* __thiscall FUN_00417ab0(unsigned n)
//
// Claims n bytes in the segmented buffer: if (m_pos + n) >= m_current->end,
// calls the grow helper to advance to the next block segment; then returns a
// pointer to the start of the claimed region (m_current->data + m_pos) and
// advances m_pos by n.
// The simpler, un-aligned sibling of FUN_00417ae0.
//
// Object layout (offsets from `this` / ECX / ESI):
//   +0x14   BlockEntry*   m_current   — pointer to the active block entry
//   +0x18   unsigned      m_pos       — write cursor within the active block
//
// BlockEntry layout (the struct pointed to by m_current):
//   +0x00   void*         data        — base pointer of this block's buffer
//   +0x04   unsigned      end         — capacity limit for this block
//
// Called functions:
//   FUN_00417970 @ 0x00417970 — __thiscall grow/advance to next block segment
//
// Reconstruction: naked asm with labeled branches. The CALL rel32 site is
// masked by compare.py's reloc-wildcard logic.
//
// Reloc-bearing sites:
//   +0x17   CALL rel32   → FUN_00417970  (RVA 0x00417970, orig e8 a4 fe ff ff)

extern "C" void FUN_00417970();   // __thiscall grow (advance to next block)

extern "C" __declspec(naked) void FUN_00417ab0() {
    __asm {
        push    esi
        mov     esi, ecx
        mov     eax, dword ptr [esi + 0x18]
        mov     ecx, dword ptr [esi + 0x14]
        push    edi
        mov     edi, dword ptr [esp + 0xc]
        add     eax, edi
        cmp     eax, dword ptr [ecx + 0x4]
        jc      skip_grow
        mov     ecx, esi
        call    FUN_00417970
    skip_grow:
        mov     ecx, dword ptr [esi + 0x18]
        mov     edx, dword ptr [esi + 0x14]
        mov     eax, dword ptr [edx]
        add     eax, ecx
        add     ecx, edi
        pop     edi
        mov     dword ptr [esi + 0x18], ecx
        pop     esi
        ret     4
    }
}
