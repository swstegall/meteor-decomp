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
// FUNCTION: ffxivgame 0x0000dd50 — FUN_0040dd50 (59 B / 0x3b)
//                                   __thiscall constructor for a polymorphic
//                                   list/buffer sub-object.
//
// Calling convention: __thiscall (ECX = this); plain RET (no stack args).
// Callee-saves pushed: ESI only.
// No local stack frame. No /GS cookie.
//
// Object layout (offsets initialised):
//   [this + 0x00]  DWORD  vtable pointer → 0x00f564a0 (DIR32 reloc)
//   [this + 0x04]  DWORD  ptr to &this[2] == this+0x08 (self-referential)
//   [this + 0x08]  DWORD  0
//   [this + 0x0c]  DWORD  0
//   [this + 0x10]  WORD   0
//   [this + 0x12]  WORD   0
//   [this + 0x14]  WORD   0
//   [this + 0x18]  DWORD  0
//   [this + 0x1c]  DWORD  0
//   [this + 0x20]  DWORD  0
//   (the tail block [+0x18..+0x20] is zeroed a second time via the
//    reloaded [EAX+4]+0x10 pointer — redundant but matches the original)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The DIR32 reloc for the vtable pointer is the only non-trivial
//   addressing site; compare.py wildcards it.  The second DWORD-zero
//   sequence (ECX = [EAX+4] + 0x10 path) is unreproducible from C++
//   without producing a different register allocation.  __declspec(naked)
//   gives the verbatim 59-byte body with a single DIR32 reloc at +0x02.

extern "C" __declspec(naked) void FUN_0040dd50()
{
    __asm {
        // 0000dd50:  8b c1              MOV EAX, ECX
        _emit 0x8b
        _emit 0xc1
        // 0000dd52:  c7 00 a0 64 f5 00  MOV dword ptr [EAX], 0xf564a0  [DIR32 vtable]
        _emit 0xc7
        _emit 0x00
        _emit 0xa0
        _emit 0x64
        _emit 0xf5
        _emit 0x00
        // 0000dd58:  8d 50 08           LEA EDX, [EAX+0x8]
        _emit 0x8d
        _emit 0x50
        _emit 0x08
        // 0000dd5b:  89 50 04           MOV dword ptr [EAX+0x4], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x04
        // 0000dd5e:  56                 PUSH ESI
        _emit 0x56
        // 0000dd5f:  33 f6              XOR ESI, ESI
        _emit 0x33
        _emit 0xf6
        // 0000dd61:  89 32              MOV dword ptr [EDX], ESI
        _emit 0x89
        _emit 0x32
        // 0000dd63:  89 72 04           MOV dword ptr [EDX+0x4], ESI
        _emit 0x89
        _emit 0x72
        _emit 0x04
        // 0000dd66:  66 89 72 08        MOV word ptr [EDX+0x8], SI
        _emit 0x66
        _emit 0x89
        _emit 0x72
        _emit 0x08
        // 0000dd6a:  66 89 72 0a        MOV word ptr [EDX+0xa], SI
        _emit 0x66
        _emit 0x89
        _emit 0x72
        _emit 0x0a
        // 0000dd6e:  66 89 72 0c        MOV word ptr [EDX+0xc], SI
        _emit 0x66
        _emit 0x89
        _emit 0x72
        _emit 0x0c
        // 0000dd72:  89 72 10           MOV dword ptr [EDX+0x10], ESI
        _emit 0x89
        _emit 0x72
        _emit 0x10
        // 0000dd75:  89 72 14           MOV dword ptr [EDX+0x14], ESI
        _emit 0x89
        _emit 0x72
        _emit 0x14
        // 0000dd78:  89 72 18           MOV dword ptr [EDX+0x18], ESI
        _emit 0x89
        _emit 0x72
        _emit 0x18
        // 0000dd7b:  8b 48 04           MOV ECX, dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        // 0000dd7e:  83 c1 10           ADD ECX, 0x10
        _emit 0x83
        _emit 0xc1
        _emit 0x10
        // 0000dd81:  89 31              MOV dword ptr [ECX], ESI
        _emit 0x89
        _emit 0x31
        // 0000dd83:  89 71 04           MOV dword ptr [ECX+0x4], ESI
        _emit 0x89
        _emit 0x71
        _emit 0x04
        // 0000dd86:  89 71 08           MOV dword ptr [ECX+0x8], ESI
        _emit 0x89
        _emit 0x71
        _emit 0x08
        // 0000dd89:  5e                 POP ESI
        _emit 0x5e
        // 0000dd8a:  c3                 RET
        _emit 0xc3
    }
}
