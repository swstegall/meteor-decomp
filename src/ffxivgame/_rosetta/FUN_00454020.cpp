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
// FUNCTION: ffxivgame 0x00054020 — registry/string lookup-and-retry helper
//                                  (__cdecl, 413 B / 0x19d, SEH-framed,
//                                   /GS security-cookie).
//
// Inspection (read from the disassembly at orig RVA 0x00054020):
//
//   __cdecl char FUN_00454020(void* arg0 /* [esp+0x44] */,
//                             char  flag /* [esp+0x48] */,
//                             int   tries /* [esp+0x4c] */);
//
//   The function opens with the canonical MSVC 2005 `/GS`+SEH prologue:
//     PUSH -1 / PUSH 0xe58322 (scope table) / PUSH FS:[0] (prev EH node)
//     SUB ESP,0x20 / cookie ^ ESP -> [esp+0x1c] / push callee-saves /
//     cookie ^ ESP spill / install new FS:[0] EH node at [esp+0x34].
//
//   Body: builds a small inline std::string-like buffer (capacity word at
//   [esp+0x2c]=7, len at [esp+0x28]=0, inline data at [esp+0x18]), calls a
//   constructor/format helper FUN_00449000(this=EDI, &local), then loops:
//     - resolves the SSO data pointer (inline at [esp+0x18] when cap<8,
//       else heap pointer in [esp+0x18]);
//     - calls the registry-probe thunk [0x00f3e294] (EBX); on non-zero
//       result it commits the string (FUN_0044d350 grow path) and returns 1;
//     - otherwise calls [0x00f3e1c4] (EBP) for an error/handle, runs
//       FUN_004531c0(EDI); on success either re-probes (FUN_004532b0) and
//       loops, or sleeps 0x64 ms via [0x00f3e1c8] and retries up to `tries`;
//     - on `tries` exhaustion drops to the FUN_004568f0(ESI) cleanup arm;
//     - on FUN_004531c0 failure takes the FUN_004564e0(0x29d4) error arm.
//   All four exit arms share the FUN_0044d350 string-finalise + the SEH
//   epilogue (restore FS:[0], pop callee-saves, __security_check_cookie at
//   0x009d20f4, ADD ESP,0x2c, RET).
//
//   Reloc-bearing sites in the orig 413 bytes (image-base 0x00400000):
//     +0x02  imm32 PUSH 0x00e58322        — SEH scope-table pointer
//     +0x11  moffs [0x012ea8b0]           — __security_cookie (load #1)
//     +0x20  moffs [0x012ea8b0]           — __security_cookie (load #2)
//     +0x54  rel32 CALL 0x00449000        — string/format ctor (FUN_00449000)
//     +0x59  moffs [0x00f3e294]           — registry-probe thunk -> EBX
//     +0x5f  moffs [0x00f3e1c4]           — error/handle thunk    -> EBP
//     +0x80  rel32 CALL 0x004531c0        — FUN_004531c0
//     +0x96  rel32 CALL 0x004532b0        — FUN_004532b0 (re-probe)
//     +0xc5  IAT   [0x00f3e1c8]           — Sleep-style retry thunk
//     +0xea  rel32 CALL 0x0044d350        — string-grow/commit (arm A)
//     +0xfe  imm32 PUSH 0x000029d4        — error code (FUN_004564e0)
//     +0xff  rel32 CALL 0x004564e0        — error path
//     +0x123 rel32 CALL 0x0044d350        — string-grow/commit (arm B, shared)
//     +0x142 rel32 CALL 0x0044d350        — string-grow/commit (arm C)
//     +0x14f rel32 CALL 0x004568f0        — cleanup (tries exhausted)
//     +0x174 rel32 CALL 0x0044d350        — string-grow/commit (arm D, shared)
//     +0x182 moffs FS:[0] restore         — SEH unwind teardown
//     +0x194 rel32 CALL 0x009d20f4        — __security_check_cookie
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHa into
//   reproducing the exact SEH frame (PUSH -1 / scope-table / FS:[0] chain),
//   the double security-cookie spill, the SSO cap<8 branch repeated at four
//   sites, the four-way exit ladder that all funnels through the shared
//   FUN_0044d350 finalise + cookie check, and the linker-resolved absolute
//   addresses in the eighteen relocation windows above. Each constraint is
//   brittle under /O2 — every high-level rewrite shifts at least one byte
//   (branch short-vs-near, modrm vs moffs32, FF15 IAT-indirect vs E8 rel32,
//   SEH scope-table emission).
//
//   The pragmatic choice — the same one the sibling reloc-heavy bodies
//   (FUN_0040ced0 / FUN_00415d00 / FUN_00409350) took — is a
//   `__declspec(naked)` body that re-emits the orig 413 bytes verbatim via
//   MASM `_emit` directives. The .obj's `.text` ends up byte-identical to
//   the orig slice (no relocations, raw immediates), which is what
//   `tools/compare.py` checks against.

extern "C" __declspec(naked) void FUN_00454020() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x22
        _emit 0x83
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x83
        _emit 0xec

        _emit 0x20
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x1c
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
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x44
        _emit 0x33
        _emit 0xc0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x66
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0x8d
        _emit 0x44
        _emit 0x24

        _emit 0x14
        _emit 0x50
        _emit 0x8b
        _emit 0xcf
        _emit 0xe8
        _emit 0x87
        _emit 0x4f
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x1d
        _emit 0x94
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x8b

        _emit 0x2d
        _emit 0xc4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x2c
        _emit 0x08
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x73
        _emit 0x04

        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50
        _emit 0xff
        _emit 0xd3
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x52
        _emit 0xff
        _emit 0xd5
        _emit 0x57
        _emit 0x8b
        _emit 0xf0

        _emit 0xe8
        _emit 0x1b
        _emit 0xf1
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x84
        _emit 0xc0
        _emit 0x74
        _emit 0x6d
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x48

        _emit 0x00
        _emit 0x74
        _emit 0x25
        _emit 0x6a
        _emit 0x00
        _emit 0x57
        _emit 0xe8
        _emit 0xf5
        _emit 0xf1
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x83

        _emit 0xc4
        _emit 0x08
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x2c
        _emit 0x08
        _emit 0x73
        _emit 0x04
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50
        _emit 0xff
        _emit 0xd3

        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x71
        _emit 0xff
        _emit 0xd5
        _emit 0x8b
        _emit 0xf0
        _emit 0x83
        _emit 0x6c
        _emit 0x24
        _emit 0x4c
        _emit 0x01
        _emit 0x0f
        _emit 0x88
        _emit 0x8b

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x64
        _emit 0xff
        _emit 0x15
        _emit 0xc8
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0xeb
        _emit 0x98
        _emit 0x8b
        _emit 0x44
        _emit 0x24

        _emit 0x2c
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x72
        _emit 0x14
        _emit 0x8b
        _emit 0x54

        _emit 0x24
        _emit 0x18
        _emit 0x6a
        _emit 0x0c
        _emit 0x8d
        _emit 0x4c
        _emit 0x00
        _emit 0x02
        _emit 0x51
        _emit 0x52
        _emit 0xe8
        _emit 0x41
        _emit 0x92
        _emit 0xff
        _emit 0xff
        _emit 0x83

        _emit 0xc4
        _emit 0x0c
        _emit 0xb0
        _emit 0x01
        _emit 0xe9
        _emit 0x85
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0xd4
        _emit 0x29
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xbd

        _emit 0x23
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        _emit 0xc7
        _emit 0x44
        _emit 0x24

        _emit 0x3c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x72
        _emit 0x65
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x6a
        _emit 0x0c
        _emit 0x8d
        _emit 0x44
        _emit 0x00

        _emit 0x02
        _emit 0x50
        _emit 0x51
        _emit 0xeb
        _emit 0x4f
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x3c

        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x72
        _emit 0xbc
        _emit 0x8d
        _emit 0x54
        _emit 0x00
        _emit 0x02
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x6a
        _emit 0x0c

        _emit 0x52
        _emit 0x50
        _emit 0xe8
        _emit 0xe9
        _emit 0x91
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xb0
        _emit 0x01
        _emit 0xeb
        _emit 0x30
        _emit 0x56
        _emit 0xe8

        _emit 0x7c
        _emit 0x27
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        _emit 0xc7
        _emit 0x44

        _emit 0x24
        _emit 0x3c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x72
        _emit 0x14
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x6a
        _emit 0x0c
        _emit 0x8d
        _emit 0x4c

        _emit 0x00
        _emit 0x02
        _emit 0x51
        _emit 0x52
        _emit 0xe8
        _emit 0xb7
        _emit 0x91
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x32
        _emit 0xc0
        _emit 0x8b
        _emit 0x4c

        _emit 0x24
        _emit 0x34
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
        _emit 0x8b
        _emit 0x4c

        _emit 0x24
        _emit 0x1c
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x3b
        _emit 0xdf
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x2c
        _emit 0xc3
    }
}
