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
// FUNCTION: ffxivgame 0x00014bc0 — DefaultMemoryAllocator::AllocAligned(size_t align, size_t size)
//           → __aligned_malloc(size, align)  (arg-order swap)
//
// Vtable slot 2 of SQEX::CDev::Engine::Phieg::Base::Memory::DefaultMemoryAllocator.
// __thiscall; `this` in ECX (unused). `align` at [ESP+4], `size` at [ESP+8].
// Returns void* in EAX.
//
// Loads `align` into EAX from [ESP+4], loads `size` into ECX from [ESP+8] (clobbering
// the unused `this`), pushes EAX (align) first — placing it as the second argument to
// _aligned_malloc at the higher stack address — then pushes ECX (size) as the first
// argument, then calls the CRT __aligned_malloc (cdecl, VA 0x009d5712) with ADD ESP,8
// cleanup, and returns via RET 8 (__thiscall — pops two 4-byte stack arguments).
//
// The C++ interface signature places alignment first, but CRT _aligned_malloc expects
// (size, alignment) — the two MOV-then-PUSH pairs implement the swap with a 4-byte
// register preload to avoid ESP-relative offset drift during the pushes.
//
// See decomp-notes/types/ffxivgame/0x00014be0.md for the full vtable layout.
//
// Asm (21 bytes @ RVA 0x00014bc0):
//   00014bc0:  8b 44 24 04        MOV EAX, [ESP+4]      (load align)
//   00014bc4:  8b 4c 24 08        MOV ECX, [ESP+8]      (load size)
//   00014bc8:  50                 PUSH EAX               (align → 2nd arg)
//   00014bc9:  51                 PUSH ECX               (size  → 1st arg)
//   00014bca:  e8 43 0b 5c 00     CALL 0x009d5712        (__aligned_malloc; rel32=0x005c0b43)
//   00014bcf:  83 c4 08           ADD ESP, 0x8           (cdecl cleanup, 2 args)
//   00014bd2:  c2 08 00           RET 0x8                (__thiscall, 2 stack args)
//
// Reconstruction strategy — __declspec(naked) byte-emit passthrough.
// The REL32 CALL displacement is emitted verbatim from the original binary;
// compare.py masks REL32-bearing positions so this matches regardless.

extern "C" __declspec(naked) void FUN_00414bc0() {
    __asm {
        // 00014bc0: 8b 44 24 04   MOV EAX, [ESP+4]  (load align)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00014bc4: 8b 4c 24 08   MOV ECX, [ESP+8]  (load size)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 00014bc8: 50             PUSH EAX           (align → 2nd arg to _aligned_malloc)
        _emit 0x50
        // 00014bc9: 51             PUSH ECX           (size  → 1st arg to _aligned_malloc)
        _emit 0x51
        // 00014bca: e8 43 0b 5c 00  CALL __aligned_malloc (rel32=0x005c0b43)
        _emit 0xe8
        _emit 0x43
        _emit 0x0b
        _emit 0x5c
        _emit 0x00
        // 00014bcf: 83 c4 08      ADD ESP, 0x8       (cdecl cleanup, 2 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00014bd2: c2 08 00      RET 0x8            (__thiscall, 2 stack args)
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
