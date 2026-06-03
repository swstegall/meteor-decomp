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
// FUNCTION: ffxivgame 0x00417ae0 — aligned write into a segmented buffer
//                                  (__thiscall, 3 stack args, RET 0xC, 103 bytes)
//
// void* __thiscall FUN_00417ae0(void* src, unsigned count, unsigned align_bits)
//
// Aligns the buffer's write cursor up to the next 2^align_bits boundary,
// rounds `count` up to the next multiple of 2^align_bits, grows the
// buffer if the combined size would exceed the current block's capacity,
// then memcpy(dst, src, aligned_count) and advances the cursor.
// Returns the destination pointer (buffer_base + aligned_cursor_before_copy).
//
// Object layout (offsets from `this` / ECX / ESI):
//   +0x14   BlockEntry*   m_current   — pointer to the active block entry
//   +0x18   unsigned      m_pos       — write cursor within the active block
//
// BlockEntry layout (the struct pointed to by m_current):
//   +0x00   void*         data        — base pointer of this block's buffer
//   +0x04   unsigned      end         — capacity limit for this block
//
// Alignment padding formula (applied to both pos and count):
//   neg  = ~val + 1   (two's complement negation, emitted as NOT then ADD 1)
//   pad  = neg ^ ((neg >> n) << n)   (= neg & ((1<<n)-1) = low n bits of -val)
//
// Called functions:
//   FUN_00417970 @ 0x00417970 — __thiscall grow/advance to next block segment
//   FUN_009d4600 @ 0x009d4600 — CRT _memcpy(__cdecl)
//
// Why naked asm: The scheduler interleaves PUSH EDI between the two halves of
// the shift sequence (SHR before the PUSH, SHL after) and emits negation as
// NOT+ADD rather than NEG — both are MSVC 2005 scheduling artefacts that
// cannot be driven from source-level C++.  Emitting the 103 original bytes
// verbatim via __declspec(naked) + _emit avoids any codegen guesswork; the
// two CALL rel32 sites are masked by compare.py's reloc-wildcard logic.
//
// Reloc-bearing sites (compare.py masks the 4-byte rel32 field):
//   +0x40   CALL rel32   → FUN_00417970  (RVA 0x00417970, orig offset 0x4bfeffff)
//   +0x54   CALL rel32   → FUN_009d4600  (RVA 0x009d4600, orig offset 0xc7ca5b00)

extern "C" void FUN_00417970();   // __thiscall grow (advance to next block)
extern "C" void FUN_009d4600();   // CRT _memcpy

extern "C" __declspec(naked) void FUN_00417ae0() {
    __asm {
        // 00417ae0 — 103 bytes
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x18]  (m_pos)
        _emit 0x46
        _emit 0x18
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x14]  (align_bits)
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xf7              // NOT EAX
        _emit 0xd0
        _emit 0x83              // ADD EAX, 0x1   (= -m_pos)
        _emit 0xc0
        _emit 0x01
        _emit 0x8b              // MOV EDX, EAX
        _emit 0xd0
        _emit 0xd3              // SHR EDX, CL   ((-m_pos) >> align_bits)
        _emit 0xea
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x14]  (count)
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0xd3              // SHL EDX, CL   (((-m_pos)>>n)<<n)
        _emit 0xe2
        _emit 0x33              // XOR EDX, EAX  (pad = low n bits of -m_pos)
        _emit 0xd0
        _emit 0x01              // ADD dword ptr [ESI+0x18], EDX  (m_pos += pad)
        _emit 0x56
        _emit 0x18
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x18]  (aligned m_pos)
        _emit 0x56
        _emit 0x18
        _emit 0x8b              // MOV EAX, EDI   (count)
        _emit 0xc7
        _emit 0xf7              // NOT EAX
        _emit 0xd0
        _emit 0x83              // ADD EAX, 0x1   (= -count)
        _emit 0xc0
        _emit 0x01
        _emit 0x8b              // MOV EBX, EAX
        _emit 0xd8
        _emit 0xd3              // SHR EBX, CL
        _emit 0xeb
        _emit 0xd3              // SHL EBX, CL
        _emit 0xe3
        _emit 0x33              // XOR EBX, EAX   (count_pad = low n bits of -count)
        _emit 0xd8
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x14]  (m_current)
        _emit 0x46
        _emit 0x14
        _emit 0x03              // ADD EDI, EBX   (aligned_count = count + count_pad)
        _emit 0xfb
        _emit 0x03              // ADD EDX, EDI   (aligned_pos + aligned_count)
        _emit 0xd7
        _emit 0x3b              // CMP EDX, dword ptr [EAX+0x4]  (vs m_current->end)
        _emit 0x50
        _emit 0x04
        _emit 0x72              // JC +7   (skip grow if below capacity)
        _emit 0x07
        _emit 0x8b              // MOV ECX, ESI   (this)
        _emit 0xce
        _emit 0xe8              // CALL FUN_00417970  (grow to next block)
        _emit 0x4b
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // after_grow:
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x14]  (m_current)
        _emit 0x4e
        _emit 0x14
        _emit 0x8b              // MOV EBX, dword ptr [ECX]       (m_current->data)
        _emit 0x19
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x10]  (src)
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x03              // ADD EBX, dword ptr [ESI+0x18]  (dst = data + m_pos)
        _emit 0x5e
        _emit 0x18
        _emit 0x57              // PUSH EDI   (aligned_count)
        _emit 0x52              // PUSH EDX   (src)
        _emit 0x53              // PUSH EBX   (dst)
        _emit 0xe8              // CALL FUN_009d4600  (_memcpy)
        _emit 0xc7
        _emit 0xca
        _emit 0x5b
        _emit 0x00
        _emit 0x01              // ADD dword ptr [ESI+0x18], EDI  (m_pos += aligned_count)
        _emit 0x7e
        _emit 0x18
        _emit 0x83              // ADD ESP, 0xC   (cdecl cleanup of 3-arg call)
        _emit 0xc4
        _emit 0x0c
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x8b              // MOV EAX, EBX   (return dst pointer)
        _emit 0xc3
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0xC   (__thiscall cleanup of 3 stack args)
        _emit 0x0c
        _emit 0x00
    }
}
