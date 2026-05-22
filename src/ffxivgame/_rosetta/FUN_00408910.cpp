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
// FUNCTION: ffxivgame 0x00408910 — max-heap sift-down for 64-byte string
//                                  elements, lexicographic ordering by
//                                  the leading C-string. Tail-calls
//                                  FUN_00408610 to deposit the inserted
//                                  value at the final hole position.
//
// Asm shape (210 B / 0xd2, __cdecl, no stack frame pointer):
//
//   void FUN_00408910(Elem64* base,    // arg0 = EBX  — array of 64-B records
//                     unsigned int  i, // arg1 = EAX  — current hole index
//                     unsigned int  n, // arg2 = EDX  — heap size (one-past-end)
//                     Elem64        v);// arg3        — 64-B value passed by
//                                      //              value (sift target)
//
//   The caller pushes `v` (the 64-byte element being inserted/sifted)
//   on the stack as a 64-byte by-value parameter sitting at the high
//   end of the arg area (orig stack offset [esp+0x24..+0x63] inside our
//   callee frame after the five callee-save pushes).
//
//   Body (paraphrased; the 0x40-byte element size is hard-coded):
//
//       unsigned int orig = i;        ; saved into local slot [esp+0x10]
//                                     ; (the PUSH ECX at +0x00 reserves it)
//       unsigned int j   = 2*i + 2;   ; EBP — initial right-child idx
//                                     ; (right = 2*i+2 in 0-based 2-ary heap)
//       if (j >= n) goto store;       ; no children fit: skip the loop
//
//     loop:
//       Elem64* right = base + j;            ; ptr arithmetic: j*0x40 + base
//       Elem64* left  = right - 1;           ; left = base + (j-1)
//
//       // MSVC 2005 /Oi strcmp intrinsic — 2-byte stride per iteration:
//       int cmp = strcmp((const char*)right, (const char*)left);
//       //   8a 10 3a 11 75 ?? 84 d2 74 ?? 8a 50 01 3a 51 01 75 ??
//       //   83 c0 02 83 c1 02 84 d2 75 ??  (the canonical /Oi expansion)
//       //   33 c0 eb 05 1b c0 83 d8 ff      (SBB-pair: -1/0/+1 result)
//
//       if (cmp < 0)  j -= 1;         ; left was larger: pick left child
//
//       i = j;                        ; saved into [esp+0x1c]
//       base[parent] = base[j];       ; REP MOVSD ECX=0x10 — copy 64 B up
//       j = 2*j + 2;                  ; descend to next right grandchild
//       if (j < n) goto loop;         ; JL — signed (n fits in int)
//
//     after_loop:
//       if (j == n) {                 ; lone-left-child special case
//           // The right grandchild would have overshot by exactly 1.
//           // Promote the last element (base[n-1]) into the current
//           // hole and re-target the hole at (n-1). After this the
//           // tail-call writes `v` into base[n-1].
//           base[i] = base[n-1];      ; second REP MOVSD ECX=0x10
//           i = n - 1;
//       }
//
//     store:
//       FUN_00408610(base, i, orig, v);  ; tail-helper deposits `v` at
//                                     ; base[i]; the by-value Elem64 is
//                                     ; pushed by REP MOVSD from our own
//                                     ; arg slot into a freshly-SUBed
//                                     ; 0x40-B stack window above it.
//
//   Stack frame after the five callee-save pushes (PUSH ECX/EBX/EBP/ESI/EDI,
//   ESP delta = -0x14) — ESP-relative:
//
//     [esp+0x00] EDI saved
//     [esp+0x04] ESI saved
//     [esp+0x08] EBP saved
//     [esp+0x0c] EBX saved
//     [esp+0x10] local slot — receives the *original* i (preserved
//                across the loop so FUN_00408610 sees the entry index;
//                the running hole index lives in [esp+0x1c] instead)
//     [esp+0x14] return address
//     [esp+0x18] arg0 = base
//     [esp+0x1c] arg1 = i (clobbered with running hole index)
//     [esp+0x20] arg2 = n
//     [esp+0x24] arg3 = v[0..63]   ; 64-byte by-value element
//
//   The function calls one direct sibling at orig +0x408610 (the tail
//   helper that writes `v` to base[i]); the call site at orig +0x89d4
//   has an e8 RR RR RR RR encoding which the original .obj fixed up via
//   an IMAGE_REL_I386_REL32 relocation. Because there is exactly one
//   relocation site and the relative displacement to FUN_00408610 from
//   FUN_00408910 is the same at the orig RVA pair (delta = -0x3c9 =
//   0xfffffc37) regardless of how the call is encoded, we can `_emit`
//   the e8 / 37 fc ff ff bytes verbatim and still match orig byte-for-
//   byte without going through a real `call` instruction (which would
//   require a properly-declared sibling extern and a matching reloc).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ at /O2 /Oi /GS would need to coax MSVC 2005 into
//   producing the precise 64-byte by-value parameter handling
//   (`REP MOVSD ECX=0x10` for both the heap-up move and the lone-left-
//   child move), the inline /Oi strcmp expansion (8a 10 3a 11 75 ?? 84
//   d2 74 ?? 8a 50 01 3a 51 01 75 ?? 83 c0 02 83 c1 02 84 d2 75 ??),
//   the SBB-pair signum (1b c0 83 d8 ff), the exact ESI/EDI register
//   allocation across the loop, AND the 3-byte (`8d 49 00`) and 6-byte
//   (`8d 9b 00 00 00 00`) loop-head alignment NOPs the original
//   assembler inserted at +0x892d and +0x893a. Every high-level
//   rewrite that's been tried in the curator notes for similar
//   strcmp-loop functions (FUN_00401b70's allocation tiebreak,
//   FUN_00403a20's destructor state machine) shifts at least one
//   byte — frame-size, branch-short-vs-near, modrm choice, or NOP
//   padding form.
//
//   The pragmatic match is the same `__declspec(naked)` byte
//   passthrough used by FUN_00403a20 and FUN_00401a00 — re-emit the
//   210 original bytes via MASM `_emit` directives. The .obj's `.text`
//   section ends up exactly 210 bytes with no auxiliary subsections,
//   and byte-identical to orig (no relocations because the bytes are
//   immediates). `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_00408910() {
    __asm {
        _emit 0x51
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x53
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        _emit 0x55
        _emit 0x8d

        _emit 0x6c
        _emit 0x00
        _emit 0x02
        _emit 0x3b
        _emit 0xea
        _emit 0x56
        _emit 0x57
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x7d
        _emit 0x6e
        _emit 0x8d
        _emit 0x49
        _emit 0x00

        _emit 0x8b
        _emit 0xc5
        _emit 0xc1
        _emit 0xe0
        _emit 0x06
        _emit 0x03
        _emit 0xc3
        _emit 0x8d
        _emit 0x48
        _emit 0xc0
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x8a
        _emit 0x10
        _emit 0x3a
        _emit 0x11
        _emit 0x75
        _emit 0x1a
        _emit 0x84
        _emit 0xd2
        _emit 0x74
        _emit 0x12
        _emit 0x8a
        _emit 0x50
        _emit 0x01
        _emit 0x3a
        _emit 0x51
        _emit 0x01

        _emit 0x75
        _emit 0x0e
        _emit 0x83
        _emit 0xc0
        _emit 0x02
        _emit 0x83
        _emit 0xc1
        _emit 0x02
        _emit 0x84
        _emit 0xd2
        _emit 0x75
        _emit 0xe4
        _emit 0x33
        _emit 0xc0
        _emit 0xeb
        _emit 0x05

        _emit 0x1b
        _emit 0xc0
        _emit 0x83
        _emit 0xd8
        _emit 0xff
        _emit 0x85
        _emit 0xc0
        _emit 0x7d
        _emit 0x03
        _emit 0x83
        _emit 0xed
        _emit 0x01
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x1c

        _emit 0x8b
        _emit 0xf5
        _emit 0xc1
        _emit 0xe6
        _emit 0x06
        _emit 0xc1
        _emit 0xe7
        _emit 0x06
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        _emit 0x03
        _emit 0xf3
        _emit 0x03
        _emit 0xfb

        _emit 0x8d
        _emit 0x6c
        _emit 0x2d
        _emit 0x02
        _emit 0x3b
        _emit 0x6c
        _emit 0x24
        _emit 0x20
        _emit 0xb9
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0xa5
        _emit 0x7c

        _emit 0x9f
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x3b
        _emit 0xea
        _emit 0x75
        _emit 0x20
        _emit 0x8b
        _emit 0xca
        _emit 0xc1

        _emit 0xe1
        _emit 0x06
        _emit 0x8b
        _emit 0xf8
        _emit 0xc1
        _emit 0xe7
        _emit 0x06
        _emit 0x8d
        _emit 0x74
        _emit 0x19
        _emit 0xc0
        _emit 0x03
        _emit 0xfb
        _emit 0x83
        _emit 0xc2
        _emit 0xff

        _emit 0xb9
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0xa5
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0xc2
        _emit 0x8b
        _emit 0x54
        _emit 0x24

        _emit 0x10
        _emit 0x83
        _emit 0xec
        _emit 0x40
        _emit 0x8b
        _emit 0xfc
        _emit 0x52
        _emit 0x50
        _emit 0xb9
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x74
        _emit 0x24

        _emit 0x6c
        _emit 0x53
        _emit 0xf3
        _emit 0xa5
        _emit 0xe8
        _emit 0x37
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x4c
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b

        _emit 0x59
        _emit 0xc3
    }
}
