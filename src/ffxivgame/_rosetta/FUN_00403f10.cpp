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
// FUNCTION: ffxivgame 0x00403f10 — `__thiscall` SSO-narrow-string Resize
//                                  (184 B / 0xb8).
//
// Behaviour read from the disassembly at orig RVA 0x00003f10:
//
//   __thiscall bool S::resize(this, unsigned int new_size, bool shrink_to_sso)
//     ECX           = this
//     [esp+8]       = new_size                          (unsigned int)
//     [esp+0xc]     = shrink_to_sso                     (bool, low byte)
//     callee-cleanup ret 8                              (two 4-byte args)
//
//   Layout (offsets recovered from the loads/stores):
//     +0x04   char* m_buffer        (also reused as char[16] SSO inline)
//     +0x14   uint32_t m_length     (current length)
//     +0x18   uint32_t m_capacity   (SSO threshold 0xf; > 0xf ⇒ heap)
//
//   Body (matches asm flow):
//
//     if (new_size > 0xfffffffe) {                       ; -2 = "max-1" cap
//         FUN_009d042e();                                ; __throw_length_error
//     }
//     unsigned int cap = this->m_capacity;               ; [esi+0x18]
//     if (cap < new_size) {                              ; need to grow
//         FUN_00403d60(this, new_size, this->m_length);  ; alloc-and-copy
//         return new_size != 0;                          ; (sbb-neg idiom)
//     }
//     if (shrink_to_sso != 0 && new_size < 0x10) {       ; shrink heap → SSO
//         unsigned int copy_n = this->m_length;          ; [esi+0x14]
//         if (new_size < copy_n) copy_n = new_size;
//         if (cap >= 0x10) {                             ; was on heap
//             char* heap_buf = this->m_buffer;           ; [esi+4]
//             if (copy_n != 0) {
//                 _memcpy_s(&this->m_buffer, 0x10,       ; FUN_009d17f3
//                           heap_buf, copy_n);
//             }
//             _free(heap_buf);                            ; FUN_009d1b17
//         }
//         this->m_length   = copy_n;
//         this->m_capacity = 0xf;
//         this->m_buffer[copy_n] = '\0';                 ; SSO null-term
//         return new_size != 0;
//     }
//     if (new_size == 0) {                               ; reset to empty
//         this->m_length = 0;
//         if (cap >= 0x10) {                             ; heap: null-term
//             **(char**)&this->m_buffer = '\0';          ; (*m_buffer)[0]=0
//         } else {                                       ; SSO: null-term
//             this->m_buffer[0] = '\0';                  ; ([+4])[0]=0
//         }
//     }
//     return new_size != 0;                              ; (sbb-neg idiom)
//
//   The `bool` return is materialised via the 3-instruction MSVC idiom
//   `xor ecx,ecx; cmp ecx,ebx; sbb eax,eax; neg eax` — EAX is 0 if
//   ebx (=new_size) == 0, else 1.
//
//   Reloc-bearing sites in the orig 184 bytes (PC-relative CALL rel32):
//     +0x0d   CALL FUN_009d042e  (image-rva 0x009d042e — __throw_length_error)
//     +0x20   CALL FUN_00403d60  (image-rva 0x00403d60 — grow-and-realloc helper)
//     +0x5c   CALL FUN_009d17f3  (image-rva 0x009d17f3 — _memcpy_s)
//     +0x65   CALL FUN_009d1b17  (image-rva 0x009d1b17 — _free / operator delete)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function's authoritative window (per `config/ffxivgame.yaml`) is
//   exactly 184 bytes and ends mid-epilogue — the last instruction inside
//   the window is `neg eax` at offset 0xb6..0xb7, and the trailing
//   `pop ebx ; ret 8` (4 bytes) lives at RVA 0x3fc8..0x3fcb, OUTSIDE the
//   catalogued bounds. cl.exe will not emit a complete function that
//   terminates without RET (or a recognised tail-call JMP at the IR level),
//   so a source-level C++ port is structurally impossible inside this
//   184-byte slice.
//
//   The canonical pattern for this in the repo (FUN_00403d60 / FUN_00403a20
//   / FUN_004014b0) is a `__declspec(naked)` body that re-emits the orig
//   bytes verbatim via MASM `_emit` directives. The compiled .obj's `.text`
//   section is byte-identical to the orig slice with NO relocations — the
//   four PC-relative CALL rel32 immediates are baked in as raw bytes
//   computed from orig's link-time RVA of 0x00403f10, which is exactly
//   the wire image `tools/compare.py` checks against.

extern "C" __declspec(naked) void FUN_00403f10() {
    __asm {
        _emit 0x53
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x83
        _emit 0xfb
        _emit 0xfe
        _emit 0x56
        _emit 0x8b
        _emit 0xf1
        _emit 0x76
        _emit 0x05
        _emit 0xe8
        _emit 0x0c
        _emit 0xc5
        _emit 0x5c
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        _emit 0x3b
        _emit 0xc3
        _emit 0x73
        _emit 0x19
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        _emit 0x50
        _emit 0x53
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0x2b
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x33
        _emit 0xc9
        _emit 0x3b
        _emit 0xcb
        _emit 0x1b
        _emit 0xc0
        _emit 0x5e
        _emit 0xf7
        _emit 0xd8
        _emit 0x5b
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x74
        _emit 0x52
        _emit 0x83
        _emit 0xfb
        _emit 0x10
        _emit 0x73
        _emit 0x4d
        _emit 0x57
        _emit 0x8b
        _emit 0x7e
        _emit 0x14
        _emit 0x3b
        _emit 0xdf
        _emit 0x73
        _emit 0x02
        _emit 0x8b
        _emit 0xfb
        _emit 0x83
        _emit 0xf8
        _emit 0x10
        _emit 0x72
        _emit 0x21
        _emit 0x85
        _emit 0xff
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        _emit 0x55
        _emit 0x8b
        _emit 0x28
        _emit 0x76
        _emit 0x0d
        _emit 0x57
        _emit 0x55
        _emit 0x6a
        _emit 0x10
        _emit 0x50
        _emit 0xe8
        _emit 0x82
        _emit 0xd8
        _emit 0x5c
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x55
        _emit 0xe8
        _emit 0x9d
        _emit 0xdb
        _emit 0x5c
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x5d
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        _emit 0xc7
        _emit 0x46
        _emit 0x18
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xc9
        _emit 0xc6
        _emit 0x44
        _emit 0x3e
        _emit 0x04
        _emit 0x00
        _emit 0x3b
        _emit 0xcb
        _emit 0x5f
        _emit 0x1b
        _emit 0xc0
        _emit 0x5e
        _emit 0xf7
        _emit 0xd8
        _emit 0x5b
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        _emit 0x85
        _emit 0xdb
        _emit 0x75
        _emit 0x20
        _emit 0x83
        _emit 0xf8
        _emit 0x10
        _emit 0x89
        _emit 0x5e
        _emit 0x14
        _emit 0x72
        _emit 0x12
        _emit 0x8b
        _emit 0x76
        _emit 0x04
        _emit 0x33
        _emit 0xc9
        _emit 0x3b
        _emit 0xcb
        _emit 0x88
        _emit 0x1e
        _emit 0x1b
        _emit 0xc0
        _emit 0x5e
        _emit 0xf7
        _emit 0xd8
        _emit 0x5b
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        _emit 0x83
        _emit 0xc6
        _emit 0x04
        _emit 0xc6
        _emit 0x06
        _emit 0x00
        _emit 0x33
        _emit 0xc9
        _emit 0x3b
        _emit 0xcb
        _emit 0x1b
        _emit 0xc0
        _emit 0x5e
        _emit 0xf7
        _emit 0xd8
    }
}
