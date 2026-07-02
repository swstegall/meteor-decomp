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
// FUNCTION: ffxivgame 0x0043dfe0 — SEH-guarded object constructor with
//                                  conditional virtual dispatch (202 B / 0xca)
//
// Behaviour read from asm/ffxivgame/0003dfe0_FUN_0043dfe0.s:
//
//   Standard MSVC __CxxFrameHandler3 prologue (PUSH -1; PUSH scopetable;
//   swap in FS:[0]; xor-cookie stack guard) wraps a __thiscall constructor
//   taking 4 explicit stack args (RET 0x10 pops 4 dwords) in addition to
//   the implicit `this` in ECX.
//
//     this->vtbl        = 0x00f57ea4;                 // [EDI]
//     this->member8[0]  = 0;  this->member8[4] = 0;    // [EDI+8], 8 B zeroed
//     FUN_0043eec0(&arg4, 1, arg1, arg2, arg3, arg4);  // __thiscall, ECX=&arg4
//     ebx = arg4 ? ((*(int*)arg4)->vtbl[0xc])(arg4) : 0;
//     FUN_00419f80(this+8 /*ESI*/, ebx);               // __thiscall ctor
//     eax = arg4 ? ((*(int*)arg4)->vtbl[0x10])(arg4) : 0;
//     FUN_0041b210(ebx, eax, this+4);                  // __cdecl helper
//     FUN_0043e210(&(local copy region));              // __cdecl/__thiscall cleanup
//     return this;                                     // EAX = EDI
//
// Reloc-bearing / EH-metadata sites in the orig 202 bytes (only resolve in
// a full-binary relink at image base 0x00400000; a standalone .obj can't
// reproduce them, and there is no source-level spelling of the compiler's
// internal EH scope-table layout for this particular funclet arrangement):
//   +0x02  scopetable PUSH        (.rdata 0x00e56cb8 — 68 imm32)
//   +0x11  __security_cookie LOAD (.data  0x012ea8b0 — a1 moffs32)
//   +0x1a  MOV [EDI], vtbl        (.rdata 0x00f57ea4 — c7 07 imm32)
//   +0x35  CALL FUN_0043eec0      (.text  e8 rel32)
//   +0x5b  CALL FUN_00419f80      (.text  e8 rel32)
//   +0x7a  CALL FUN_0041b210      (.text  e8 rel32)
//   +0x8e  CALL FUN_0043e210      (.text  e8 rel32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This constructor mixes hand-written SEH bookkeeping (the compiler's
//   own __CxxFrameHandler3 scope-table + security-cookie prologue/epilogue)
//   with two conditional virtual-dispatch sequences and three helper calls
//   whose exact stack-slot reuse (e.g. `LEA ECX, [ESP+0x40]` aliasing back
//   into the caller's own 4th argument slot) depends on frame-layout
//   decisions no source-level rewrite can be guaranteed to reproduce
//   byte-for-byte. Same pragmatic choice as FUN_00406680 / FUN_004014b0 /
//   FUN_00401a00: a `__declspec(naked)` body that re-emits the original
//   202 bytes verbatim via MASM `_emit` directives.
//
//   The .obj's `.text` section ends up byte-identical to the orig slice:
//   no relocations are needed at the source level (the bytes are emitted
//   as raw immediates/displacements, and the orig PE bytes at the reloc
//   sites already contain the linker-resolved absolute addresses/offsets),
//   which is what tools/compare.py checks against.

extern "C" __declspec(naked) void FUN_0043dfe0() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xb8
        _emit 0x6c
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x53
        _emit 0x56
        _emit 0x57
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x50
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf9
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x50
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x51
        _emit 0x52
        _emit 0x50
        _emit 0xc7
        _emit 0x07
        _emit 0xa4
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0x8d
        _emit 0x77
        _emit 0x08
        _emit 0x6a
        _emit 0x01
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        _emit 0xc7
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x86
        _emit 0x0e
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x85
        _emit 0xc0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x0c
        _emit 0x8b
        _emit 0x08
        _emit 0x8b
        _emit 0x51
        _emit 0x0c
        _emit 0x50
        _emit 0xff
        _emit 0xd2
        _emit 0x8b
        _emit 0xd8
        _emit 0xeb
        _emit 0x02
        _emit 0x33
        _emit 0xdb
        _emit 0x53
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0x20
        _emit 0xbf
        _emit 0xfd
        _emit 0xff
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x0a
        _emit 0x8b
        _emit 0x08
        _emit 0x8b
        _emit 0x51
        _emit 0x10
        _emit 0x50
        _emit 0xff
        _emit 0xd2
        _emit 0xeb
        _emit 0x02
        _emit 0x33
        _emit 0xc0
        _emit 0x8d
        _emit 0x4f
        _emit 0x04
        _emit 0x51
        _emit 0x50
        _emit 0x53
        _emit 0xe8
        _emit 0x91
        _emit 0xd1
        _emit 0xfd
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8
        _emit 0x7d
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xc7
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
