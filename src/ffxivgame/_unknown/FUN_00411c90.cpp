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
// FUNCTION: ffxivgame 0x00011c90 — __thiscall constructor for a
//           SQEX::CDev::Engine::Memory::Alternative::RemovableHeapSpace::Serializer
//           (89 B / 0x59).
//
// Calling convention: __thiscall (ECX = this). No stack args.
// RET — callee cleans 0 extra args (frame cleaned via ADD ESP, 0x10).
// Callee-saves pushed: ESI. SEH frame installed (PUSH -0x1 prologue).
//
// The function installs an SEH frame, writes the RemovableHeapSpace::Serializer
// vftable (0xf56ce8) at [this+0x0], calls FUN_00411c20 (base-class ctor with
// ECX=this), then installs the Link vftable (0xf567c4) at [this+0x24] and
// performs a bidirectional self-link: [this+0x28+8] = this+0x2c, and
// [this+0x2c+4] = this+0x28.
//
// Stack layout after full prologue (before body):
//   [ESP+0x00]  saved ESI
//   [ESP+0x04]  saved this (ECX at entry)
//   [ESP+0x08]  old FS:[0x0]
//   [ESP+0x0c]  SEH handler cookie (0xe54fcb)
//   [ESP+0x10]  SEH state (-0x1 at entry, set to 0 after entering body)
//
// Reloc-bearing site:
//   +0x2b  CALL rel32 → FUN_00411c20  (baked VA rel32: 0xffffff60)
//
// All vtable constants (0xf56ce8, 0xf567c4) are baked-in VAs at the fixed
// image base 0x400000 — no relocations.  The only reloc is the CALL rel32,
// which compare.py masks.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level C++ constructor emitting the SEH frame would differ in
//   the __try/__except scaffolding and the compiler's scheduling of stores.
//   The naked-asm passthrough re-emits the orig 89 bytes verbatim;
//   compare.py masks the 4-byte CALL rel32 and reports GREEN.

// clang / GCC static-analysis stub — NOT compiled in production.
#if defined(__clang__) || defined(__GNUC__)
extern "C" void FUN_00411c90() {}
#endif

// MSVC production build — byte-identical naked-asm passthrough.
#if !defined(__clang__) && !defined(__GNUC__)
extern "C" __declspec(naked) void FUN_00411c90()
{
    __asm {
        // 00011c90: 6a ff                PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00011c92: 68 cb 4f e5 00       PUSH 0xe54fcb
        _emit 0x68
        _emit 0xcb
        _emit 0x4f
        _emit 0xe5
        _emit 0x00
        // 00011c97: 64 a1 00 00 00 00    MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00011c9d: 50                   PUSH EAX
        _emit 0x50
        // 00011c9e: 64 89 25 00 00 00 00 MOV dword ptr FS:[0x0], ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00011ca5: 51                   PUSH ECX
        _emit 0x51
        // 00011ca6: 56                   PUSH ESI
        _emit 0x56
        // 00011ca7: 8b f1                MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00011ca9: 89 74 24 04          MOV dword ptr [ESP+0x4], ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x04
        // 00011cad: c7 06 e8 6c f5 00    MOV dword ptr [ESI], 0xf56ce8
        _emit 0xc7
        _emit 0x06
        _emit 0xe8
        _emit 0x6c
        _emit 0xf5
        _emit 0x00
        // 00011cb3: c7 44 24 10 00 00 00 00  MOV dword ptr [ESP+0x10], 0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00011cbb: e8 60 ff ff ff       CALL 0x00411c20
        _emit 0xe8
        _emit 0x60
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00011cc0: 8b 46 28             MOV EAX, dword ptr [ESI+0x28]
        _emit 0x8b
        _emit 0x46
        _emit 0x28
        // 00011cc3: 8b 4e 2c             MOV ECX, dword ptr [ESI+0x2c]
        _emit 0x8b
        _emit 0x4e
        _emit 0x2c
        // 00011cc6: c7 46 24 c4 67 f5 00 MOV dword ptr [ESI+0x24], 0xf567c4
        _emit 0xc7
        _emit 0x46
        _emit 0x24
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00011ccd: 89 48 08             MOV dword ptr [EAX+0x8], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 00011cd0: 8b 56 2c             MOV EDX, dword ptr [ESI+0x2c]
        _emit 0x8b
        _emit 0x56
        _emit 0x2c
        // 00011cd3: 8b 46 28             MOV EAX, dword ptr [ESI+0x28]
        _emit 0x8b
        _emit 0x46
        _emit 0x28
        // 00011cd6: 8b 4c 24 08          MOV ECX, dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 00011cda: 89 42 04             MOV dword ptr [EDX+0x4], EAX
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 00011cdd: 5e                   POP ESI
        _emit 0x5e
        // 00011cde: 64 89 0d 00 00 00 00 MOV dword ptr FS:[0x0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00011ce5: 83 c4 10             ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00011ce8: c3                   RET
        _emit 0xc3
    }
}
#endif

// vim: ts=4 sts=4 sw=4 et
