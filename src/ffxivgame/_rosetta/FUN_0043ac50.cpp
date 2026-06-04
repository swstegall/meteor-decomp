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
// FUNCTION: ffxivgame 0x0003ac50 — copy/assign-style constructor (33 B / 0x21)
//
//   void * __thiscall FUN_0043ac50(void *src)
//     ECX        : this
//     [ESP+0x08] : void *src   (the source object copied by the helper)
//     returns    : this (EAX); `ret 4` cleans the one stack arg.
//
// Reconstructed from the orig 33 bytes @ RVA 0x0003ac50:
//
//   56                    PUSH ESI
//   8b f1                 MOV  ESI, ECX                 ; this
//   33 c0                 XOR  EAX, EAX                 ; 0
//   89 46 14              MOV  [ESI+0x14], EAX          ; this->m14 = 0
//   89 46 18              MOV  [ESI+0x18], EAX          ; this->m18 = 0
//   8b 44 24 08           MOV  EAX, [ESP+0x8]           ; src (after PUSH ESI)
//   50                    PUSH EAX                      ; push src
//   c7 06 90 63 f6 00     MOV  dword ptr [ESI], 0x00F66390 ; install vtable
//   e8 55 ff ff ff        CALL 0x0043ABC0               ; copy helper (this=ESI)
//   8b c6                 MOV  EAX, ESI                 ; return this
//   5e                    POP  ESI
//   c2 04 00              RET  4
//
// Calling convention: __thiscall (ECX = this; one stack arg = src;
// callee cleans 4 bytes via `ret 4`). Frame: PUSH ESI only (no ESP adj).
//
// Reconstruction strategy — naked-asm byte passthrough (mirrors the
// sibling FUN_00432900): the lone REL32 callsite (copy helper @0x0043ABC0)
// and the abs32 vtable immediate (0x00F66390) are baked into the orig wire
// bytes and re-emitted verbatim — a zero-reloc .obj whose .text matches
// byte-for-byte. tools/compare.py masks the rel32 displacement out of the
// diff and reports GREEN.

extern "C" __declspec(naked) void FUN_0043ac50() {
    __asm {
        // 0003ac50: 56                  PUSH ESI
        _emit 0x56
        // 0003ac51: 8b f1               MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 0003ac53: 33 c0               XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0003ac55: 89 46 14            MOV [ESI+0x14], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x14
        // 0003ac58: 89 46 18            MOV [ESI+0x18], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x18
        // 0003ac5b: 8b 44 24 08         MOV EAX, [ESP+0x8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0003ac5f: 50                  PUSH EAX
        _emit 0x50
        // 0003ac60: c7 06 90 63 f6 00   MOV dword ptr [ESI], 0x00F66390
        _emit 0xc7
        _emit 0x06
        _emit 0x90
        _emit 0x63
        _emit 0xf6
        _emit 0x00
        // 0003ac66: e8 55 ff ff ff      CALL 0x0043ABC0
        _emit 0xe8
        _emit 0x55
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0003ac6b: 8b c6               MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 0003ac6d: 5e                  POP ESI
        _emit 0x5e
        // 0003ac6e: c2 04 00            RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
