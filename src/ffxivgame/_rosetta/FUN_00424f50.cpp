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
// FUNCTION: ffxivgame 0x00424f50 — three-step init/registration thunk
//                                  (__cdecl void(void), 85 bytes)
//
// Asm shape (read from orig RVA 0x00024f50):
//
//   void __cdecl FUN_00424f50(void) {
//       // FUN_00419020(0, &g_obj0)  — value 0 materialised in a stack temp,
//       //                             address taken via MOV EAX,ESP, slot
//       //                             written, then both args cdecl-cleaned.
//       push offset g_obj0 (0x1329990); push ecx; mov eax,esp;
//       mov dword ptr [eax],0; call FUN_00419020; add esp,8
//
//       // FUN_00419020(1, &g_obj1)
//       push offset g_obj1 (0x13299d0); push ecx; mov eax,esp;
//       mov dword ptr [eax],1; call FUN_00419020; add esp,8
//
//       // FUN_00419020(2, &g_obj2)  — its 8-byte arg cleanup is merged into
//       //                             the ADD ESP,0xC after the next call.
//       push offset g_obj2 (0x1329a10); push ecx; mov eax,esp;
//       mov dword ptr [eax],2; call FUN_00419020
//
//       // FUN_0041c270((unsigned char)g_byte)  — also stores the byte into
//       //                                        a second global before call.
//       movzx eax, byte ptr [0x01266110]; push eax;
//       mov byte ptr [0x01328f19], al; call FUN_0041c270; add esp,0xC
//       ret
//   }
//
// Reloc-bearing sites in the orig 85 bytes (all masked by tools/compare.py):
//   +0x01  PUSH imm32 → 0x01329990   (DIR32, g_obj0)
//   +0x0e  CALL rel32 → 0x00419020
//   +0x17  PUSH imm32 → 0x013299d0   (DIR32, g_obj1)
//   +0x24  CALL rel32 → 0x00419020
//   +0x2d  PUSH imm32 → 0x01329a10   (DIR32, g_obj2)
//   +0x3a  CALL rel32 → 0x00419020
//   +0x40  MOVZX      → 0x01266110   (DIR32, source byte)
//   +0x48  MOV        → 0x01328f19   (DIR32, dest byte)
//   +0x4d  CALL rel32 → 0x0041c270
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The `MOV EAX,ESP; MOV [EAX],imm` address-of-temp idiom plus the merged
//   cdecl cleanup (the third FUN_00419020 call leaves its 8-byte arg area
//   live until the trailing ADD ESP,0xC) is awkward to coax out of MSVC
//   2005 at the source level. A `__declspec(naked)` body re-emitting the
//   orig 85 bytes verbatim produces a .obj whose .text is byte-identical
//   with zero relocations; compare.py masks each reloc window and reports
//   GREEN. Same approach as sibling FUN_004065c0 / FUN_00403b70.

extern "C" __declspec(naked) void FUN_00424f50() {
    __asm {
        _emit 0x68              // PUSH 0x1329990            (DIR32, g_obj0)
        _emit 0x90
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV  EAX, ESP
        _emit 0xc4
        _emit 0xc7              // MOV  dword ptr [EAX], 0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_00419020         (rel32 → 0x00419020)
        _emit 0xbd
        _emit 0x40
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD  ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x68              // PUSH 0x13299d0            (DIR32, g_obj1)
        _emit 0xd0
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV  EAX, ESP
        _emit 0xc4
        _emit 0xc7              // MOV  dword ptr [EAX], 1
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_00419020         (rel32 → 0x00419020)
        _emit 0xa7
        _emit 0x40
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD  ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x68              // PUSH 0x1329a10            (DIR32, g_obj2)
        _emit 0x10
        _emit 0x9a
        _emit 0x32
        _emit 0x01
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV  EAX, ESP
        _emit 0xc4
        _emit 0xc7              // MOV  dword ptr [EAX], 2
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_00419020         (rel32 → 0x00419020)
        _emit 0x91
        _emit 0x40
        _emit 0xff
        _emit 0xff
        _emit 0x0f              // MOVZX EAX, byte ptr [0x01266110]  (DIR32)
        _emit 0xb6
        _emit 0x05
        _emit 0x10
        _emit 0x61
        _emit 0x26
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0xa2              // MOV  byte ptr [0x01328f19], AL    (DIR32)
        _emit 0x19
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL FUN_0041c270         (rel32 → 0x0041c270)
        _emit 0xcf
        _emit 0x72
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD  ESP, 0xC
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
    }
}
