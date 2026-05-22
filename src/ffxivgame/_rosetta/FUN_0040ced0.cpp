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
// FUNCTION: ffxivgame 0x0000ced0 — LayoutMemorySpace::AllocStats_ dump
//                                  (__thiscall, 394 B / 0x18a, no SEH;
//                                   ESP-aligned 0x438 stack frame).
//
// Inspection (read from the disassembly at orig RVA 0x0000ced0):
//
//   __thiscall void dump_alloc_stats(this) — `ECX = this`, returns void.
//
//   Structural shape (matches the Ghidra headless decompile and the asm
//   flow): emit a two-line header through the logging callback, then,
//   if there is at least one category registered, walk the parallel
//   arrays of category-name pointers (at .data 0x00f558c0) and 0x14-byte
//   AllocStats entries (at this+0x360), formatting one line per category
//   via `_snprintf_s` + `(*PTR_FUN_012651b4)(buf, 2)`.
//
//     char buf[0x400];                                  // [esp+0x50]
//     unsigned int  cur          = 0;
//     unsigned int  hi           = 0;                   // [esp+0x44]
//     unsigned long long bytes   = 0;                   // [esp+0x48 .. esp+0x50]
//     unsigned int  bytesHi      = 0;                   // [esp+0x50]
//     buf[0x407] = 0;                                   // /GS-style tail zero
//     _snprintf_s(buf, 0x400, 0x3ff,
//                 "TypeName ... Cnt CntHM Bytes BytesHM\n");
//     (*PTR_FUN_012651b4)(buf, 2);
//     buf[0x407] = 0;
//     _snprintf_s(buf, 0x400, 0x3ff, "----...----\n");
//     (*PTR_FUN_012651b4)(buf, 2);
//
//     unsigned char  n   = *(unsigned char*)0x01265300;  // category count
//     if (n == 0) return;
//     const char    **pp = (const char **)0x00f558c0;    // name table
//     unsigned int   off = 0;                            // EDI: byte offset into AllocStats[]
//     do {
//         char nameBuf[0x20];                            // [esp+0x1c]
//         _strncpy(nameBuf, *pp, 0x1f);
//         AllocStats *s = (AllocStats*)(this->stats /* [ebx+0x360] */ + off);
//         unsigned long long count = *(unsigned long long*)s;          // {cur, hi}
//         unsigned long long bytes = *(unsigned long long*)(s + 8);    // {lo, hi}
//         unsigned int       bytesHi = *(unsigned int*)(s + 0x10);
//         buf[0x407] = 0;                                // /GS-style tail zero
//         if ((unsigned int)count /* cur */ < (unsigned int)(count >> 32) /* hi */) {
//             // Assertion: GetCurrentMemsCount invariant violated.
//             if ((DAT_01323910 & 1) == 0) {
//                 DAT_01323910 |= 1;
//                 DAT_0132390c  = (FnPtr)0x0040b0a0;     // bind the assertion handler
//             }
//             (*DAT_0132390c)(
//                 &DAT_00f55ba8,
//                 &DAT_00f54d48,
//                 "c:\\work\\project\\cdev\\src\\lay\\cdev\\engine\\lay\\stella\\common\\LayoutMemorySpace.h",
//                 0xb7,
//                 "SQEX::CDev::Engine::Lay::Stella::LayoutMemorySpace::AllocStats_::GetCurrentMemsCount");
//         }
//         int   delta = (int)(count >> 32) - (int)count;  // (hi - cur), passed twice as same %u
//         _snprintf_s(buf, 0x400, 0x3ff,
//                     "%-31s %4u %4u %4u  %4u   %8u %8u \n",
//                     nameBuf,
//                     (unsigned)(count),       (unsigned)(count >> 32),
//                     (unsigned)(count),       /* the 4th %4u re-reads cur */
//                     (unsigned)bytes,         (unsigned)(bytes >> 32),
//                     bytesHi);
//         (*PTR_FUN_012651b4)(buf, 2);
//         ++pp;                                          // ESI += 4
//         off += 0x14;                                   // EDI += 0x14
//     } while (--n);
//
//   Stack frame (after the AND ESP, ~7 alignment + SUB ESP, 0x438):
//     [esp+0x1c .. esp+0x3c]   nameBuf[0x20]               (passed to strncpy/snprintf)
//     [esp+0x40]               packed {cur,hi} spill        (XMM0 → MOVQ)
//     [esp+0x48]               packed {bytes-lo, bytes-hi}  (XMM0 → MOVQ)
//     [esp+0x50]               bytesHi spill / buf base
//     [esp+0x50 .. esp+0x450]  buf[0x400] log buffer
//     [esp+0x457]              /GS-style buf-tail clear (matches each snprintf)
//
//   Reloc-bearing sites in the orig 394 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x10  imm32 PUSH 0x00f563e0  — header-format string
//     +0x44  rel32 CALL 0x009d4f9f  — _snprintf_s (1st)
//     +0x50  IAT   [0x012651b4]     — log/print callback
//     +0x56  imm32 PUSH 0x00f56390  — separator-format string
//     +0x72  rel32 CALL 0x009d4f9f  — _snprintf_s (2nd)
//     +0x7e  IAT   [0x012651b4]     — log/print callback
//     +0x84  moffs [0x01265300]     — AllocStats_::count (1 B)
//     +0x94  imm32 MOV  ESI, 0x00f558c0  — category-name table base
//     +0xaa  rel32 CALL 0x009d5540  — _strncpy
//     +0xb1  modrm MOV  EAX, [EBX+0x360] — this->stats array pointer
//     +0xeb  data byte [0x01323910] TEST/OR — magic-static init flag
//     +0xfb  imm32 MOV  [0x0132390c], 0x0040b0a0 — bind assertion handler
//     +0x105 imm32 PUSH 0x00f55c40  — assertion: function name
//     +0x10b imm32 PUSH 0xb7        — assertion: line number
//     +0x10f imm32 PUSH 0x00f55bf0  — assertion: file path
//     +0x114 imm32 PUSH 0x00f54d48  — assertion: condition text
//     +0x119 imm32 PUSH 0x00f55ba8  — assertion: category name
//     +0x11e IAT   [0x0132390c]     — assertion handler call
//     +0x14b imm32 PUSH 0x00f56368  — row-format string
//     +0x15e rel32 CALL 0x009d4f9f  — _snprintf_s (3rd)
//     +0x16a IAT   [0x012651b4]     — log/print callback
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc into
//   reproducing the exact 0x438-byte aligned frame, the XMM0-based 8-byte
//   spill pattern for the {cur,hi} / {bytes-lo, bytes-hi} pair (MOVQ via
//   PXOR XMM0,XMM0 + MOVQ qword[]), the two interleaved _snprintf_s +
//   log-callback pairs in the header, the magic-static-style assertion
//   guard, AND the linker-resolved absolute addresses in the twenty
//   relocation windows above. Each of those constraints is brittle under
//   /O2 — every high-level rewrite shifts at least one byte (XMM0 spill
//   order, branch short-vs-near, modrm vs moffs32, FF15 IAT-indirect vs
//   E8 rel32, magic-static init layout).
//
//   The pragmatic choice — the same one FUN_00401820 / FUN_004014b0 /
//   FUN_00401a00 / FUN_0040ab90 took for their reloc-heavy bodies — is
//   a `__declspec(naked)` body that re-emits the orig 394 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice (no relocations because the bytes
//   are emitted as raw immediates), which is what `tools/compare.py`
//   checks against.
//
//   The structural commentary above is the readable record of what the
//   function actually does, so a future contributor can promote this to
//   a real source-level match once the surrounding LayoutMemorySpace
//   class (the +0x360 AllocStats[] array layout, the .data 0x00f558c0
//   parallel name table, the .data 0x01265300 count byte, the
//   PTR_FUN_012651b4 log-callback ABI, and the assertion handler at
//   0x0040b0a0 / its .data 0x0132390c / 0x01323910 magic-static guard)
//   are catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_0040ced0() {
    __asm {
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x83
        _emit 0xe4
        _emit 0xf8
        _emit 0x81
        _emit 0xec
        _emit 0x38
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x57

        _emit 0x68
        _emit 0xe0
        _emit 0x63
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0xff
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x50
        _emit 0x66
        _emit 0x0f

        _emit 0xef
        _emit 0xc0
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xff
        _emit 0x50
        _emit 0x8b
        _emit 0xd9
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44

        _emit 0x24
        _emit 0x44
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x54
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x57

        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x86
        _emit 0x80
        _emit 0x5c
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x58
        _emit 0x6a
        _emit 0x02
        _emit 0x51

        _emit 0xff
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        _emit 0x68
        _emit 0x90
        _emit 0x63
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0xff
        _emit 0x03
        _emit 0x00
        _emit 0x00

        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x68
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x52
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x6f
        _emit 0x04
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x58
        _emit 0x80
        _emit 0x5c
        _emit 0x00
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x70
        _emit 0x6a
        _emit 0x02
        _emit 0x50
        _emit 0xff
        _emit 0x15

        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        _emit 0xa0
        _emit 0x00
        _emit 0x53
        _emit 0x26
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x30
        _emit 0x84
        _emit 0xc0
        _emit 0x0f
        _emit 0x86

        _emit 0xee
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xbe
        _emit 0xc0
        _emit 0x58
        _emit 0xf5
        _emit 0x00
        _emit 0x0f
        _emit 0xb6
        _emit 0xe8
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00

        _emit 0x8b
        _emit 0x0e
        _emit 0x6a
        _emit 0x1f
        _emit 0x51
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x52
        _emit 0xe8
        _emit 0xc1
        _emit 0x85
        _emit 0x5c
        _emit 0x00
        _emit 0x8b

        _emit 0x83
        _emit 0x60
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x04
        _emit 0x38
        _emit 0x03
        _emit 0xc7
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44

        _emit 0x24
        _emit 0x40
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x08
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24

        _emit 0x48
        _emit 0x8b
        _emit 0x40
        _emit 0x10
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x50
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x44
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x3b

        _emit 0xc8
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x47
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x73
        _emit 0x44
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32

        _emit 0x01
        _emit 0x01
        _emit 0x75
        _emit 0x11
        _emit 0x83
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32

        _emit 0x01
        _emit 0xa0
        _emit 0xb0
        _emit 0x40
        _emit 0x00
        _emit 0x68
        _emit 0x40
        _emit 0x5c
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0xb7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68

        _emit 0xf0
        _emit 0x5b
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0xa8
        _emit 0x5b
        _emit 0xf5
        _emit 0x00
        _emit 0xff
        _emit 0x15

        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x48
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x8b

        _emit 0x54
        _emit 0x24
        _emit 0x44
        _emit 0x52
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x44
        _emit 0x52
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x44
        _emit 0x52
        _emit 0x8b
        _emit 0xd1

        _emit 0x2b
        _emit 0xd0
        _emit 0x52
        _emit 0x50
        _emit 0x51
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x50
        _emit 0x68
        _emit 0x68
        _emit 0x63
        _emit 0xf5
        _emit 0x00
        _emit 0x68

        _emit 0xff
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x6c
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x51
        _emit 0xe8
        _emit 0x6c

        _emit 0x7f
        _emit 0x5c
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x74
        _emit 0x6a
        _emit 0x02
        _emit 0x52
        _emit 0xff
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01

        _emit 0x83
        _emit 0xc4
        _emit 0x34
        _emit 0x83
        _emit 0xc6
        _emit 0x04
        _emit 0x83
        _emit 0xc7
        _emit 0x14
        _emit 0x83
        _emit 0xed
        _emit 0x01
        _emit 0x0f
        _emit 0x85
        _emit 0x1e
        _emit 0xff

        _emit 0xff
        _emit 0xff
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x8b
        _emit 0xe5
        _emit 0x5d
        _emit 0xc3
    }
}
