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
// FUNCTION: ffxivgame 0x00017b50 — __thiscall bump-allocator Alloc step
//   (90 B / 0x5a, no SEH, one internal CALL).
//
// Inspection (read from asm/ffxivgame/00017b50_FUN_00417b50.s):
//
//   __thiscall void* Alloc(int size, int align_shift);
//     // `this` in ECX; arg1=size at [ESP+4] on entry, arg2=align_shift at [ESP+8]
//
//   Struct layout (offsets the asm touches on `this`):
//     struct Arena {
//         /* ... */
//         void*   m_desc;    // +0x14 — pointer to buffer descriptor:
//                            //           [+0x00] base pointer (void*)
//                            //           [+0x04] capacity (int)
//         int     m_pos;     // +0x18 — current write offset (byte cursor)
//     };
//
//   Body:
//     // 1. Align the current cursor up to 2^align_shift boundary.
//     //    MSVC emits the alignment-delta idiom as:
//     //      neg  = NOT(m_pos) + 1  (= -m_pos, via NOT+ADD instead of NEG)
//     //      delta = neg - ((neg >> align_shift) << align_shift)
//     //            = neg & ((1 << align_shift) - 1)
//     //      m_pos += delta
//     int neg = ~this->m_pos + 1;         // NOT eax; ADD eax,1
//     this->m_pos += (neg ^ ((neg >> align_shift) << align_shift));
//
//     // 2. Align requested size the same way.
//     int neg2 = ~size + 1;
//     int aligned_size = size + (neg2 ^ ((neg2 >> align_shift) << align_shift));
//
//     // 3. Grow backing storage if cursor + aligned_size would exceed capacity.
//     if (this->m_pos + aligned_size >= *(int*)((char*)this->m_desc + 4))
//         FUN_00417970(this);             // grow / realloc
//
//     // 4. Compute return address (base + old cursor) and advance cursor.
//     void* result = (char*)*(void**)this->m_desc + this->m_pos;
//     this->m_pos += aligned_size;
//     return result;
//
// Calling convention: __thiscall — ECX = `this`; RET 8 pops 2 stack args.
// Return value (EAX): pointer into buffer at the pre-advance cursor.
//
// Reloc-bearing site:
//   +0x40   CALL FUN_00417970 (rel32, masked by tools/compare.py)
//
// Reconstruction strategy — inline MASM (no _emit):
//   All instructions are straightforward MOV/NOT/ADD/SHR/SHL/XOR/CMP/CALL/POP/RET;
//   MASM inline asm generates the exact encodings used by the orig binary.
//   The CALL to FUN_00417970 carries a COFF rel32 reloc which compare.py masks.

extern "C" void FUN_00417970();

extern "C" __declspec(naked) void FUN_00417b50() {
    __asm {
        push    ebx
        push    esi
        mov     esi, ecx
        mov     eax, dword ptr [esi + 0x18]
        mov     ecx, dword ptr [esp + 0x10]
        not     eax
        add     eax, 1
        mov     edx, eax
        shr     edx, cl
        push    edi
        mov     edi, dword ptr [esp + 0x10]
        shl     edx, cl
        xor     edx, eax
        add     dword ptr [esi + 0x18], edx
        mov     edx, dword ptr [esi + 0x18]
        mov     eax, edi
        not     eax
        add     eax, 1
        mov     ebx, eax
        shr     ebx, cl
        shl     ebx, cl
        xor     ebx, eax
        mov     eax, dword ptr [esi + 0x14]
        add     edi, ebx
        add     edx, edi
        cmp     edx, dword ptr [eax + 0x4]
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
        pop     ebx
        ret     8
    }
}
