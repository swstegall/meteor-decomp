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
// FUNCTION: ffxivgame 0x000041f0 — `__thiscall` SSO narrow-string Reset/Clear
//                                  (46 B / 0x2e)
//
// Inspection (read from the orig .text slice at RVA 0x000041f0):
//
//   __thiscall void Reset(this) — `ECX = this`, no args, no return.
//
//   Layout (offsets derived from the loads/stores):
//     +0x04   void* m_buffer        (also reused as char[16] SSO inline)
//     +0x14   uint32_t m_length     (set to 0 on Reset)
//     +0x18   uint32_t m_capacity   (threshold 15; if > 15 the buffer was heap-allocated)
//
//   Body (matches asm flow):
//
//     if (this->m_capacity > 15) {
//         FUN_0044d350(this->m_buffer, this->m_capacity + 1, 0xc);
//     }
//     this->m_capacity = 15;
//     this->m_length = 0;
//     *(uint8_t*)&this->m_buffer = 0;   // SSO null terminator
//
//   The `capacity + 1` argument is the heap allocation byte size for
//   `m_capacity + 1` char slots — the narrow-char analog of the
//   wchar_t version at 0x00403fd0. 0x0c is the deallocator pool/type
//   token.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The CALL site at +0x15 carries a REL32 relocation to FUN_0044d350
//   which the .obj emits as a 4-byte zero fixup; `tools/compare.py`
//   masks reloc bytes out of the diff. Everything else is raw bytes.

extern "C" int FUN_0044d350();

extern "C" __declspec(naked) void FUN_004041f0() {
    __asm {
        push esi
        mov esi, ecx
        mov eax, [esi+0x18]
        cmp eax, 0x10
        jb skip
        add eax, 1
        push 0x0c
        push eax
        mov eax, [esi+4]
        push eax
        call FUN_0044d350
        add esp, 0x0c
    skip:
        xor eax, eax
        mov dword ptr [esi+0x18], 0x0f
        mov dword ptr [esi+0x14], eax
        mov byte ptr [esi+4], al
        pop esi
        ret
    }
}
