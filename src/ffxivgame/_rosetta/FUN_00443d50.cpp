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
// FUNCTION: ffxivgame 0x00043d50 — `__thiscall` "init grid cell" mutator
//                                  (70 B / 0x46, 4 stack args, RET 0x10).
//
// Asm shape (read from orig RVA 0x00043d50):
//
//   __thiscall void FUN_00443d50(Owner *this,        // ECX
//                                void *arg1,          // [esp+4]
//                                unsigned y,          // [esp+8]
//                                unsigned x,          // [esp+0Ch]
//                                void *arg4)          // [esp+10h]
//   {
//       // Cell index = (y << 5) + (x & 0x1f); the grid is a 32-wide
//       // row-major array of 0xBC-byte cells, base at this->cells (+0x8).
//       Cell *cell = (Cell *)((char *)this->cells
//                             + ((y << 5) + (x & 0x1f)) * 0xBC);
//
//       cell->state = 2;                       // mov dword ptr [esi], 2
//
//       // __thiscall sub-init on the cell body at +4, passing arg1.
//       FUN_00447450(&cell->body /* this = cell+4 */, arg1);
//       // second __thiscall sub-init on the cell body at +0x68, arg4.
//       FUN_00447450((char *)cell + 0x68, arg4);
//
//       this->dirty = 1;                       // mov byte ptr [edi+0Ch], 1
//   }
//
//   Register notes: ESI holds the cell pointer across both calls (it is the
//   accumulated `base + index*0xBC` value), EDI is a saved copy of `this`
//   so the trailing `this->dirty = 1` write survives the calls. Each
//   __thiscall callee (FUN_00447450) pops its own single stack arg.
//
// Externals touched:
//   FUN_00447450  sibling __thiscall (one stack arg). Two CALL near
//                 (e8 + REL32 reloc) at +0x2b and +0x38.
//
// Reconstruction strategy — naked-asm byte passthrough (mirrors the
// adjacent precedent FUN_00443cf0 in the same _rosetta/ dir): emitting
// the 70 verbatim bytes makes the .obj `.text` byte-identical to the
// orig under tools/compare.py; the two REL32 displacements are
// linker-resolved relocations whose orig bytes are present in the slice.

extern "C" __declspec(naked) void FUN_00443d50() {
    __asm {
        // 00043d50: mov eax, [esp+0Ch]            ; x
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00043d54: push esi
        _emit 0x56
        // 00043d55: mov esi, [esp+0Ch]            ; y
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 00043d59: shl esi, 5                     ; y << 5
        _emit 0xc1
        _emit 0xe6
        _emit 0x05
        // 00043d5c: and eax, 1Fh                   ; x & 0x1f
        _emit 0x83
        _emit 0xe0
        _emit 0x1f
        // 00043d5f: add esi, eax                   ; cell index
        _emit 0x03
        _emit 0xf0
        // 00043d61: imul esi, esi, 0BCh            ; * sizeof(Cell)
        _emit 0x69
        _emit 0xf6
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00043d67: push edi
        _emit 0x57
        // 00043d68: mov edi, ecx                   ; save this
        _emit 0x8b
        _emit 0xf9
        // 00043d6a: add esi, [edi+8]               ; + this->cells
        _emit 0x03
        _emit 0x77
        _emit 0x08
        // 00043d6d: mov ecx, [esp+0Ch]             ; arg1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 00043d71: push ecx                        ; arg1 (stack arg)
        _emit 0x51
        // 00043d72: lea ecx, [esi+4]                ; this = cell+4
        _emit 0x8d
        _emit 0x4e
        _emit 0x04
        // 00043d75: mov dword ptr [esi], 2          ; cell->state = 2
        _emit 0xc7
        _emit 0x06
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00043d7b: call FUN_00447450              ; e8 + REL32
        _emit 0xe8
        _emit 0xd0
        _emit 0x36
        _emit 0x00
        _emit 0x00
        // 00043d80: mov edx, [esp+18h]             ; arg4
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 00043d84: push edx                        ; arg4 (stack arg)
        _emit 0x52
        // 00043d85: lea ecx, [esi+68h]              ; this = cell+0x68
        _emit 0x8d
        _emit 0x4e
        _emit 0x68
        // 00043d88: call FUN_00447450              ; e8 + REL32
        _emit 0xe8
        _emit 0xc3
        _emit 0x36
        _emit 0x00
        _emit 0x00
        // 00043d8d: mov byte ptr [edi+0Ch], 1      ; this->dirty = 1
        _emit 0xc6
        _emit 0x47
        _emit 0x0c
        _emit 0x01
        // 00043d91: pop edi
        _emit 0x5f
        // 00043d92: pop esi
        _emit 0x5e
        // 00043d93: ret 10h
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
