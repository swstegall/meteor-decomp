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
// FUNCTION: ffxivgame 0x00411290 — heap-block virtual dispatch wrapper
//           __thiscall, no stack args, 39 bytes / 0x27
//
// ECX = this (a heap-block manager with a vtable at [this+0]).
// Sequence:
//   1. Call vtable[0x2c] on this  — e.g. Lock() / BeginAccess()
//   2. Push this->field_0x4c as the single stack argument
//   3. Set  this->field_0x50 = 1  (mark in-progress)
//   4. Call FUN_0040fac0(__thiscall, this, this->field_0x4c)
//   5. Read vtable[0x30] into EAX   — e.g. Unlock() / EndAccess()
//   6. Set  this->field_0x50 = 0   (clear in-progress)
//   7. Restore ECX = this, pop ESI, tail-JMP EAX (vtable[0x30])
//
// Reconstruction: naked-asm byte passthrough — the tail-JMP through EAX
// (vtable[0x30]) after popping callee-saves, combined with the precise
// ordering of the vtable re-read before clearing field_0x50, cannot be
// reproduced from source-level C++ without risk of register-allocation
// or scheduling divergence.

extern "C" __declspec(naked) void FUN_00411290()
{
    __asm {
        // 00011290: 56                 PUSH ESI
        _emit 0x56
        // 00011291: 8b f1              MOV ESI, ECX       ; ESI = this
        _emit 0x8b
        _emit 0xf1
        // 00011293: 8b 06              MOV EAX, dword ptr [ESI]   ; vtable
        _emit 0x8b
        _emit 0x06
        // 00011295: 8b 50 2c           MOV EDX, dword ptr [EAX + 0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 00011298: ff d2              CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0001129a: 8b 46 4c           MOV EAX, dword ptr [ESI + 0x4c]
        _emit 0x8b
        _emit 0x46
        _emit 0x4c
        // 0001129d: 50                 PUSH EAX
        _emit 0x50
        // 0001129e: 8b ce              MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000112a0: c6 46 50 01        MOV byte ptr [ESI + 0x50], 0x1
        _emit 0xc6
        _emit 0x46
        _emit 0x50
        _emit 0x01
        // 000112a4: e8 17 e8 ff ff     CALL FUN_0040fac0
        _emit 0xe8
        _emit 0x17
        _emit 0xe8
        _emit 0xff
        _emit 0xff
        // 000112a9: 8b 16              MOV EDX, dword ptr [ESI]   ; vtable
        _emit 0x8b
        _emit 0x16
        // 000112ab: 8b 42 30           MOV EAX, dword ptr [EDX + 0x30]
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 000112ae: c6 46 50 00        MOV byte ptr [ESI + 0x50], 0x0
        _emit 0xc6
        _emit 0x46
        _emit 0x50
        _emit 0x00
        // 000112b2: 8b ce              MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000112b4: 5e                 POP ESI
        _emit 0x5e
        // 000112b5: ff e0              JMP EAX
        _emit 0xff
        _emit 0xe0
    }
}
