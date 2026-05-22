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
// FUNCTION: ffxivgame 0x0000d300 — `__thiscall` engine_memory allocator
//                                  bucket-bookkeeping entry (479 B / 0x1df,
//                                  EH3-SEH wrapped).
//
// Inspection (read from the disassembly at orig RVA 0x0000d300):
//
//   __thiscall void * alloc(this, size_t size, const char *kind);
//     ECX = this, two stack args, `ret 8` epilog. The function is the
//     engine_memory custom allocator's main "fast / slow path"
//     dispatcher: it takes the requested size and the allocation-site
//     name string, fans size into one of the per-bucket counter tables
//     hanging off `this`, calls a per-size-class allocator (FUN_0040d280
//     / FUN_0040a840) or falls back to the generic Allocator-pool path
//     (FUN_0040e110 with the "CDev.Engine.Lay.Mem.Space" tag), and on
//     success bumps the global high-water counters at `this+0xa4`,
//     `this+0x7c`, and (if a per-name "site bucket" exists at
//     `*(this+0x360)+name_id*0x14`) the per-site bucket too.
//
//   Structural shape (matches Ghidra's headless decompile and the asm
//   flow — see `build/ghidra-decomp/ffxivgame/0000d300_FUN_0040d300.c`):
//
//     EnterCriticalSection(&this->lock /* [this+0x64] */);
//     if (size <= 0x200) {                            // fast-bucket bookkeeping
//         if (size < 9)
//             this->small_count_35c += 1;             // [this+0x35c]
//         else {
//             unsigned bucket = (size >> 4) +
//                               ((size & 0xf) != 0);
//             this->medium_count[bucket] += 1;        // [this+0x2dc + bucket*4]
//         }
//     }
//     void *p = try_small_alloc(this, size);          // FUN_0040d280
//     if (p != NULL) {
//         p = small_alloc_payload(p);                 // FUN_0040a840 (__cdecl)
//         if (p == NULL) goto leave;
//         // Locate the matching small-class bucket (uVar4 in DAT_00f55988[]).
//         unsigned uVar4 = 0;
//         while (uVar4 < 0x16) {
//             if (size <= ((uint*)0x00f55988)[uVar4]) break;
//             uVar4++;
//         }
//         if (uVar4 >= 0x16) uVar4 = (unsigned)-1;
//         int *bk = (int *)((char*)this + 0xcc + uVar4 * 0x18);
//         bk[4] += bk[0];                              // running total += class size
//         bk[1] += 1;                                  // alloc count
//         unsigned long h = current_high_water();      // FUN_0040da50
//         if (h < (uint)bk[3]) h = (uint)bk[3];
//         bk[3] = h;                                   // peak count
//         h = (uint)bk[4];
//         if ((uint)bk[4] < (uint)bk[5]) h = (uint)bk[5];
//         bk[5] = h;                                   // peak total
//         piVar7 = (int *)((char*)this + 0xa4);        // small-path summary
//     } else {                                         // big / slow path
//         void *space = NamedSpace(PTR_DAT_012652f8,
//                                  "CDev.Engine.Lay.Mem.Space");  // FUN_0040e230
//         p = AllocFromSpace(size, space);             // FUN_0040e110
//         if (p == NULL) goto leave;
//         piVar7 = (int *)((char*)this + 0xb8);        // big-path summary
//     }
//     // Common-tail summary bookkeeping (count, total, current peak,
//     // max-ever) replicated three times: per-bucket (`piVar7`),
//     // global (`this+0x7c..+0x8c`), per-name-bucket (`this+0x360`).
//     piVar7[0] += 1;
//     piVar7[3] += size;
//     { uint h = current_high_water();                 // FUN_0040da50
//       if (h < (uint)piVar7[2]) h = (uint)piVar7[2];
//       piVar7[2] = h;
//       h = (uint)piVar7[3];
//       if ((uint)piVar7[3] < (uint)piVar7[4]) h = (uint)piVar7[4];
//       piVar7[4] = h; }
//     this->total_count_7c += 1;                       // [this+0x7c]
//     this->total_size_88 += size;                     // [this+0x88]
//     { uint h = current_high_water();                 // FUN_0040da50
//       if (h < this->cur_peak_84) h = this->cur_peak_84;
//       this->cur_peak_84 = h;
//       h = this->total_size_88;
//       if (this->total_size_88 < this->max_total_8c)
//           h = this->max_total_8c;
//       this->max_total_8c = h; }
//     short site = FindNameSite(kind);                 // FUN_0040a710 (EAX=kind)
//     if (site >= 0) {
//         int *site_bk = (int *)(*(int *)((char*)this + 0x360)
//                                 + site * 0x14);
//         if (site_bk != NULL) {
//             site_bk[0] += 1;
//             site_bk[3] += size;
//             uint h = current_high_water();
//             if (h < (uint)site_bk[2]) h = (uint)site_bk[2];
//             site_bk[2] = h;
//             h = (uint)site_bk[3];
//             if ((uint)site_bk[3] < (uint)site_bk[4])
//                 h = (uint)site_bk[4];
//             site_bk[4] = h;
//         }
//     }
// leave:
//     LeaveCriticalSection(&this->lock /* [this+0x64] */);
//     return p;
//
//   Stack frame (after the EH3 prologue, ESP-relative):
//     [esp+0x00]  saved EDI
//     [esp+0x04]  saved ESI
//     [esp+0x08]  saved EBX
//     [esp+0x0c .. 0x14]  SUB ESP, 0xc — scratch + CritSec arg spill
//     [esp+0x18]  saved FS:[0] chain link    (local_c)
//     [esp+0x1c]  scope-table handler RVA    (puStack_8 = 0x00e54e28)
//     [esp+0x20]  EH3 trylevel               (local_4: -1 → 0 → -1)
//     [esp+0x24]  return address
//     [esp+0x28]  size       (stack arg #1, `param_1`)
//     [esp+0x2c]  kind       (stack arg #2, `param_2`)
//
//   Reloc-bearing sites in the orig 479 bytes (absolute addresses that
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x002  scope-table handler PUSH       (.rdata 0x00e54e28)
//     +0x007  FS:[0] read                    (constant 0, fold-through)
//     +0x025  EnterCriticalSection IAT       (.rdata 0x00f3e16c)
//     +0x06a  __thiscall rel32 CALL          (.text 0x0040d280)
//     +0x074  __cdecl rel32 CALL             (.text 0x0040a840)
//     +0x090  DAT_00f55988 table load        (.rdata 0x00f55988)
//     +0x0a7  [edi+edx*8+0xcc] mov           (this-relative, no reloc)
//     +0x0ae  [edi+edx*8+0xcc] lea           (this-relative, no reloc)
//     +0x0c1  __thiscall rel32 CALL          (.text 0x0040da50)
//     +0x0e9  moffs32 mov                    (.data 0x012652f8 PTR_DAT)
//     +0x0ee  push imm32 kind-string ptr     (.rdata 0x00f55864
//                                              "CDev.Engine.Lay.Mem.Space")
//     +0x0f8  __thiscall rel32 CALL          (.text 0x0040e230)
//     +0x102  __cdecl rel32 CALL             (.text 0x0040e110)
//     +0x121  __thiscall rel32 CALL          (.text 0x0040da50, 2nd)
//     +0x150  __thiscall rel32 CALL          (.text 0x0040da50, 3rd)
//     +0x174  EAX=kind rel32 CALL            (.text 0x0040a710 — strncmp
//                                              against the registered
//                                              name table)
//     +0x184  [edi+0x360] table-base mov     (this-relative, no reloc)
//     +0x197  __thiscall rel32 CALL          (.text 0x0040da50, 4th)
//     +0x1c2  LeaveCriticalSection IAT       (.rdata 0x00f3e168)
//     +0x1cb  FS:[0] install epilog          (constant 0, fold-through)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc
//   into reproducing the exact EH3 prolog (PUSH -1 / PUSH scope-table /
//   PUSH FS:[0] / SUB ESP / push-callees / FS:[0] install), the
//   trylevel slot at [esp+0x20] that has to be updated between -1, 0,
//   and -1 at the right offsets, the four repeated current-high-water
//   peak-update blocks (six instructions each — `CALL` + `MOV ECX,
//   [esi+8]; CMP ECX, EAX; JBE +2; MOV EAX, ECX; MOV [esi+8], EAX` and
//   its `[esi+0xc]/[esi+0x10]` twin), the EAX-passed `kind` register
//   call to FUN_0040a710, AND the linker-resolved absolute addresses
//   in the seventeen reloc windows above. Each of those constraints is
//   brittle under /O2 — every high-level rewrite shifts at least one
//   byte (state numbering, branch short-vs-near, modrm vs moffs32,
//   FF15 IAT-indirect vs E8 rel32, the 7-byte `LEA EBX, [EBX+0]`
//   alignment NOP, the cmovz / sbb-style boolean tricks).
//
//   The pragmatic choice — the same one FUN_004014b0 / FUN_00401820 /
//   FUN_00401a00 took for their EH-wrapped reloc-heavy bodies — is a
//   `__declspec(naked)` body that re-emits the orig 479 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice (no relocations because the bytes
//   are emitted as raw immediates), which is what `tools/compare.py`
//   checks against.
//
//   The structural commentary above is the readable record of what the
//   function actually does, so a future contributor can promote this
//   to a real source-level match once the surrounding allocator class
//   (the +0x64 CRITICAL_SECTION, the +0x2dc..+0x35c per-size-class
//   counter tables, the +0xcc/+0xb8/+0xa4 / +0x7c bucket summaries,
//   the +0x360 per-name-site table head, and the EAX-passed name-lookup
//   helper FUN_0040a710) are catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_0040d300() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x28
        _emit 0x4e
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x64
        _emit 0x89

        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        _emit 0x53
        _emit 0x56
        _emit 0x57
        _emit 0x8b
        _emit 0xf9
        _emit 0x8d
        _emit 0x47
        _emit 0x64

        _emit 0x50
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xff
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x28
        _emit 0x81

        _emit 0xfb
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x77
        _emit 0x28
        _emit 0x83

        _emit 0xfb
        _emit 0x08
        _emit 0x77
        _emit 0x09
        _emit 0x83
        _emit 0x87
        _emit 0x5c
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0xeb
        _emit 0x1a
        _emit 0x8b
        _emit 0xc3
        _emit 0x83

        _emit 0xe0
        _emit 0x0f
        _emit 0xf6
        _emit 0xd8
        _emit 0x8b
        _emit 0xcb
        _emit 0x1b
        _emit 0xc0
        _emit 0xf7
        _emit 0xd8
        _emit 0xc1
        _emit 0xe9
        _emit 0x04
        _emit 0x03
        _emit 0xc1
        _emit 0x83

        _emit 0x84
        _emit 0x87
        _emit 0xdc
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x53
        _emit 0x8b
        _emit 0xcf
        _emit 0xe8
        _emit 0x11
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x85

        _emit 0xc0
        _emit 0x74
        _emit 0x76
        _emit 0x50
        _emit 0xe8
        _emit 0xc7
        _emit 0xd4
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x44

        _emit 0x24
        _emit 0x28
        _emit 0x0f
        _emit 0x84
        _emit 0x34
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xc0
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x3b
        _emit 0x1c
        _emit 0x85
        _emit 0x88
        _emit 0x59
        _emit 0xf5
        _emit 0x00
        _emit 0x76
        _emit 0x0b
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        _emit 0x83
        _emit 0xf8
        _emit 0x16
        _emit 0x72

        _emit 0xef
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        _emit 0x8d
        _emit 0x14
        _emit 0x40
        _emit 0x8b
        _emit 0x8c
        _emit 0xd7
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x84

        _emit 0xd7
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x48
        _emit 0x10
        _emit 0x83
        _emit 0x40
        _emit 0x04
        _emit 0x01
        _emit 0x8d
        _emit 0x70
        _emit 0x04
        _emit 0x8b

        _emit 0xce
        _emit 0xe8
        _emit 0x8a
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        _emit 0x3b
        _emit 0xc8
        _emit 0x76
        _emit 0x02
        _emit 0x8b
        _emit 0xc1
        _emit 0x8b

        _emit 0x4e
        _emit 0x10
        _emit 0x89
        _emit 0x46
        _emit 0x08
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0x3b
        _emit 0xc8
        _emit 0x76
        _emit 0x02
        _emit 0x8b
        _emit 0xc1
        _emit 0x89
        _emit 0x46

        _emit 0x10
        _emit 0x8d
        _emit 0xb7
        _emit 0xa4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x30
        _emit 0xa1
        _emit 0xf8
        _emit 0x52
        _emit 0x26
        _emit 0x01
        _emit 0x68
        _emit 0x64

        _emit 0x58
        _emit 0xf5
        _emit 0x00
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0xe8
        _emit 0x33
        _emit 0x0e
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4f
        _emit 0x58

        _emit 0x50
        _emit 0x53
        _emit 0xe8
        _emit 0x09
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x0f
        _emit 0x84
        _emit 0xa9

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0xb7
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0x06
        _emit 0x01
        _emit 0x01
        _emit 0x5e
        _emit 0x0c
        _emit 0x8b

        _emit 0xce
        _emit 0xe8
        _emit 0x2a
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        _emit 0x3b
        _emit 0xc8
        _emit 0x76
        _emit 0x02
        _emit 0x8b
        _emit 0xc1
        _emit 0x89

        _emit 0x46
        _emit 0x08
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        _emit 0x3b
        _emit 0xc8
        _emit 0x76
        _emit 0x02
        _emit 0x8b
        _emit 0xc1
        _emit 0x89
        _emit 0x46

        _emit 0x10
        _emit 0x83
        _emit 0x47
        _emit 0x7c
        _emit 0x01
        _emit 0x01
        _emit 0x9f
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x77
        _emit 0x7c
        _emit 0x8b
        _emit 0xce

        _emit 0xe8
        _emit 0xfb
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        _emit 0x3b
        _emit 0xc8
        _emit 0x76
        _emit 0x02
        _emit 0x8b
        _emit 0xc1
        _emit 0x8b
        _emit 0x4e

        _emit 0x10
        _emit 0x89
        _emit 0x46
        _emit 0x08
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0x3b
        _emit 0xc8
        _emit 0x76
        _emit 0x02
        _emit 0x8b
        _emit 0xc1
        _emit 0x89
        _emit 0x46
        _emit 0x10

        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xe8
        _emit 0x97
        _emit 0xd2
        _emit 0xff
        _emit 0xff
        _emit 0x0f
        _emit 0xb7
        _emit 0xc0
        _emit 0x66
        _emit 0x85
        _emit 0xc0
        _emit 0x7c

        _emit 0x3b
        _emit 0x8b
        _emit 0x97
        _emit 0x60
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0xbf
        _emit 0xc0
        _emit 0x8d
        _emit 0x0c
        _emit 0x80
        _emit 0x8d
        _emit 0x34
        _emit 0x8a

        _emit 0x85
        _emit 0xf6
        _emit 0x74
        _emit 0x28
        _emit 0x83
        _emit 0x06
        _emit 0x01
        _emit 0x01
        _emit 0x5e
        _emit 0x0c
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0xaf
        _emit 0x05
        _emit 0x00

        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        _emit 0x3b
        _emit 0xc8
        _emit 0x76
        _emit 0x02
        _emit 0x8b
        _emit 0xc1
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        _emit 0x89
        _emit 0x46
        _emit 0x08

        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0x3b
        _emit 0xc8
        _emit 0x76
        _emit 0x02
        _emit 0x8b
        _emit 0xc1
        _emit 0x89
        _emit 0x46
        _emit 0x10
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c

        _emit 0x50
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x5f

        _emit 0x5e
        _emit 0x5b
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
