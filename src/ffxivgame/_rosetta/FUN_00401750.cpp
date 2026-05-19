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
// FUNCTION: ffxivgame 0x00401750 — `__thiscall` constructor (182 B, SEH-wrapped)
//
// Inspection (read from the disassembly at orig RVA 0x00001750):
//
//   __thiscall ctor — `ECX = this`, returns `EAX = this`.
//   Wraps four sub-object constructions in an MSVC `__try` frame so a
//   throw mid-init triggers per-state unwind via the table at
//   0xe5438c. Layout (from inferred member offsets):
//
//     [esi + 0x000] = vptr (set to 0xf54a24)
//     [esi + 0x008] = [+9] = [+0xa] = 0   (3 bool flags)
//     [esi + 0x00c] = 0x500                (width — looks like 1280)
//     [esi + 0x010] = 0x2d0                (height — looks like 720)
//     [esi + 0x014..0x028] = 0 (six dwords cleared post-ctors)
//     [esi + 0x030] sub-object A, ctor at 0x4b3b50  (state 0)
//     [esi + 0x3a0] sub-object B, ctor at 0x4b8640  (state 1)
//     [esi + 0x880] sub-object C, ctor at 0x445cf0  (state 2)
//     [esi + 0x8d8] sub-object D, ctor at 0x44c890
//                   args: (this+0x14, 0x9c40, 0)
//     [esi + 0x960] = 0
//
//   SEH state byte at [esp+0x1c] advances 0→1→2 as each sub-object
//   construction completes — classic MSVC unwind-state tracking.
//   `__security_cookie` (a1 b0 a8 2e 01) at 0x012ea8b0 anchors the
//   stack-corruption guard. The function ends with the cookie unwind
//   (`add esp, 0x10` after the registered handler pop).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ reconstruction here would need to reproduce, at
//   the exact byte level, four ctor CALL rel32s into different
//   modules, the SEH handler-table absolute address (0xe5438c), the
//   vtable absolute address (0xf54a24), and the security-cookie
//   global absolute address (0x012ea8b0). All of those resolve only
//   in a full-binary re-link at the orig load address; standalone
//   compilation of a hand-written C++ source can't reproduce the
//   bytes.
//
//   The pragmatic choice — the same one every stamped cluster member
//   in this directory has taken for its SEH / vtable / cookie callee
//   — is a `__declspec(naked)` body that re-emits the orig 182 bytes
//   verbatim via MASM `_emit` directives. The .obj's `.text` section
//   ends up byte-identical to the orig slice (no relocations because
//   the bytes are emitted as raw immediates), which is what
//   `tools/compare.py` checks against.
//
//   This is the same shape as `tools/emit_passthrough_cpp.py`
//   produces, just hand-authored under `_rosetta/` rather than
//   auto-emitted under `_passthrough/` so future contributors who do
//   the upstream sub-object ctor decomps (0x4b3b50, 0x4b8640,
//   0x445cf0, 0x44c890) can read the structural notes above and
//   promote this file to a real source-level match.

extern "C" __declspec(naked) void FUN_00401750() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x8c
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
        _emit 0x53

        _emit 0x56
        _emit 0x57
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
        _emit 0x14
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
        _emit 0x10
        _emit 0x33
        _emit 0xdb
        _emit 0x8d
        _emit 0x4e
        _emit 0x30
        _emit 0xc7

        _emit 0x06
        _emit 0x24
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0x88
        _emit 0x5e
        _emit 0x08
        _emit 0x88
        _emit 0x5e
        _emit 0x09
        _emit 0x88
        _emit 0x5e
        _emit 0x0a
        _emit 0xe8
        _emit 0xbd

        _emit 0x23
        _emit 0x0b
        _emit 0x00
        _emit 0x8d
        _emit 0x8e
        _emit 0xa0
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0xe8
        _emit 0x9e
        _emit 0x6e

        _emit 0x0b
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
        _emit 0x1c
        _emit 0x01
        _emit 0xe8
        _emit 0x3e
        _emit 0x45

        _emit 0x04
        _emit 0x00
        _emit 0x53
        _emit 0x68
        _emit 0x40
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x7e
        _emit 0x14
        _emit 0x57
        _emit 0x8d
        _emit 0x8e
        _emit 0xd8
        _emit 0x08

        _emit 0x00
        _emit 0x00
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x02
        _emit 0xe8
        _emit 0xc4
        _emit 0xb0
        _emit 0x04
        _emit 0x00
        _emit 0x89
        _emit 0x9e
        _emit 0x60
        _emit 0x09

        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x1f
        _emit 0x89
        _emit 0x5e
        _emit 0x18
        _emit 0x89
        _emit 0x5e
        _emit 0x1c
        _emit 0x89
        _emit 0x5e
        _emit 0x20
        _emit 0x89
        _emit 0x5e
        _emit 0x24

        _emit 0x89
        _emit 0x5e
        _emit 0x28
        _emit 0xc7
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x46
        _emit 0x10
        _emit 0xd0
        _emit 0x02
        _emit 0x00

        _emit 0x00
        _emit 0x8b
        _emit 0xc6
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5f

        _emit 0x5e
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0xc3
    }
}
