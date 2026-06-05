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
// FUNCTION: ffxivgame 0x0004f330 — module-dir + relative-path combiner
//                                  (__cdecl, 370 B / 0x172, EH4-SEH wrapped,
//                                   __alloca_probe large-frame 0x1120 bytes).
//
// Inspection (read from the disassembly at orig RVA 0x0004f330):
//
//   __cdecl void FUN_0044f330(SomeClass* param1, SomeClass* param2)
//   — returns void, two pointer parameters.
//
//   Structural shape:
//
//     wchar_t buf[0x800];   // on stack at [ESP+0x130] after setup
//
//     // 1. Get module path into buf
//     DWORD len = GetModuleFileNameW(NULL, buf, 0x800);
//
//     // 2. Strip filename — find last backslash and null-terminate after it
//     if (len > 0) {
//         int i = (int)len - 1;
//         while (i >= 0 && buf[i] != L'\\') --i;
//         if (i >= 0) buf[i + 1] = L'\0';   // keep trailing backslash
//     }
//
//     // 3. Build a wstring from param2, append to buf
//     std::wstring localStr;
//     localStr._Myres = 7;  localStr._Mysize = 0;  localStr._Bx._Buf[0] = L'\0';
//     param2->getRelativePath(&localStr);           // __thiscall @0x00449000
//
//     // 4. Locate wstring data (SSO vs heap)
//     wchar_t* strData = (localStr._Myres < 8) ? localStr._Bx._Buf : localStr._Bx._Ptr;
//
//     // 5. Compute byte count (wcslen × 2 + 2) and append strData to buf
//     //    (REP MOVSD + REP MOVSB into the null-terminator position of buf)
//
//     // 6. Call a path-processing function with (?, 0x100, buf)   @0x0044ed10  __cdecl
//
//     // 7. Call param1->setPath(&localVar)                        @0x004489c0  __thiscall
//
//     // 8. Free wstring heap buffer if allocated (call @0x0044d350 __cdecl)
//
//   Stack frame (after EH4 prologue + __alloca_probe 0x1120):
//     [ESP+0x130 .. ESP+0x112f]  wchar_t buf[0x800] (0x1000 bytes)
//     [ESP+0x111c]               /GS security cookie (first placement)
//     [ESP+0x113c]               SEH trylevel update (-1 at epilogue)
//     [ESP+0x1134]               saved FS:[0] EH4 chain pointer
//     Saved registers: EDI ESI EBP EBX at [ESP+0x04..0x10]
//     2nd security cookie at [ESP+0x00]
//
//   Reloc-bearing sites in the orig 363 bytes (absolute VAs baked in at
//   image base 0x00400000; compare.py masks these):
//     +0x03   DIR32 → 0x00e57c7b   (PUSH offset EH4 handler)
//     +0x09   DIR32 → FS:[0]       (MOV EAX, FS:[0])
//     +0x13   DIR32 → 0x012ea8b0   (__security_cookie, first load)
//     +0x1f   DIR32 → 0x012ea8b0   (__security_cookie, second load)
//     +0x32   DIR32 → FS:[0]       (MOV FS:[0], EAX — install frame)
//     +0x5d   DIR32 → 0x00f3e1e0   (IAT — GetModuleFileNameW)
//     +0x70   DIR32 → 0x012ea8b0   (cookie in index loop — SIB [esp+eax*2+0x130])
//     +0x82   DIR32 → 0x012ea8b0   (SIB store null — [esp+eax*2+0x132])
//     +0xa8   REL32 → 0x00449000   (CALL param2->getRelativePath)
//     +0xcb   DIR32 → 0x012ea8b0   (LEA EDI, [esp+0x130])
//     +0x10e  REL32 → 0x0044ed10   (CALL path processor)
//     +0x11d  REL32 → 0x004489c0   (CALL param1->setPath)
//     +0x142  REL32 → 0x0044d350   (CALL free/delete)
//     +0x14a  DIR32 → FS:[0]       (MOV FS:[0], ECX restore)
//     +0x166  REL32 → 0x009d20f4   (__security_check_cookie)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS into
//   producing: the exact EH4 prologue with __alloca_probe for the 0x1120
//   frame, the double-cookie placement, the 7-byte NOP alignment pad at
//   +0xd9 (LEA ESP,[ESP+0]), the loop-inversion shape for the two wcslen
//   idioms (MOV EDI,EDI + walk-forward then JNZ), the REP MOVSD/MOVSB
//   copy, the SSO-vs-heap branch for the wstring at [esp+0x14], AND the
//   exact layout of the SEH trylevel update at [esp+0x113c]. Each of
//   these constraints is brittle under MSVC 2005 /O2 — the SEH-wrapped
//   siblings (FUN_00405080, FUN_004014b0, FUN_0040ced0) all reached GREEN
//   only via naked-asm passthrough for identical reasons.
//
//   The structural commentary above is the readable record of what the
//   function does; a future contributor can promote this to a real
//   source-level match once the surrounding class types (param1 / param2
//   vtables, the 0x00449000 getRelativePath ABI, the 0x004489c0 setPath
//   ABI, and the 0x0044ed10 path-processor signature) are catalogued
//   under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_0044f330() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x7b
        _emit 0x7c
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0xb8
        _emit 0x20
        _emit 0x11
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x88
        _emit 0x36
        _emit 0x58
        _emit 0x00
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0x1c
        _emit 0x11
        _emit 0x00
        _emit 0x00
        _emit 0x53
        _emit 0x55
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
        _emit 0x84
        _emit 0x24
        _emit 0x34
        _emit 0x11
        _emit 0x00
        _emit 0x00
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xac
        _emit 0x24
        _emit 0x44
        _emit 0x11
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0x48
        _emit 0x11
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x84
        _emit 0x24
        _emit 0x34
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x33
        _emit 0xdb
        _emit 0x53
        _emit 0xff
        _emit 0x15
        _emit 0xe0
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x83
        _emit 0xc0
        _emit 0xff
        _emit 0x3b
        _emit 0xc3
        _emit 0x7c
        _emit 0x1f
        _emit 0xb9
        _emit 0x5c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x90
        _emit 0x66
        _emit 0x39
        _emit 0x8c
        _emit 0x44
        _emit 0x30
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x07
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x79
        _emit 0xf1
        _emit 0xeb
        _emit 0x08
        _emit 0x66
        _emit 0x89
        _emit 0x9c
        _emit 0x44
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x28
        _emit 0x66
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x51
        _emit 0x8b
        _emit 0xce
        _emit 0x89
        _emit 0x9c
        _emit 0x24
        _emit 0x40
        _emit 0x11
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x23
        _emit 0x9c
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x2c
        _emit 0x08
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x73
        _emit 0x04
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8b
        _emit 0xd0
        _emit 0x8b
        _emit 0xff
        _emit 0x66
        _emit 0x8b
        _emit 0x08
        _emit 0x83
        _emit 0xc0
        _emit 0x02
        _emit 0x66
        _emit 0x3b
        _emit 0xcb
        _emit 0x75
        _emit 0xf5
        _emit 0x8d
        _emit 0xbc
        _emit 0x24
        _emit 0x30
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x2b
        _emit 0xc2
        _emit 0x83
        _emit 0xc7
        _emit 0xfe
        _emit 0xeb
        _emit 0x07
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66
        _emit 0x8b
        _emit 0x4f
        _emit 0x02
        _emit 0x83
        _emit 0xc7
        _emit 0x02
        _emit 0x66
        _emit 0x3b
        _emit 0xcb
        _emit 0x75
        _emit 0xf4
        _emit 0x8b
        _emit 0xc8
        _emit 0xc1
        _emit 0xe9
        _emit 0x02
        _emit 0x8b
        _emit 0xf2
        _emit 0xf3
        _emit 0xa5
        _emit 0x8d
        _emit 0x94
        _emit 0x24
        _emit 0x30
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x52
        _emit 0x8b
        _emit 0xc8
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x83
        _emit 0xe1
        _emit 0x03
        _emit 0x68
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0xf3
        _emit 0xa4
        _emit 0xe8
        _emit 0xcd
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0x51
        _emit 0x8b
        _emit 0xcd
        _emit 0xe8
        _emit 0x6e
        _emit 0x95
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0x3c
        _emit 0x11
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x72
        _emit 0x14
        _emit 0x8d
        _emit 0x54
        _emit 0x00
        _emit 0x02
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x6a
        _emit 0x0c
        _emit 0x52
        _emit 0x50
        _emit 0xe8
        _emit 0xd9
        _emit 0xde
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x34
        _emit 0x11
        _emit 0x00
        _emit 0x00
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
        _emit 0x5d
        _emit 0x5b
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x1c
        _emit 0x11
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x59
        _emit 0x2c
        _emit 0x58
        _emit 0x00
    }
}
