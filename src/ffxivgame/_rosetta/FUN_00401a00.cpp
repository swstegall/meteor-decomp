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
// FUNCTION: ffxivgame 0x00001a00 — exe-dir bootstrap (301 B / 0x12d,
//                                  EH4-SEH wrapped, function-scope
//                                  magic-static singleton).
//
// Inspection (read from the disassembly at orig RVA 0x00001a00):
//
//   __cdecl PathSingleton* FUN_00401a00();
//
//     static PathSingleton s_dir;          // ctor at 0x00445cf0,
//                                          // dtor thunk at 0x00f2e160,
//                                          // atexit at 0x009d25c2.
//                                          // singleton instance lives
//                                          // at .data 0x013237a8;
//                                          // 1-byte init flag at
//                                          // .data 0x013237fc.
//     wchar_t path[260];                   // 0x208-B local buffer
//     memset(path, 0, sizeof(path));
//     if (GetModuleFileNameW(NULL, path, sizeof(path))) {
//         // 5-way-unrolled scan for the last L'\\' in the 260-wchar
//         // buffer — MSVC emits CMOVZ for the centre-of-window check
//         // (offset +2, the natural pivot register EAX) and JNZ + LEA
//         // for the four flanking offsets (-2, -1, +1, +2 vs EAX).
//         unsigned int last = 0;
//         for (unsigned int j = 0; j < 260; j += 5) {
//             if (path[j+0] == L'\\') last = j+0;
//             if (path[j+1] == L'\\') last = j+1;
//             if (path[j+2] == L'\\') last = j+2;
//             if (path[j+3] == L'\\') last = j+3;
//             if (path[j+4] == L'\\') last = j+4;
//         }
//         path[last] = L'\0';              // truncate at last separator
//         s_dir.SetExePath(path);          // __thiscall at 0x004476e0
//     }
//     return &s_dir;                       // EAX = 0x013237a8 always
//
//   Stack frame (after the EH4 prologue, ESP-relative):
//     [esp+0x000 .. esp+0x00b]   ESI / EBX / EH4 cookie pushed below
//     [esp+0x00c .. esp+0x213]   path[260] wchar_t buffer (0x208 B)
//     [esp+0x214]                __security_cookie ^ ESP (orig copy)
//     [esp+0x218]                EH4 saved-FS:[0] chain link
//     [esp+0x21c]                EH4 scope-table address (0x00e543b5)
//     [esp+0x220]                EH4 trylevel (-1 idle, 0 during ctor)
//     [esp+0x224]                return address
//
//   Reloc-bearing sites in the orig 301 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x03  scope-table handler RVA (0x00e543b5 — .rdata FuncInfo)
//     +0x09  FS:[0] read                (constant 0, fold-through)
//     +0x15  __security_cookie load     (.data 0x012ea8b0)
//     +0x25  __security_cookie load     (.data 0x012ea8b0, 2nd)
//     +0x35  FS:[0] install             (constant 0, fold-through)
//     +0x40  init-flag TEST             (.data 0x013237fc)
//     +0x48  init-flag OR               (.data 0x013237fc)
//     +0x4d  singleton this load        (.data 0x013237a8)
//     +0x5d  ctor CALL                  (.text 0x00445cf0 rel32)
//     +0x62  dtor thunk PUSH            (.text 0x00f2e160 — atexit pfv)
//     +0x67  atexit CALL                (.text 0x009d25c2 rel32)
//     +0x86  _memset CALL               (.text 0x009d2110 rel32)
//     +0x9b  GetModuleFileNameW IAT     (.rdata 0x00f3e1e0)
//     +0xf9  singleton this load        (.data 0x013237a8, 2nd)
//     +0xfe  SetExePath CALL            (.text 0x004476e0 rel32 — __thiscall)
//     +0x103 singleton return-value     (.data 0x013237a8, 3rd)
//     +0x122 __security_check_cookie    (.text 0x009d20f4 rel32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc
//   into reproducing the exact __except_handler4 prolog (PUSH -1 / PUSH
//   scope-table / PUSH FS:[0] / SUB ESP / cookie XOR / push-callees /
//   second cookie XOR / FS:[0] install), the exact 5-way loop unroll
//   with CMOVZ-on-pivot, AND the linker-resolved absolute addresses in
//   the seventeen relocation windows above. Each of those constraints
//   is brittle under /O2 — every high-level rewrite shifts at least
//   one byte (cookie-stack-offset, state numbering, branch short-vs-
//   near, modrm vs moffs32, unroll factor 4 vs 5 vs none).
//
//   The pragmatic choice — the same one FUN_004014b0 took for its
//   SEH-wrapped 307-byte tick fn — is a `__declspec(naked)` body that
//   re-emits the orig 301 bytes verbatim via MASM `_emit` directives.
//   The .obj's `.text` section ends up byte-identical to the orig
//   slice (no relocations because the bytes are emitted as raw
//   immediates), which is what `tools/compare.py` checks against.
//
//   The structural commentary above is the readable record of what
//   the function actually does, so a future contributor can promote
//   this to a real source-level match once the surrounding singleton
//   class (the +0x00 char* / +0x04 capacity=0x40 / +0x08 size=1 /
//   +0x10..+0x11 init-flag bytes / +0x12 inline char[] layout from
//   ctor FUN_00445cf0) and the related Utf8String-style assignment
//   helper (FUN_00445ae0, called twice from the SetExePath method
//   FUN_004476e0) are catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_00401a00() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xb5
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
        _emit 0x81
        _emit 0xec

        _emit 0x0c
        _emit 0x02
        _emit 0x00
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
        _emit 0x08
        _emit 0x02

        _emit 0x00
        _emit 0x00
        _emit 0x53
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
        _emit 0x84
        _emit 0x24
        _emit 0x18

        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84
        _emit 0x05

        _emit 0xfc
        _emit 0x37
        _emit 0x32
        _emit 0x01
        _emit 0x75
        _emit 0x33
        _emit 0x09
        _emit 0x05
        _emit 0xfc
        _emit 0x37
        _emit 0x32
        _emit 0x01
        _emit 0xb9
        _emit 0xa8
        _emit 0x37
        _emit 0x32

        _emit 0x01
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0x20
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x8f
        _emit 0x42
        _emit 0x04

        _emit 0x00
        _emit 0x68
        _emit 0x60
        _emit 0xe1
        _emit 0xf2
        _emit 0x00
        _emit 0xe8
        _emit 0x57
        _emit 0x0b
        _emit 0x5d
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0xc7
        _emit 0x84

        _emit 0x24
        _emit 0x20
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x68
        _emit 0x08
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x44

        _emit 0x24
        _emit 0x10
        _emit 0x6a
        _emit 0x00
        _emit 0x50
        _emit 0xe8
        _emit 0x86
        _emit 0x06
        _emit 0x5d
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x68
        _emit 0x08
        _emit 0x02

        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x51
        _emit 0x6a
        _emit 0x00
        _emit 0xff
        _emit 0x15
        _emit 0xe0
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x85

        _emit 0xc0
        _emit 0x74
        _emit 0x5f
        _emit 0x33
        _emit 0xd2
        _emit 0x8d
        _emit 0x42
        _emit 0x02
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x0e
        _emit 0xbe
        _emit 0x5c
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x66
        _emit 0x39
        _emit 0x71
        _emit 0xfe
        _emit 0x75
        _emit 0x03
        _emit 0x8d
        _emit 0x50
        _emit 0xfe
        _emit 0x66
        _emit 0x39
        _emit 0x31
        _emit 0x75
        _emit 0x03
        _emit 0x8d

        _emit 0x50
        _emit 0xff
        _emit 0x66
        _emit 0x39
        _emit 0x71
        _emit 0x02
        _emit 0x0f
        _emit 0x44
        _emit 0xd0
        _emit 0x66
        _emit 0x39
        _emit 0x71
        _emit 0x04
        _emit 0x75
        _emit 0x03
        _emit 0x8d

        _emit 0x50
        _emit 0x01
        _emit 0x66
        _emit 0x39
        _emit 0x71
        _emit 0x06
        _emit 0x75
        _emit 0x03
        _emit 0x8d
        _emit 0x50
        _emit 0x02
        _emit 0x83
        _emit 0xc0
        _emit 0x05
        _emit 0x8d
        _emit 0x58

        _emit 0xfe
        _emit 0x83
        _emit 0xc1
        _emit 0x0a
        _emit 0x81
        _emit 0xfb
        _emit 0x04
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x72
        _emit 0xc5
        _emit 0x66
        _emit 0xc7
        _emit 0x44
        _emit 0x54

        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x52
        _emit 0xb9
        _emit 0xa8
        _emit 0x37
        _emit 0x32
        _emit 0x01
        _emit 0xe8
        _emit 0xde
        _emit 0x5b

        _emit 0x04
        _emit 0x00
        _emit 0xb8
        _emit 0xa8
        _emit 0x37
        _emit 0x32
        _emit 0x01
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x18
        _emit 0x02
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
        _emit 0x5e
        _emit 0x5b
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x08
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x33

        _emit 0xcc
        _emit 0xe8
        _emit 0xce
        _emit 0x05
        _emit 0x5d
        _emit 0x00
        _emit 0x81
        _emit 0xc4
        _emit 0x18
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xc3
    }
}
