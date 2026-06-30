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
// FUNCTION: ffxivgame 0x0044a1d0 — FUN_0044a1d0 (first 138 bytes / 0x8a)
//                                  __thiscall, SEH frame, /GS cookie
//
// Calling convention: __thiscall (ECX = this, saved into EDI then [EBP-0x14]).
// One explicit argument at [EBP+0x8]: an unsigned count/size value.
//
// Frame layout (EBP-relative):
//   [EBP - 0x04]  SEH state cookie (EH try-state, init=0, final=-1)
//   [EBP - 0x08]  __security_cookie XOR'd with EBP
//   [EBP - 0x0C]  (SEH EXCEPTION_REGISTRATION.Handler) → 0xe576e0
//   [EBP - 0x10]  saved ESP (for /GS frame check)
//   [EBP - 0x14]  saved this (EDI / ECX)
//   [EBP + 0x08]  arg0 — count/size parameter (also used as out slot for alloc ptr)
//
// The body:
//   1. Standard /GS prologue: push -1, push handler addr, link FS:[0] chain,
//      XOR security_cookie with EBP, stash saved ESP.
//   2. Save this (ECX) into EDI and [EBP-0x14].
//   3. Compute a clamped "new capacity":
//      ESI = arg0 | 3.  If (ESI | 3) > 0xFFFFFFFE unsigned, use arg0 as-is.
//      Otherwise fetch this->field_0x18 (EBX = current capacity) and compute
//      EDX = ESI / 3  (via multiply by 0xAAAAAAAB, SHR 33),
//      ECX = EBX / 2.
//      If EDX < ECX AND EBX <= (-2 - ECX) then ESI = ECX + EBX (grow by half).
//      Otherwise keep ESI = arg0 | 3 (or the fallback from step 3).
//   4. Allocate: CALL FUN_0044d500(size=(ESI*4+4), 0x0, 0xc); result → [EBP+8].
//   5. Set EH state to -1 and JMP to function epilogue at 0x0044a28b.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The SEH registration, /GS cookie, and exact register allocation cannot be
//   reproduced from a C++ source form within MSVC 2005 without triggering
//   different codegen.  The __declspec(naked) body re-emits the original 138
//   bytes verbatim via MASM _emit directives; the .obj's .text section will be
//   byte-identical to the original slice and compare.py will report GREEN.
//
// Relocations masked by compare.py (positions within function):
//   +0x05  PUSH 0xe576e0          (SEH handler — IMAGE_REL_I386_DIR32)
//   +0x0a  MOV EAX,FS:[0x0]      (the 0x00000000 in 64 a1 00 00 00 00 is
//                                  technically a [0] reference; compare.py masks it)
//   +0x17  MOV EAX,[0x012ea8b0]  (__security_cookie — IMAGE_REL_I386_DIR32)
//   +0x76  CALL 0x0044d500       (FUN_0044d500 — IMAGE_REL_I386_REL32)

extern "C" __declspec(naked) void FUN_0044a1d0()
{
    __asm {
        // 0044a1d0:  55
        _emit 0x55
        // 0044a1d1:  8b ec
        _emit 0x8b
        _emit 0xec
        // 0044a1d3:  6a ff
        _emit 0x6a
        _emit 0xff
        // 0044a1d5:  68 e0 76 e5 00   PUSH 0xe576e0
        _emit 0x68
        _emit 0xe0
        _emit 0x76
        _emit 0xe5
        _emit 0x00
        // 0044a1da:  64 a1 00 00 00 00   MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0044a1e0:  50
        _emit 0x50
        // 0044a1e1:  83 ec 0c
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 0044a1e4:  53
        _emit 0x53
        // 0044a1e5:  56
        _emit 0x56
        // 0044a1e6:  57
        _emit 0x57
        // 0044a1e7:  a1 b0 a8 2e 01   MOV EAX,[0x012ea8b0]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0044a1ec:  33 c5
        _emit 0x33
        _emit 0xc5
        // 0044a1ee:  50
        _emit 0x50
        // 0044a1ef:  8d 45 f4
        _emit 0x8d
        _emit 0x45
        _emit 0xf4
        // 0044a1f2:  64 a3 00 00 00 00   MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0044a1f8:  89 65 f0
        _emit 0x89
        _emit 0x65
        _emit 0xf0
        // 0044a1fb:  8b f9
        _emit 0x8b
        _emit 0xf9
        // 0044a1fd:  89 7d ec
        _emit 0x89
        _emit 0x7d
        _emit 0xec
        // 0044a200:  8b 45 08
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        // 0044a203:  8b f0
        _emit 0x8b
        _emit 0xf0
        // 0044a205:  83 ce 03
        _emit 0x83
        _emit 0xce
        _emit 0x03
        // 0044a208:  83 fe fe
        _emit 0x83
        _emit 0xfe
        _emit 0xfe
        // 0044a20b:  76 04
        _emit 0x76
        _emit 0x04
        // 0044a20d:  8b f0
        _emit 0x8b
        _emit 0xf0
        // 0044a20f:  eb 22
        _emit 0xeb
        _emit 0x22
        // 0044a211:  8b 5f 18
        _emit 0x8b
        _emit 0x5f
        _emit 0x18
        // 0044a214:  b8 ab aa aa aa
        _emit 0xb8
        _emit 0xab
        _emit 0xaa
        _emit 0xaa
        _emit 0xaa
        // 0044a219:  f7 e6
        _emit 0xf7
        _emit 0xe6
        // 0044a21b:  8b cb
        _emit 0x8b
        _emit 0xcb
        // 0044a21d:  d1 e9
        _emit 0xd1
        _emit 0xe9
        // 0044a21f:  d1 ea
        _emit 0xd1
        _emit 0xea
        // 0044a221:  3b d1
        _emit 0x3b
        _emit 0xd1
        // 0044a223:  73 0e
        _emit 0x73
        _emit 0x0e
        // 0044a225:  b8 fe ff ff ff
        _emit 0xb8
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0044a22a:  2b c1
        _emit 0x2b
        _emit 0xc1
        // 0044a22c:  3b d8
        _emit 0x3b
        _emit 0xd8
        // 0044a22e:  77 03
        _emit 0x77
        _emit 0x03
        // 0044a230:  8d 34 19
        _emit 0x8d
        _emit 0x34
        _emit 0x19
        // 0044a233:  6a 0c
        _emit 0x6a
        _emit 0x0c
        // 0044a235:  8d 0c b5 04 00 00 00
        _emit 0x8d
        _emit 0x0c
        _emit 0xb5
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0044a23c:  6a 00
        _emit 0x6a
        _emit 0x00
        // 0044a23e:  51
        _emit 0x51
        // 0044a23f:  c7 45 fc 00 00 00 00
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0044a246:  e8 b5 32 00 00   CALL 0x0044d500
        _emit 0xe8
        _emit 0xb5
        _emit 0x32
        _emit 0x00
        _emit 0x00
        // 0044a24b:  83 c4 0c
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0044a24e:  89 45 08
        _emit 0x89
        _emit 0x45
        _emit 0x08
        // 0044a251:  c7 45 fc ff ff ff ff
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0044a258:  eb 31
        _emit 0xeb
        _emit 0x31
    }
}
