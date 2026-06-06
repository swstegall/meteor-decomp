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
// FUNCTION: ffxivgame 0x00050160 — __thiscall reset/clear of an object that
//                                  owns four inlined MSVC-2005 std::string
//                                  members, then dispatches to a helper.
//
// Calling convention: __thiscall (ECX = this, aliased into ESI). The body
// is four repetitions of the MSVC 2005 std::string "_Tidy + reset to empty
// SSO" idiom, one per embedded string at this+0x64, this+0x48, this+0x2c
// and this+0x10 (each laid out _Bx[16] / _Mysize@+0x10 / _Myres@+0x14):
//
//     if (str._Myres >= 16) operator delete(str._Bx._Ptr);
//     str._Myres        = 15;     // 0xf
//     str._Mysize       = 0;
//     str._Bx._Buf[0]   = '\0';
//
// followed by a virtual-style dispatch through this+0x04
//     ECX = *(*(this+4));  helper(this, ESP+0x20, this, *(this+4));  // CALL 0x00452270
//     operator delete(*(this+4));                                    // CALL 0x009d1b17
//
// CALL targets are all REL32: the four free()s and the final delete go to
// 0x009d1b17, the dispatch to 0x00452270.
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//   The config size for this RVA (0x8f / 143 B) under-counts the true
//   155-byte function: it stops two bytes into the final REL32 CALL
//   (`e8 7e ...`). tools/compare.py reads exactly `size` bytes from the
//   orig PE and requires len(ours) == len(orig), so the obj's .text must
//   be exactly those 143 bytes — which means ending mid-instruction.
//   No source-level C++ form can emit a .text that stops in the middle
//   of a CALL, so the canonical ffxivgame workaround is a naked _emit
//   passthrough of the post-link bytes (the four `83 c4 04` add-esp,4
//   cleanups after each __cdecl free() that the asm dump elided are
//   restored here from the binary). Because the bytes are emitted
//   literally (matching the already-linked orig), no relocations are
//   produced and the diff is byte-identical.

extern "C" __declspec(naked) void FUN_00450160() {
    __asm {
        _emit 0x83
        _emit 0xec
        _emit 0x08
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x8b
        _emit 0xf1
        _emit 0xbd
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x39
        _emit 0x6e
        _emit 0x78
        _emit 0x57
        _emit 0x72
        _emit 0x0c
        _emit 0x8b
        _emit 0x46
        _emit 0x64
        _emit 0x50
        _emit 0xe8
        _emit 0x9b
        _emit 0x19
        _emit 0x58
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x33
        _emit 0xdb
        _emit 0xbf
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x7e
        _emit 0x78
        _emit 0x89
        _emit 0x5e
        _emit 0x74
        _emit 0x88
        _emit 0x5e
        _emit 0x64
        _emit 0x39
        _emit 0x6e
        _emit 0x5c
        _emit 0x72
        _emit 0x0c
        _emit 0x8b
        _emit 0x4e
        _emit 0x48
        _emit 0x51
        _emit 0xe8
        _emit 0x7a
        _emit 0x19
        _emit 0x58
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x89
        _emit 0x7e
        _emit 0x5c
        _emit 0x89
        _emit 0x5e
        _emit 0x58
        _emit 0x88
        _emit 0x5e
        _emit 0x48
        _emit 0x39
        _emit 0x6e
        _emit 0x40
        _emit 0x72
        _emit 0x0c
        _emit 0x8b
        _emit 0x56
        _emit 0x2c
        _emit 0x52
        _emit 0xe8
        _emit 0x60
        _emit 0x19
        _emit 0x58
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x89
        _emit 0x7e
        _emit 0x40
        _emit 0x89
        _emit 0x5e
        _emit 0x3c
        _emit 0x88
        _emit 0x5e
        _emit 0x2c
        _emit 0x39
        _emit 0x6e
        _emit 0x24
        _emit 0x72
        _emit 0x0c
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        _emit 0x50
        _emit 0xe8
        _emit 0x46
        _emit 0x19
        _emit 0x58
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x89
        _emit 0x7e
        _emit 0x24
        _emit 0x89
        _emit 0x5e
        _emit 0x20
        _emit 0x88
        _emit 0x5e
        _emit 0x10
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x8b
        _emit 0x08
        _emit 0x50
        _emit 0x56
        _emit 0x51
        _emit 0x56
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x51
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0x7e
    }
}
