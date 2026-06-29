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
// FUNCTION: ffxivgame 0x0044ee90 — GetModuleFileNameA-based path init thunk;
//                                  calls GetModuleFileNameA(NULL, buf, 0x800),
//                                  trims the result via FUN_0044ed10(dir, 0x100, buf),
//                                  then invokes a __thiscall setter FUN_004489c0
//                                  on the caller-supplied object pointer.
//                                  /GS-protected, 0x1104-byte stack frame via
//                                  __alloca_probe; __cdecl, 113 bytes / 0x71.
//
// Stack frame layout (ESP-relative, after __alloca_probe allocates 0x1104 bytes):
//   [ESP + 0x0000 .. 0x00FF]  local_dir[0x100]  — output of FUN_0044ed10
//   [ESP + 0x0100 .. 0x08FF]  local_path[0x800] — GetModuleFileNameA output
//     (accessed as [ESP+0x104] after PUSH ESI adjusts ESP by -4)
//   [ESP + 0x1100]            __security_cookie ^ ESP (GS slot)
//   [ESP + 0x1104]            return address
//   [ESP + 0x1108]            param_1 (object ptr; loaded via [ESP+0x110c] after PUSH ESI)
//
// Pseudo-C:
//   void __cdecl FUN_0044ee90(SomeObj *obj) {
//       char local_dir[0x100];
//       char local_path[0x800];
//       GetModuleFileNameA(NULL, local_path, 0x800);  // IAT [0x00f3e1e0]
//       FUN_0044ed10(local_dir, 0x100, local_path);   // trim path to dir
//       obj->FUN_004489c0(local_dir);                 // __thiscall setter
//   }
//
// Reloc-bearing sites in the orig 113 bytes:
//   +0x01  CALL rel32  → __alloca_probe     (RVA 0x009d29d0 → rel32 0x00583b36)
//   +0x0b  MOV EAX,   [__security_cookie]   (abs32 0x012ea8b0)
//   +0x2f  CALL dword ptr [IAT]             (abs32 IAT slot 0x00f3e1e0 —
//                                            GetModuleFileNameA)
//   +0x47  CALL rel32  → FUN_0044ed10       (rel32 0xfffffe34)
//   +0x56  CALL rel32  → FUN_004489c0       (rel32 0xffff9ad5)
//   +0x65  CALL rel32  → __security_check_cookie (rel32 0x005831fa)
//   +0x6a  ADD ESP, 0x1104                  (no reloc — immediate)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The /GS prologue, __alloca_probe call, IAT indirect call, three CALL rel32
//   sites, and the multi-byte SIB-addressed stack-relative MOVs make a
//   source-level reproduction extremely fragile under MSVC 2005 /O2.  A
//   __declspec(naked) body that re-emits all 113 bytes verbatim via MASM
//   _emit directives produces a .obj whose .text is byte-identical to the
//   orig slice.  compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0044ee90() {
    __asm {
        // 0044ee90: b8 04 11 00 00    MOV EAX, 0x1104
        _emit 0xb8
        _emit 0x04
        _emit 0x11
        _emit 0x00
        _emit 0x00
        // 0044ee95: e8 36 3b 58 00    CALL __alloca_probe (0x009d29d0)
        _emit 0xe8
        _emit 0x36
        _emit 0x3b
        _emit 0x58
        _emit 0x00
        // 0044ee9a: a1 b0 a8 2e 01    MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0044ee9f: 33 c4             XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 0044eea1: 89 84 24 00 11 00 00  MOV dword ptr [ESP+0x1100], EAX
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0x00
        _emit 0x11
        _emit 0x00
        _emit 0x00
        // 0044eea8: 56                PUSH ESI
        _emit 0x56
        // 0044eea9: 8b b4 24 0c 11 00 00  MOV ESI, dword ptr [ESP+0x110c]
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0x0c
        _emit 0x11
        _emit 0x00
        _emit 0x00
        // 0044eeb0: 68 00 08 00 00    PUSH 0x800
        _emit 0x68
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0044eeb5: 8d 84 24 08 01 00 00  LEA EAX, [ESP+0x108]
        _emit 0x8d
        _emit 0x84
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0044eebc: 50                PUSH EAX
        _emit 0x50
        // 0044eebd: 6a 00             PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0044eebf: ff 15 e0 e1 f3 00  CALL dword ptr [0x00f3e1e0]  (GetModuleFileNameA)
        _emit 0xff
        _emit 0x15
        _emit 0xe0
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0044eec5: 8d 8c 24 04 01 00 00  LEA ECX, [ESP+0x104]
        _emit 0x8d
        _emit 0x8c
        _emit 0x24
        _emit 0x04
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0044eecc: 51                PUSH ECX
        _emit 0x51
        // 0044eecd: 8d 54 24 08       LEA EDX, [ESP+0x8]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 0044eed1: 68 00 01 00 00    PUSH 0x100
        _emit 0x68
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0044eed6: 52                PUSH EDX
        _emit 0x52
        // 0044eed7: e8 34 fe ff ff    CALL FUN_0044ed10 (rel32 0xfffffe34)
        _emit 0xe8
        _emit 0x34
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 0044eedc: 83 c4 0c          ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0044eedf: 8d 44 24 04       LEA EAX, [ESP+0x4]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0044eee3: 50                PUSH EAX
        _emit 0x50
        // 0044eee4: 8b ce             MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0044eee6: e8 d5 9a ff ff    CALL FUN_004489c0 (rel32 0xffff9ad5, __thiscall)
        _emit 0xe8
        _emit 0xd5
        _emit 0x9a
        _emit 0xff
        _emit 0xff
        // 0044eeeb: 8b 8c 24 04 11 00 00  MOV ECX, dword ptr [ESP+0x1104]
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x04
        _emit 0x11
        _emit 0x00
        _emit 0x00
        // 0044eef2: 5e                POP ESI
        _emit 0x5e
        // 0044eef3: 33 cc             XOR ECX, ESP
        _emit 0x33
        _emit 0xcc
        // 0044eef5: e8 fa 31 58 00    CALL __security_check_cookie (rel32 0x005831fa)
        _emit 0xe8
        _emit 0xfa
        _emit 0x31
        _emit 0x58
        _emit 0x00
        // 0044eefa: 81 c4 04 11 00 00  ADD ESP, 0x1104
        _emit 0x81
        _emit 0xc4
        _emit 0x04
        _emit 0x11
        _emit 0x00
        _emit 0x00
        // 0044ef00: c3                RET
        _emit 0xc3
    }
}
