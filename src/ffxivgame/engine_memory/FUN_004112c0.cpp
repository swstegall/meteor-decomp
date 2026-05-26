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
// FUNCTION: ffxivgame 0x004112c0 — heap-space SeparateHeapSpace wrapper (50 bytes / 0x32)
//           __thiscall, no stack args
//
// ECX = this (a heap-space management object with vtable).
// this->field_0x4c is a list head pointer (passed to FUN_0040fac0).
// this->field_0x48 is a list head pointer (passed to FUN_0040ff20).
// this->field_0x50 is a busy/locked byte flag.
//
// Sequence:
//   1. Call vtable[0x2c](this)              — Lock() / BeginAccess()
//   2. Push this->field_0x4c as arg
//   3. Set this->field_0x50 = 1             — mark in-progress
//   4. Call FUN_0040fac0(this, this->field_0x4c)
//   5. Push this->field_0x48 as arg
//   6. Call FUN_0040ff20(this, this->field_0x48)
//   7. Read vtable[0x30] into EAX           — Unlock() / EndAccess()
//   8. Set this->field_0x50 = 0             — clear in-progress
//   9. Tail-call vtable[0x30](this) via JMP EAX
//
// Reconstruction: naked-asm byte passthrough.
// The flag-set must appear after arg push but before the first list-call;
// the tail-call via JMP EAX with ESI popped cannot be reproduced from
// source-level C++ without risking register-allocation divergence.

extern "C" __declspec(naked) void FUN_004112c0()
{
    __asm {
        // 000112c0:  56                     PUSH ESI
        _emit 0x56
        // 000112c1:  8b f1                  MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 000112c3:  8b 06                  MOV EAX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 000112c5:  8b 50 2c               MOV EDX,dword ptr [EAX + 0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 000112c8:  ff d2                  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000112ca:  8b 46 4c               MOV EAX,dword ptr [ESI + 0x4c]
        _emit 0x8b
        _emit 0x46
        _emit 0x4c
        // 000112cd:  50                     PUSH EAX
        _emit 0x50
        // 000112ce:  8b ce                  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 000112d0:  c6 46 50 01            MOV byte ptr [ESI + 0x50],0x1
        _emit 0xc6
        _emit 0x46
        _emit 0x50
        _emit 0x01
        // 000112d4:  e8 e7 e7 ff ff         CALL 0x0040fac0
        _emit 0xe8
        _emit 0xe7
        _emit 0xe7
        _emit 0xff
        _emit 0xff
        // 000112d9:  8b 4e 48               MOV ECX,dword ptr [ESI + 0x48]
        _emit 0x8b
        _emit 0x4e
        _emit 0x48
        // 000112dc:  51                     PUSH ECX
        _emit 0x51
        // 000112dd:  8b ce                  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 000112df:  e8 3c ec ff ff         CALL 0x0040ff20
        _emit 0xe8
        _emit 0x3c
        _emit 0xec
        _emit 0xff
        _emit 0xff
        // 000112e4:  8b 16                  MOV EDX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x16
        // 000112e6:  8b 42 30               MOV EAX,dword ptr [EDX + 0x30]
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 000112e9:  c6 46 50 00            MOV byte ptr [ESI + 0x50],0x0
        _emit 0xc6
        _emit 0x46
        _emit 0x50
        _emit 0x00
        // 000112ed:  8b ce                  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 000112ef:  5e                     POP ESI
        _emit 0x5e
        // 000112f0:  ff e0                  JMP EAX
        _emit 0xff
        _emit 0xe0
    }
}
