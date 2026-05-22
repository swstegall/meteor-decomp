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
// FUNCTION: ffxivgame 0x0000eeb0 — engine_memory block-bucket free-list
// allocator: `__thiscall void * alloc(this, requested_size, int *alignment_p,
// owner_a, owner_b)` (4 stack args + ECX=this; `ret 0x10`).
//
// Walks the size-class bucket head's doubly-linked free list (lookup
// via `bsr(requested_size_padded) + 1`) and finds the first free block
// (high-bit-clear flags word) whose payload satisfies the requested
// size *after* enforcing the caller's alignment. On a hit:
//
//   1. Unlinks the block from its current free-list.
//   2. If the alignment forces a non-zero leading gap, splits the
//      block in two: a new "head remainder" free chunk (re-inserted
//      into the matching size-class bucket) and the aligned payload.
//   3. If the aligned remainder is larger than the request, splits
//      again: keeps the requested size for the caller, re-inserts the
//      trailing remainder into its matching bucket.
//   4. Marks the resulting block as in-use (sets flags bit 0x80000000),
//      stamps the two owner pointers (`+0x8` / `+0xc`) and the payload
//      size (`+0x0`).
//   5. If the global "fill new allocations with 0xAA" debug switch is
//      set (`*(uint8_t *)(owner_a + 0x14) != 0`), memset()s the
//      payload to 0xAA before returning.
//
// Block header layout (16 bytes):
//
//   +0x00  uint32  payload_size  (bytes of payload after the header)
//   +0x04  uint32  flags | encoded_bucket_idx  (high bit = in-use)
//   +0x08  uint32  next  (in free list while free; owner_b while busy)
//   +0x0c  uint32  prev  (in free list while free; owner_a while busy)
//
// Each free block also carries a trailing footer at `header + 0x10 +
// payload_size + 0x14` (i.e. last 4 bytes of payload) which is the
// previous-block-size XOR'd with a sentinel — used by the merge path
// (not exercised in this function) to verify physical-neighbor links.
//
// The bucket head sentinel ring lives at `this + 4`. The per-bucket
// list head is at `this + 0xc + bucket_idx * 0x10`. (Each bucket entry
// is 16 bytes — head/tail/etc. — matched by the `shl 4` index.)
//
// Reloc-masked positions (compare.py ignores these 4-byte fields):
//   +0x1a5  REL32 → _memset (runtime memset at 0x009d2110)

// memset symbol at RVA 0x009d2110. Declaring extern "C" __cdecl without
// __declspec(dllimport) so the call site gets a direct e8 REL32, which
// the linker resolves to the orig memset address. The symbol name
// itself doesn't matter for byte-matching — compare.py masks the 4
// reloc bytes — but we keep the canonical "_memset_fn" name used by
// other engine_memory rosetta translations for consistency.
extern "C" void *__cdecl _memset_fn(void *dst, int c, unsigned int n);

#pragma code_seg(".text$X0000eeb0")

extern "C" __declspec(naked) void FUN_0040eeb0()
{
    __asm {
        // --- prologue: save ECX (stack-slot 'this'), EBX/EBP/EDI ------
        push    ecx                                  // 51
        mov     edx, dword ptr [esp + 0x10]          // 8b 54 24 10  edx = param_3 (alignment_p)
        push    ebx                                  // 53
        mov     ebx, dword ptr [esp + 0x10]          // 8b 5c 24 10  ebx = param_2 (size)
        push    ebp                                  // 55
        push    edi                                  // 57
        mov     edi, dword ptr [edx]                 // 8b 3a        edi = *alignment_p
        add     ebx, 0xf                             // 83 c3 0f
        add     edi, 0xf                             // 83 c7 0f
        and     ebx, 0xfffffff0                      // 83 e3 f0     ebx = ROUND_UP(size, 16)
        xor     eax, eax                             // 33 c0        eax = 0 (default return)
        and     edi, 0xfffffff0                      // 83 e7 f0     edi = ROUND_UP(*align, 16)
        bsr     edx, ebx                             // 0f bd d3
        mov     dword ptr [esp + 0xc], ecx           // 89 4c 24 0c  spill 'this' to local slot
        lea     ebp, [ecx + 4]                       // 8d 69 04     ebp = &this->bucket_head_sentinel
        je      L_size_zero                          // 74 05
        add     edx, 1                               // 83 c2 01
        jmp     L_after_size_bsr                     // eb 02
    L_size_zero:
        xor     edx, edx                             // 33 d2
    L_after_size_bsr:
        shl     edx, 4                               // c1 e2 04
        mov     edx, dword ptr [edx + ecx + 0xc]     // 8b 54 0a 0c  edx = bucket[idx].head
        cmp     edx, ebp                             // 3b d5
        je      L_return                             // 0f 84 71 01 00 00  bucket empty

        push    esi                                  // 56
        nop                                          // 90  (loop-top align padding)

        // --- walk the free list looking for a fitting block ----------
    L_walk:
        mov     ecx, dword ptr [edx + 4]             // 8b 4a 04     ecx = blk->flags
        shr     ecx, 0x1f                            // c1 e9 1f
        test    cl, 1                                // f6 c1 01     skip if in-use bit set
        jne     L_walk_next                          // 75 13
        mov     esi, edi                             // 8b f7
        sub     esi, edx                             // 2b f2
        sub     esi, 0x10                            // 83 ee 10
        lea     ecx, [edi - 1]                       // 8d 4f ff     align mask
        and     esi, ecx                             // 23 f1
        lea     ecx, [esi + ebx]                     // 8d 0c 1e
        cmp     dword ptr [edx], ecx                 // 39 0a        blk->size >= gap + req?
        jae     L_walk_hit                           // 73 0f
    L_walk_next:
        mov     edx, dword ptr [edx + 8]             // 8b 52 08     edx = blk->next
        cmp     edx, ebp                             // 3b d5
        jne     L_walk                               // 75 db

        // bucket-end reached without finding — return eax (still 0).
        pop     esi                                  // 5e
        pop     edi                                  // 5f
        pop     ebp                                  // 5d
        pop     ebx                                  // 5b
        pop     ecx                                  // 59
        ret     0x10                                 // c2 10 00

    L_walk_hit:
        // --- unlink edx from its current free-list -------------------
        mov     eax, dword ptr [edx + 0xc]           // 8b 42 0c     eax = blk->prev
        mov     ecx, dword ptr [edx + 8]             // 8b 4a 08     ecx = blk->next
        mov     dword ptr [eax + 8], ecx             // 89 48 08     prev->next = next
        mov     eax, dword ptr [edx + 8]             // 8b 42 08
        mov     ecx, dword ptr [edx + 0xc]           // 8b 4a 0c
        mov     dword ptr [eax + 0xc], ecx           // 89 48 0c     next->prev = prev

        mov     ebp, dword ptr [edx + 4]             // 8b 6a 04     ebp = blk->flags (& mask)
        and     ebp, 0x7fffffff                      // 81 e5 ff ff ff 7f

        test    esi, esi                             // 85 f6
        mov     edi, edx                             // 8b fa        edi = original block start
        je      L_no_head_split                      // 74 72

        // --- alignment forces a leading gap; split off a head chunk --
        lea     edi, [edx + esi]                     // 8d 3c 32     edi = aligned payload header
        test    edi, edi                             // 85 ff
        lea     ebp, [esi - 0x10]                    // 8d 6e f0     ebp = gap - 16 = new size of head chunk
        je      L_head_zero                          // 74 15
        mov     dword ptr [edi + 0xc], edi           // 89 7f 0c     init self-loops & zeros
        mov     dword ptr [edi + 8], edi             // 89 7f 08
        mov     dword ptr [edi + 4], 0               // c7 47 04 00 00 00 00
        mov     dword ptr [edi], 0                   // c7 07 00 00 00 00
        jmp     L_head_zero_done                     // eb 02
    L_head_zero:
        xor     edi, edi                             // 33 ff
    L_head_zero_done:
        mov     eax, dword ptr [edx]                 // 8b 02        eax = original blk->size
        mov     ecx, dword ptr [edi + 4]             // 8b 4f 04
        xor     ecx, ebp                             // 33 cd
        sub     eax, esi                             // 2b c6
        and     ecx, 0x7fffffff                      // 81 e1 ff ff ff 7f
        xor     dword ptr [edi + 4], ecx             // 31 4f 04     stamp aligned blk flags = gap-encoded
        mov     dword ptr [edi], eax                 // 89 07        aligned blk->size = orig - gap
        mov     ecx, dword ptr [eax + edi + 0x14]    // 8b 4c 38 14  footer @ (size + 0x14)
        lea     esi, [eax + edi + 0x14]              // 8d 74 38 14
        xor     ecx, eax                             // 33 c8
        and     ecx, 0x7fffffff                      // 81 e1 ff ff ff 7f
        xor     dword ptr [esi], ecx                 // 31 0e        stamp neighbor footer
        bsr     eax, ebp                             // 0f bd c5
        mov     dword ptr [edx], ebp                 // 89 2a        original blk->size = gap - 16
        je      L_head_bsr_zero                      // 74 05
        add     eax, 1                               // 83 c0 01
        jmp     L_head_bsr_done                      // eb 02
    L_head_bsr_zero:
        xor     eax, eax                             // 33 c0
    L_head_bsr_done:
        mov     ecx, dword ptr [esp + 0x10]          // 8b 4c 24 10  reload this from stack slot
        shl     eax, 4                               // c1 e0 04
        lea     eax, [eax + ecx + 4]                 // 8d 44 08 04  eax = &bucket[idx]
        mov     ecx, dword ptr [eax + 8]             // 8b 48 08
        mov     dword ptr [ecx + 0xc], edx           // 89 51 0c
        mov     ecx, dword ptr [eax + 8]             // 8b 48 08
        mov     dword ptr [edx + 8], ecx             // 89 4a 08
        mov     dword ptr [edx + 0xc], eax           // 89 42 0c
        mov     dword ptr [eax + 8], edx             // 89 50 08

    L_no_head_split:
        // --- tail split? blk->size > requested → carve out a remainder
        cmp     dword ptr [edi], ebx                 // 39 1f
        jbe     L_no_tail_split                      // 76 77

        lea     eax, [edi + ebx + 0x10]              // 8d 44 1f 10  eax = tail-remainder header
        test    eax, eax                             // 85 c0
        je      L_tail_zero                          // 74 15
        mov     dword ptr [eax + 0xc], eax           // 89 40 0c
        mov     dword ptr [eax + 8], eax             // 89 40 08
        mov     dword ptr [eax + 4], 0               // c7 40 04 00 00 00 00
        mov     dword ptr [eax], 0                   // c7 00 00 00 00 00
        jmp     L_tail_zero_done                     // eb 02
    L_tail_zero:
        xor     eax, eax                             // 33 c0
    L_tail_zero_done:
        mov     edx, dword ptr [edi]                 // 8b 17        edx = aligned blk->size
        mov     ecx, dword ptr [eax + 4]             // 8b 48 04
        xor     ecx, ebx                             // 33 cb
        sub     edx, ebx                             // 2b d3
        sub     edx, 0x10                            // 83 ea 10
        and     ecx, 0x7fffffff                      // 81 e1 ff ff ff 7f
        xor     dword ptr [eax + 4], ecx             // 31 48 04
        mov     dword ptr [eax], edx                 // 89 10        tail blk->size
        mov     ecx, dword ptr [edx + eax + 0x14]    // 8b 4c 02 14
        lea     esi, [edx + eax + 0x14]              // 8d 74 02 14
        xor     ecx, edx                             // 33 ca
        and     ecx, 0x7fffffff                      // 81 e1 ff ff ff 7f
        xor     dword ptr [esi], ecx                 // 31 0e        stamp neighbor footer
        bsr     ecx, dword ptr [eax]                 // 0f bd 08
        je      L_tail_bsr_zero                      // 74 05
        add     ecx, 1                               // 83 c1 01
        jmp     L_tail_bsr_done                      // eb 02
    L_tail_bsr_zero:
        xor     ecx, ecx                             // 33 c9
    L_tail_bsr_done:
        mov     edx, dword ptr [esp + 0x10]          // 8b 54 24 10  reload this
        shl     ecx, 4                               // c1 e1 04
        lea     ecx, [ecx + edx + 4]                 // 8d 4c 11 04  &bucket[idx]
        mov     edx, dword ptr [ecx]                 // 8b 11
        mov     ecx, dword ptr [edx + ecx + 0x1c]    // 8b 4c 0a 1c
        mov     edx, dword ptr [ecx + 8]             // 8b 51 08
        mov     dword ptr [edx + 0xc], eax           // 89 42 0c
        mov     edx, dword ptr [ecx + 8]             // 8b 51 08
        mov     dword ptr [eax + 8], edx             // 89 50 08
        mov     dword ptr [eax + 0xc], ecx           // 89 48 0c
        mov     dword ptr [ecx + 8], eax             // 89 41 08

    L_no_tail_split:
        // --- stamp allocated block, set in-use bit, return payload ---
        mov     eax, dword ptr [esp + 0x18]          // 8b 44 24 18  param_1 (owner_a)
        mov     ecx, dword ptr [esp + 0x24]          // 8b 4c 24 24  param_4 (owner_b)
        or      ebp, 0x80000000                      // 81 cd 00 00 00 80
        mov     dword ptr [edi + 0xc], eax           // 89 47 0c
        mov     dword ptr [edi + 8], ecx             // 89 4f 08
        mov     dword ptr [edi + 4], ebp             // 89 6f 04
        mov     dword ptr [edi], ebx                 // 89 1f
        add     edi, 0x10                            // 83 c7 10     edi = payload

        cmp     byte ptr [eax + 0x14], 0             // 80 78 14 00
        je      L_no_fill                            // 74 0f
        push    ebx                                  // 53          memset(payload, 0xAA, size)
        push    0xaa                                 // 68 aa 00 00 00
        push    edi                                  // 57
        call    _memset_fn                           // e8 <REL32>
        add     esp, 0xc                             // 83 c4 0c
    L_no_fill:
        mov     eax, edi                             // 8b c7
        pop     esi                                  // 5e
    L_return:
        pop     edi                                  // 5f
        pop     ebp                                  // 5d
        pop     ebx                                  // 5b
        pop     ecx                                  // 59
        ret     0x10                                 // c2 10 00
    }
}

#pragma code_seg()

// vim: ts=4 sts=4 sw=4 et
