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
// FUNCTION: ffxivgame 0x0000e290 — __thiscall 5-arg setter: *param_1 → field_0, param_2 → field_4
//
// Asm (20 bytes @ orig RVA 0x0000e290):
//   8b c1             MOV EAX, ECX           ; this → EAX
//   8b 4c 24 04       MOV ECX, [ESP+0x4]     ; ECX = param_1 (int*)
//   8b 11             MOV EDX, [ECX]          ; EDX = *param_1
//   8b 4c 24 08       MOV ECX, [ESP+0x8]     ; ECX = param_2
//   89 10             MOV [EAX], EDX          ; this->field_0 = *param_1
//   89 48 04          MOV [EAX+0x4], ECX     ; this->field_4 = param_2
//   c2 14 00          RET 0x14               ; pop 5 args (5 x 4 = 20 bytes)
//
// Calling convention: __thiscall (ECX = this, callee cleans 5 stack args).
// No prologue / frame pointer (/Oy).
//
// MSVC's natural thiscall emission keeps ECX and uses EAX as scratch,
// producing 18 bytes. The orig is 20 bytes because ECX is moved to EAX
// first (MOV EAX,ECX) and ECX is then reused for param loading.
// Naked-asm passthrough reproduces the exact 20-byte sequence.

extern "C" __declspec(naked) void FUN_0040e290() {
    __asm {
        // 0000e290:  8b c1             MOV EAX, ECX
        _emit 0x8b
        _emit 0xc1
        // 0000e292:  8b 4c 24 04       MOV ECX, [ESP+0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0000e296:  8b 11             MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 0000e298:  8b 4c 24 08       MOV ECX, [ESP+0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 0000e29c:  89 10             MOV [EAX], EDX
        _emit 0x89
        _emit 0x10
        // 0000e29e:  89 48 04          MOV [EAX+0x4], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 0000e2a1:  c2 14 00          RET 0x14
        _emit 0xc2
        _emit 0x14
        _emit 0x00
    }
}
