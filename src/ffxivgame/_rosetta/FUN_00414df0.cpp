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
// FUNCTION: ffxivgame 0x00014df0 — `srand(_time64(NULL))`-style PRNG seeder
//                                  (27 B, __cdecl, no frame pointer).
//
// Asm (27 bytes @ orig RVA 0x00014df0):
//   8d 44 24 f8         LEA  EAX, [ESP - 0x8]            ; eax = &t (pre-sub)
//   83 ec 08            SUB  ESP, 0x8                    ; allocate __time64_t t
//   50                  PUSH EAX                         ; arg = &t
//   e8 28 09 5c 00      CALL __time64        @ 0x009d5725  ; _time64(&t)
//   8b 4c 24 04         MOV  ECX, dword ptr [ESP + 0x4]  ; ecx = low dword of t
//   51                  PUSH ECX                         ; arg = (unsigned)t
//   e8 5a 09 5c 00      CALL FUN_009d5761    @ 0x009d5761  ; srand(seed)
//   83 c4 10            ADD  ESP, 0x10                   ; pop 2 args + free buffer (16 B)
//   c3                  RET
//
// High-level shape:
//   void seed_rand(void) {
//       __time64_t t;
//       _time64(&t);
//       srand((unsigned int)t);            // truncates 64-bit time → 32-bit seed
//   }
//
// Calling convention: __cdecl — no args in, no return, plain RET. With
// /Oy the function has no `push ebp / mov ebp, esp` prologue. The `lea
// [esp-8]; sub esp, 8; push eax` triple is MSVC 2005's scheduling of
// "compute &t against the *future* ESP, then allocate, then pass the
// pointer" — equivalent to but byte-distinct from the more common
// `sub esp, 8; lea eax, [esp]; push eax` form. The combined cleanup
// `add esp, 0x10` collapses both cdecl-arg pops (4 + 4) and the buffer
// free (8) into a single instruction, which is why no separate
// `add esp, 4` appears after the first call.
//
// FUN_009d5761 is the CRT srand implementation (gets the per-thread
// PTD and stores the seed at PTD+0x14). __time64 is the CRT _time64
// helper. Both are reloc-bearing call targets; this passthrough emits
// the orig relative offsets verbatim so the .obj's `.text` is byte-
// identical to the orig slice (no relocations in the .obj, no risk of
// the source-level form rescheduling the lea/sub/push triple).

extern "C" __declspec(naked) void FUN_00414df0() {
    __asm {
        _emit 0x8d              // LEA EAX, [ESP - 0x8]
        _emit 0x44
        _emit 0x24
        _emit 0xf8
        _emit 0x83              // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL __time64 (rel32)
        _emit 0x28
        _emit 0x09
        _emit 0x5c
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL FUN_009d5761 (rel32)
        _emit 0x5a
        _emit 0x09
        _emit 0x5c
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3              // RET
    }
}
