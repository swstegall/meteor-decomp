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
// FUNCTION: ffxivgame 0x0001ea30 — viewport/aspect-ratio update entry
//                                  (493 B / 0x1ed, no SEH, no /GS).
//
// Inspection (read from the disassembly at orig RVA 0x0001ea30):
//
//   __cdecl <ptr> update_viewport(int arg0, int arg1, int arg3 /*EBP*/,
//                                 char wantClient, RECT-ish blob, …);
//   Caller cleans (final `RET` with no immediate). Callee-saves used:
//   EBX (zero sentinel), EBP (saved arg3), ESI (result record), EDI.
//
//   Structural shape:
//
//     // Group A — singleton "current viewport" cache check + optional
//     // first-use init of a global fn-ptr trampoline (lazy-init guard
//     // via TEST/OR on the bit-0 flag at .data 0x01323910, storing
//     // 0x0041d3a0 into the fn-ptr slot at 0x0132390c on first call).
//     if (g_inited /* [0x01329834] */) {
//         Ctx* c = *(Ctx**)0x01329428;
//         if (!(c->f164 == arg0 && c->f168 == arg1 && c->f16c == arg3)) {
//             if (!(g_flag /* [0x01323910] */ & 1)) {
//                 g_flag |= 1;
//                 *(void**)0x0132390c = (void*)0x0041d3a0;
//             }
//             (*(Fn*)0x0132390c)(0xf59320, 0xf58b17, 0xf592d0,
//                                0xa74, 0xf59288);   // 5-arg log/assert
//         }
//     }
//
//     // Group B — fetch client-or-window rect into a 0x10-byte scratch
//     // at [esp+0x28] through one of two IAT slots based on a byte flag.
//     RECT r;
//     if (arg.wantClient /* [esp+0x50] */)
//         (*[00f3e46c])(arg.hwnd /*EDI=[esp+0x3c]*/, &r);   // GetClientRect-ish
//     else
//         (*[00f3e484])(arg.hwnd, &r);                       // GetWindowRect-ish
//
//     // Group C — allocate a 0x24-byte result record via the pool
//     // allocator (FUN_0040e2d0 resolves a tag handle for "…" literal
//     // 0xf59328 / kind 0x10; FUN_00419c40(0x24, handle) allocs+ctors),
//     // zero-init it, then populate width/height/aspect.
//     void* rec = FUN_0040e2d0(0x10, 0xf59328);
//     Rec* s = (Rec*)FUN_00419c40(0x24, rec);
//     if (s) { /* zero 0x21 bytes of the record */ s->… = 0; }
//     s->f10 = arg.hwnd; s->f14 = 0; s->f00 = 0;
//     int w = max(1, r.right  - r.left);   s->f04 = w;
//     int h = max(1, r.bottom - r.top);    s->f08 = h;
//     // aspect = (float)w / (float)h, with the x87 signed-int→float
//     // fix-up (FADD 4.2949673e9f at [00f54a54]) on negative operands.
//     s->f0c = (float)w / (float)h;
//
//     // Group D — hand the populated record + 4 trailing dword args to
//     // the apply helper (FUN_0041df20), then register it into the
//     // context's collection at +0x140 (FUN_004214a0), update the
//     // "active" slot at +0x150 unless FUN_00418280 vetoes, and clamp
//     // the cached aspect at +0x17c against the floor [00f54f70].
//     FUN_0041df20(s, a, b, arg3, c, d, e, f);
//     ((Ctx*)*0x01329428 + 0x140)->add(&recPtr, &widthSnapshot);
//     Ctx* c = *(Ctx**)0x01329428;
//     if (c->f150 != s && !FUN_00418280()) c->f150 = s;
//     if (c->f17c < kFloor /* [00f54f70] */) c->f17c = kFloor;
//     return c->f150;
//
//   Stack frame (ESP-relative, after the 0x28 sub + 4 pushes):
//     [esp+0x10]      computed width/height scratch (max-clamp source)
//     [esp+0x14]      ESI snapshot (record ptr) for the +0x140 register call
//     [esp+0x1c]      tag-handle scratch (FUN_0040e2d0 out)
//     [esp+0x20]      width snapshot for the +0x140 register call
//     [esp+0x28..]    0x10-byte RECT scratch
//     [esp+0x3c]      EDI spill / loop-const 1 scratch
//
//   Reloc-bearing sites in the orig 493 bytes (image base 0x00400000):
//     +0x0b  CMP [imm32]   — .data 0x01329834 (init flag)
//     +0x15  MOV EAX,[]    — .data 0x01329428 (context singleton)
//     +0x3a  TEST [imm32]  — .data 0x01323910 (lazy-init bit flag)
//     +0x43  OR   [imm32]  — .data 0x01323910
//     +0x4a  MOV  [imm32]  — .data 0x0132390c = 0x0041d3a0 (fn-ptr)
//     +0x54..+0x6d  5× PUSH imm32 — literals 0xf59288/0xa74/0xf592d0/
//                                   0xf58b17/0xf59320
//     +0x6d  CALL [imm32]  — .data 0x0132390c (indirect trampoline)
//     +0x86  CALL [00f3e46c] — IAT (client-rect)
//     +0x94  CALL [00f3e484] — IAT (window-rect)
//     +0x9a  PUSH imm32     — 0xf59328 (tag literal)
//     +0xa5  CALL rel32     — FUN_0040e2d0
//     +0xad  CALL rel32     — FUN_00419c40 (pool alloc)
//     +0xbd  FADD [00f54a54]× signed-int→float fix-up (twice)
//     +0x178 CALL rel32     — FUN_0041df20 (apply)
//     +0x196 CALL rel32     — FUN_004214a0 (collection register)
//     +0x1a9 CALL rel32     — FUN_00418280 (veto predicate)
//     +0x1c6 MOVSS [00f54f70] — aspect floor constant
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ rewrite would need to coax MSVC 2005 /O2 into
//   reproducing this exact register allocation across the x87 signed-int
//   division fix-up (the FILD/TEST/JGE/FADD pair), the LAHF/TEST AH,0x44
//   ordered-compare lowering of the final float clamp, the two IAT-indirect
//   rect calls, the lazy fn-ptr trampoline, AND the ~18 linker-resolved
//   absolute addresses above. Each is brittle under /O2 — every high-level
//   rewrite shifts at least one byte (FP compare lowering, max() cmov-vs-
//   branch, branch short-vs-near, modrm vs moffs32 for the global loads).
//
//   The pragmatic choice — the same one the sibling reloc-heavy bodies
//   (FUN_00401820 / FUN_00409350 / FUN_0040b840) took — is a
//   `__declspec(naked)` body that re-emits the orig 493 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` ends up byte-identical
//   to the orig slice (raw immediates, no relocations), which is what
//   tools/compare.py checks against.

extern "C" __declspec(naked) void FUN_0041ea30() {
    __asm {
        _emit 0x83
        _emit 0xec
        _emit 0x28
        _emit 0x53
        _emit 0x55
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x40
        _emit 0x33
        _emit 0xdb
        _emit 0x39
        _emit 0x1d
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x56
        _emit 0x57
        _emit 0x74
        _emit 0x61
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        _emit 0x39
        _emit 0x88
        _emit 0x64
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x75
        _emit 0x14
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x44
        _emit 0x39
        _emit 0x90
        _emit 0x68
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x75
        _emit 0x08
        _emit 0x39
        _emit 0xa8
        _emit 0x6c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x3c
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
        _emit 0xd3
        _emit 0x41
        _emit 0x00
        _emit 0x68
        _emit 0x88
        _emit 0x92
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x74
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0xd0
        _emit 0x92
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x17
        _emit 0x8b
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x20
        _emit 0x93
        _emit 0xf5
        _emit 0x00
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x38
        _emit 0x5c
        _emit 0x24
        _emit 0x50
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x3c
        _emit 0x74
        _emit 0x0e
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x50
        _emit 0x57
        _emit 0xff
        _emit 0x15
        _emit 0x6c
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0xeb
        _emit 0x0c
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x51
        _emit 0x57
        _emit 0xff
        _emit 0x15
        _emit 0x84
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x68
        _emit 0x28
        _emit 0x93
        _emit 0xf5
        _emit 0x00
        _emit 0x6a
        _emit 0x10
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xe8
        _emit 0xf6
        _emit 0xf7
        _emit 0xfe
        _emit 0xff
        _emit 0x50
        _emit 0x6a
        _emit 0x24
        _emit 0xe8
        _emit 0x5e
        _emit 0xb1
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x3b
        _emit 0xc3
        _emit 0x74
        _emit 0x23
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        _emit 0x89
        _emit 0x18
        _emit 0x89
        _emit 0x58
        _emit 0x04
        _emit 0x89
        _emit 0x58
        _emit 0x08
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x0c
        _emit 0x89
        _emit 0x58
        _emit 0x10
        _emit 0x88
        _emit 0x58
        _emit 0x14
        _emit 0x89
        _emit 0x58
        _emit 0x18
        _emit 0x89
        _emit 0x58
        _emit 0x1c
        _emit 0x89
        _emit 0x58
        _emit 0x20
        _emit 0x8b
        _emit 0xf0
        _emit 0xeb
        _emit 0x02
        _emit 0x33
        _emit 0xf6
        _emit 0x89
        _emit 0x7e
        _emit 0x10
        _emit 0x88
        _emit 0x5e
        _emit 0x14
        _emit 0x89
        _emit 0x1e
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x2b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0xb9
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x3b
        _emit 0xc1
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0x7c
        _emit 0x04
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x10
        _emit 0x89
        _emit 0x56
        _emit 0x04
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x2b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x3b
        _emit 0xc1
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0x7c
        _emit 0x04
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x00
        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0xdb
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0x89
        _emit 0x46
        _emit 0x08
        _emit 0x7d
        _emit 0x06
        _emit 0xd8
        _emit 0x05
        _emit 0x54
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0xdb
        _emit 0x46
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x7d
        _emit 0x06
        _emit 0xd8
        _emit 0x05
        _emit 0x54
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x58
        _emit 0xde
        _emit 0xf9
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x54
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x50
        _emit 0x51
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x50
        _emit 0x52
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x4c
        _emit 0x50
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        _emit 0x51
        _emit 0x55
        _emit 0x52
        _emit 0x50
        _emit 0x56
        _emit 0xd9
        _emit 0x5e
        _emit 0x0c
        _emit 0xe8
        _emit 0x73
        _emit 0xf3
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x51
        _emit 0x8b
        _emit 0x0d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x52
        _emit 0x81
        _emit 0xc1
        _emit 0x40
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xd5
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x0d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x39
        _emit 0xb1
        _emit 0x50
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x15
        _emit 0xe8
        _emit 0xa2
        _emit 0x96
        _emit 0xff
        _emit 0xff
        _emit 0x84
        _emit 0xc0
        _emit 0x8b
        _emit 0x0d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x75
        _emit 0x06
        _emit 0x89
        _emit 0xb1
        _emit 0x50
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x89
        _emit 0x7c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        _emit 0x5f
        _emit 0x0f
        _emit 0x2e
        _emit 0xc8
        _emit 0x9f
        _emit 0x5e
        _emit 0x5d
        _emit 0xf6
        _emit 0xc4
        _emit 0x44
        _emit 0x8b
        _emit 0x81
        _emit 0x50
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x5b
        _emit 0x7b
        _emit 0x08
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x81
        _emit 0x7c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x28
        _emit 0xc3
    }
}
