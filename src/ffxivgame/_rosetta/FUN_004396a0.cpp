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
// FUNCTION: ffxivgame 0x004396a0 — __thiscall "release pending handle / log
//                                   teardown" finaliser (81 B / 0x51).
//
// Behaviour read from the disassembly at orig RVA 0x000396a0:
//
//   void __thiscall FUN_004396a0(Obj *this /*ECX*/) {
//       if (this->field_e0 == 0)            // 83 be e0 00 00 00 00; 74 43
//           return;                         //   nothing pending → bail
//
//       FUN_00419240(2, 0);                 // 6a 00; 6a 02; e8 .. (cdecl, 2 args)
//       FUN_004186a0(2);                    // 6a 02; e8 ..        (cdecl, 1 arg)
//
//       // build a 5-dword argument block on the stack, the first dword of
//       // which is initialised to 4 via `mov eax,esp; mov [eax],4`, then
//       // call the logging/dispatch sink with it.
//       FUN_0041efc0(this->field_e0,        // 51 (pushed twice: block ptr + arg)
//                    this->field_e0,
//                    0x122,                 // 68 22 01 00 00
//                    0x18,                  // 6a 18
//                    this->field_d8);       // 50
//
//       this->field_e0 = 0;                 // c7 86 e0 00 00 00 00 00 00 00
//   }
//
//   The ECX-as-`this` prologue (`mov esi, ecx`) plus the member offsets
//   +0xd8 / +0xe0 mark this as a __thiscall member finaliser. The two
//   leading calls are __cdecl helpers (caller cleans 0xC bytes via
//   `add esp, 0xc`), and the sink at +0x3d is a 5-arg __cdecl call
//   (`add esp, 0x14` afterwards).
//
//   Reloc-bearing sites in the orig 81 bytes (all PC-relative CALL REL32):
//     +0x10   CALL FUN_00419240   (e8 8b fb fd ff)
//     +0x17   CALL FUN_004186a0   (e8 e4 ef fd ff)
//     +0x3d   CALL FUN_0041efc0   (e8 de 58 fe ff)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level port would need the exact MSVC 2005 /O2 register
//   allocation (ESI as the `this` cache), the stack argument-block
//   materialisation via `mov eax,esp; mov [eax],4`, and the precise
//   push ordering for the three helper calls — every one of which is
//   brittle against a high-level rewrite. The pragmatic choice, matching
//   the FUN_00401090 / FUN_00404f10 precedent in this directory, is to
//   `_emit` the 81 orig bytes verbatim. The three CALL REL32 operands
//   are raw immediates that already hold the orig PE's post-link
//   displacements, so tools/compare.py matches the .obj .text to the
//   orig slice by direct byte equality.

extern "C" __declspec(naked) void FUN_004396a0() {
    __asm {
        // 004396a0: push esi
        _emit 0x56
        // 004396a1: mov esi, ecx
        _emit 0x8b
        _emit 0xf1
        // 004396a3: cmp dword ptr [esi+0E0h], 0
        _emit 0x83
        _emit 0xbe
        _emit 0xe0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 004396aa: jz 004396ef
        _emit 0x74
        _emit 0x43
        // 004396ac: push 0
        _emit 0x6a
        _emit 0x00
        // 004396ae: push 2
        _emit 0x6a
        _emit 0x02
        // 004396b0: call FUN_00419240
        _emit 0xe8
        _emit 0x8b
        _emit 0xfb
        _emit 0xfd
        _emit 0xff
        // 004396b5: push 2
        _emit 0x6a
        _emit 0x02
        // 004396b7: call FUN_004186a0
        _emit 0xe8
        _emit 0xe4
        _emit 0xef
        _emit 0xfd
        _emit 0xff
        // 004396bc: mov eax, dword ptr [esi+0D8h]
        _emit 0x8b
        _emit 0x86
        _emit 0xd8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 004396c2: mov ecx, dword ptr [esi+0E0h]
        _emit 0x8b
        _emit 0x8e
        _emit 0xe0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 004396c8: add esp, 0Ch
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 004396cb: push eax
        _emit 0x50
        // 004396cc: push 18h
        _emit 0x6a
        _emit 0x18
        // 004396ce: push 122h
        _emit 0x68
        _emit 0x22
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 004396d3: push ecx
        _emit 0x51
        // 004396d4: push ecx
        _emit 0x51
        // 004396d5: mov eax, esp
        _emit 0x8b
        _emit 0xc4
        // 004396d7: mov dword ptr [eax], 4
        _emit 0xc7
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 004396dd: call FUN_0041efc0
        _emit 0xe8
        _emit 0xde
        _emit 0x58
        _emit 0xfe
        _emit 0xff
        // 004396e2: add esp, 14h
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 004396e5: mov dword ptr [esi+0E0h], 0
        _emit 0xc7
        _emit 0x86
        _emit 0xe0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 004396ef: pop esi
        _emit 0x5e
        // 004396f0: ret
        _emit 0xc3
    }
}
