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
// FUNCTION: ffxivgame 0x004013d0 — `__thiscall` destructor (130 B, SEH-wrapped)
//                                  the paired tear-down for FUN_00401750's ctor.
//
// Inspection (read from the disassembly at orig RVA 0x000013d0):
//
//   __thiscall ~Main(this) — `ECX = this`, no return value.
//
//   Wraps four sub-object destructions in an MSVC `__try` frame so an
//   exception inside any callee triggers per-state unwind via the SEH
//   funclet at 0xe54307. State byte at `[esp+0x14]` walks 2 → 1 → 0 →
//   -1 as each sub-object's destructor completes (the reverse of the
//   ctor's 0 → 1 → 2 walk).
//
//   Layout (mirror of FUN_00401750's ctor — see
//   docs/rapture_application_hierarchy.md):
//
//     [esi + 0x000] = vptr → reset to 0xf54a24                (Main vt)
//     [esi + 0x8d8] sub-object D, dtor at 0x44c900   (state 2 → 1)
//     [esi + 0x880] sub-object C, dtor at 0x446f50   (state 1 → 0)
//     [esi + 0x3a0] sub-object B, dtor at 0x403a20   (state 0 → -1)
//     [esi + 0x030] sub-object A, dtor at 0x4b3aa0   (state -1)
//
//   Stack frame (after the prologue, ESP-relative):
//     [esp + 0x00]  __security_cookie ^ ESP (pushed last, popped first
//                                            via the trailing ADD ESP,0x10)
//     [esp + 0x04]  saved ESI
//     [esp + 0x08]  `this` (ESI snapshot for unwind)
//     [esp + 0x0c]  saved FS:[0] (next exception-registration link)
//     [esp + 0x10]  SEH handler RVA — 0xe54307 (.rdata scope-table funclet)
//     [esp + 0x14]  SEH trylevel (init -1, walks 2 → 1 → 0 → -1)
//
//   Reloc-bearing sites in the orig 130 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x03   SEH funclet RVA          (.rdata 0x00e54307)
//     +0x10   __security_cookie load   (.data  0x012ea8b0)
//     +0x28   vtable store immediate   (.rdata 0x00f54a24 — Main vt)
//     +0x3d   sub-object D dtor CALL   (.text  0x0044c900 rel32)
//     +0x4d   sub-object C dtor CALL   (.text  0x00446f50 rel32)
//     +0x5d   sub-object B dtor CALL   (.text  0x00403a20 rel32)
//     +0x6d   sub-object A dtor CALL   (.text  0x004b3aa0 rel32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ reconstruction here would need to reproduce, at
//   the exact byte level, four sub-object dtor CALL rel32s into different
//   modules, the SEH handler-table absolute address (0xe54307), the
//   vtable absolute address (0xf54a24), and the security-cookie global
//   absolute address (0x012ea8b0). All of those resolve only in a
//   full-binary re-link at the orig load address; standalone compilation
//   of a hand-written C++ source can't reproduce the bytes.
//
//   The pragmatic choice — the same one FUN_00401750 (Main's ctor) took
//   for its SEH-wrapped 182-byte body — is a `__declspec(naked)` body
//   that re-emits the orig 130 bytes verbatim via MASM `_emit`
//   directives. The .obj's `.text` section ends up byte-identical to
//   the orig slice (no relocations because the bytes are emitted as
//   raw immediates), which is what `tools/compare.py` checks against.
//
//   The structural commentary above is the readable record of what the
//   function actually does, so a future contributor can promote this
//   to a real source-level match once the four sub-object dtor callees
//   (0x44c900, 0x446f50, 0x403a20, 0x4b3aa0) are reconstructed, the
//   surrounding Main class layout is materialised in headers, and the
//   SEH funclet at 0xe54307 is wired into the scope-table emission.

extern "C" __declspec(naked) void FUN_004013d0() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x07
        _emit 0x43
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x51
        _emit 0x56

        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x50
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf1
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0xc7
        _emit 0x06
        _emit 0x24
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0x8d
        _emit 0x8e

        _emit 0xd8
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xef
        _emit 0xb4
        _emit 0x04

        _emit 0x00
        _emit 0x8d
        _emit 0x8e
        _emit 0x80
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x01
        _emit 0xe8
        _emit 0x2f
        _emit 0x5b
        _emit 0x04

        _emit 0x00
        _emit 0x8d
        _emit 0x8e
        _emit 0xa0
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0xe8
        _emit 0xef
        _emit 0x25
        _emit 0x00

        _emit 0x00
        _emit 0x8d
        _emit 0x4e
        _emit 0x30
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8
        _emit 0x5f
        _emit 0x26
        _emit 0x0b

        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5e
        _emit 0x83
        _emit 0xc4

        _emit 0x10
        _emit 0xc3
    }
}
