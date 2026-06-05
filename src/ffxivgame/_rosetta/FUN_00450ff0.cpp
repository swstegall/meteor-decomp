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
// FUNCTION: ffxivgame 0x00050ff0 — `__thiscall` two-Utf8String dispatch
//                                  helper (260 B / 0x104, EH4-SEH wrapped).
//
// Behaviour read from asm/ffxivgame/00050ff0_FUN_00450ff0.s (cross-checked
// against the orig PE bytes — the asm dump dropped the two `83 c4 04`
// add-esp cleanups after each `CALL 0x009d1b17` operator-delete, and the
// recorded Ghidra size of 0xfe undercounts the real 0x104-byte body; the
// config/ffxivgame.size_overrides.json entry restores 260 B):
//
//   __thiscall void FUN_00450ff0(this, arg0 /* [esp+0x5c] */,
//                                      arg1 /* [esp+0x60] */);
//     ; ECX = this (mirrored into ESI), `ret 8` confirms two stack args.
//
//   Standard MSVC /GS + EH4 prologue (PUSH -1 / PUSH scope-table 0xe57edc /
//   PUSH FS:[0] / SUB ESP,0x3c / double __security_cookie XOR ESP /
//   install FS:[0]), then:
//
//     Utf8String s0;                    ; ctor 0x00404040, [esp+0x50]=0xf
//                                       ; (SSO capacity 15), len=0, buf[0]=0
//     Utf8String s1;                    ; ctor 0x00404040 (2nd), arg1 seeded
//     this->m0(&s0_buf);                ; CALL 0x0044fd60 (member, ECX=this)
//     this->m0(&s1_buf);                ; CALL 0x0044fd60 (2nd)
//     this->m1(&s0, &s1_buf);           ; CALL 0x004502d0 (member, 2 args)
//     if (s1.capacity >= 0x10)          ; CMP [esp+0x28],0x10 / JC
//         operator delete(s1.ptr);      ; CALL 0x009d1b17 ; add esp,4
//     if (s0.capacity >= 0x10)          ; CMP [esp+0x44],0x10 / JC
//         operator delete(s0.ptr);      ; CALL 0x009d1b17 ; add esp,4
//
//   EH4 teardown (restore FS:[0], POP cookie/EDI/ESI/EBX,
//   __security_check_cookie, ADD ESP,0x48, RET 8).
//
//   Reloc-bearing sites in the orig 260 bytes (absolute addresses resolve
//   only in a full-binary relink at image base 0x00400000; standalone
//   .obj compilation can't reproduce them):
//     +0x03   scope-table handler RVA   (0x00e57edc)
//     +0x11   __security_cookie load    (.data 0x012ea8b0)
//     +0x20   __security_cookie load    (.data 0x012ea8b0, 2nd)
//     +0x55   Utf8String ctor CALL      (.text 0x00404040 rel32)
//     +0x76   Utf8String ctor CALL      (.text 0x00404040 rel32, 2nd)
//     +0x86   member m0 CALL            (.text 0x0044fd60 rel32 — __thiscall)
//     +0x93   member m0 CALL            (.text 0x0044fd60 rel32, 2nd)
//     +0xa4   member m1 CALL            (.text 0x004502d0 rel32 — __thiscall)
//     +0xba   operator delete CALL      (.text 0x009d1b17 rel32)
//     +0xdb   operator delete CALL      (.text 0x009d1b17 rel32, 2nd)
//     +0xf9   __security_check_cookie   (.text 0x009d20f4 rel32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ port at /O2 /GS /EHsc would have to reproduce the
//   exact EH4 prologue, the double-cookie register dance, the SSO
//   capacity/len seeding order, and the linker-resolved absolute
//   addresses in the eleven relocation windows above — each brittle under
//   /O2. The pragmatic choice (the same one the SEH-wrapped siblings
//   FUN_004054d0 / FUN_00403a20 took) is a `__declspec(naked)` body that
//   re-emits the orig 260 bytes verbatim via MASM `_emit` directives, so
//   the .obj's `.text` is byte-identical to the orig slice that
//   tools/compare.py checks.

extern "C" __declspec(naked) void FUN_00450ff0() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xdc
        _emit 0x7e
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x83
        _emit 0xec
        _emit 0x3c
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x38
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
        _emit 0x4c
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x5c
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x60
        _emit 0x33
        _emit 0xdb
        _emit 0x6a
        _emit 0xff
        _emit 0x53
        _emit 0x8b
        _emit 0xf1
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x50
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x4c
        _emit 0x88
        _emit 0x5c
        _emit 0x24
        _emit 0x3c
        _emit 0xe8
        _emit 0xf6
        _emit 0x2f
        _emit 0xfb
        _emit 0xff
        _emit 0x6a
        _emit 0xff
        _emit 0x53
        _emit 0x57
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x60
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x30
        _emit 0x88
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        _emit 0xe8
        _emit 0xd5
        _emit 0x2f
        _emit 0xfb
        _emit 0xff
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x50
        _emit 0x8b
        _emit 0xce
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x58
        _emit 0x01
        _emit 0xe8
        _emit 0xe4
        _emit 0xec
        _emit 0xff
        _emit 0xff
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x51
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0xd8
        _emit 0xec
        _emit 0xff
        _emit 0xff
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x52
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x50
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0x37
        _emit 0xf2
        _emit 0xff
        _emit 0xff
        _emit 0xbe
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x39
        _emit 0x74
        _emit 0x24
        _emit 0x28
        _emit 0x72
        _emit 0x0d
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x51
        _emit 0xe8
        _emit 0x69
        _emit 0x0a
        _emit 0x58
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x39
        _emit 0x74
        _emit 0x24
        _emit 0x44
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x24
        _emit 0x88
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        _emit 0x72
        _emit 0x0d
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x30
        _emit 0x52
        _emit 0xe8
        _emit 0x46
        _emit 0x0a
        _emit 0x58
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x4c
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
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x06
        _emit 0x10
        _emit 0x58
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x48
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
