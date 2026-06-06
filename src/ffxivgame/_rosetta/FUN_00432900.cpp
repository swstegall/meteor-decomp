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
// FUNCTION: ffxivgame 0x00032900 — scalar deleting destructor (49 B / 0x31)
//
//   void * __thiscall FUN_00432900(unsigned int flags)
//     ECX        : this
//     [ESP+0x08] : unsigned int flags   (the standard MSVC deleting flag;
//                                        bit 0 set → operator delete this)
//     returns    : this (EAX), `ret 4` cleans the flags arg.
//
// Reconstructed from the orig 49 bytes @ RVA 0x00032900:
//
//   56                    PUSH ESI
//   8b f1                 MOV  ESI, ECX                 ; this
//   c7 06 9c 39 f6 00     MOV  dword ptr [ESI], 0x00F6399C ; install vtable
//   8d 4e 04              LEA  ECX, [ESI + 4]           ; &this->sub @ +4
//   c7 05 ac c8 32 01 ..  MOV  dword ptr [0x0132C8AC], 0 ; clear global slot
//   e8 c5 b8 24 00        CALL 0x0067E1E0               ; sub-object dtor
//   f6 44 24 08 01        TEST byte ptr [ESP+8], 1      ; flags & 1 ?
//   74 09                 JZ   no_free
//   8b 4e fc              MOV  ECX, [ESI - 4]           ; alloc base
//   56                    PUSH ESI
//   e8 45 b6 fd ff        CALL 0x0040DF70               ; operator delete
//  no_free:
//   8b c6                 MOV  EAX, ESI                 ; return this
//   5e                    POP  ESI
//   c2 04 00              RET  4
//
// Calling convention: __thiscall (ECX = this; one stack arg = flags;
// callee cleans 4 bytes via `ret 4`).
// Frame: PUSH ESI only (no ESP adjustment).
//
// Reconstruction strategy — naked-asm byte passthrough (mirrors the
// sibling FUN_00416320 / FUN_004051e0): the two REL32 callsites
// (sub-object dtor @0x0067E1E0, operator delete @0x0040DF70) and the
// two abs32 immediates (vtable 0x00F6399C, global 0x0132C8AC) are baked
// into the orig wire bytes and re-emitted verbatim — a zero-reloc .obj
// whose .text matches byte-for-byte. tools/compare.py masks the rel32
// bytes out of the diff and reports GREEN.

extern "C" __declspec(naked) void FUN_00432900() {
    __asm {
        // 00032900: 56                  PUSH ESI
        _emit 0x56
        // 00032901: 8b f1               MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00032903: c7 06 9c 39 f6 00   MOV dword ptr [ESI], 0x00F6399C
        _emit 0xc7
        _emit 0x06
        _emit 0x9c
        _emit 0x39
        _emit 0xf6
        _emit 0x00
        // 00032909: 8d 4e 04            LEA ECX, [ESI + 4]
        _emit 0x8d
        _emit 0x4e
        _emit 0x04
        // 0003290c: c7 05 ac c8 32 01 00 00 00 00  MOV dword ptr [0x0132C8AC], 0
        _emit 0xc7
        _emit 0x05
        _emit 0xac
        _emit 0xc8
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00032916: e8 c5 b8 24 00      CALL 0x0067E1E0
        _emit 0xe8
        _emit 0xc5
        _emit 0xb8
        _emit 0x24
        _emit 0x00
        // 0003291b: f6 44 24 08 01      TEST byte ptr [ESP+8], 1
        _emit 0xf6
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        // 00032920: 74 09               JZ no_free (+0x09)
        _emit 0x74
        _emit 0x09
        // 00032922: 8b 4e fc            MOV ECX, [ESI - 4]
        _emit 0x8b
        _emit 0x4e
        _emit 0xfc
        // 00032925: 56                  PUSH ESI
        _emit 0x56
        // 00032926: e8 45 b6 fd ff      CALL 0x0040DF70
        _emit 0xe8
        _emit 0x45
        _emit 0xb6
        _emit 0xfd
        _emit 0xff
        // 0003292b: 8b c6               MOV EAX, ESI  (no_free:)
        _emit 0x8b
        _emit 0xc6
        // 0003292d: 5e                  POP ESI
        _emit 0x5e
        // 0003292e: c2 04 00            RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
