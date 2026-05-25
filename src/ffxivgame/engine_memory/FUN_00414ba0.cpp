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
// FUNCTION: ffxivgame 0x00014ba0 — __stdcall 1-arg wrapper around the
// MSVC 2005 CRT `_aligned_malloc` with the alignment hard-wired to 16
// bytes. This is the engine's canonical "give me a 16-byte-aligned
// block of N bytes" entry point — most SIMD-friendly allocations in
// the renderer / memory-pool subsystems funnel through here. (18 B)
//
// Asm (18 bytes):
//   00014ba0:  8b 44 24 04   MOV EAX, [ESP+4]       ; eax = size
//   00014ba4:  6a 10         PUSH 0x10              ; alignment = 16
//   00014ba6:  50            PUSH EAX               ; size
//   00014ba7:  e8 ?? ?? ?? ? CALL _aligned_malloc   ; REL32 reloc → 0x005d5712
//   00014bac:  83 c4 08      ADD ESP, 8             ; cdecl callee cleanup
//   00014baf:  c2 04 00      RET 4                  ; __stdcall — pop 1 arg
//
// Calling convention: __stdcall (RET 4). The callee, `_aligned_malloc`,
// is __cdecl, hence the `ADD ESP, 8` after the call.
//
// The orig codegen does `MOV EAX, [ESP+4]; PUSH EAX` (5 B) rather than
// the size-optimised `PUSH [ESP+8]` (4 B). That's the default /O2
// (speed) lowering — keep `#pragma optimize("s", on)` OFF here so MSVC
// preserves the orig form.

#include <stddef.h>

extern "C" void * __cdecl _aligned_malloc(size_t size, size_t alignment);

extern "C" void * __stdcall FUN_00414ba0(size_t size) {
    return _aligned_malloc(size, 16);
}
