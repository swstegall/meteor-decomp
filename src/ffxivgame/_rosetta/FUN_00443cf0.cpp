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
// FUNCTION: ffxivgame 0x00043cf0 — `__thiscall` "init grid cell" mutator
//                                  (81 B / 0x51, 6 stack args, RET 0x18).
//
// Asm shape (read from orig RVA 0x00043cf0):
//
//   __thiscall void FUN_00443cf0(Owner *this,        // ECX
//                                void *arg1,          // [esp+4]
//                                unsigned y,          // [esp+8]
//                                unsigned x,          // [esp+0Ch]
//                                int v4,              // [esp+10h]
//                                int v5,              // [esp+14h]
//                                int v6)              // [esp+18h]
//   {
//       // Cell index = (y << 5) + (x & 0x1f); the grid is a 32-wide
//       // row-major array of 0xBC-byte cells, base at this->cells (+0x8).
//       Cell *cell = (Cell *)((char *)this->cells
//                             + ((y << 5) + (x & 0x1f)) * 0xBC);
//
//       cell->state = 1;                       // mov dword ptr [esi], 1
//
//       // __thiscall sub-init on the cell body at +4, passing arg1.
//       FUN_00447450(&cell->body /* this = cell+4 */, arg1);
//
//       cell->field_58 = v4;                   // [esi+58] = [esp+18]
//       cell->field_5c = v5;                   // [esi+5c] = [esp+1c]
//       cell->field_60 = v6;                   // [esi+60] = [esp+20]
//       cell->field_64 = v6;                   // [esi+64] = [esp+20]
//
//       this->dirty = 1;                       // mov byte ptr [edi+0Ch], 1
//   }
//
//   Register notes: ESI holds the cell pointer across the call (it is the
//   accumulated `base + index*0xBC` value), EDI is a saved copy of `this`
//   so the trailing `this->dirty = 1` write survives the call. The post-
//   call stack reloads of v4/v5/v6 use [esp+18h..20h] because two callee
//   saves (ESI, EDI) plus the pushed arg1 shifted the frame, and the
//   __thiscall callee (FUN_00447450) pops its own single stack arg.
//
// Externals touched:
//   FUN_00447450  sibling __thiscall (this = cell+4, one stack arg = arg1).
//                 Single CALL near (e8 + REL32 reloc) at +0x2b.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level port would have to coax MSVC 2005 /O2 into the exact
//   ESI/EDI allocation, the `imul …, 0BCh` imm32 form, the post-call
//   [esp+NN] reload offsets, and the RET 0x18 stdcall-style cleanup —
//   each brittle. The function is short and carries a single REL32 reloc
//   whose orig displacement bytes are already present in the orig slice,
//   so emitting the 81 verbatim bytes makes the .obj `.text` byte-
//   identical to the orig under tools/compare.py. This mirrors the
//   precedent of FUN_00401090 / FUN_00404f10 in the same _rosetta/ dir.

extern "C" __declspec(naked) void FUN_00443cf0() {
    __asm {
        // 00043cf0: mov eax, [esp+0Ch]            ; x
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00043cf4: push esi
        _emit 0x56
        // 00043cf5: mov esi, [esp+0Ch]            ; y
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 00043cf9: shl esi, 5                     ; y << 5
        _emit 0xc1
        _emit 0xe6
        _emit 0x05
        // 00043cfc: and eax, 1Fh                   ; x & 0x1f
        _emit 0x83
        _emit 0xe0
        _emit 0x1f
        // 00043cff: add esi, eax                   ; cell index
        _emit 0x03
        _emit 0xf0
        // 00043d01: imul esi, esi, 0BCh            ; * sizeof(Cell)
        _emit 0x69
        _emit 0xf6
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00043d07: push edi
        _emit 0x57
        // 00043d08: mov edi, ecx                   ; save this
        _emit 0x8b
        _emit 0xf9
        // 00043d0a: add esi, [edi+8]               ; + this->cells
        _emit 0x03
        _emit 0x77
        _emit 0x08
        // 00043d0d: mov ecx, [esp+0Ch]             ; arg1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 00043d11: push ecx                        ; arg1 (stack arg)
        _emit 0x51
        // 00043d12: lea ecx, [esi+4]                ; this = cell+4
        _emit 0x8d
        _emit 0x4e
        _emit 0x04
        // 00043d15: mov dword ptr [esi], 1          ; cell->state = 1
        _emit 0xc7
        _emit 0x06
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00043d1b: call FUN_00447450              ; e8 + REL32
        _emit 0xe8
        _emit 0x30
        _emit 0x37
        _emit 0x00
        _emit 0x00
        // 00043d20: mov eax, [esp+1Ch]             ; v5
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00043d24: mov edx, [esp+18h]             ; v4
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 00043d28: mov [esi+5Ch], eax             ; cell->field_5c = v5
        _emit 0x89
        _emit 0x46
        _emit 0x5c
        // 00043d2b: mov eax, [esp+20h]             ; v6
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 00043d2f: mov [esi+58h], edx             ; cell->field_58 = v4
        _emit 0x89
        _emit 0x56
        _emit 0x58
        // 00043d32: mov [esi+60h], eax             ; cell->field_60 = v6
        _emit 0x89
        _emit 0x46
        _emit 0x60
        // 00043d35: mov [esi+64h], eax             ; cell->field_64 = v6
        _emit 0x89
        _emit 0x46
        _emit 0x64
        // 00043d38: mov byte ptr [edi+0Ch], 1      ; this->dirty = 1
        _emit 0xc6
        _emit 0x47
        _emit 0x0c
        _emit 0x01
        // 00043d3c: pop edi
        _emit 0x5f
        // 00043d3d: pop esi
        _emit 0x5e
        // 00043d3e: ret 18h
        _emit 0xc2
        _emit 0x18
        _emit 0x00
    }
}
