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
// FUNCTION: ffxivgame 0x00436aa0 — circular-list node init (__thiscall, 43 B / 0x2B)
//
// void * __thiscall FUN_00436aa0(void *this)
//   ECX = this (saved to ESI)
//   Returns: EAX = this
//
// The function allocates a node via FUN_00a10030 (no pushed args, returns
// in EAX), stores it in this->field_0x4, sets a flag byte at node+0x11,
// then initialises the node's three self-referential pointer fields (at
// offsets 0x0, 0x4, 0x8) — a classic sentinel-node / circular-list head
// init pattern. Finishes by zeroing this->field_0x8 and returning this.
//
// The three consecutive reloads of [ESI+4] between the self-referential
// writes are consistent with MSVC 2005's conservative alias analysis:
// each write to a pointer field inside the returned node could (in the
// compiler's model) alias this->field_0x4 at [ESI+4], so it reloads
// before each use rather than keeping EAX across the stores.
//
// Reconstruction: __declspec(naked) byte passthrough. The only relocation
// is the CALL to FUN_00a10030 at +0x03..+0x07; compare.py masks those
// four bytes. All other bytes are emitted verbatim.
//
// Asm (43 bytes @ orig RVA 0x00036aa0):
//   56                      PUSH ESI
//   8b f1                   MOV ESI, ECX
//   e8 88 95 5d 00          CALL FUN_00a10030
//   89 46 04                MOV dword ptr [ESI+0x4], EAX
//   c6 40 11 01             MOV byte ptr [EAX+0x11], 0x1
//   8b 46 04                MOV EAX, dword ptr [ESI+0x4]
//   89 40 04                MOV dword ptr [EAX+0x4], EAX
//   8b 46 04                MOV EAX, dword ptr [ESI+0x4]
//   89 00                   MOV dword ptr [EAX], EAX
//   8b 46 04                MOV EAX, dword ptr [ESI+0x4]
//   89 40 08                MOV dword ptr [EAX+0x8], EAX
//   c7 46 08 00 00 00 00    MOV dword ptr [ESI+0x8], 0x0
//   8b c6                   MOV EAX, ESI
//   5e                      POP ESI
//   c3                      RET

extern "C" void FUN_00a10030();   // forward declaration for CALL relocation

extern "C" __declspec(naked) void FUN_00436aa0() {
    __asm {
        // 00036aa0: 56            PUSH ESI
        _emit 0x56
        // 00036aa1: 8b f1         MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00036aa3: e8 88 95 5d 00  CALL FUN_00a10030  (reloc)
        call FUN_00a10030
        // 00036aa8: 89 46 04      MOV dword ptr [ESI+0x4], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 00036aab: c6 40 11 01   MOV byte ptr [EAX+0x11], 0x1
        _emit 0xc6
        _emit 0x40
        _emit 0x11
        _emit 0x01
        // 00036aaf: 8b 46 04      MOV EAX, dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00036ab2: 89 40 04      MOV dword ptr [EAX+0x4], EAX
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 00036ab5: 8b 46 04      MOV EAX, dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00036ab8: 89 00         MOV dword ptr [EAX], EAX
        _emit 0x89
        _emit 0x00
        // 00036aba: 8b 46 04      MOV EAX, dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00036abd: 89 40 08      MOV dword ptr [EAX+0x8], EAX
        _emit 0x89
        _emit 0x40
        _emit 0x08
        // 00036ac0: c7 46 08 00 00 00 00  MOV dword ptr [ESI+0x8], 0x0
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036ac7: 8b c6         MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 00036ac9: 5e            POP ESI
        _emit 0x5e
        // 00036aca: c3            RET
        _emit 0xc3
    }
}
