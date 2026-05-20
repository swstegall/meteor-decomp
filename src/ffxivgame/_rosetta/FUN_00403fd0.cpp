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
// FUNCTION: ffxivgame 0x00003fd0 — `__thiscall` SSO-wide-string Reset/Clear
//                                  (48 B / 0x30)
//
// Inspection (read from the orig .text slice at RVA 0x00003fd0):
//
//   __thiscall void Reset(this) — `ECX = this`, no args, no return.
//
//   Layout (offsets derived from the loads/stores):
//     +0x04   void* m_buffer        (also reused as wchar_t[8] SSO inline)
//     +0x14   uint32_t m_length     (set to 0 on Reset)
//     +0x18   uint32_t m_capacity   (threshold 7; if > 7 the buffer was heap-allocated)
//
//   Body (matches asm flow):
//
//     if (this->m_capacity >= 8) {
//         FUN_0044d350(this->m_buffer, this->m_capacity * 2 + 2, 0xc);
//     }
//     this->m_capacity = 7;
//     this->m_length = 0;
//     *(uint16_t*)&this->m_buffer = 0;   // SSO null terminator
//
//   The `capacity * 2 + 2` argument is the heap allocation byte size for
//   `m_capacity + 1` wchar_t (UTF-16) slots — confirming the SSO buffer
//   is wchar_t-shaped. 0x0c is the deallocator pool/type token.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The CALL site at +0x16 carries a REL32 relocation to FUN_0044d350
//   which the .obj emits as a 4-byte zero fixup; `tools/compare.py`
//   masks reloc bytes out of the diff. Everything else is raw bytes.
//   Using MASM mnemonics through the inline assembler matches the orig
//   encoding byte-for-byte (MSVC's inline-assembler picks the same
//   short-form LEA / JB encodings the original codegen used).

extern "C" int FUN_0044d350();

extern "C" __declspec(naked) void FUN_00403fd0() {
    __asm {
        push esi
        mov esi, ecx
        mov eax, [esi+0x18]
        cmp eax, 8
        jb skip
        mov ecx, [esi+4]
        push 0x0c
        lea eax, [eax+eax+2]
        push eax
        push ecx
        call FUN_0044d350
        add esp, 0x0c
    skip:
        xor eax, eax
        mov dword ptr [esi+0x18], 7
        mov dword ptr [esi+0x14], eax
        mov word ptr [esi+4], ax
        pop esi
        ret
    }
}
