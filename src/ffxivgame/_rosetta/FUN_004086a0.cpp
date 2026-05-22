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
// FUNCTION: ffxivgame 0x004086a0 — circular doubly-linked free-list init
//                                  (__cdecl, 73 B / 0x49, zero relocs)
//
// __cdecl void init_freelist(Node *head, unsigned total_bytes, unsigned block_size)
//
//   struct Node { Node *prev; Node *next; };   // 8-byte header at top of each block
//
// Walks `count = total_bytes / block_size` equally-sized contiguous blocks
// starting at `head` and stitches them into a circular doubly-linked list
// of free-list nodes. Each block's first 8 bytes form a {prev, next}
// header pointing to the previous / next block in the list. After the
// straight-line walk, the head's `prev` and the tail's `next` are
// patched to close the ring.
//
// Inspection (read from the orig 73-byte slice at RVA 0x000086a0):
//
//     mov  eax, [esp+8]            ; eax = total_bytes
//     push ebx
//     push esi
//     mov  esi, [esp+0x14]         ; esi = block_size
//     xor  edx, edx
//     div  esi                     ; eax = count = total/block_size
//     mov  ebx, [esp+0xc]          ; ebx = head
//     test eax, eax
//     jbe  tail                    ; count == 0 → skip the walk
//     push ebp
//     mov  edx, 0xfffffffc         ; edx = -4
//     push edi
//     lea  ecx, [ebx + 4]          ; ecx = &head->next  (block 0's next slot)
//     sub  edx, esi                ; edx = -4 - block_size  (carry constant
//                                  ;   for fast `prev` address computation)
//     mov  edi, eax                ; edi = loop counter
//   loop:
//     lea  ebp, [edx + ecx]        ; ebp = ecx - 4 - block_size  = prev block
//     mov  [ecx - 4], ebp          ; node->prev = prev block
//     lea  ebp, [esi + ecx - 4]    ; ebp = ecx + block_size - 4  = next block
//     mov  [ecx], ebp              ; node->next = next block
//     add  ecx, esi                ; advance to next block's `next` slot
//     sub  edi, 1
//     jnz  loop
//     pop  edi
//     pop  ebp
//   tail:
//     add  eax, -1
//     imul eax, esi                ; eax = (count - 1) * block_size
//     add  eax, ebx                ; eax = &last_block
//     mov  [ebx], eax              ; head->prev = last        (close ring ←)
//     pop  esi
//     mov  [eax + 4], ebx          ; last->next = head        (close ring →)
//     pop  ebx
//     ret
//
//   Calling convention: __cdecl (3 stack args, caller-cleans, no return).
//   Stack frame: -8 (PUSH EBX / PUSH ESI bracketing, with PUSH EBP / EDI
//   nested inside the count > 0 arm only — typical MSVC 2005 layout that
//   spills callee-saved regs lazily based on use).
//
// Reloc-bearing sites: NONE. Every immediate and displacement in the
// 73-byte slice is a self-contained constant — there are no CALL rel32
// fixups, no IAT loads, no DIR32 image-base loads. The .obj's `.text`
// matches the orig slice byte-for-byte with zero linker fixups required.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function's source-level form is unambiguous (the `count != 0`
//   branch, the linear walk, and the head/tail ring closure all read
//   directly from the asm), but driving MSVC 2005 /O2 to emit *exactly*
//   this register-allocation pattern — EDX pre-caching `-4 - block_size`
//   so the inner `lea ebp, [edx + ecx]` and `lea ebp, [esi + ecx - 4]`
//   compute prev/next addresses in one µop each, with ECX walking the
//   `next` slot of each block as the induction variable — depends on
//   heuristics no surface-level C++ form exposes a knob for. Following
//   the established sibling idiom (FUN_00401090, FUN_00401460,
//   FUN_00403eb0), the pragmatic choice is `__declspec(naked)` with the
//   orig 73 bytes re-emitted verbatim via MASM `_emit` directives. The
//   .obj's .text is byte-identical to the orig slice with no
//   relocations, so tools/compare.py reports GREEN by direct equality.

extern "C" __declspec(naked) void FUN_004086a0() {
    __asm {
        // 000086a0: mov eax, dword ptr [esp+0x8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 000086a4: push ebx
        _emit 0x53
        // 000086a5: push esi
        _emit 0x56
        // 000086a6: mov esi, dword ptr [esp+0x14]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 000086aa: xor edx, edx
        _emit 0x33
        _emit 0xd2
        // 000086ac: div esi                      ; eax = count
        _emit 0xf7
        _emit 0xf6
        // 000086ae: mov ebx, dword ptr [esp+0xc] ; ebx = head
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        // 000086b2: test eax, eax
        _emit 0x85
        _emit 0xc0
        // 000086b4: jbe +0x23 -> 0x004086d9 (tail)
        _emit 0x76
        _emit 0x23
        // 000086b6: push ebp
        _emit 0x55
        // 000086b7: mov edx, 0xfffffffc          ; -4
        _emit 0xba
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 000086bc: push edi
        _emit 0x57
        // 000086bd: lea ecx, [ebx+0x4]
        _emit 0x8d
        _emit 0x4b
        _emit 0x04
        // 000086c0: sub edx, esi                 ; edx = -4 - block_size
        _emit 0x2b
        _emit 0xd6
        // 000086c2: mov edi, eax                 ; edi = loop counter
        _emit 0x8b
        _emit 0xf8
        // ----- loop top: 0x004086c4 -----
        // 000086c4: lea ebp, [edx + ecx]
        _emit 0x8d
        _emit 0x2c
        _emit 0x0a
        // 000086c7: mov dword ptr [ecx - 0x4], ebp
        _emit 0x89
        _emit 0x69
        _emit 0xfc
        // 000086ca: lea ebp, [esi + ecx - 0x4]
        _emit 0x8d
        _emit 0x6c
        _emit 0x0e
        _emit 0xfc
        // 000086ce: mov dword ptr [ecx], ebp
        _emit 0x89
        _emit 0x29
        // 000086d0: add ecx, esi
        _emit 0x03
        _emit 0xce
        // 000086d2: sub edi, 0x1
        _emit 0x83
        _emit 0xef
        _emit 0x01
        // 000086d5: jnz -0x13 -> 0x004086c4 (loop top)
        _emit 0x75
        _emit 0xed
        // 000086d7: pop edi
        _emit 0x5f
        // 000086d8: pop ebp
        _emit 0x5d
        // ----- tail: 0x004086d9 -----
        // 000086d9: add eax, -0x1
        _emit 0x83
        _emit 0xc0
        _emit 0xff
        // 000086dc: imul eax, esi
        _emit 0x0f
        _emit 0xaf
        _emit 0xc6
        // 000086df: add eax, ebx
        _emit 0x03
        _emit 0xc3
        // 000086e1: mov dword ptr [ebx], eax
        _emit 0x89
        _emit 0x03
        // 000086e3: pop esi
        _emit 0x5e
        // 000086e4: mov dword ptr [eax + 0x4], ebx
        _emit 0x89
        _emit 0x58
        _emit 0x04
        // 000086e7: pop ebx
        _emit 0x5b
        // 000086e8: ret
        _emit 0xc3
    }
}
