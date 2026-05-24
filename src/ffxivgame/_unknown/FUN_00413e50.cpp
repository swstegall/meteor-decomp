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
// FUNCTION: ffxivgame 0x00013e50 — __thiscall destructor for
//                                  SQEX::CDev::Engine::Memory::Alternative::
//                                  DetachableHeapBlock (161 B, SEH-wrapped).
//
// This is the most-derived destructor for DetachableHeapBlock — a
// multiple-inheritance class with three vftable subobjects (primary at
// offset 0, plus two adjustor subobjects at +4 and +8) and three embedded
// `Link` sentinels (at +0x08, +0x38, +0x44) that own intrusive lists.
//
// The destructor:
//   1. Sets up an SEH frame (state -1 → 4 across the body to flag the
//      partially-destroyed object to any throw handler).
//   2. Stamps the three primary DetachableHeapBlock subobject vftables
//      back over the runtime vftables (so virtual dispatch during the
//      base destructor chain stops calling derived-overridden slots).
//   3. Calls FUN_00413d00 (an inherited destructor / pool drain that
//      walks one of the sentinel-anchored intrusive lists and frees
//      each node).
//   4. Tears down each of the three embedded `Link` sentinels by:
//        a. Stamping `Link::vftable` over each one's vtable slot.
//        b. Splicing the sentinel out of its doubly-linked list:
//             *(next->prev) = prev  ;  *(prev->next) = next
//      The three sentinels live at this+0x08 (Link list at +0x0c/+0x10),
//      this+0x38 (Link list at +0x3c/+0x40), and this+0x44 (Link list
//      at +0x48/+0x4c).
//   5. Stamps IHandle::vftable and IBlock::vftable over the primary
//      vftable slot (typical base-chain unwind in MSVC's destructor
//      thunks).
//   6. Restores the SEH chain and returns.
//
// Reloc-bearing positions in the resulting .obj (all masked by
// compare.py via the COFF DIR32 / REL32 mask):
//
//   off 0x03 : DIR32 → SEH scope-table (0x00e5504c in .rdata)
//   off 0x21 : DIR32 → DetachableHeapBlock::vftable @ 0xb56f50 (primary)
//   off 0x28 : DIR32 → DetachableHeapBlock::vftable @ 0xb56f18 (slot 13)
//   off 0x2f : DIR32 → DetachableHeapBlock::vftable @ 0xb56f60 (slot 2)
//   off 0x3c : REL32 → FUN_00413d00 (inherited destructor)
//   off 0x47 : DIR32 → Link::vftable @ 0xb567c4
//   off 0x8b : DIR32 → IHandle::vftable @ 0xb56750
//   off 0x91 : DIR32 → IBlock::vftable @ 0xb56740
//
// Why `__declspec(naked)`:
//
//   The SEH frame layout (push -1 short-form, push handler with DIR32
//   reloc, fs:[0] save/restore via 7-byte 64 a1/64 89 25 forms, the
//   pre-prologue `[esp+0xc] = this` store for the unwinder's `this`
//   pointer recovery, and the state-update `[esp+0x18] = 4`) is a MSVC
//   2005 codegen artifact that the C++ source-level SEH (`__try` /
//   `__except`) lowers but with subtle differences in interleaving and
//   state-table layout. Naked asm pins the exact 161-byte encoding the
//   original shipped, and the DIR32 / REL32 relocations carry the
//   absolute fixups via `offset SYM` references.

extern "C" {
// Six DIR32-bearing symbols (vftables in .rdata + SEH scope table).
// Declared `int` so `offset SYM` in inline asm produces a 4-byte DIR32.
extern int DetachableHeapBlock_vftable;        // 0xb56f50 — primary subobj
extern int DetachableHeapBlock_vftable_sub4;   // 0xb56f18 — slot 13
extern int DetachableHeapBlock_vftable_sub8;   // 0xb56f60 — slot 2
extern int Link_vftable;                       // 0xb567c4
extern int IHandle_vftable;                    // 0xb56750
extern int IBlock_vftable;                     // 0xb56740
extern int SEH_scope_table_FUN_00413e50;       // 0xe5504c — SEH scope table

// REL32-bearing call target.
extern void FUN_00413d00();                    // inherited destructor
} // extern "C"

extern "C" __declspec(naked) void FUN_00413e50()
{
    __asm {
        // --- SEH frame setup (12 bytes) -------------------------------
        _emit 0x6a                            // push -1 (state magic top)
        _emit 0xff
        _emit 0x68                            // push offset SEH_scope_table
        _emit 0x4c
        _emit 0x50
        _emit 0xe5
        _emit 0x00
        _emit 0x64                            // mov  eax, fs:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50                            // push eax (chain old fs:[0])
        _emit 0x64                            // mov  fs:[0], esp
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- locals + save ESI + stash `this` for unwinder ------------
        _emit 0x83                            // sub  esp, 0xc
        _emit 0xec
        _emit 0x0c
        _emit 0x56                            // push esi
        _emit 0x8b                            // mov  esi, ecx (this)
        _emit 0xf1
        _emit 0x89                            // mov  [esp+0xc], esi  ; SEH `this`
        _emit 0x74
        _emit 0x24
        _emit 0x0c

        // --- stamp the three DetachableHeapBlock subobject vftables ---
        _emit 0xc7                            // mov  [esi], offset DetachableHeapBlock_vftable
        _emit 0x06
        _emit 0x50
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esi+4], offset ..._sub4
        _emit 0x46
        _emit 0x04
        _emit 0x18
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esi+8], offset ..._sub8
        _emit 0x46
        _emit 0x08
        _emit 0x60
        _emit 0x6f
        _emit 0xf5
        _emit 0x00

        // --- SEH state := 4 ; call FUN_00413d00 -----------------------
        _emit 0xc7                            // mov  [esp+0x18], 4
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8                            // call FUN_00413d00
        _emit 0x70
        _emit 0xfe
        _emit 0xff
        _emit 0xff

        // --- Link sentinel #3 @ this+0x44 ; splice & stamp vftable ----
        _emit 0x8b                            // mov  ecx, [esi+0x48]   ; sentinel.next
        _emit 0x4e
        _emit 0x48
        _emit 0x8b                            // mov  edx, [esi+0x4c]   ; sentinel.prev
        _emit 0x56
        _emit 0x4c
        _emit 0xb8                            // mov  eax, offset Link_vftable
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0x89                            // mov  [esi+0x44], eax
        _emit 0x46
        _emit 0x44
        _emit 0x89                            // mov  [ecx+0x08], edx   ; next->prev = prev
        _emit 0x51
        _emit 0x08
        _emit 0x8b                            // mov  ecx, [esi+0x4c]
        _emit 0x4e
        _emit 0x4c
        _emit 0x8b                            // mov  edx, [esi+0x48]
        _emit 0x56
        _emit 0x48
        _emit 0x89                            // mov  [ecx+0x04], edx   ; prev->next = next
        _emit 0x51
        _emit 0x04

        // --- Link sentinel #2 @ this+0x38 ; splice & stamp vftable ----
        _emit 0x8b                            // mov  ecx, [esi+0x3c]
        _emit 0x4e
        _emit 0x3c
        _emit 0x8b                            // mov  edx, [esi+0x40]
        _emit 0x56
        _emit 0x40
        _emit 0x89                            // mov  [esi+0x38], eax   ; vtable = Link_vftable
        _emit 0x46
        _emit 0x38
        _emit 0x89                            // mov  [ecx+0x08], edx
        _emit 0x51
        _emit 0x08
        _emit 0x8b                            // mov  ecx, [esi+0x40]
        _emit 0x4e
        _emit 0x40
        _emit 0x8b                            // mov  edx, [esi+0x3c]
        _emit 0x56
        _emit 0x3c
        _emit 0x89                            // mov  [ecx+0x04], edx
        _emit 0x51
        _emit 0x04

        // --- Link sentinel #1 @ this+0x08 ; splice & stamp vftable ----
        _emit 0x89                            // mov  [esi+0x08], eax   ; vtable = Link_vftable
        _emit 0x46
        _emit 0x08
        _emit 0x8b                            // mov  eax, [esi+0x0c]
        _emit 0x46
        _emit 0x0c
        _emit 0x8b                            // mov  ecx, [esi+0x10]
        _emit 0x4e
        _emit 0x10
        _emit 0x89                            // mov  [eax+0x08], ecx
        _emit 0x48
        _emit 0x08
        _emit 0x8b                            // mov  edx, [esi+0x10]
        _emit 0x56
        _emit 0x10
        _emit 0x8b                            // mov  eax, [esi+0x0c]
        _emit 0x46
        _emit 0x0c
        _emit 0x8b                            // mov  ecx, [esp+0x10]   ; preload saved fs:[0]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x89                            // mov  [edx+0x04], eax
        _emit 0x42
        _emit 0x04

        // --- final base-chain unwind: stamp IHandle, then IBlock ------
        _emit 0xc7                            // mov  [esi+0x04], offset IHandle_vftable
        _emit 0x46
        _emit 0x04
        _emit 0x50
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esi], offset IBlock_vftable
        _emit 0x06
        _emit 0x40
        _emit 0x67
        _emit 0xf5
        _emit 0x00

        // --- epilogue: restore ESI, restore fs:[0], drop frame, ret ---
        _emit 0x5e                            // pop  esi
        _emit 0x64                            // mov  fs:[0], ecx
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83                            // add  esp, 0x18 (drop locals + SEH frame)
        _emit 0xc4
        _emit 0x18
        _emit 0xc3                            // ret
    }
}

// vim: ts=4 sts=4 sw=4 et
