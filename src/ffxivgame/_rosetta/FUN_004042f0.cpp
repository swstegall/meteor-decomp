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
// FUNCTION: ffxivgame 0x000042f0 — `__stdcall` SSO-narrow-string Reset/Clear
//                                  (42 B / 0x2a)
//
// Inspection (read from the orig .text slice at RVA 0x000042f0):
//
//   __stdcall void Reset(string* this)   — `this` arrives on the stack
//   at [esp+8] after the prologue PUSH ESI; epilogue is `ret 4`.
//
//   Layout (offsets derived from the loads/stores):
//     +0x04   char* m_buffer        (also reused as char[16] SSO inline)
//     +0x14   uint32_t m_length     (set to 0 on Reset)
//     +0x18   uint32_t m_capacity   (threshold 15; if >= 16 the buffer
//                                    was heap-allocated)
//
//   Body (matches asm flow):
//
//     if (this->m_capacity >= 16) {
//         free(this->m_buffer);
//     }
//     this->m_capacity = 15;
//     this->m_length   = 0;
//     *(uint8_t*)&this->m_buffer = 0;   // SSO null terminator (byte-wide)
//
//   The narrow-string twin of the wide-string Tidy at 0x00003fd0
//   (which uses 8-element SSO + `FUN_0044d350` for de-alloc); this
//   one uses 16-element SSO + the plain CRT `_free` thunk because the
//   buffer is owned by `malloc`/`free` directly rather than the
//   pool/type-tagged allocator.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The CALL site at +0xf carries a REL32 relocation to `_free` which
//   the .obj emits as a 4-byte zero fixup; `tools/compare.py` masks
//   reloc bytes out of the byte-level diff. Everything else is raw
//   bytes. Using MASM mnemonics through the MSVC inline assembler
//   matches the orig encoding byte-for-byte (the short-form JB / disp8
//   CMP / SIB-form [esp+8] / 89 46 14 store-eax all pick the same
//   encodings the original codegen used).

extern "C" int _free();

extern "C" __declspec(naked) void FUN_004042f0() {
    __asm {
        push esi
        mov esi, [esp+8]
        cmp dword ptr [esi+0x18], 0x10
        jb skip
        mov eax, [esi+4]
        push eax
        call _free
        add esp, 4
    skip:
        xor eax, eax
        mov dword ptr [esi+0x18], 0x0f
        mov dword ptr [esi+0x14], eax
        mov byte ptr [esi+4], al
        pop esi
        ret 4
    }
}
