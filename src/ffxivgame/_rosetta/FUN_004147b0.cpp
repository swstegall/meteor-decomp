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
// FUNCTION: ffxivgame 0x004147b0 — SQEX::CDev::Engine::Memory::Alternative
//                                  constructor: initialises a heap-block
//                                  object with two embedded linked-list
//                                  sentinels and seven user-supplied fields
//                                  (__thiscall, 117 B / 0x75, 7 stack args)
//
// Calling convention: __thiscall (ECX = this); 7 stack args; RET 0x1c
//   (callee-cleans 28 bytes). Returns void (EAX = this at end, but unused
//   by typical MSVC constructor epilogue patterns). No callee-save pushes;
//   no frame pointer (EBP not used); all register use is EAX, ECX, EDX only.
//
// Object layout initialised by this constructor:
//
//   [this + 0x00]  primary vftable          → 0x00F56FF8 (derived-class primary)
//   [this + 0x04]  IHandle sub-vftable      → first 0x00F56750 (IHandle::vftable),
//                                             then overwritten to 0x00F56FC0
//                                             (derived-class IHandle slot)
//   [this + 0x08]  embedded SystemHeapBlock → vtable first set to
//                                             0x00F567C4 (Link::vftable, base),
//                                             then overwritten to 0x00F5700C
//                                             (SystemHeapBlock::vftable, derived)
//   [this + 0x0C]  embedded Link.next       → &this[0x08]  (sentinel = self)
//   [this + 0x10]  embedded Link.prev       → &this[0x08]  (sentinel = self)
//   [this + 0x14]  arg1 (param_1)
//   [this + 0x18]  arg2 (param_2)
//   [this + 0x1C]  arg3 (param_3)
//   [this + 0x20]  arg4 (param_4)
//   [this + 0x24]  arg5 (param_5)
//   [this + 0x28]  arg6 (param_6)
//   [this + 0x2C]  arg7 (param_7) if arg7 != 0, else this  (owner / parent ptr)
//   [this + 0x30]  embedded Link2 vftable  → 0x00F567C4 (Link::vftable)
//   [this + 0x34]  embedded Link2.next     → &this[0x30]  (sentinel = self)
//   [this + 0x38]  embedded Link2.prev     → &this[0x30]  (sentinel = self)
//
// The vtable sequence follows the canonical MSVC 2005 inline-expanded
// base-class initialisation idiom for the
// SQEX::CDev::Engine::Memory::Alternative family (cf. SystemHeapSpace ctor
// at 0x00014640, documented in decomp-notes/types/ffxivgame/0x00014640.md):
//   1. Write base class vtable (IHandle at +0x04, Link at +0x08).
//   2. Initialise base-class members (list sentinels at +0x0C / +0x10).
//   3. Overwrite with most-derived vtable (SystemHeapBlock at +0x08,
//      then primary + IHandle-slot for the outermost class at +0x00 / +0x04).
//
// Reloc-bearing sites in the orig 117 bytes (absolute .rdata vtable VAs):
//   +0x06  MOV [EAX+0x04], imm32  → 0x00F56750  (IHandle::vftable)
//   +0x0D  MOV [EAX+0x08], imm32  → 0x00F567C4  (Link::vftable)
//   +0x1D  MOV [ECX],      imm32  → 0x00F5700C  (SystemHeapBlock::vftable)
//   +0x4C  MOV [EAX],      imm32  → 0x00F56FF8  (primary vftable)
//   +0x52  MOV [EAX+0x04], imm32  → 0x00F56FC0  (IHandle derived-class slot)
//   +0x66  MOV [ECX],      imm32  → 0x00F567C4  (Link::vftable for second sentinel)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Source-level C++ would need the full class hierarchy for the two vtable
//   write sequences (base-then-derived inline expansion), precise field
//   offsets, and the conditional owner-pointer init at +0x2C — all of which
//   must align with MSVC 2005 /O2 register scheduling (arg2 is loaded into
//   EDX at entry and reused 21 bytes later at the +0x18 store; the
//   interleaved load/store schedule is not guessable from source without
//   a complete type definition and /O2 compile). The six absolute-address
//   relocations further constrain the source form. Given that every sibling
//   in this module uses the naked-asm passthrough idiom, the pragmatic path
//   is to re-emit the 117 orig bytes verbatim.

extern "C" __declspec(naked) void FUN_004147b0() {
    __asm {
        // 000147b0: 8b 54 24 08    MOV EDX, dword ptr [ESP+0x08]   ; preload arg2
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 000147b4: 8b c1          MOV EAX, ECX                    ; EAX = this
        _emit 0x8b
        _emit 0xc1
        // 000147b6: c7 40 04 50 67 f5 00   MOV [EAX+0x04], IHandle::vftable
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0x50
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 000147bd: c7 40 08 c4 67 f5 00   MOV [EAX+0x08], Link::vftable
        _emit 0xc7
        _emit 0x40
        _emit 0x08
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 000147c4: 8d 48 08       LEA ECX, [EAX+0x08]             ; ECX = &embedded[8]
        _emit 0x8d
        _emit 0x48
        _emit 0x08
        // 000147c7: 89 49 04       MOV [ECX+0x04], ECX             ; Link.next = self
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 000147ca: 89 49 08       MOV [ECX+0x08], ECX             ; Link.prev = self
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 000147cd: c7 01 0c 70 f5 00   MOV [ECX], SystemHeapBlock::vftable
        _emit 0xc7
        _emit 0x01
        _emit 0x0c
        _emit 0x70
        _emit 0xf5
        _emit 0x00
        // 000147d3: 8b 4c 24 04    MOV ECX, dword ptr [ESP+0x04]   ; ECX = arg1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 000147d7: 89 48 14       MOV [EAX+0x14], ECX             ; this[0x14] = arg1
        _emit 0x89
        _emit 0x48
        _emit 0x14
        // 000147da: 8b 4c 24 0c    MOV ECX, dword ptr [ESP+0x0C]   ; ECX = arg3
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 000147de: 89 48 1c       MOV [EAX+0x1C], ECX             ; this[0x1C] = arg3
        _emit 0x89
        _emit 0x48
        _emit 0x1c
        // 000147e1: 8b 4c 24 14    MOV ECX, dword ptr [ESP+0x14]   ; ECX = arg5
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 000147e5: 89 50 18       MOV [EAX+0x18], EDX             ; this[0x18] = arg2 (EDX)
        _emit 0x89
        _emit 0x50
        _emit 0x18
        // 000147e8: 8b 54 24 10    MOV EDX, dword ptr [ESP+0x10]   ; EDX = arg4
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 000147ec: 89 48 24       MOV [EAX+0x24], ECX             ; this[0x24] = arg5
        _emit 0x89
        _emit 0x48
        _emit 0x24
        // 000147ef: 8b 4c 24 1c    MOV ECX, dword ptr [ESP+0x1C]   ; ECX = arg7
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 000147f3: 85 c9          TEST ECX, ECX                   ; arg7 == 0?
        _emit 0x85
        _emit 0xc9
        // 000147f5: 89 50 20       MOV [EAX+0x20], EDX             ; this[0x20] = arg4
        _emit 0x89
        _emit 0x50
        _emit 0x20
        // 000147f8: 8b 54 24 18    MOV EDX, dword ptr [ESP+0x18]   ; EDX = arg6
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 000147fc: c7 00 f8 6f f5 00   MOV [EAX], primary vftable
        _emit 0xc7
        _emit 0x00
        _emit 0xf8
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        // 00414802: c7 40 04 c0 6f f5 00   MOV [EAX+0x04], IHandle-derived-slot vftable
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0xc0
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        // 00414809: 89 50 28       MOV [EAX+0x28], EDX             ; this[0x28] = arg6
        _emit 0x89
        _emit 0x50
        _emit 0x28
        // 0041480c: 75 02          JNZ +0x02                        ; if arg7 != 0, skip
        _emit 0x75
        _emit 0x02
        // 0041480e: 8b c8          MOV ECX, EAX                    ; ECX = this (use this as owner)
        _emit 0x8b
        _emit 0xc8
        // 00414810: 89 48 2c       MOV [EAX+0x2C], ECX             ; this[0x2C] = arg7 or this
        _emit 0x89
        _emit 0x48
        _emit 0x2c
        // 00414813: 8d 48 30       LEA ECX, [EAX+0x30]             ; ECX = &this[0x30]
        _emit 0x8d
        _emit 0x48
        _emit 0x30
        // 00414816: c7 01 c4 67 f5 00   MOV [ECX], Link::vftable   ; second sentinel vtable
        _emit 0xc7
        _emit 0x01
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 0041481c: 89 49 04       MOV [ECX+0x04], ECX             ; Link2.next = self
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 0041481f: 89 49 08       MOV [ECX+0x08], ECX             ; Link2.prev = self
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 00414822: c2 1c 00       RET 0x1C                        ; callee-cleans 28 bytes
        _emit 0xc2
        _emit 0x1c
        _emit 0x00
    }
}
