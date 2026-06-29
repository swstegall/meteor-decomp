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
// FUNCTION: ffxivgame 0x00040d40 — __thiscall list-of-strings clear/destroy
//                                  (72 B / 0x48)
//
// Calling convention: __thiscall (ECX = this).
// Register map:
//   EDI = this
//   ESI = current node being freed (walks the list)
//   EBP = next node (saved before the current node is freed)
//   EBX = 0 (used to zero size/first-byte fields)
//   EAX = scratch
//
// This object (at [EDI]) appears to be an intrusive doubly-linked list of
// string-like nodes:
//
//   struct Container {
//       /* +0x00 */  void*  ???;            // not touched here
//       /* +0x04 */  Node*  sentinel;       // circular list head
//       /* +0x08 */  int    count;
//   };
//
//   struct Node {
//       /* +0x00 */  Node*  next;
//       /* +0x04 */  Node*  prev;
//       /* +0x08 */  ...                   // 8 bytes not touched here
//       /* +0x10 */  char   buf[16];       // SSO inline buffer / heap ptr
//       /* +0x20 */  int    size;
//       /* +0x24 */  int    capacity;      // 0xf = inline, >=0x10 = heap
//   };
//
// Body (matches asm flow exactly):
//
//   1. Load old sentinel->next into ESI (the first real node, or sentinel).
//   2. Reset sentinel to circular-self (empty list): next = prev = sentinel.
//   3. Zero count ([EDI+8]).
//   4. If list was already empty (ESI == sentinel), jump to epilogue.
//   5. Push EBP; align loop top with LEA ECX,[ECX] NOP.
//   6. Loop:
//        a. Compare node->capacity with 0x10.
//        b. Save next node in EBP (before freeing current).
//        c. If capacity < 0x10: skip heap-buffer free.
//        d. free(node->buf heap ptr).   [__cdecl, ADD ESP,4 cleanup]
//        e. node->capacity = 0xf; node->size = 0; node->buf[0] = 0.
//        f. free(node).                [__cdecl, first byte of ADD ESP,4
//                                       is the last byte of our 72-byte
//                                       slice; MSVC shared the epilogue
//                                       with the following function]
//        g. Advance ESI = EBP, loop while ESI != sentinel.
//      (The loop continuation, POP EBP, and POP EDI/ESI/EBX/RET all
//       reside in the adjacent function's slice and are excluded from
//       the 72-byte comparison window.)
//
// Reconstruction strategy — naked byte passthrough:
//   The function body is fully determined from the orig binary bytes.
//   Two CALL rel32 displacements are linker-resolved relocations; the .obj
//   emits the address of _free at those positions and tools/compare.py
//   masks them as wildcards.  Every other byte is emitted verbatim via
//   _emit so the encoding — including the JZ +0x36 short jump to the
//   out-of-range epilogue, the LEA ECX,[ECX] 3-byte NOP, the JC +0x0c
//   short jump over the heap-buffer free, and the final lone 0x83 byte
//   that is the first byte of the shared ADD ESP,4 — is pinned exactly.
//
// Reloc-bearing positions (masked by tools/compare.py):
//   off 0x2d–0x30   IMAGE_REL_I386_REL32  → _free  (first CALL)
//   off 0x43–0x46   IMAGE_REL_I386_REL32  → _free  (second CALL)

extern "C" void _free();

extern "C" __declspec(naked) void FUN_00440d40() {
    __asm {
        // ----- prologue -----
        _emit 0x53          // 00: push    ebx
        _emit 0x56          // 01: push    esi
        _emit 0x57          // 02: push    edi
        _emit 0x8b          // 03: mov     edi, ecx
        _emit 0xf9
        // ----- reset sentinel to empty circular list -----
        _emit 0x8b          // 05: mov     eax, dword ptr [edi+4]
        _emit 0x47
        _emit 0x04
        _emit 0x8b          // 08: mov     esi, dword ptr [eax]
        _emit 0x30
        _emit 0x89          // 0a: mov     dword ptr [eax], eax
        _emit 0x00
        _emit 0x8b          // 0c: mov     eax, dword ptr [edi+4]
        _emit 0x47
        _emit 0x04
        _emit 0x89          // 0f: mov     dword ptr [eax+4], eax
        _emit 0x40
        _emit 0x04
        _emit 0x33          // 12: xor     ebx, ebx
        _emit 0xdb
        _emit 0x3b          // 14: cmp     esi, dword ptr [edi+4]
        _emit 0x77
        _emit 0x04
        _emit 0x89          // 17: mov     dword ptr [edi+8], ebx
        _emit 0x5f
        _emit 0x08
        _emit 0x74          // 1a: jz      +0x36  (→ epilogue outside slice)
        _emit 0x36
        // ----- loop prologue -----
        _emit 0x55          // 1c: push    ebp
        _emit 0x8d          // 1d: lea     ecx, [ecx]  (3-byte NOP, alignment)
        _emit 0x49
        _emit 0x00
        // ----- loop top (0x20) -----
        _emit 0x83          // 20: cmp     dword ptr [esi+0x24], 0x10
        _emit 0x7e
        _emit 0x24
        _emit 0x10
        _emit 0x8b          // 24: mov     ebp, dword ptr [esi]
        _emit 0x2e
        _emit 0x72          // 26: jc      +0x0c  (→ skip heap-buf free)
        _emit 0x0c
        // ----- free heap buffer -----
        _emit 0x8b          // 28: mov     eax, dword ptr [esi+0x10]
        _emit 0x46
        _emit 0x10
        _emit 0x50          // 2b: push    eax
        call _free          // 2c: call    _free (heap buf)   [reloc 0x2d–0x30]
        _emit 0x83          // 31: add     esp, 4
        _emit 0xc4
        _emit 0x04
        // ----- reset string fields and free node -----
        _emit 0xc7          // 34: mov     dword ptr [esi+0x24], 0xf
        _emit 0x46
        _emit 0x24
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89          // 3b: mov     dword ptr [esi+0x20], ebx
        _emit 0x5e
        _emit 0x20
        _emit 0x56          // 3e: push    esi
        _emit 0x88          // 3f: mov     byte ptr [esi+0x10], bl
        _emit 0x5e
        _emit 0x10
        call _free          // 42: call    _free (node)       [reloc 0x43–0x46]
        // ----- last byte of slice (0x47) = first byte of shared ADD ESP,4 -----
        _emit 0x83          // 47: 0x83 (first byte of `add esp, 4`;
                            //          remainder + loop-back + epilogue
                            //          live in the adjacent function slice)
    }
}
