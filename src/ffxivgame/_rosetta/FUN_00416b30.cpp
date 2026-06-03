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
// FUNCTION: ffxivgame 0x00016b30 — __thiscall element-address getter (41 B / 0x29)
//
// int __thiscall FUN_00416b30(SomeObj *this, unsigned int param_1)
//
// Bounds-checks param_1 against this->field_0x0C (unsigned compare).
// If this->field_0x0C < param_1, returns 0 immediately (early exit before
// saving any callee-save registers). Otherwise computes:
//
//   (uint8)this->field_0x16 + (uint8)this->field_0x15 + (uint8)this->field_0x14
//   multiplied by param_1, then adds this->field_0x04.
//
// This is a stride-based element-pointer calculation: three byte fields
// encode the element size (in three parts), param_1 is the index, and
// field_0x04 is the base pointer/offset. Returns the computed address.
//
// Calling convention: __thiscall (ECX = this, one stack arg = [ESP+4]).
//   Epilogue: RET 0x4 (callee pops one arg).
// Frame: /Oy — no frame pointer; ESI saved with PUSH/POP.
//   The PUSH ESI is deferred past the early-exit branch (MSVC /O2 idiom:
//   only save callee-preserving regs on the path that actually uses them).
//
// No external calls or global-address references → no relocations.
// Exact 41-byte byte passthrough via __declspec(naked) + _emit.
//
// Asm (41 bytes @ orig RVA 0x00016b30):
//   8b 54 24 04       MOV  EDX, dword ptr [ESP+0x4]       ; param_1
//   39 51 0c          CMP  dword ptr [ECX+0xC], EDX        ; count vs param_1
//   73 05             JNC  +0x05                           ; if count >= param jump ahead
//   33 c0             XOR  EAX, EAX                        ; return 0
//   c2 04 00          RET  0x4
//   0f b6 41 16       MOVZX EAX, byte ptr [ECX+0x16]       ; stride part 2
//   56                PUSH ESI
//   0f b6 71 15       MOVZX ESI, byte ptr [ECX+0x15]       ; stride part 1
//   03 c6             ADD  EAX, ESI
//   0f b6 71 14       MOVZX ESI, byte ptr [ECX+0x14]       ; stride part 0
//   03 c6             ADD  EAX, ESI                        ; EAX = total stride
//   0f af c2          IMUL EAX, EDX                        ; stride * index
//   03 41 04          ADD  EAX, dword ptr [ECX+0x4]        ; + base
//   5e                POP  ESI
//   c2 04 00          RET  0x4

extern "C" __declspec(naked) void FUN_00416b30() {
    __asm {
        // 00016b30: 8b 54 24 04   MOV EDX, [ESP+4]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x04
        // 00016b34: 39 51 0c      CMP [ECX+0xC], EDX
        _emit 0x39
        _emit 0x51
        _emit 0x0c
        // 00016b37: 73 05         JNC +5
        _emit 0x73
        _emit 0x05
        // 00016b39: 33 c0         XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00016b3b: c2 04 00      RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 00016b3e: 0f b6 41 16   MOVZX EAX, byte ptr [ECX+0x16]
        _emit 0x0f
        _emit 0xb6
        _emit 0x41
        _emit 0x16
        // 00016b42: 56            PUSH ESI
        _emit 0x56
        // 00016b43: 0f b6 71 15   MOVZX ESI, byte ptr [ECX+0x15]
        _emit 0x0f
        _emit 0xb6
        _emit 0x71
        _emit 0x15
        // 00016b47: 03 c6         ADD EAX, ESI
        _emit 0x03
        _emit 0xc6
        // 00016b49: 0f b6 71 14   MOVZX ESI, byte ptr [ECX+0x14]
        _emit 0x0f
        _emit 0xb6
        _emit 0x71
        _emit 0x14
        // 00016b4d: 03 c6         ADD EAX, ESI
        _emit 0x03
        _emit 0xc6
        // 00016b4f: 0f af c2      IMUL EAX, EDX
        _emit 0x0f
        _emit 0xaf
        _emit 0xc2
        // 00016b52: 03 41 04      ADD EAX, [ECX+4]
        _emit 0x03
        _emit 0x41
        _emit 0x04
        // 00016b55: 5e            POP ESI
        _emit 0x5e
        // 00016b56: c2 04 00      RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
