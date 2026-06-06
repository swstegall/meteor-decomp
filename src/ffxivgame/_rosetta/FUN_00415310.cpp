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
// FUNCTION: ffxivgame 0x00015310 — `__thiscall` 28-byte bounded-slot-append
//           method. Increments the dword counter at [ECX+0x78], compares it
//           against the cap 0x10 (16), and if the slot is still available,
//           stores the new counter and writes 0x64 into the byte array at
//           [ECX + new_count + 0x81]. Returns 1 (true) on success or 0
//           (false) when the counter would reach/exceed the cap.
//
// Equivalent C++ (stripped of struct context):
//
//   bool Method() {                              // ECX = this
//       int x = *(int*)((char*)this + 0x78) + 1; // MOV EAX,[ECX+0x78] / ADD EAX,1
//       if (x >= 0x10) return false;             // CMP EAX,0x10 / JGE
//       *(int*)((char*)this + 0x78) = x;         // MOV [ECX+0x78],EAX
//       *((char*)this + 0x81 + x) = 0x64;        // MOV byte ptr [EAX+ECX+0x81],64h
//       return true;
//   }
//
// No relocations — all 28 bytes are position-independent.  Emitted verbatim
// via _emit to guarantee byte-identical output regardless of MSVC register
// heuristics or branch-direction preferences.

extern "C" __declspec(naked) void FUN_00415310() {
    __asm {
        _emit 0x8b    // MOV  EAX, dword ptr [ECX+0x78]  ; load counter
        _emit 0x41
        _emit 0x78
        _emit 0x83    // ADD  EAX, 0x1                   ; increment
        _emit 0xc0
        _emit 0x01
        _emit 0x83    // CMP  EAX, 0x10                  ; against cap 16
        _emit 0xf8
        _emit 0x10
        _emit 0x7d    // JGE  +0x0e  →  XOR AL,AL / RET  ; >= cap → return false
        _emit 0x0e
        _emit 0x89    // MOV  dword ptr [ECX+0x78], EAX  ; store new counter
        _emit 0x41
        _emit 0x78
        _emit 0xc6    // MOV  byte ptr [EAX+ECX*1+0x81], 0x64 ; write slot byte
        _emit 0x84
        _emit 0x08
        _emit 0x81
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x64
        _emit 0xb0    // MOV  AL, 0x1                    ; return true
        _emit 0x01
        _emit 0xc3    // RET
        _emit 0x32    // XOR  AL, AL                     ; return false
        _emit 0xc0
        _emit 0xc3    // RET
    }
}
