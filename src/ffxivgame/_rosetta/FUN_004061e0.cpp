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
// FUNCTION: ffxivgame 0x000061e0 — __thiscall raw-byte buffer reserve helper
//                                  for an STL-style triple-pointer container
//                                  (149 B / 0x95, ret 4).
//
// __thiscall void Reserve(this, unsigned int n):
//   ECX = this (Container { void *_unused, char *begin, char *end,
//   char *cap_end } at +0x0/+0x4/+0x8/+0xc); one stack arg `n` giving
//   the requested capacity in bytes. Cleans 4 bytes of stack args on
//   return (`ret 4`).
//
// Behaviour (recovered from asm @ 0x000061e0):
//
//   void Container::Reserve(unsigned int n) {
//       if (n > (unsigned)-1)                 // unreachable — MSVC keeps
//           throw_length_error();              //   the max_size() check
//       unsigned avail = (this->begin == nullptr)
//           ? 0
//           : (unsigned)(this->cap_end - this->begin);
//       if (avail >= n) return;
//
//       char *buf = (char *) FUN_00401090(n, 0);  // operator new(nothrow)
//       if (this->begin > this->end) _invalid_parameter();
//       unsigned old_begin = this->begin;
//       if (old_begin > this->end) _invalid_parameter();
//       unsigned size = this->end - old_begin;
//       if (size != 0)
//           memcpy_s(buf, n, old_begin, size);
//
//       // Recompute size from the field again (no register reuse — this
//       // is a second inlined `end - begin` with its own null-guard).
//       char *old = this->begin;
//       unsigned new_size = old ? (unsigned)(this->end - old) : 0;
//       if (old) free(old);
//
//       this->end     = buf + new_size;
//       this->begin   = buf;
//       this->cap_end = buf + n;
//   }
//
// CALL targets (all REL32, wildcarded by tools/compare.py):
//   +0x0c   CALL FUN_00cb0e40   — length_error throw helper (same family
//                                 as FUN_00404520's max_size guard)
//   +0x2a   CALL FUN_00401090   — 2-arg __cdecl operator-new wrapper
//                                 (see _rosetta/FUN_00403c60.cpp)
//   +0x3c   CALL FUN_009d22b4   — _invalid_parameter range-check helper
//   +0x4a   CALL FUN_009d22b4   — same; second inlined invariant test
//   +0x57   CALL FUN_009d186e   — memcpy_s
//   +0x75   CALL FUN_009d1b17   — free
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Source-level C++ at /O2 needs to coax MSVC 2005 into reproducing
//   the unusual sequence where the size recomputation at offset 0x5f
//   re-reads `this->begin` and recomputes `end - begin` (rather than
//   carrying the value forward in a register). Source-level inlining
//   wouldn't naturally emit two independent null-guarded subtract
//   blocks for the same field — coaxing MSVC's CSE / register
//   allocator to drop both copies fails on the local-allocation
//   tiebreaker (the prologue picks ESI ↔ ECX differently in an
//   isolated TU than in the full-binary build). The rosetta naked-asm
//   path is the canonical workaround for ffxivgame and produces a
//   `.text` whose 149 bytes match orig byte-for-byte modulo the six
//   relocation windows (compare.py masks those out via the COFF
//   reloc table).

extern "C" {
    // Internal direct-call targets within the binary (REL32 relocations).
    int FUN_00cb0e40();    // length_error throw helper (noreturn)
    int FUN_00401090();    // 2-arg __cdecl void* operator-new(size, flag)
    int FUN_009d22b4();    // _invalid_parameter (noreturn)
    int FUN_009d186e();    // memcpy_s
    int FUN_009d1b17();    // free
}

extern "C" __declspec(naked) void FUN_004061e0() {
    __asm {
        mov     edx, dword ptr [esp + 4]            // 8b 54 24 04   n
        cmp     edx, -1                             // 83 fa ff
        push    esi                                 // 56
        mov     esi, ecx                            // 8b f1         this
        jbe     short size_ok                       // 76 05
        call    FUN_00cb0e40                        // e8 ?? ?? ?? ??

    size_ok:
        mov     ecx, dword ptr [esi + 4]            // 8b 4e 04      begin
        test    ecx, ecx                            // 85 c9
        jnz     short have_begin                    // 75 04
        xor     eax, eax                            // 33 c0
        jmp     short check_capacity                // eb 05
    have_begin:
        mov     eax, dword ptr [esi + 0xc]          // 8b 46 0c      cap
        sub     eax, ecx                            // 2b c1
    check_capacity:
        cmp     eax, edx                            // 3b c2
        jnc     short done                          // 73 6c

        push    ebx                                 // 53
        push    edi                                 // 57
        push    0                                   // 6a 00
        push    edx                                 // 52
        call    FUN_00401090                        // e8 ?? ?? ?? ??
        mov     edi, dword ptr [esi + 8]            // 8b 7e 08      end
        add     esp, 8                              // 83 c4 08
        cmp     dword ptr [esi + 4], edi            // 39 7e 04
        mov     ebx, eax                            // 8b d8         buf
        jbe     short range_a_ok                    // 76 05
        call    FUN_009d22b4                        // e8 ?? ?? ?? ??

    range_a_ok:
        push    ebp                                 // 55
        mov     ebp, dword ptr [esi + 4]            // 8b 6e 04      begin
        cmp     ebp, dword ptr [esi + 8]            // 3b 6e 08
        jbe     short range_b_ok                    // 76 05
        call    FUN_009d22b4                        // e8 ?? ?? ?? ??

    range_b_ok:
        sub     edi, ebp                            // 2b fd         size
        jz      short skip_copy                     // 74 0c
        push    edi                                 // 57
        push    ebp                                 // 55
        push    edi                                 // 57
        push    ebx                                 // 53
        call    FUN_009d186e                        // e8 ?? ?? ?? ??
        add     esp, 0x10                           // 83 c4 10

    skip_copy:
        mov     eax, dword ptr [esi + 4]            // 8b 46 04      begin (re-read)
        test    eax, eax                            // 85 c0
        pop     ebp                                 // 5d
        jnz     short have_old                      // 75 04
        xor     edi, edi                            // 33 ff
        jmp     short check_free                    // eb 05
    have_old:
        mov     edi, dword ptr [esi + 8]            // 8b 7e 08
        sub     edi, eax                            // 2b f8
    check_free:
        test    eax, eax                            // 85 c0
        jz      short skip_free                     // 74 09
        push    eax                                 // 50
        call    FUN_009d1b17                        // e8 ?? ?? ?? ??
        add     esp, 4                              // 83 c4 04

    skip_free:
        mov     eax, dword ptr [esp + 0x10]         // 8b 44 24 10   n (arg)
        add     edi, ebx                            // 03 fb         new_end
        lea     ecx, [ebx + eax]                    // 8d 0c 03      new_cap
        mov     dword ptr [esi + 8], edi            // 89 7e 08
        pop     edi                                 // 5f
        mov     dword ptr [esi + 4], ebx            // 89 5e 04
        mov     dword ptr [esi + 0xc], ecx          // 89 4e 0c
        pop     ebx                                 // 5b

    done:
        pop     esi                                 // 5e
        ret     4                                   // c2 04 00
    }
}
