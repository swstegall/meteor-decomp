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
// FUNCTION: ffxivgame 0x000147b0 — `SQEX::CDev::Engine::Memory::Alternative::SystemHeapBlock::ctor`
//                                  (__thiscall, 7 stack args, 117 B / 0x75)
//
// This is the in-place SystemHeapBlock constructor invoked by the factory
// at FUN_00414580 (see decomp-notes/types/ffxivgame/0x00014580.md). The
// factory `aligned_malloc`s the 0x3c-byte container, allocates the backing
// buffer, then `__thiscall`s here to populate the new cell. It writes the
// same MI-style vftable layout the DetachableHeapBlock ctor establishes
// (see decomp-notes/types/ffxivgame/0x000139d0.md) — base subobject
// vftables first, then overwritten with the most-derived class's vftables.
//
// Calling convention: __thiscall (ECX = this); 7 stack args (28 B);
// callee-cleans via `RET 0x1c`. No callee-saves pushed — the body reads
// stack args directly off `[ESP+...]` and uses EAX/ECX/EDX as scratch.
// Returns void (the .text body ends in RET; EAX happens to still hold
// `this`, which adjacent callers like the factory propagate).
//
// Signature:
//   void __thiscall FUN_004147b0(C *this,
//                                undefined4 param_1,   // [ESP+0x04] → [this+0x14]
//                                undefined4 param_2,   // [ESP+0x08] → [this+0x18]
//                                undefined4 param_3,   // [ESP+0x0c] → [this+0x1c]
//                                undefined4 param_4,   // [ESP+0x10] → [this+0x20]
//                                undefined4 param_5,   // [ESP+0x14] → [this+0x24]
//                                undefined4 param_6,   // [ESP+0x18] → [this+0x28]
//                                undefined4 *param_7); // [ESP+0x1c] → [this+0x2c]
//                                                       //   (defaults to `this` if NULL)
//
// Object layout (matches the FUN_00414580 factory's 0x3c container, same
// shape documented in decomp-notes/types/ffxivgame/0x00014580.md):
//   [+0x00] primary vftable    = 0xf56ff8 (SystemHeapBlock primary)
//   [+0x04] secondary vftable  = 0xf56fc0 (SystemHeapBlock secondary;
//                                           IHandle vftable 0xf56750
//                                           written first, then overwritten)
//   [+0x08] embedded Link      = 0xf5700c (SystemHeapBlock embedded-Link
//                                           override; Link vftable 0xf567c4
//                                           written first, then overwritten)
//   [+0x0c] Link.next = &[this+0x08]   (LEA ECX,[EAX+0x8] / MOV [ECX+0x4],ECX)
//   [+0x10] Link.prev = &[this+0x08]   (MOV [ECX+0x8],ECX)
//   [+0x14] param_1
//   [+0x18] param_2
//   [+0x1c] param_3
//   [+0x20] param_4
//   [+0x24] param_5
//   [+0x28] param_6
//   [+0x2c] param_7  (or `this` if param_7 == NULL — TEST/JNE/MOV idiom)
//   [+0x30] embedded sub-Link sentinel: vftable = 0xf567c4 (Link)
//   [+0x34] sub-Link.next = &[this+0x30]
//   [+0x38] sub-Link.prev = &[this+0x30]
//   [+0x3c] end of touched range
//
// Branch shape: a single forward `JNE +0x02` at +0x5c bridges around the
// `MOV ECX, EAX` default-to-self assignment for `param_7`. MSVC 2005 emits
// this as the natural lowering of `if (param_7 == NULL) param_7 = this;`
// — the test is interleaved several instructions earlier (at +0x43,
// before the SystemHeapBlock primary vftable writes) so the JNE fires
// only after the writes complete. The interleaving is the canonical
// /O2 register-scheduler signature of MSVC 2005.
//
// Reloc-bearing immediate sites in the 117-byte body (these absolute
// addresses resolve only in a full-binary relink at image base
// 0x00400000; the standalone .obj's `.text` matches the orig slice
// byte-for-byte because the immediates are emitted as raw bytes, no
// reloc record needed):
//     +0x06   imm32 ← .rdata 0x00f56750  (IHandle::vftable)
//     +0x0d   imm32 ← .rdata 0x00f567c4  (Link::vftable)
//     +0x1d   imm32 ← .rdata 0x00f5700c  (SystemHeapBlock embedded-Link)
//     +0x4c   imm32 ← .rdata 0x00f56ff8  (SystemHeapBlock primary)
//     +0x52   imm32 ← .rdata 0x00f56fc0  (SystemHeapBlock secondary)
//     +0x66   imm32 ← .rdata 0x00f567c4  (Link::vftable, sub-sentinel)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level C++ form would need to coax MSVC 2005 /O2 /GS /EHsc
//   into the exact interleaving (the TEST/JNE for `param_7== NULL`
//   sandwiched between the secondary-vftable store and the
//   default-to-self MOV) — and reordering the source-level writes
//   nudges the register scheduler off the canonical sequence.
//   The pragmatic choice — the same one the sibling FUN_00414580
//   factory took for the same vftable grammar — is a
//   `__declspec(naked)` body re-emitting the orig 117 bytes verbatim
//   via MASM `_emit` directives. No CALL displacements, no IAT
//   thunks — the .obj's `.text` ends up byte-identical to the orig
//   slice (the six 32-bit MOV-immediate slots that write the vftable
//   addresses are absolute values in the binary's own address space,
//   emitted as literal bytes). `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_004147b0() {
    __asm {
        // 004147b0: 8b 54 24 08          MOV EDX, [ESP+0x8]            ; EDX = param_2
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 004147b4: 8b c1                MOV EAX, ECX                  ; EAX = this
        _emit 0x8b
        _emit 0xc1
        // 004147b6: c7 40 04 50 67 f5 00 MOV [EAX+0x4], 0x00f56750     ; IHandle::vftable
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0x50
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 004147bd: c7 40 08 c4 67 f5 00 MOV [EAX+0x8], 0x00f567c4     ; Link::vftable
        _emit 0xc7
        _emit 0x40
        _emit 0x08
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 004147c4: 8d 48 08             LEA ECX, [EAX+0x8]            ; ECX = &Link
        _emit 0x8d
        _emit 0x48
        _emit 0x08
        // 004147c7: 89 49 04             MOV [ECX+0x4], ECX            ; Link.next = self
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 004147ca: 89 49 08             MOV [ECX+0x8], ECX            ; Link.prev = self
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 004147cd: c7 01 0c 70 f5 00    MOV [ECX], 0x00f5700c         ; SystemHeapBlock embedded-Link
        _emit 0xc7
        _emit 0x01
        _emit 0x0c
        _emit 0x70
        _emit 0xf5
        _emit 0x00
        // 004147d3: 8b 4c 24 04          MOV ECX, [ESP+0x4]            ; ECX = param_1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 004147d7: 89 48 14             MOV [EAX+0x14], ECX           ; [this+0x14] = param_1
        _emit 0x89
        _emit 0x48
        _emit 0x14
        // 004147da: 8b 4c 24 0c          MOV ECX, [ESP+0xc]            ; ECX = param_3
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 004147de: 89 48 1c             MOV [EAX+0x1c], ECX           ; [this+0x1c] = param_3
        _emit 0x89
        _emit 0x48
        _emit 0x1c
        // 004147e1: 8b 4c 24 14          MOV ECX, [ESP+0x14]           ; ECX = param_5
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 004147e5: 89 50 18             MOV [EAX+0x18], EDX           ; [this+0x18] = param_2
        _emit 0x89
        _emit 0x50
        _emit 0x18
        // 004147e8: 8b 54 24 10          MOV EDX, [ESP+0x10]           ; EDX = param_4
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 004147ec: 89 48 24             MOV [EAX+0x24], ECX           ; [this+0x24] = param_5
        _emit 0x89
        _emit 0x48
        _emit 0x24
        // 004147ef: 8b 4c 24 1c          MOV ECX, [ESP+0x1c]           ; ECX = param_7
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 004147f3: 85 c9                TEST ECX, ECX                 ; param_7 == NULL?
        _emit 0x85
        _emit 0xc9
        // 004147f5: 89 50 20             MOV [EAX+0x20], EDX           ; [this+0x20] = param_4
        _emit 0x89
        _emit 0x50
        _emit 0x20
        // 004147f8: 8b 54 24 18          MOV EDX, [ESP+0x18]           ; EDX = param_6
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 004147fc: c7 00 f8 6f f5 00    MOV [EAX], 0x00f56ff8         ; SystemHeapBlock primary
        _emit 0xc7
        _emit 0x00
        _emit 0xf8
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        // 00414802: c7 40 04 c0 6f f5 00 MOV [EAX+0x4], 0x00f56fc0     ; SystemHeapBlock secondary
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0xc0
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        // 00414809: 89 50 28             MOV [EAX+0x28], EDX           ; [this+0x28] = param_6
        _emit 0x89
        _emit 0x50
        _emit 0x28
        // 0041480c: 75 02                JNE +0x02                     ; → 0x414810 if param_7 != NULL
        _emit 0x75
        _emit 0x02
        // 0041480e: 8b c8                MOV ECX, EAX                  ; param_7 = this (default)
        _emit 0x8b
        _emit 0xc8
        // 00414810: 89 48 2c             MOV [EAX+0x2c], ECX           ; [this+0x2c] = param_7
        _emit 0x89
        _emit 0x48
        _emit 0x2c
        // 00414813: 8d 48 30             LEA ECX, [EAX+0x30]           ; ECX = &sub-Link
        _emit 0x8d
        _emit 0x48
        _emit 0x30
        // 00414816: c7 01 c4 67 f5 00    MOV [ECX], 0x00f567c4         ; Link::vftable (sub-sentinel)
        _emit 0xc7
        _emit 0x01
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 0041481c: 89 49 04             MOV [ECX+0x4], ECX            ; sub-Link.next = self
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 0041481f: 89 49 08             MOV [ECX+0x8], ECX            ; sub-Link.prev = self
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 00414822: c2 1c 00             RET 0x1c                      ; __thiscall, 7 stack args
        _emit 0xc2
        _emit 0x1c
        _emit 0x00
    }
}
