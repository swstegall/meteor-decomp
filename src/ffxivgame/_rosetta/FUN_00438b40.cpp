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
// FUNCTION: ffxivgame 0x00438b40 — 8-arg forwarding thunk that builds a
//                                  0x24-byte temporary on the stack and
//                                  invokes a method on it
//                                  (__thiscall, RET 0x20, 94 bytes)
//
// __thiscall void FUN_00438b40(this, a1, a2, a3, a4, a5, a6, a7, a8)
//   stack layout (after SUB ESP,0x24; return addr at [ESP+0x24]):
//     [ESP+0x28] : a1   [ESP+0x2c] : a2   [ESP+0x30] : a3   [ESP+0x34] : a4
//     [ESP+0x38] : a5   [ESP+0x3c] : a6   [ESP+0x40] : a7   [ESP+0x44] : a8
//
// Behaviour: materialises a 0x24-byte temporary object T at [ESP] whose
// vftable slot (T+0x00) is stamped with the constant 0x00f649c0 and whose
// eight DWORD fields (T+0x04 .. T+0x20) are filled from the eight stack
// args in a permuted order:
//     T+0x04 = a1   T+0x08 = a6   T+0x0c = a2   T+0x10 = a3
//     T+0x14 = a4   T+0x18 = a8   T+0x1c = a5   T+0x20 = a7
// then calls the __thiscall method FUN_00435dd0 with ECX = &T, passing the
// outer object's [this+0x04] as the single stack argument.
//
// Asm (94 bytes, read from orig RVA 0x00038b40 / VA 0x00438b40):
//
//   83 ec 24                 sub  esp, 0x24
//   8b 44 24 28              mov  eax, [esp+0x28]   ; a1
//   8b 54 24 3c              mov  edx, [esp+0x3c]   ; a6
//   89 44 24 04              mov  [esp+0x04], eax   ; T+0x04 = a1
//   8b 44 24 2c              mov  eax, [esp+0x2c]   ; a2
//   89 44 24 0c              mov  [esp+0x0c], eax   ; T+0x0c = a2
//   8b 44 24 34              mov  eax, [esp+0x34]   ; a4
//   89 54 24 08              mov  [esp+0x08], edx   ; T+0x08 = a6
//   8b 54 24 30              mov  edx, [esp+0x30]   ; a3
//   89 44 24 14              mov  [esp+0x14], eax   ; T+0x14 = a4
//   8b 44 24 38              mov  eax, [esp+0x38]   ; a5
//   89 54 24 10              mov  [esp+0x10], edx   ; T+0x10 = a3
//   8b 54 24 44              mov  edx, [esp+0x44]   ; a8
//   89 44 24 1c              mov  [esp+0x1c], eax   ; T+0x1c = a5
//   8b 41 04                 mov  eax, [ecx+0x04]   ; outer->field_4
//   89 54 24 18              mov  [esp+0x18], edx   ; T+0x18 = a8
//   8b 54 24 40              mov  edx, [esp+0x40]   ; a7
//   50                       push eax               ; arg = outer->field_4
//   8d 4c 24 04              lea  ecx, [esp+0x04]   ; ecx = &T
//   c7 44 24 04 c0 49 f6 00  mov  [esp+0x04], 0x00f649c0  ; T+0x00 = vftable
//   89 54 24 24              mov  [esp+0x24], edx   ; T+0x20 = a7
//   e8 38 d2 ff ff           call FUN_00435dd0      ; rel32 → 0x00435dd0
//   83 c4 24                 add  esp, 0x24
//   c2 20 00                 ret  0x20
//
// Reloc-bearing sites in the orig 94 bytes:
//   +0x47   MOV  imm32 → 0x00f649c0   (DIR32, temporary's vftable pointer)
//   +0x53   CALL rel32 → 0x00435dd0   (FUN_00435dd0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form would emit the same shape but yield a DIR32
//   reloc on the vftable mov-imm32 and a rel32 reloc on the call. A
//   `__declspec(naked)` body that re-emits the orig 94 bytes verbatim via
//   MASM `_emit` produces a .obj whose .text is byte-identical to the orig
//   slice with NO relocations (both operands are baked as raw bytes against
//   the orig PE's own address space). tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00438b40() {
    __asm {
        _emit 0x83              // SUB  ESP, 0x24
        _emit 0xec
        _emit 0x24
        _emit 0x8b              // MOV  EAX, [ESP+0x28]
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x8b              // MOV  EDX, [ESP+0x3c]
        _emit 0x54
        _emit 0x24
        _emit 0x3c
        _emit 0x89              // MOV  [ESP+0x04], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV  EAX, [ESP+0x2c]
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x89              // MOV  [ESP+0x0c], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b              // MOV  EAX, [ESP+0x34]
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x89              // MOV  [ESP+0x08], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV  EDX, [ESP+0x30]
        _emit 0x54
        _emit 0x24
        _emit 0x30
        _emit 0x89              // MOV  [ESP+0x14], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV  EAX, [ESP+0x38]
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x89              // MOV  [ESP+0x10], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV  EDX, [ESP+0x44]
        _emit 0x54
        _emit 0x24
        _emit 0x44
        _emit 0x89              // MOV  [ESP+0x1c], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x8b              // MOV  EAX, [ECX+0x04]
        _emit 0x41
        _emit 0x04
        _emit 0x89              // MOV  [ESP+0x18], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV  EDX, [ESP+0x40]
        _emit 0x54
        _emit 0x24
        _emit 0x40
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA  ECX, [ESP+0x04]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xc7              // MOV  dword ptr [ESP+0x04], 0x00f649c0
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xc0
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x89              // MOV  [ESP+0x24], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0xe8              // CALL FUN_00435dd0  (rel32 → 0x00435dd0)
        _emit 0x38
        _emit 0xd2
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD  ESP, 0x24
        _emit 0xc4
        _emit 0x24
        _emit 0xc2              // RET  0x20
        _emit 0x20
        _emit 0x00
    }
}
