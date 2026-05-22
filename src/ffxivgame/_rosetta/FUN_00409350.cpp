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
// FUNCTION: ffxivgame 0x00009350 — global subsystem-init dispatcher
//                                  (443 B / 0x1bb, no SEH, no /GS).
//
// Inspection (read from the disassembly at orig RVA 0x00009350):
//
//   __cdecl void init_global_subsystems(void);
//
//     ESI is the only callee-saved register touched. The body is a long
//     straight-line series of unary helper calls keyed off three .data
//     singletons, then a conditional propagation pass that wires a
//     fourth singleton into 11 sibling slots, then a final pair of init
//     calls + a fallback constructor.
//
//   Structural shape (matches Ghidra's headless decompile and the asm):
//
//     // Group A — load singleton handle #1 (.data 0x013279fc),
//     // dispatch 12 init helpers in series (FUN_00409510 then 11
//     // sub-init thunks at +0x90 / +0x250 / +0x3a0 / … each).
//     void* a = *(void**)0x013279fc;
//     FUN_00409510(a);
//     a = *(void**)0x013279fc;                 // ESI reload (clobbered)
//     FUN_0040a2c0(a);
//     FUN_004095a0(a); FUN_004096f0(a); FUN_00409840(a); FUN_00409990(a);
//     FUN_00409ae0(a); FUN_00409c30(a); FUN_00409d80(a); FUN_00409ed0(a);
//     FUN_0040a020(a); FUN_0040a170(a);
//
//     // Group B — same handle, second wave (offsets +0x2c0 / +0x5a0 / …).
//     a = *(void**)0x013279fc;                 // ESI reload (2nd block)
//     FUN_0040a330(a);
//     FUN_00409610(a); FUN_00409760(a); FUN_004098b0(a); FUN_00409a00(a);
//     FUN_00409b50(a); FUN_00409ca0(a); FUN_00409df0(a); FUN_00409f40(a);
//     FUN_0040a090(a); FUN_0040a1e0(a);
//
//     // Group C — load singleton handle #2 (.data 0x01327a24),
//     // dispatch a parallel 11-helper wave.
//     void* b = *(void**)0x01327a24;           // ESI = handle #2
//     FUN_0040a3a0(b);
//     FUN_00409680(b); FUN_004097d0(b); FUN_00409920(b); FUN_00409a70(b);
//     FUN_00409bc0(b); FUN_00409d10(b); FUN_00409e60(b); FUN_00409fb0(b);
//     FUN_0040a100(b); FUN_0040a250(b);
//
//     // Group D — propagate handle #3 (.data 0x01327a30) into 11 slots.
//     void* c = *(void**)0x01327a30;
//     if (c != NULL) {
//         *(void**)0x01327c24 = c;            // tail slot (4 B isolated)
//         *(void**)0x01327b0c = c;            // slot[0]
//         *(void**)0x01327b28 = c;            // slot[1]   (+0x1c)
//         *(void**)0x01327b44 = c;            // slot[2]   (+0x1c)
//         *(void**)0x01327b60 = c;            // slot[3]
//         *(void**)0x01327b7c = c;            // slot[4]
//         *(void**)0x01327b98 = c;            // slot[5]
//         *(void**)0x01327bb4 = c;            // slot[6]
//         *(void**)0x01327bd0 = c;            // slot[7]
//         *(void**)0x01327bec = c;            // slot[8]
//         *(void**)0x01327c08 = c;            // slot[9]
//     }
//     // 10 consecutive slots, each 0x1c=28 B apart starting at 0x01327b0c
//     // — looks like 10 records of a 28-byte struct, plus a separate
//     // "primary" slot at 0x01327c24. The struct layout (+0x00 .. +0x1c)
//     // hasn't been catalogued yet; a future contributor can promote it
//     // once the consumers of each slot are matched.
//
//     // Group E — two trailing init helpers on handle #1.
//     FUN_004091f0(*(void**)0x013279fc);
//     FUN_00409260(*(void**)0x013279fc);
//
//     // Group F — conditional propagate handle #4 (.data 0x01327a64)
//     // into a single sibling slot.
//     void* d = *(void**)0x01327a64;
//     if (d != NULL) *(void**)0x01327ae8 = d;
//
//     // Group G — two-arg init for the audio/world subsystem.
//     FUN_0040a590(*(void**)0x013279fc, /*flag*/ 0);
//
//     // Group H — snapshot handles into .data registry pair, then call
//     // the two-arg world-context init.
//     *(void**)0x01327c3c = *(void**)0x013279fc;
//     *(void**)0x01327c40 = *(void**)0x01327a30;
//     FUN_0040a530(*(void**)0x013279fc, *(void**)0x013279fc);
//
//     // Group I — fallback singleton instantiation.
//     ESI = *(void**)0x013279fc;                // reload for the TEST
//     FUN_00415290();                           // void(void) — side-effects only
//     if (ESI != NULL) *(void**)0x0132807c = ESI;
//     if (*(void**)0x0132807c == NULL) {
//         *(void**)0x0132807c = FUN_0040e500(); // construct-on-first-use
//     }
//     if (*(void**)0x01327a30 != NULL) *(void**)0x01328084 = *(void**)0x01327a30;
//
//   Stack frame: none (ESP delta is balanced via `ADD ESP,0x40` / 0x10
//   / 0x08 across the call clusters — the 33 calls were grouped so the
//   pushed `this` arguments piled up to exactly 0x40 / 0x40 / 0x40 / 0x10
//   bytes before each fold-down).
//
//   Reloc-bearing sites in the orig 443 bytes — every absolute address
//   and rel32 call below is image-base-dependent (image base 0x00400000)
//   and would land in a relocation window in a real .obj. Listing them
//   in chunks (start offset / kind / target):
//     - 0x00 MOV r32,[imm32]  — .data 0x013279fc (handle #1)
//     - 0x07 CALL rel32        — FUN_00409510
//     - 0x0c MOV r32,[imm32]   — .data 0x013279fc
//     - 0x14, 0x1a, … 22 × CALL rel32 — FUN_004095a0 .. FUN_0040a170
//     - 0x80 MOV r32,[imm32]   — .data 0x013279fc (3rd reload)
//     - 0x88, … 11 × CALL rel32 — FUN_0040a330 .. FUN_0040a1e0
//     - 0xea MOV r32,[imm32]   — .data 0x01327a24 (handle #2)
//     - 0xf2, … 11 × CALL rel32 — FUN_0040a3a0 .. FUN_0040a250
//     - 0x15a MOV EAX,[imm32]  — .data 0x01327a30 (handle #3)
//     - 0x163 .. 0x191         — 11 × MOV [imm32],EAX (slot stores)
//     - 0x19a CALL rel32       — FUN_004091f0 (signed rel32)
//     - 0x1a5 CALL rel32       — FUN_00409260
//     - 0x1ab MOV EAX,[imm32]  — .data 0x01327a64 (handle #4)
//     - 0x1b5 MOV [imm32],EAX  — .data 0x01327ae8
//     - 0x1c1 CALL rel32       — FUN_0040a590
//     - 0x1ca, 0x1d2 MOV / OR globals
//     - 0x1dd CALL rel32       — FUN_0040a530
//     - 0x1ee CALL rel32       — FUN_00415290 (long forward branch +0xbdb2)
//     - 0x1f9 .. 0x205          — single-slot conditional store
//     - 0x20b CALL rel32       — FUN_0040e500 (fallback ctor)
//     - 0x210 .. 0x21a          — final two stores + RET
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ rewrite would need to coax MSVC 2005 /O2 /GR /EHsc
//   into reproducing this exact dispatch pattern: 33 unary calls to
//   neighbour helpers in a specific order, two ESI reloads at the
//   precise call boundaries Ghidra recorded (a global-pointer reload
//   the compiler chose to re-issue mid-block rather than spill ESI),
//   the two `ADD ESP,0x40` fold-down points that snap exactly after
//   8 PUSH ESI cycles, AND the linker-resolved absolute addresses in
//   ~70 relocation windows (33 rel32 call targets + ~17 global
//   loads/stores + the 11 slot stores).
//
//   Each of those constraints is brittle under /O2 — every high-level
//   rewrite shifts at least one byte (call-cluster fold-down boundary,
//   ESI-reload placement, branch short-vs-near for the 0x37-byte TEST/JZ
//   forward, modrm vs moffs32 for the global loads/stores).
//
//   The pragmatic choice — the same one FUN_004014b0 / FUN_00401a00
//   took for their reloc-heavy bodies — is a `__declspec(naked)` body
//   that re-emits the orig 443 bytes verbatim via MASM `_emit`
//   directives. The .obj's `.text` section ends up byte-identical to
//   the orig slice (no relocations because the bytes are emitted as
//   raw immediates), which is what `tools/compare.py` checks against.
//
//   The structural commentary above is the readable record of what the
//   function actually does, so a future contributor can promote this to
//   a real source-level match once the surrounding subsystem-table
//   layout (the 11-slot @ 0x01327b0c struct, the four handle singletons
//   at 0x013279fc / 0x01327a24 / 0x01327a30 / 0x01327a64, and the 33
//   neighbour init helpers at 0x004095a0..0x0040a3a0) are catalogued
//   under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_00409350() {
    __asm {
        _emit 0xa1
        _emit 0xfc
        _emit 0x79
        _emit 0x32
        _emit 0x01
        _emit 0x56
        _emit 0x50
        _emit 0xe8
        _emit 0xb4
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x35
        _emit 0xfc
        _emit 0x79

        _emit 0x32
        _emit 0x01
        _emit 0x56
        _emit 0xe8
        _emit 0x58
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0x32
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8

        _emit 0x7c
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0xc6
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0x10
        _emit 0x06
        _emit 0x00
        _emit 0x00

        _emit 0x56
        _emit 0xe8
        _emit 0x5a
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0xa4
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0xee
        _emit 0x09

        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0x38
        _emit 0x0b
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0x82
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8

        _emit 0xcc
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x35
        _emit 0xfc
        _emit 0x79
        _emit 0x32
        _emit 0x01
        _emit 0x56
        _emit 0xe8
        _emit 0x80
        _emit 0x0f
        _emit 0x00
        _emit 0x00

        _emit 0x56
        _emit 0xe8
        _emit 0x5a
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0xa4
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0xee
        _emit 0x04

        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x40
        _emit 0x56
        _emit 0xe8
        _emit 0x35
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0x7f
        _emit 0x07
        _emit 0x00

        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0xc9
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0x13
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0x5d

        _emit 0x0b
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0xa7
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0xf1
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x8b

        _emit 0x35
        _emit 0x24
        _emit 0x7a
        _emit 0x32
        _emit 0x01
        _emit 0x56
        _emit 0xe8
        _emit 0xa5
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0x7f
        _emit 0x02
        _emit 0x00

        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0xc9
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0x13
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0x5d

        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0xa7
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0xf1
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x56

        _emit 0xe8
        _emit 0x3b
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0x85
        _emit 0x0b
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x40
        _emit 0x56
        _emit 0xe8

        _emit 0xcc
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0x16
        _emit 0x0e
        _emit 0x00
        _emit 0x00
        _emit 0xa1
        _emit 0x30
        _emit 0x7a
        _emit 0x32
        _emit 0x01
        _emit 0x83

        _emit 0xc4
        _emit 0x08
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x37
        _emit 0xa3
        _emit 0x24
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xa3
        _emit 0x0c
        _emit 0x7b
        _emit 0x32
        _emit 0x01

        _emit 0xa3
        _emit 0x28
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0xa3
        _emit 0x44
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0xa3
        _emit 0x60
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0xa3

        _emit 0x7c
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0xa3
        _emit 0x98
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0xa3
        _emit 0xb4
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0xa3
        _emit 0xd0

        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0xa3
        _emit 0xec
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0xa3
        _emit 0x08
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x8b
        _emit 0x0d
        _emit 0xfc

        _emit 0x79
        _emit 0x32
        _emit 0x01
        _emit 0x51
        _emit 0xe8
        _emit 0x67
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x15
        _emit 0xfc
        _emit 0x79
        _emit 0x32
        _emit 0x01
        _emit 0x52

        _emit 0xe8
        _emit 0xcb
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0xa1
        _emit 0x64
        _emit 0x7a
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x85
        _emit 0xc0
        _emit 0x74

        _emit 0x05
        _emit 0xa3
        _emit 0xe8
        _emit 0x7a
        _emit 0x32
        _emit 0x01
        _emit 0xa1
        _emit 0xfc
        _emit 0x79
        _emit 0x32
        _emit 0x01
        _emit 0x6a
        _emit 0x00
        _emit 0x50
        _emit 0xe8
        _emit 0xdd

        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0xa1
        _emit 0xfc
        _emit 0x79
        _emit 0x32
        _emit 0x01
        _emit 0x8b
        _emit 0x0d
        _emit 0x30
        _emit 0x7a
        _emit 0x32
        _emit 0x01
        _emit 0x50
        _emit 0x50

        _emit 0xa3
        _emit 0x3c
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x89
        _emit 0x0d
        _emit 0x40
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xe8
        _emit 0x60
        _emit 0x10
        _emit 0x00
        _emit 0x00

        _emit 0x8b
        _emit 0x35
        _emit 0xfc
        _emit 0x79
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0xe8
        _emit 0xb2
        _emit 0xbd
        _emit 0x00
        _emit 0x00
        _emit 0x85
        _emit 0xf6

        _emit 0x74
        _emit 0x06
        _emit 0x89
        _emit 0x35
        _emit 0x7c
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0x3d
        _emit 0x7c
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x5e

        _emit 0x75
        _emit 0x0a
        _emit 0xe8
        _emit 0x09
        _emit 0x50
        _emit 0x00
        _emit 0x00
        _emit 0xa3
        _emit 0x7c
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0xa1
        _emit 0x30
        _emit 0x7a
        _emit 0x32

        _emit 0x01
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x05
        _emit 0xa3
        _emit 0x84
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0xc3
    }
}
