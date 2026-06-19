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
// FUNCTION: ffxivgame 0x00046e90 — __thiscall buffer "less-or-equal" predicate
//                                  (45 bytes / 0x2D).
//
// Layout (inferred from the asm — a small {ptr,len} buffer/string view):
//   Buffer (this, ECX):
//     +0x00  void  *data
//     +0x08  size_t len
//
// Source shape:
//
//   bool __thiscall Buffer::operator<=(const Buffer &rhs) const {
//       size_t n = this->len;                 // EAX
//       if (n >= rhs.len)                     // CMP / JC (unsigned)
//           n = rhs.len;                      //   n = min(this->len, rhs.len)
//       int r = memcmp(this->data, rhs.data, n);   // peer @0x00445b70
//       return r <= 0;                        // XOR EDX / TEST / SETLE DL
//   }
//
// Calling convention: __thiscall (ECX = this; one stack arg = const Buffer*;
// callee cleans 4 bytes via `ret 4`). The inner memcmp-like call is __cdecl
// (three pushes, `add esp, 0xc`).
//
// Frame:
//   PUSH ESI                              ; lone callee-save (holds rhs)
//   [no ESP adjustment]
//
// Quirks MSVC's scheduler picked here:
//   - min() via "branch carries the smaller": `CMP EAX,EDX / JC keep /
//     MOV EAX,EDX` keeps EAX when this->len < rhs.len (JC = below,
//     unsigned), else overwrites with rhs.len.
//   - Boolean result materialised through EDX: `XOR EDX,EDX / TEST EAX,EAX
//     / SETLE DL / MOV AL,DL` — signed `<= 0` on memcmp's int return.
//
// Structurally identical to FUN_00446f20 (the "is-greater" predicate,
// peer at RVA 0x00046f20) but uses SETLE (0x0F 0x9E) instead of SETG
// (0x0F 0x9F), and carries a different REL32 for the CALL to 0x00445b70.
//
// Naked __asm so the exact short-form JC, the push ordering, and the REL32
// callsite to the memcmp-like peer (0x00445b70) are pinned to the orig
// encoding. The REL32 CALL is emitted as raw bytes; tools/compare.py masks
// it out of the diff.

extern "C" __declspec(naked) void FUN_00446e90() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ECX + 0x8]
        _emit 0x41
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP + 0x8]
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ESI + 0x8]
        _emit 0x56
        _emit 0x08
        _emit 0x3b              // CMP EAX, EDX
        _emit 0xc2
        _emit 0x72              // JC +0x02
        _emit 0x02
        _emit 0x8b              // MOV EAX, EDX
        _emit 0xc2
        _emit 0x8b              // MOV ECX, dword ptr [ECX]
        _emit 0x09
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL 0x00445b70 (rel32)
        _emit 0xc3
        _emit 0xec
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x33              // XOR EDX, EDX
        _emit 0xd2
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x0f              // SETLE DL
        _emit 0x9e
        _emit 0xc2
        _emit 0x8a              // MOV AL, DL
        _emit 0xc2
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
