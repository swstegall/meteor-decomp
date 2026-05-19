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
// FUNCTION: ffxivgame 0x00003b70 — operator new[] wrapper for a 0x54-byte
// element type (__cdecl, 86 B).
//
// Compiler-synthesized wrapper around `::operator new(size_t)` with the
// standard MSVC overflow check that throws `std::bad_alloc` when
// `count * sizeof(T)` would wrap 32-bit. Element size is 0x54 (84 bytes);
// the function is one of a family of identical wrappers, each pinned to
// a different element size (see e.g. FUN_00403bd0 — same shape, 0x1c).
//
// Pseudo-source (logical structure):
//
//   void* __cdecl FUN_00403b70(unsigned int count) {
//       if (count > 0 && (0xFFFFFFFFu / count) < 0x54) {
//           // count * 0x54 would overflow 32 bits → throw bad_alloc.
//           // Inlined construction: stack-allocate a 12-byte exception
//           // body, run std::exception::exception(this, &NULL_msg) on
//           // it, patch the vftable slot to std::bad_alloc's vtable,
//           // then _CxxThrowException.
//           const char* msg = NULL;
//           std::exception ex(msg);              // @ 0x009d18da
//           *((void**)&ex) = bad_alloc_vftable;  // @ 0x00f54a10
//           _CxxThrowException(&ex, &bad_alloc_throw_info);  // 0x011a8c90
//       }
//       return ::operator new(count * 0x54);     // @ 0x009d1b35
//   }
//
// Branch shape (per asm/ffxivgame/00003b70_FUN_00403b70.s):
//   +0x09  JA  +0x1d  (count > 0 → overflow_check)
//   +0x27  JNB +0x0d  (no overflow → do_alloc, count untouched)
// When `count == 0` we fall through into do_alloc with ECX zeroed by
// the preceding XOR — IMUL ECX,ECX,0x54 then yields 0 and we PUSH 0
// into operator new (which returns a valid 0-byte allocation).
//
// Why naked asm: this is a compiler-synthesized stub. There's no clean
// C++ source — the bad_alloc construction is inlined (it builds the
// exception body via the 1-arg std::exception ctor + vftable patch
// rather than calling std::bad_alloc::bad_alloc), and the JNC-back-
// over-the-XOR control flow joining the count-zero and no-overflow
// arms at the same IMUL is not something /O2 will reproduce from
// plain C++. Naked asm pins every byte; the reloc-masking diff
// (compare.py) sees a byte-exact match modulo the 5 reloc windows.
//
// Reloc-bearing sites within the function (4-byte windows, wildcarded
// by compare.py against orig):
//   +0x12  CALL FUN_009d1b35     (rel32, ::operator new)
//   +0x3b  CALL FUN_009d18da     (rel32, std::exception::exception(char const* const&))
//   +0x40  PUSH offset data_011a8c90  (dir32, __ThrowInfo for std::bad_alloc)
//   +0x4d  MOV  [ESP+8], offset data_00f54a10  (dir32, std::bad_alloc vftable)
//   +0x52  CALL FUN_009d1b9f     (rel32, __CxxThrowException@8)

extern "C" {
    // Internal direct-call targets within the binary (REL32 relocations).
    int FUN_009d1b35();   // ::operator new(size_t)
    int FUN_009d18da();   // std::exception::exception(char const* const&)
    int FUN_009d1b9f();   // __CxxThrowException@8

    // Absolute data references (DIR32 relocations).
    extern int data_011a8c90;   // __ThrowInfo for std::bad_alloc
    extern int data_00f54a10;   // std::bad_alloc vftable
}

extern "C" __declspec(naked) void FUN_00403b70() {
    __asm {
        // --- prologue + zero-count short-circuit -----------------------
        mov     ecx, dword ptr [esp + 4]              // 8b 4c 24 04   arg1 = count
        sub     esp, 0xc                              // 83 ec 0c      reserve bad_alloc body
        test    ecx, ecx                              // 85 c9
        ja      short overflow_check                  // 77 12         count > 0 → check overflow
        xor     ecx, ecx                              // 33 c9         count == 0 path

    do_alloc:
        imul    ecx, ecx, 0x54                        // 6b c9 54      ECX = count * 0x54
        push    ecx                                   // 51
        call    FUN_009d1b35                          // e8 ?? ?? ?? ??  ::operator new
        add     esp, 4                                // 83 c4 04
        add     esp, 0xc                              // 83 c4 0c
        ret                                           // c3

    overflow_check:
        or      eax, -1                               // 83 c8 ff      EAX = 0xFFFFFFFF
        xor     edx, edx                              // 33 d2
        div     ecx                                   // f7 f1         EAX = 0xFFFFFFFFu / count
        cmp     eax, 0x54                             // 83 f8 54
        jnb     short do_alloc                        // 73 e4         no overflow → allocate

        // --- inlined `throw std::bad_alloc()` --------------------------
        lea     eax, [esp + 0x10]                     // 8d 44 24 10   &msg (caller-arg slot reused)
        push    eax                                   // 50
        lea     ecx, [esp + 4]                        // 8d 4c 24 04   this = &ex (12-B body)
        mov     dword ptr [esp + 0x14], 0             // c7 44 24 14 00 00 00 00  msg = NULL
        call    FUN_009d18da                          // e8 ?? ?? ?? ??  exception::exception(&msg)
        push    offset data_011a8c90                  // 68 ?? ?? ?? ??  __ThrowInfo
        lea     ecx, [esp + 4]                        // 8d 4c 24 04
        push    ecx                                   // 51            &ex
        mov     dword ptr [esp + 8], offset data_00f54a10  // c7 44 24 08 ?? ?? ?? ??  patch vftable
        call    FUN_009d1b9f                          // e8 ?? ?? ?? ??  __CxxThrowException — noreturn
    }
}
