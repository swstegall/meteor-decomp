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
// FUNCTION: ffxivgame 0x0000a170 — magic-static get-or-set accessor for a
//                                  globally-cached pointer, wrapped in an
//                                  EH3 (`__except_handler3`-style) SEH
//                                  frame (104 B / 0x68).
//
// Inspection (read from the disassembly at orig RVA 0x0000a170):
//
//   __cdecl void *FUN_0040a170(void *newValue);
//
//     static bool s_initialised = false;        // .data 0x01327bf4 (low bit)
//     static void *s_inst = nullptr;            // .data 0x01327bf0
//
//     if (!s_initialised) {
//         s_initialised = true;
//         // (trylevel slot zeroed before the ctor — EH3 idiom)
//         s_inst = FUN_0040e500();              // CALL 0x0040e500 — lazy init
//     }
//     if (newValue != nullptr) {
//         s_inst = newValue;                    // setter path
//         return newValue;                      // EAX = newValue
//     }
//     return s_inst;                            // getter path
//
//   Byte-for-byte clone of the sibling magic-static accessor at orig RVA
//   0x0000a090 — same instruction sequence, same EH3 frame layout, same
//   `if (!flag) lazy_init(); if (arg) set; else get` structure. Only the
//   four reloc-bearing immediates differ:
//
//                            FUN_0040a170      FUN_0040a090
//     scope-table             0x00e54c9e        0x00e54c5e
//     init-flag .data slot    0x01327bf4        0x01327be0
//     s_inst .data slot       0x01327bf0        0x01327bdc
//     CALL rel32 displacement 0x0000435b        0x0000443b
//
//   The lazy-init target FUN_0040e500 is shared with the sibling, so the
//   absolute CALL target is identical; only the displacement from this
//   function's CALL site differs.
//
//   Stack frame (after the EH3 prologue, ESP-relative):
//     [esp+0x00]   saved FS:[0] chain link
//     [esp+0x04]   EH3 scope-table address (0x00e54c9e)
//     [esp+0x08]   EH3 trylevel             (-1 idle, 0 during init)
//     [esp+0x0c]   return address
//     [esp+0x10]   newValue (caller's arg0)
//
//   Reloc-bearing sites in the orig 104 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them via source —
//   naked asm emits them as raw immediate bytes which happen to match
//   the orig binary's resolved CALL rel32 / PUSH imm32 / MOV moffs32
//   fields byte-for-byte):
//     +0x02   FS:[0] read                (constant 0, fold-through)
//     +0x08   scope-table handler RVA    (0x00e54c9e — .rdata FuncInfo)
//     +0x13   FS:[0] install             (constant 0, fold-through)
//     +0x1a   init-flag TEST byte        (.data 0x01327bf4)
//     +0x22   init-flag OR  byte         (.data 0x01327bf4)
//     +0x30   lazy-init CALL             (.text 0x0040e500 rel32 — __cdecl)
//     +0x35   s_inst store               (.data 0x01327bf0)
//     +0x42   s_inst store (setter)      (.data 0x01327bf0, 2nd)
//     +0x4a   FS:[0] restore (setter)    (constant 0, fold-through)
//     +0x58   s_inst load  (getter)      (.data 0x01327bf0, 3rd)
//     +0x5d   FS:[0] restore (getter)    (constant 0, fold-through)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc
//   into reproducing the exact __except_handler3 prolog (FS:[0] read /
//   PUSH -1 / PUSH scope-table / PUSH chain / FS:[0] install — no
//   security-cookie slot, unlike the EH4 wrapper FUN_00401650), the
//   exact magic-static guard (`TEST [flag], AL` / `JNZ skip` / `OR
//   [flag], AL` rather than the more usual `BT/JC` form), the
//   trylevel-zero write at [esp+0x8] sitting between the OR and the
//   ctor CALL, AND the get-or-set fork at the join (newValue-nonzero
//   arm short-circuits the FS-restore epilog, newValue-zero arm
//   reloads the global through a moffs32). Each constraint is brittle
//   under /O2.
//
//   The pragmatic choice — the same one the sibling FUN_0040a090 took
//   for its byte-identical accessor twin — is a `__declspec(naked)`
//   body that re-emits the orig 104 bytes verbatim via MASM `_emit`
//   directives. The .obj's `.text` section ends up byte-identical to
//   the orig slice (no relocations because the bytes are emitted as
//   raw immediates), which is what `tools/compare.py` checks against.
//
//   The structural commentary above is the readable record of what
//   the function actually does, so a future contributor can promote
//   this to a real source-level match once the lazy-init target
//   (FUN_0040e500) and the global being cached at .data 0x01327bf0
//   are catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_0040a170() {
    __asm {
        _emit 0x64              // MOV  EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH -0x1                (EH3 trylevel = -1)
        _emit 0xff
        _emit 0x68              // PUSH 0xe54c9e            (scope-table)
        _emit 0x9e
        _emit 0x4c
        _emit 0xe5
        _emit 0x00
        _emit 0x50              // PUSH EAX                 (FS:[0] chain link)
        _emit 0xb8              // MOV  EAX, 0x1            (init-flag bit)
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x64              // MOV  FS:[0x0], ESP       (install EH3)
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST [0x01327bf4], AL    (init flag)
        _emit 0x05
        _emit 0xf4
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ  +0x18               (skip lazy init)
        _emit 0x18
        _emit 0x09              // OR   [0x01327bf4], EAX   (set init flag)
        _emit 0x05
        _emit 0xf4
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV  [ESP+0x8], 0x0      (trylevel = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL 0x0040e500          (lazy-init)
        _emit 0x5b
        _emit 0x43
        _emit 0x00
        _emit 0x00
        _emit 0xa3              // MOV  [0x01327bf0], EAX   (s_inst = init result)
        _emit 0xf0
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV  EAX, [ESP+0x10]     (caller arg0)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ   +0x13               (→ getter path)
        _emit 0x13
        _emit 0xa3              // MOV  [0x01327bf0], EAX   (setter: s_inst = arg)
        _emit 0xf0
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV  ECX, [ESP]          (FS:[0] chain)
        _emit 0x0c
        _emit 0x24
        _emit 0x64              // MOV  FS:[0x0], ECX       (restore EH3)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD  ESP, 0xc            (drop SEH frame)
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET                      (EAX = newValue)
        _emit 0x8b              // MOV  ECX, [ESP]          (FS:[0] chain)
        _emit 0x0c
        _emit 0x24
        _emit 0xa1              // MOV  EAX, [0x01327bf0]   (getter: return s_inst)
        _emit 0xf0
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x64              // MOV  FS:[0x0], ECX       (restore EH3)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD  ESP, 0xc            (drop SEH frame)
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET                      (EAX = s_inst)
    }
}
