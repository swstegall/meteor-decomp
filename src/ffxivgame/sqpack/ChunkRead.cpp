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
// FUNCTION: ffxivgame 0x000ebd40 — Sqex::Data::ChunkRead<u32, u32>::ReadNextChunkHeader
//
// The base-class chunk-walker for the (chunk_id_size = u32, chunk_size = u32)
// instantiation. Reads the next chunk's 4-byte size header from cursor+4,
// optionally byte-swaps it (depending on m_flag15 — endianness flag),
// advances m_cursor by `size + 8` bytes (8 = chunk header overhead), and
// returns the new chunk count (m_field18+1) on success or -1 on EOF /
// out-of-bounds.
//
// Defined in a SEPARATE translation unit from PackRead.cpp so MSVC under
// /O2 doesn't observe that the callee preserves ECX and optimise away
// the `MOV ESI, ECX` save in PackRead::ReadNext (which would regress
// that GREEN match — orig has the save). See docs/sqpack.md §
// "TU split rationale" for the codegen recipe.

class ChunkReadUInt {
public:
    virtual ~ChunkReadUInt() {}
    int ReadNextChunkHeader();

protected:
    const char *m_data_start;     // +0x04
    const char *m_data_end;       // +0x08
    const char *m_cursor;         // +0x0c
    int         m_field10;        // +0x10
    char        m_pad14;          // +0x14
    char        m_flag15;         // +0x15 — endian flag
    short       m_pad16;          // +0x16
    int         m_field18;        // +0x18 — running chunk count
};

// Original bytes (81 B):
//   000ebd40: 83 ec 08 80 79 15 00 56 8b 71 0c 8b 46 04 89 44
//   000ebd50: 24 08 74 1e 0f b6 54 24 0b 88 54 24 04 0f b6 54
//   000ebd60: 24 0a 88 54 24 05 88 64 24 06 88 44 24 07 8b 44
//   000ebd70: 24 04 8d 44 06 08 3b 41 08 5e 72 07 83 c8 ff 83
//   000ebd80: c4 08 c3 83 41 18 01 89 41 0c 8b 41 18 83 c4 08
//   000ebd90: c3
//
// Decoded:
//   SUB  ESP, 8                          ; allocate 8-byte local
//   CMP  byte ptr [ECX+0x15], 0          ; flags-only — m_flag15 == 0?
//   PUSH ESI                              ; (CMP flags survive PUSH/MOV)
//   MOV  ESI, [ECX+0xc]                  ; ESI = m_cursor
//   MOV  EAX, [ESI+4]                    ; EAX = *(uint32 *)(cursor+4)
//   MOV  [ESP+8], EAX                    ; [ESP+8..0xb] = original size
//   JZ   skip_swap                        ; if (m_flag15 == 0) skip
//   ; --- byte-swap branch ---
//   MOVZX EDX, byte ptr [ESP+0xb]        ; high byte
//   MOV   [ESP+4], DL                    ; → new byte 0
//   MOVZX EDX, byte ptr [ESP+0xa]        ; high-mid
//   MOV   [ESP+5], DL                    ; → new byte 1
//   MOV   [ESP+6], AH                    ; AH still has mid-low byte
//   MOV   [ESP+7], AL                    ; AL still has low byte
//   MOV   EAX, [ESP+4]                   ; reload swapped value
//   skip_swap:
//   LEA  EAX, [ESI + EAX + 8]            ; next = cursor + size + 8
//   CMP  EAX, [ECX+8]                    ; vs m_data_end
//   POP  ESI
//   JC   ok                               ; jump if (unsigned) next < end
//   OR   EAX, 0xffffffff                  ; OOB → return -1
//   ADD  ESP, 8
//   RET
//   ok:
//   ADD  DWORD PTR [ECX+0x18], 1          ; ++m_field18
//   MOV  [ECX+0xc], EAX                   ; m_cursor = next
//   MOV  EAX, [ECX+0x18]                  ; return m_field18
//   ADD  ESP, 8
//   RET
//
// Iteration history:
//   #1 [PARTIAL — 74/81 bytes match (91%); 7 register-allocation
//      differences clustered in the LEA dest + cursor-store reorder]
//      MSVC chose EDX as the LEA destination (mine: `LEA EDX, [EAX+ESI+8]`)
//      where orig used EAX (`LEA EAX, [ESI+EAX+8]`). Mine kept the
//      result in EDX through the bounds check, then in the OK path
//      did `MOV EAX, field18; MOV [ECX+0xc], EDX`; orig did
//      `MOV [ECX+0xc], EAX; MOV EAX, field18`. Both correct, but the
//      register choice ripples through 7 byte positions.
//
//   Experiments tried to coax EAX:
//     (a) `size = (unsigned)(m_cursor + size + 8);` — got worse
//         (also flipped the SIB base/index encoding to 8 mismatches).
//     (b) Inverted condition `if (next < m_data_end) {success;} return -1;`
//         — got much worse (20 mismatches), flipped the JC/JNC and
//         reordered the OOB/OK paths.
//
//   The 91% partial is the closest with straightforward C++. Closing
//   the gap likely needs `__declspec(naked)` for the LEA + branch
//   sequence or a deeper MSVC register-allocator nudge.
int ChunkReadUInt::ReadNextChunkHeader() {
    unsigned size = *(unsigned *)(m_cursor + 4);
    if (m_flag15) {
        unsigned char swapped[4];
        unsigned char *src = (unsigned char *)&size;
        swapped[0] = src[3];
        swapped[1] = src[2];
        swapped[2] = src[1];
        swapped[3] = src[0];
        size = *(unsigned *)swapped;
    }
    const char *next = m_cursor + size + 8;
    if (next >= m_data_end) {
        return -1;
    }
    ++m_field18;
    m_cursor = next;
    return m_field18;
}
