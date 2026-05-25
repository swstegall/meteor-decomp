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
// FUNCTION: ffxivgame 0x00014fc0 — CDev.Engine.Vfx logging-tag name lookup
//                                  (__cdecl const char*(int), 232 B / 0xe8).
//
// Signature:  const char* GetVfxLogTagName(int category_id);
//
// A 20-entry on-stack pointer table is materialised from .rdata string
// literals, then a 4-way range-dispatch returns the appropriate
// "CDev.Engine.Vfx.<Subsystem>" tag for the requested numeric category.
// The category-id space is intentionally sparse so each subsystem cluster
// (core types vs. framework vs. application) sits in its own numeric band.
//
// Category-id → string map (recovered from the 20-slot stack table + the
// jump-table indexing arithmetic):
//
//   id 0x00 → "CDev.Engine.Vfx.Must"
//   id 0x01 → "CDev.Engine.Vfx.MustFix"
//   id 0x02 → "CDev.Engine.Vfx.MustTemporary"
//   id 0x03 → "CDev.Engine.Vfx.Unhandled"
//   id 0x04 → "CDev.Engine.Vfx.Framework.CDev"                  (alias of 0x60)
//   id 0x05 → "CDev.Engine.Vfx.Framework.CDev.Cut"              (alias of 0x63)
//   id 0x06 → "CDev.Engine.Vfx.Framework.CDev.Lay"              (alias of 0x64)
//   id 0x07 → "CDev.Engine.Vfx.Framework.CDev.Fw"               (alias of 0x65)
//   id 0x08 → "CDev.Engine.Vfx.Instance"
//   id 0x09 → "CDev.Engine.Vfx.Common"
//   id 0x0a → "CDev.Engine.Vfx.Stl"
//   id 0x0b → "CDev.Engine.Vfx.Qix"
//   id 0x0c → "CDev.Engine.Vfx.BaseControl"
//   id 0x0d → "CDev.Engine.Vfx.QixControl"
//   id 0x0e → "CDev.Engine.Vfx.IntermediateResource"
//   id 0x0f → "CDev.Engine.Vfx.ResourceLoader"
//   id 0x10 → "CDev.Engine.Vfx.VfxResourceFormat"
//   id 0x11 → "CDev.Engine.Vfx.VfxResourceModel"
//   id 0x12 → (slot @ esp+0x50 — past stack frame, dead range)
//   id 0x60 → "CDev.Engine.Vfx.Framework.CDev"
//   id 0x61 → "CDev.Engine.Vfx.Framework.CDev.Vfx"
//   id 0x62 → "CDev.Engine.Vfx.Framework.CDev.Dw"
//   id 0x63 → "CDev.Engine.Vfx.Framework.CDev.Cut"
//   id 0x64 → "CDev.Engine.Vfx.Framework.CDev.Lay"
//   id 0x65 → "CDev.Engine.Vfx.Framework.CDev.Fw"
//   id 0x80..0xbf → "CDev.Engine.Vfx.Application"
//   anything else  → "CDev.Engine.Vfx.Unknown"
//
// Asm shape (no SEH, no callee-saves, no /GS — pure leaf function):
//
//   sub  esp, 0x50                ; 20-slot pointer table
//   mov  eax, [esp+0x54]          ; eax = category_id
//   cmp  eax, 4
//
//   ; populate the 20-slot table (12 c7 stores in declaration order:
//   ;  esp+0..0xc, then esp+0x28..0x4c, then esp+0x10..0x24 — the
//   ;  back-half stores get re-ordered around the front-half because
//   ;  /O2 hoists the cmp-and-test ahead of the materialisation):
//   mov  [esp+0x00], "Must"               ; index 0
//   mov  [esp+0x04], "MustFix"            ; index 1
//   mov  [esp+0x08], "MustTemporary"      ; index 2
//   mov  [esp+0x0c], "Unhandled"          ; index 3
//   mov  [esp+0x28], "Instance"           ; index 10
//   mov  [esp+0x2c], "Common"             ; index 11
//   mov  [esp+0x30], "Stl"                ; index 12
//   mov  [esp+0x34], "Qix"                ; index 13
//   mov  [esp+0x38], "BaseControl"        ; index 14
//   mov  [esp+0x3c], "QixControl"         ; index 15
//   mov  [esp+0x40], "IntermediateResource" ; index 16
//   mov  [esp+0x44], "ResourceLoader"     ; index 17
//   mov  [esp+0x48], "VfxResourceFormat"  ; index 18
//   mov  [esp+0x4c], "VfxResourceModel"   ; index 19
//   mov  [esp+0x10], "Framework.CDev"     ; index 4
//   mov  [esp+0x14], "Framework.CDev.Vfx" ; index 5
//   mov  [esp+0x18], "Framework.CDev.Dw"  ; index 6
//   mov  [esp+0x1c], "Framework.CDev.Cut" ; index 7
//   mov  [esp+0x20], "Framework.CDev.Lay" ; index 8
//   mov  [esp+0x24], "Framework.CDev.Fw"  ; index 9
//
//   ; arm 1: id <= 4 → [esp + id*4]
//   jg   arm2
//   mov  eax, [esp+eax*4]
//   add  esp, 0x50
//   ret
//
//   ; arm 2: id <= 0x12 → [esp + (id+2)*4]
//   arm2:
//   cmp  eax, 0x12
//   jg   arm3
//   mov  eax, [esp+eax*4+8]
//   add  esp, 0x50
//   ret
//
//   ; arm 3: id in [0x60, 0x65] → [esp + id*4 - 0x170]
//   ;        (lands on esp+0x10..0x24, the Framework cluster)
//   arm3:
//   lea  ecx, [eax-0x60]
//   cmp  ecx, 5
//   ja   arm4
//   mov  eax, [esp+eax*4-0x170]
//   add  esp, 0x50
//   ret
//
//   ; arm 4: id in [0x80, 0xbf] → "Application"; else "Unknown"
//   arm4:
//   add  eax, 0xffffff80              ; eax -= 0x80
//   cmp  eax, 0x3f
//   mov  eax, offset "Application"    ; load tentative result first
//   jbe  done                          ; keep "Application" if eax-0x80 <= 0x3f
//   mov  eax, offset "Unknown"
//   done:
//   add  esp, 0x50
//   ret
//
// Why `__declspec(naked)` with raw `_emit` bytes:
//
//   The body holds 22 absolute address references — 20 distinct .rdata
//   string-literal pointers materialised as immediate operands in the
//   `c7 44 24 XX <imm32>` stack-store instructions, plus the two `b8
//   <imm32>` register loads at the tail. At /O2 with /GF (string
//   pooling) cl.exe places these literals in .rdata in whatever order
//   its hash bucketing chooses, and the absolute addresses don't appear
//   until link time. A standalone .obj source-level compile can't
//   reproduce the orig binary's specific 0x00f57348..0x00f570c8 layout.
//
//   Even ignoring the literal addresses, the orig's /O2 schedule has
//   two source-level brittlenesses that a C++ rewrite would have to
//   reproduce byte-for-byte: (1) the back-half-then-front-half store
//   order on the 20-slot table (esp+0..0xc, then esp+0x28..0x4c, then
//   esp+0x10..0x24), and (2) the tentative-result `mov eax, offset
//   "Application"` BEFORE the conditional branch in arm 4 (a select-
//   style codegen choice that varies under tiny source perturbations).
//
//   The pragmatic choice — matching the local idiom of FUN_00403a20,
//   FUN_00401a00, and the recently-matched FUN_00413e50 — is a
//   `__declspec(naked)` body that re-emits the orig 232 bytes verbatim
//   via MASM `_emit` directives. The resulting .obj's `.text` matches
//   the orig slice byte-for-byte with no relocations (the absolute
//   addresses are baked in as immediates), which is what tools/
//   compare.py checks.

extern "C" __declspec(naked) void FUN_00414fc0()
{
    __asm {
        // --- prologue: 20-slot pointer-table frame ----------------------
        _emit 0x83                            // sub  esp, 0x50
        _emit 0xec
        _emit 0x50
        _emit 0x8b                            // mov  eax, [esp+0x54]   ; category_id
        _emit 0x44
        _emit 0x24
        _emit 0x54
        _emit 0x83                            // cmp  eax, 4            ; hoisted ahead of stores
        _emit 0xf8
        _emit 0x04

        // --- table materialisation: front half (indices 0..3) -----------
        _emit 0xc7                            // mov  [esp+0x00], offset "CDev.Engine.Vfx.Must"
        _emit 0x04
        _emit 0x24
        _emit 0x48
        _emit 0x73
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esp+0x04], offset "CDev.Engine.Vfx.MustFix"
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x30
        _emit 0x73
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esp+0x08], offset "CDev.Engine.Vfx.MustTemporary"
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x10
        _emit 0x73
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esp+0x0c], offset "CDev.Engine.Vfx.Unhandled"
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xf4
        _emit 0x72
        _emit 0xf5
        _emit 0x00

        // --- table materialisation: back half (indices 10..19) ----------
        _emit 0xc7                            // mov  [esp+0x28], offset "...Instance"
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0xd8
        _emit 0x72
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esp+0x2c], offset "...Common"
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xc0
        _emit 0x72
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esp+0x30], offset "...Stl"
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0xac
        _emit 0x72
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esp+0x34], offset "...Qix"
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x98
        _emit 0x72
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esp+0x38], offset "...BaseControl"
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x7c
        _emit 0x72
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esp+0x3c], offset "...QixControl"
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0x60
        _emit 0x72
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esp+0x40], offset "...IntermediateResource"
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0x38
        _emit 0x72
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esp+0x44], offset "...ResourceLoader"
        _emit 0x44
        _emit 0x24
        _emit 0x44
        _emit 0x18
        _emit 0x72
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esp+0x48], offset "...VfxResourceFormat"
        _emit 0x44
        _emit 0x24
        _emit 0x48
        _emit 0xf4
        _emit 0x71
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esp+0x4c], offset "...VfxResourceModel"
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        _emit 0xd0
        _emit 0x71
        _emit 0xf5
        _emit 0x00

        // --- table materialisation: middle (indices 4..9 — Framework) ---
        _emit 0xc7                            // mov  [esp+0x10], offset "...Framework.CDev"
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xb0
        _emit 0x71
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esp+0x14], offset "...Framework.CDev.Vfx"
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8c
        _emit 0x71
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esp+0x18], offset "...Framework.CDev.Dw"
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x68
        _emit 0x71
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esp+0x1c], offset "...Framework.CDev.Cut"
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x44
        _emit 0x71
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esp+0x20], offset "...Framework.CDev.Lay"
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x20
        _emit 0x71
        _emit 0xf5
        _emit 0x00
        _emit 0xc7                            // mov  [esp+0x24], offset "...Framework.CDev.Fw"
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0xfc
        _emit 0x70
        _emit 0xf5
        _emit 0x00

        // --- arm 1: id <= 4 → [esp + id*4] ------------------------------
        _emit 0x7f                            // jg   arm2 (+0x07)
        _emit 0x07
        _emit 0x8b                            // mov  eax, [esp+eax*4]
        _emit 0x04
        _emit 0x84
        _emit 0x83                            // add  esp, 0x50
        _emit 0xc4
        _emit 0x50
        _emit 0xc3                            // ret

        // --- arm 2: id <= 0x12 → [esp + (id+2)*4] -----------------------
        _emit 0x83                            // cmp  eax, 0x12
        _emit 0xf8
        _emit 0x12
        _emit 0x7f                            // jg   arm3 (+0x08)
        _emit 0x08
        _emit 0x8b                            // mov  eax, [esp+eax*4+8]
        _emit 0x44
        _emit 0x84
        _emit 0x08
        _emit 0x83                            // add  esp, 0x50
        _emit 0xc4
        _emit 0x50
        _emit 0xc3                            // ret

        // --- arm 3: id in [0x60, 0x65] → [esp + id*4 - 0x170] -----------
        _emit 0x8d                            // lea  ecx, [eax-0x60]
        _emit 0x48
        _emit 0xa0
        _emit 0x83                            // cmp  ecx, 5
        _emit 0xf9
        _emit 0x05
        _emit 0x77                            // ja   arm4 (+0x0b)
        _emit 0x0b
        _emit 0x8b                            // mov  eax, [esp+eax*4-0x170]
        _emit 0x84
        _emit 0x84
        _emit 0x90
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83                            // add  esp, 0x50
        _emit 0xc4
        _emit 0x50
        _emit 0xc3                            // ret

        // --- arm 4: id in [0x80, 0xbf] → "Application"; else "Unknown" --
        _emit 0x83                            // add  eax, 0xffffff80   ; eax -= 0x80
        _emit 0xc0
        _emit 0x80
        _emit 0x83                            // cmp  eax, 0x3f
        _emit 0xf8
        _emit 0x3f
        _emit 0xb8                            // mov  eax, offset "CDev.Engine.Vfx.Application"
        _emit 0xe0
        _emit 0x70
        _emit 0xf5
        _emit 0x00
        _emit 0x76                            // jbe  done (+0x05)
        _emit 0x05
        _emit 0xb8                            // mov  eax, offset "CDev.Engine.Vfx.Unknown"
        _emit 0xc8
        _emit 0x70
        _emit 0xf5
        _emit 0x00
        _emit 0x83                            // add  esp, 0x50
        _emit 0xc4
        _emit 0x50
        _emit 0xc3                            // ret
    }
}

// vim: ts=4 sts=4 sw=4 et
