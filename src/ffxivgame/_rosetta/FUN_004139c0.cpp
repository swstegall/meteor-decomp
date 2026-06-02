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
// FUNCTION: ffxivgame 0x004139c0 — DetachableHeapSpace embedded debug-space
//           init accessor (7 B, __thiscall, no frame, no relocations).
//
// Sets the `debug_space_inner` backpointer at this+0x58 to `this`, then
// returns a pointer to the embedded DebugDetachableHeapSpace subobject at
// this+0x54.
//
// Asm:
//   89 49 58    MOV dword ptr [ECX + 0x58], ECX   ; this->debug_space_inner = this
//   8d 41 54    LEA EAX, [ECX + 0x54]             ; EAX = &this->debug_space_at_54
//   c3          RET

extern "C" __declspec(naked) void FUN_004139c0() {
    __asm {
        mov dword ptr [ecx + 0x58], ecx
        lea eax, [ecx + 0x54]
        ret
    }
}
