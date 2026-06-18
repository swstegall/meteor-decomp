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
// FUNCTION: ffxivgame 0x004178a0 — `__thiscall` field-zero initialiser
//                                   (20 B / 0x14).
//
// void __thiscall FUN_004178a0(this /*ECX*/)
//
// Behaviour read from the 20-byte binary slice at RVA 0x000178a0:
//
//   Saves `this` (ECX) to EAX, then zeroes ECX (XOR ECX,ECX) and uses
//   ECX as the zero constant for five sequential DWORD stores into the
//   object:
//
//     this->field_04 = 0;   // [this+0x04]
//     this->field_08 = 0;   // [this+0x08]
//     this->field_0c = 0;   // [this+0x0c]
//     this->field_14 = 0;   // [this+0x14]
//     this->field_18 = 0;   // [this+0x18]
//
//   Offset 0x10 is deliberately skipped (not zeroed here).
//
//   No stack frame, no callee-saved registers, plain RET.
//
//   This function sits between FUN_00417820 (vector destructor, 109 B)
//   and FUN_004178c0 (container initialise-and-push, 175 B).
//   FUN_004178c0 also writes [ESI+0x14] and [ESI+0x18], so all three
//   functions likely operate on the same container-like object.
//
// Byte map (20 bytes, RVA 0x178a0 … 0x178b3 inclusive, no relocations):
//
//   8b c1                 MOV EAX, ECX
//   33 c9                 XOR ECX, ECX
//   89 48 04              MOV dword ptr [EAX+0x4], ECX
//   89 48 08              MOV dword ptr [EAX+0x8], ECX
//   89 48 0c              MOV dword ptr [EAX+0xc], ECX
//   89 48 14              MOV dword ptr [EAX+0x14], ECX
//   89 48 18              MOV dword ptr [EAX+0x18], ECX
//   c3                    RET

extern "C" __declspec(naked) void FUN_004178a0() {
    __asm {
        // 000178a0: 8b c1        MOV EAX, ECX
        _emit 0x8b
        _emit 0xc1
        // 000178a2: 33 c9        XOR ECX, ECX
        _emit 0x33
        _emit 0xc9
        // 000178a4: 89 48 04     MOV dword ptr [EAX+0x4], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 000178a7: 89 48 08     MOV dword ptr [EAX+0x8], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 000178aa: 89 48 0c     MOV dword ptr [EAX+0xc], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x0c
        // 000178ad: 89 48 14     MOV dword ptr [EAX+0x14], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x14
        // 000178b0: 89 48 18     MOV dword ptr [EAX+0x18], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x18
        // 000178b3: c3           RET
        _emit 0xc3
    }
}
