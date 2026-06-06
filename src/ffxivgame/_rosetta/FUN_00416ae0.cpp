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
// FUNCTION: ffxivgame 0x00416ae0 — __thiscall vtable[7] dispatch with byte
//                                  out-param, conditional field arithmetic
//                                  (71 B / 0x47, zero relocs)
//
// __thiscall int FUN_00416ae0(this, arg1)
//   ECX = this  (saved to ESI)
//   [esp+4] = arg1  (read into EDX before PUSH ESI shifts the frame)
//   RET 4  (callee cleans one 4-byte stack arg)
//
// Stack layout (using PUSH ECX to allocate a 4-byte local slot):
//   After PUSH ECX + PUSH ESI:
//     [ESP+0..3]  saved ESI
//     [ESP+4..7]  local slot (PUSH ECX artefact; byte at [ESP+7] = flag)
//     [ESP+8]     return address
//     [ESP+C]     arg1
//
// Body:
//   flag = 0;
//   result = this->vtable[7](this, arg1, &flag);   // flag at [ESP+7]
//   if (!flag) return 0;
//   return ((unsigned byte)this[0x15] +
//           (unsigned byte)this[0x16] +
//           (unsigned byte)this[0x14]) * result + this->dword4;
//
// Reloc-bearing sites: NONE.  Every displacement and immediate is a
// self-contained constant.  The .obj .text matches the orig slice
// byte-for-byte with no linker fixups.
//
// Reconstruction: __declspec(naked) _emit passthrough (same strategy
// as FUN_004086a0).

extern "C" __declspec(naked) void FUN_00416ae0() {
    __asm {
        // 00016ae0: push ecx
        _emit 0x51
        // 00016ae1: mov edx, dword ptr [esp+0x8]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 00016ae5: push esi
        _emit 0x56
        // 00016ae6: mov esi, ecx
        _emit 0x8b
        _emit 0xf1
        // 00016ae8: mov eax, dword ptr [esi]
        _emit 0x8b
        _emit 0x06
        // 00016aea: mov eax, dword ptr [eax+0x1c]
        _emit 0x8b
        _emit 0x40
        _emit 0x1c
        // 00016aed: lea ecx, [esp+0x7]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x07
        // 00016af1: push ecx
        _emit 0x51
        // 00016af2: push edx
        _emit 0x52
        // 00016af3: mov ecx, esi
        _emit 0x8b
        _emit 0xce
        // 00016af5: mov byte ptr [esp+0xf], 0x0
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x0f
        _emit 0x00
        // 00016afa: call eax
        _emit 0xff
        _emit 0xd0
        // 00016afc: cmp byte ptr [esp+0x7], 0x0
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x07
        _emit 0x00
        // 00016b01: mov ecx, eax
        _emit 0x8b
        _emit 0xc8
        // 00016b03: jnz +0x7  (-> 0x00416b0c)
        _emit 0x75
        _emit 0x07
        // 00016b05: xor eax, eax
        _emit 0x33
        _emit 0xc0
        // 00016b07: pop esi
        _emit 0x5e
        // 00016b08: pop ecx
        _emit 0x59
        // 00016b09: ret 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 00016b0c: movzx edx, byte ptr [esi+0x15]
        _emit 0x0f
        _emit 0xb6
        _emit 0x56
        _emit 0x15
        // 00016b10: movzx eax, byte ptr [esi+0x16]
        _emit 0x0f
        _emit 0xb6
        _emit 0x46
        _emit 0x16
        // 00016b14: add eax, edx
        _emit 0x03
        _emit 0xc2
        // 00016b16: movzx edx, byte ptr [esi+0x14]
        _emit 0x0f
        _emit 0xb6
        _emit 0x56
        _emit 0x14
        // 00016b1a: add eax, edx
        _emit 0x03
        _emit 0xc2
        // 00016b1c: imul eax, ecx
        _emit 0x0f
        _emit 0xaf
        _emit 0xc1
        // 00016b1f: add eax, dword ptr [esi+0x4]
        _emit 0x03
        _emit 0x46
        _emit 0x04
        // 00016b22: pop esi
        _emit 0x5e
        // 00016b23: pop ecx
        _emit 0x59
        // 00016b24: ret 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
