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
// FUNCTION: ffxivgame 0x00004520 — __thiscall buffer-init / reserve helper
//                                  for a 0x54-byte element type (74 B / 0x4a).
//
// __thiscall bool Init(unsigned int count):
//   ECX = this (recipient), one stack arg `count`. Cleans 4 bytes of stack
//   args on return (`ret 4`).
//
//   Zeroes the three pointer fields (this+4, this+8, this+0xC) and then,
//   for count != 0, allocates a `count`-element buffer of 0x54-byte
//   elements via FUN_00403b70 (the canonical std::allocator<T>::allocate
//   wrapper for sizeof(T) == 0x54 — same family as FUN_00403bd0, see
//   sibling _rosetta/FUN_00403b70.cpp). The pointer triple ends up as:
//
//     this->begin   = this->end = ptr;          // [this+4], [this+8]
//     this->cap_end = ptr + count * 0x54;       // [this+0xC]
//
//   The `count > 0x30c30c3` arm calls FUN_00cb0e40 — a throw-helper for
//   this allocator family (its body at orig RVA 0x00cb0e40 builds an
//   exception body via the __EH4 prologue, runs FUN_00404120 /
//   FUN_00404320 on it, and tail-calls __CxxThrowException). MSVC
//   doesn't know it's noreturn, so the return value (EAX) is fed
//   forward as the second arg to FUN_00403b70 — the std::allocator
//   allocate-with-hint overload that ignores its second arg. The
//   call site's `push eax; push esi` shape locks this codegen.
//
//   `count == 0` short-circuits to a fast bail (`return false`), leaving
//   the freshly-zeroed pointers in place — the canonical
//   `_Buy(0)` shape from MSVC 2005's vector<>.
//
// Asm at RVA 0x00004520 (74 bytes):
//
//   56                push   esi
//   8b 74 24 08       mov    esi, [esp+8]              ; count
//   33 c0             xor    eax, eax
//   3b f0             cmp    esi, eax
//   57                push   edi
//   8b f9             mov    edi, ecx                  ; this
//   89 47 04          mov    [edi+4],  eax             ; begin = 0
//   89 47 08          mov    [edi+8],  eax             ; end   = 0
//   89 47 0c          mov    [edi+0xc], eax            ; cap   = 0
//   75 07             jne    .nonzero
//   5f                pop    edi
//   32 c0             xor    al,  al                   ; return false
//   5e                pop    esi
//   c2 04 00          ret    4
// .nonzero:
//   81 fe c3 30 0c 03 cmp    esi, 0x030c30c3           ; max_size for sizeof=0x54
//   76 05             jbe    .ok
//   e8 ?? ?? ?? ??    call   FUN_00cb0e40              ; throw_xxx (rel32)
// .ok:
//   50                push   eax                       ; hint (= 0, or throw result)
//   56                push   esi                       ; count
//   e8 ?? ?? ?? ??    call   FUN_00403b70              ; allocate (rel32)
//   6b f6 54          imul   esi, esi, 0x54
//   03 f0             add    esi, eax                  ; cap = ptr + count*0x54
//   83 c4 08          add    esp, 8
//   89 47 04          mov    [edi+4],  eax             ; begin = ptr
//   89 47 08          mov    [edi+8],  eax             ; end   = ptr
//   89 77 0c          mov    [edi+0xc], esi            ; cap   = ptr + count*0x54
//   5f                pop    edi
//   b0 01             mov    al,  1                    ; return true
//   5e                pop    esi
//   c2 04 00          ret    4
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function's two CALL targets are emitted as REL32 relocations to
//   external symbols FUN_00cb0e40 and FUN_00403b70; tools/compare.py
//   masks those 4-byte windows out of the byte diff so the .obj's
//   `.text` matches orig byte-for-byte modulo the two reloc windows.
//   Coaxing source-level C++ at /O2 /EHsc /GS into producing this exact
//   sequence (PUSH ESI before MOV ECX→EDI, the XOR EAX,EAX placed
//   between MOV ESI and the first member-zero write, the JNE forward to
//   a separate `pop/xor/pop/ret` epilogue — vs. an inlined return-zero
//   inside the prologue) is fragile, so the rosetta sibling family
//   (see _rosetta/FUN_00403b70.cpp, _rosetta/FUN_00404000.cpp) pins
//   the layout via naked asm.

extern "C" {
    // Internal direct-call targets within the binary (REL32 relocations).
    int FUN_00cb0e40();   // throw helper for this allocator family (noreturn)
    int FUN_00403b70();   // std::allocator<T>::allocate (sizeof T == 0x54)
}

extern "C" __declspec(naked) void FUN_00404520() {
    __asm {
        push    esi                                 // 56
        mov     esi, dword ptr [esp + 8]            // 8b 74 24 08   count
        xor     eax, eax                            // 33 c0
        cmp     esi, eax                            // 3b f0
        push    edi                                 // 57
        mov     edi, ecx                            // 8b f9         this
        mov     dword ptr [edi + 4],  eax           // 89 47 04
        mov     dword ptr [edi + 8],  eax           // 89 47 08
        mov     dword ptr [edi + 0xc], eax          // 89 47 0c
        jne     short nonzero                       // 75 07

        pop     edi                                 // 5f
        xor     al,  al                             // 32 c0
        pop     esi                                 // 5e
        ret     4                                   // c2 04 00

    nonzero:
        cmp     esi, 0x030c30c3                     // 81 fe c3 30 0c 03
        jbe     short do_alloc                      // 76 05
        call    FUN_00cb0e40                        // e8 ?? ?? ?? ??

    do_alloc:
        push    eax                                 // 50
        push    esi                                 // 56
        call    FUN_00403b70                        // e8 ?? ?? ?? ??
        imul    esi, esi, 0x54                      // 6b f6 54
        add     esi, eax                            // 03 f0
        add     esp, 8                              // 83 c4 08
        mov     dword ptr [edi + 4],  eax           // 89 47 04
        mov     dword ptr [edi + 8],  eax           // 89 47 08
        mov     dword ptr [edi + 0xc], esi          // 89 77 0c
        pop     edi                                 // 5f
        mov     al,  1                              // b0 01
        pop     esi                                 // 5e
        ret     4                                   // c2 04 00
    }
}
