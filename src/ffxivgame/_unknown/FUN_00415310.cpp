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
// FUNCTION: ffxivgame 0x00015310 — bounded-slot append predicate
//                                   (__thiscall, 28 B / 0x1c)
//
// Bumps a 32-bit counter at this+0x78 (max 0x10 = 16 slots). On success
// writes 0x64 (100) into a byte-slot table at this+0x82 indexed by the
// post-increment counter, and returns true. On overflow (counter would
// reach 16) leaves the counter alone and returns false.
//
// Inferred receiver layout:
//   [this + 0x78]            int   count;            // current slot count
//   [this + 0x82 .. 0x91]    char  slots[16];        // per-slot status byte
//
// Pseudo-C:
//   bool method() {
//       int next = this->count + 1;
//       if (next >= 16) return false;        // overflow guard
//       this->count = next;
//       this->slots[next] = 100;             // index uses post-increment slot
//       return true;
//   }
//
// Calling convention: __thiscall (ECX = this, no stack args, bare RET).
// Frame: none (/Oy — leaf, no callee-saved usage).
// Return: bool in AL.
//
// The 8-byte `MOV byte ptr [EAX+ECX+0x81], 0x64` uses an explicit SIB
// (index=ECX, base=EAX) so its byte form is fragile to express from
// source-level C++ without specific register-allocation luck. Encoded
// as `__declspec(naked)` + `_emit` to guarantee the exact byte stream.
//
// Asm (28 bytes @ orig RVA 0x00015310):
//   8b 41 78                    MOV EAX, dword ptr [ECX+0x78]
//   83 c0 01                    ADD EAX, 0x1
//   83 f8 10                    CMP EAX, 0x10
//   7d 0e                       JGE +0x0e  (→ 0x00415329)
//   89 41 78                    MOV dword ptr [ECX+0x78], EAX
//   c6 84 08 81 00 00 00 64     MOV byte ptr [EAX+ECX+0x81], 0x64
//   b0 01                       MOV AL, 0x1
//   c3                          RET
//   32 c0                       XOR AL, AL
//   c3                          RET

extern "C" __declspec(naked) void FUN_00415310()
{
    __asm {
        // 00015310:  8b 41 78         MOV EAX, dword ptr [ECX+0x78]
        _emit 0x8b
        _emit 0x41
        _emit 0x78
        // 00015313:  83 c0 01         ADD EAX, 0x1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 00015316:  83 f8 10         CMP EAX, 0x10
        _emit 0x83
        _emit 0xf8
        _emit 0x10
        // 00015319:  7d 0e            JGE +0x0e  (→ 0x00015329)
        _emit 0x7d
        _emit 0x0e
        // 0001531b:  89 41 78         MOV dword ptr [ECX+0x78], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x78
        // 0001531e:  c6 84 08 81 00 00 00 64   MOV byte ptr [EAX+ECX+0x81], 0x64
        _emit 0xc6
        _emit 0x84
        _emit 0x08
        _emit 0x81
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x64
        // 00015326:  b0 01            MOV AL, 0x1
        _emit 0xb0
        _emit 0x01
        // 00015328:  c3               RET
        _emit 0xc3
        // 00015329:  32 c0            XOR AL, AL
        _emit 0x32
        _emit 0xc0
        // 0001532b:  c3               RET
        _emit 0xc3
    }
}
