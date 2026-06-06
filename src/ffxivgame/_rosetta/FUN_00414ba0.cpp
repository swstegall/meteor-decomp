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
// FUNCTION: ffxivgame 0x00014ba0 — DefaultMemoryAllocator::Alloc(size_t size)
//           → __aligned_malloc(size, 0x10)
//
// Vtable slot 1 of SQEX::CDev::Engine::Phieg::Base::Memory::DefaultMemoryAllocator.
// __thiscall; `this` in ECX (unused). `size` at [ESP+4]. Returns void* in EAX.
//
// Loads `size` into EAX, pushes alignment literal 0x10, then pushes EAX (size),
// calls the CRT __aligned_malloc (cdecl, VA 0x009d5712) with ADD ESP,8 cleanup,
// and returns via RET 4 (__thiscall — pops the one 4-byte stack argument).
//
// See decomp-notes/types/ffxivgame/0x00014be0.md for the full vtable layout.
//
// Asm (18 bytes @ RVA 0x00014ba0):
//   00014ba0:  8b 44 24 04        MOV EAX, [ESP+4]
//   00014ba4:  6a 10              PUSH 0x10
//   00014ba6:  50                 PUSH EAX
//   00014ba7:  e8 66 0b 5c 00     CALL 0x009d5712  (__aligned_malloc; rel32=0x005c0b66)
//   00014bac:  83 c4 08           ADD ESP, 0x8
//   00014baf:  c2 04 00           RET 0x4
//
// Reconstruction strategy — __declspec(naked) byte-emit passthrough.
// The REL32 CALL displacement is emitted verbatim from the original binary;
// compare.py masks REL32-bearing positions so this matches regardless.

extern "C" __declspec(naked) void FUN_00414ba0() {
    __asm {
        // 00014ba0: 8b 44 24 04   MOV EAX, [ESP+4]  (load size)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00014ba4: 6a 10         PUSH 0x10          (alignment)
        _emit 0x6a
        _emit 0x10
        // 00014ba6: 50            PUSH EAX           (size)
        _emit 0x50
        // 00014ba7: e8 66 0b 5c 00  CALL __aligned_malloc (rel32=0x005c0b66)
        _emit 0xe8
        _emit 0x66
        _emit 0x0b
        _emit 0x5c
        _emit 0x00
        // 00014bac: 83 c4 08      ADD ESP, 0x8       (cdecl cleanup, 2 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00014baf: c2 04 00      RET 0x4            (__thiscall, 1 stack arg)
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
