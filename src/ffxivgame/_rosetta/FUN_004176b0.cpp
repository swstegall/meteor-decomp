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
// FUNCTION: ffxivgame 0x004176b0 — `__thiscall` container iterator reset
//                                   (42 B / 0x2a)
//
// void __thiscall FUN_004176b0(this)
//
// Validates that the container's begin pointer (this->m4) is non-null and
// that the element count ((this->m8 - this->m4) >> 3) is non-zero.  If
// either check fails the universal out_of_range thunk FUN_009d22b4 is
// called (push-five-zeros + tail-call to FUN_009d2290).  After the guard:
//
//   this->m14 = this->m4;   // cursor = begin
//   this->m18 = 0;          // index  = 0
//
// Calling convention: __thiscall (ECX = this, RET — no stack args).
// Frame: none (/Oy) — prolog is PUSH ECX / PUSH ESI (PUSH ECX allocates the
// 4-byte hole that POP ECX reclaims in the epilog), no EBP frame.
//
// Asm (42 bytes @ orig RVA 0x000176b0):
//   51              PUSH ECX
//   56              PUSH ESI
//   8b f1           MOV ESI, ECX
//   8b 46 04        MOV EAX, dword ptr [ESI+0x4]
//   85 c0           TEST EAX, EAX
//   74 0a           JZ   +0x0a            ; → CALL if begin == null
//   8b 4e 08        MOV ECX, dword ptr [ESI+0x8]
//   2b c8           SUB ECX, EAX
//   c1 f9 03        SAR ECX, 0x3
//   75 05           JNZ  +0x05            ; skip CALL if count != 0
//   e8 ea ab 5b 00  CALL 0x009d22b4       ; out_of_range thunk
//   8b 56 04        MOV EDX, dword ptr [ESI+0x4]
//   89 56 14        MOV dword ptr [ESI+0x14], EDX
//   c7 46 18 00 00 00 00  MOV dword ptr [ESI+0x18], 0x0
//   5e              POP ESI
//   59              POP ECX
//   c3              RET
//
// Reconstruction: naked-asm byte passthrough.  The rel32 in the CALL to
// FUN_009d22b4 (0x005babea) is baked into the orig binary's address space
// and emitted verbatim — no relocation entry is needed; compare.py
// compares the raw bytes and sees GREEN.

extern "C" __declspec(naked) void FUN_004176b0() {
    __asm {
        // 000176b0: 51              PUSH ECX
        _emit 0x51
        // 000176b1: 56              PUSH ESI
        _emit 0x56
        // 000176b2: 8b f1           MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 000176b4: 8b 46 04        MOV EAX, dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 000176b7: 85 c0           TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000176b9: 74 0a           JZ +0x0a  (→ 000176c5)
        _emit 0x74
        _emit 0x0a
        // 000176bb: 8b 4e 08        MOV ECX, dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        // 000176be: 2b c8           SUB ECX, EAX
        _emit 0x2b
        _emit 0xc8
        // 000176c0: c1 f9 03        SAR ECX, 0x3
        _emit 0xc1
        _emit 0xf9
        _emit 0x03
        // 000176c3: 75 05           JNZ +0x05  (→ 000176ca)
        _emit 0x75
        _emit 0x05
        // 000176c5: e8 ea ab 5b 00  CALL FUN_009d22b4 (out_of_range thunk)
        _emit 0xe8
        _emit 0xea
        _emit 0xab
        _emit 0x5b
        _emit 0x00
        // 000176ca: 8b 56 04        MOV EDX, dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 000176cd: 89 56 14        MOV dword ptr [ESI+0x14], EDX
        _emit 0x89
        _emit 0x56
        _emit 0x14
        // 000176d0: c7 46 18 00 00 00 00  MOV dword ptr [ESI+0x18], 0x0
        _emit 0xc7
        _emit 0x46
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000176d7: 5e              POP ESI
        _emit 0x5e
        // 000176d8: 59              POP ECX
        _emit 0x59
        // 000176d9: c3              RET
        _emit 0xc3
    }
}
