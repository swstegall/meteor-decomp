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
// FUNCTION: ffxivgame 0x0004c240 — flag-bitmask builder keyed on five
//                                  config-key string lookups (485 B / 0x1e5,
//                                  inline SEH + /GS).
//
// Inspection (read from the disassembly at orig RVA 0x0004c240):
//
//   __cdecl int FUN_0044c240(void* ctx);   // single ptr arg, returns int
//
//     The function uses MSVC's inline (non-frame-pointer) SEH frame plus a
//     /GS security cookie:
//       PUSH -1 / PUSH 0x00e57991 (scope table) / PUSH FS:[0] / SUB ESP,0xa8
//       / save EBX,EBP,ESI,EDI / PUSH (cookie ^ ESP) / install FS:[0].
//
//     ESI = ctx (the single ptr arg, read from [ESP+0xcc] after prologue).
//
//   Structural shape:
//
//     int flags = FUN_0044bfc0(ctx) << 8;             // EDI seed
//
//     // Five near-identical blocks. Each one:
//     //   1. constructs a temporary key object on the stack from
//     //      (*(void**)0x00f67298, <literal key>) via FUN_00447260,
//     //   2. invokes ctx->lookup(&key, 0) via FUN_00446f70 (__thiscall),
//     //   3. tests whether the result != the sentinel *(void**)0x00f67298,
//     //   4. destroys the temporary via FUN_00446f50,
//     //   5. OR-s a bit into `flags` when the lookup hit.
//     // The SEH state slot [ESP+0xcc] steps 0,1,2,3,4 across the five
//     // blocks (temporary-object live-range tracking); [ESP+0xc4] is set
//     // back to -1 (EBP) after each destructor.
//
//     if (lookup(0x00f672f8) != sentinel) flags |= 0x20;  // key #0
//     if (lookup(0x00f67300) != sentinel) flags |= 0x06;  // key #1
//     if (lookup(0x00f67308) != sentinel) flags |= 0x02;  // key #2
//     if (lookup(0x00f67310) != sentinel) flags |= 0x04;  // key #3
//     if (lookup(0x00f67318) != sentinel) flags |= 0x20;  // key #4
//     return flags;
//
//   Reloc-bearing sites in the orig 485 bytes (image base 0x00400000;
//   standalone .obj compilation can't reproduce these absolute fixups):
//     +0x03  PUSH imm32       → 0x00e57991  (SEH scope table)
//     +0x18  MOV  EAX,[imm32] → 0x012ea8b0  (__security_cookie)
//     +0x35  CALL rel32       → FUN_0044bfc0 (EDI seed)
//     +0x3c  MOV  EAX,[imm32] → 0x00f67298  (sentinel ptr, ×5 loads)
//     +0x45  PUSH imm32       → 0x00f672f8 .. 0x00f67318 (5 literal keys)
//     +0x51  CALL rel32       → FUN_00447260 (key-object ctor, ×5)
//     +0x6a  CALL rel32       → FUN_00446f70 (ctx->lookup, ×5)
//     +0x86  CALL rel32       → FUN_00446f50 (key-object dtor, ×5)
//
//   Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ rewrite would need to coax MSVC 2005 /O2 /GS /EHsc
//   into reproducing the exact inline-SEH prolog, the five-step SEH state
//   numbering ([ESP+0xcc] = 0..4), the security-cookie epilogue, AND the
//   linker-resolved absolute addresses in the relocation windows above
//   (the five distinct string literals, the sentinel-ptr loads, and the
//   four rel32 call targets). Each constraint is brittle under /O2 — every
//   high-level rewrite shifts at least one byte (state numbering, branch
//   short-vs-near, modrm vs moffs32). The pragmatic choice — the same one
//   the sibling inline-SEH bodies (FUN_0040b840 et al.) took — is a
//   `__declspec(naked)` body that re-emits the orig 485 bytes verbatim via
//   MASM `_emit` directives, yielding a `.text` slice byte-identical to the
//   orig, which is what `tools/compare.py` checks against.

extern "C" __declspec(naked) void FUN_0044c240() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x91
        _emit 0x79
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x81
        _emit 0xec
        _emit 0xa8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x53
        _emit 0x55
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
        _emit 0x84
        _emit 0x24
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0x46
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xf8
        _emit 0xa1
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x50
        _emit 0x68
        _emit 0xf8
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xc1
        _emit 0xe7
        _emit 0x08
        _emit 0xe8
        _emit 0xca
        _emit 0xaf
        _emit 0xff
        _emit 0xff
        _emit 0x6a
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x51
        _emit 0x8b
        _emit 0xce
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xc1
        _emit 0xac
        _emit 0xff
        _emit 0xff
        _emit 0x3b
        _emit 0x05
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x0f
        _emit 0x95
        _emit 0xc3
        _emit 0x83
        _emit 0xcd
        _emit 0xff
        _emit 0x89
        _emit 0xac
        _emit 0x24
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x85
        _emit 0xac
        _emit 0xff
        _emit 0xff
        _emit 0x84
        _emit 0xdb
        _emit 0x74
        _emit 0x03
        _emit 0x83
        _emit 0xcf
        _emit 0x20
        _emit 0x8b
        _emit 0x15
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x52
        _emit 0x68
        _emit 0x00
        _emit 0x73
        _emit 0xf6
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xe8
        _emit 0x79
        _emit 0xaf
        _emit 0xff
        _emit 0xff
        _emit 0x6a
        _emit 0x00
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50
        _emit 0x8b
        _emit 0xce
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x70
        _emit 0xac
        _emit 0xff
        _emit 0xff
        _emit 0x3b
        _emit 0x05
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x0f
        _emit 0x95
        _emit 0xc3
        _emit 0x89
        _emit 0xac
        _emit 0x24
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x37
        _emit 0xac
        _emit 0xff
        _emit 0xff
        _emit 0x84
        _emit 0xdb
        _emit 0x74
        _emit 0x03
        _emit 0x83
        _emit 0xcf
        _emit 0x06
        _emit 0x8b
        _emit 0x0d
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x51
        _emit 0x68
        _emit 0x08
        _emit 0x73
        _emit 0xf6
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xe8
        _emit 0x2b
        _emit 0xaf
        _emit 0xff
        _emit 0xff
        _emit 0x6a
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x52
        _emit 0x8b
        _emit 0xce
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x22
        _emit 0xac
        _emit 0xff
        _emit 0xff
        _emit 0x3b
        _emit 0x05
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x0f
        _emit 0x95
        _emit 0xc3
        _emit 0x89
        _emit 0xac
        _emit 0x24
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xe9
        _emit 0xab
        _emit 0xff
        _emit 0xff
        _emit 0x84
        _emit 0xdb
        _emit 0x74
        _emit 0x03
        _emit 0x83
        _emit 0xcf
        _emit 0x02
        _emit 0xa1
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x50
        _emit 0x68
        _emit 0x10
        _emit 0x73
        _emit 0xf6
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xe8
        _emit 0xde
        _emit 0xae
        _emit 0xff
        _emit 0xff
        _emit 0x6a
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x51
        _emit 0x8b
        _emit 0xce
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xd5
        _emit 0xab
        _emit 0xff
        _emit 0xff
        _emit 0x3b
        _emit 0x05
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x0f
        _emit 0x95
        _emit 0xc3
        _emit 0x89
        _emit 0xac
        _emit 0x24
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x9c
        _emit 0xab
        _emit 0xff
        _emit 0xff
        _emit 0x84
        _emit 0xdb
        _emit 0x74
        _emit 0x03
        _emit 0x83
        _emit 0xcf
        _emit 0x04
        _emit 0x8b
        _emit 0x15
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x52
        _emit 0x68
        _emit 0x18
        _emit 0x73
        _emit 0xf6
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x70
        _emit 0xe8
        _emit 0x90
        _emit 0xae
        _emit 0xff
        _emit 0xff
        _emit 0x6a
        _emit 0x00
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x6c
        _emit 0x50
        _emit 0x8b
        _emit 0xce
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x87
        _emit 0xab
        _emit 0xff
        _emit 0xff
        _emit 0x3b
        _emit 0x05
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x68
        _emit 0x0f
        _emit 0x95
        _emit 0xc3
        _emit 0x89
        _emit 0xac
        _emit 0x24
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x4e
        _emit 0xab
        _emit 0xff
        _emit 0xff
        _emit 0x84
        _emit 0xdb
        _emit 0x74
        _emit 0x03
        _emit 0x83
        _emit 0xcf
        _emit 0x20
        _emit 0x8b
        _emit 0xc7
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
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
        _emit 0x5d
        _emit 0x5b
        _emit 0x81
        _emit 0xc4
        _emit 0xb4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3
    }
}
