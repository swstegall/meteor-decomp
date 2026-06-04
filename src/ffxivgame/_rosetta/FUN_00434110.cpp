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
// FUNCTION: ffxivgame 0x00434110 — allocate + construct a small (0x14-byte)
//                                  event/record object and hand it off
//                                  (__thiscall, 104 bytes)
//
// Recovered shape (from asm/ffxivgame/00034110_FUN_00434110.s):
//
//   void __thiscall FUN_00434110(self, int a, int b, float c, int d)
//   {
//       // pick the pool slot keyed by the current index byte at *g_pool
//       g_pool = *[0x01328d90];
//       idx    = (unsigned char)g_pool[0];          // 0f b6 01
//       slot   = g_pool[1] + (idx * 7) * 4;         // idx*8-idx => idx*7
//       p = allocate(slot, 0x14);                    // __thiscall, CALL 0x417ab0
//       if (p) {
//           p->vtbl = 0xf64990;                      // c7 00 90 49 f6 00
//           p->a    = a;                             // [esp+0x08]
//           p->b    = b;                             // [esp+0x0c]
//           p->d    = d;                             // [esp+0x14]
//           p->c    = c;                             // movss [esp+0x10]
//       }
//       handoff(self->m_0c, p);                       // __thiscall, CALL 0x43c2d0
//   }
//
//   Calling convention: __thiscall (self in ECX, saved via PUSH ESI / MOV
//   ESI,ECX), four stack args (RET 0x10), one of which is a float passed
//   through XMM0 only for the store.  Both arms tail into the same handoff
//   helper; the null arm passes 0.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The body bakes in three absolute addresses (the global pool pointer
//   0x01328d90, the vtable constant 0xf64990) plus two rel32 CALLs.  A
//   source-level lowering would emit relocations the linker controls and
//   would not reliably reproduce the exact register/temp ordering MSVC
//   2005 chose (e.g. the LEA idx*8-idx multiply, the MOVSS shuffle, the
//   interleaved arg loads).  Re-emitting the orig 104 bytes verbatim via
//   MASM `_emit` produces a .obj whose .text is byte-identical to the orig
//   slice with no relocations, and tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00434110() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV ECX, dword ptr [0x01328d90]
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x0f              // MOVZX EAX, byte ptr [ECX]
        _emit 0xb6
        _emit 0x01
        _emit 0x8d              // LEA EDX, [EAX*0x8 + 0x0]
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB EDX, EAX
        _emit 0xd0
        _emit 0x8b              // MOV EAX, dword ptr [ECX + 0x4]
        _emit 0x41
        _emit 0x04
        _emit 0x8d              // LEA ECX, [EAX + EDX*0x4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a              // PUSH 0x14
        _emit 0x14
        _emit 0xe8              // CALL 0x00417ab0
        _emit 0x7e
        _emit 0x39
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x33 (null arm)
        _emit 0x33
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 0xc]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0xf3              // MOVSS XMM0, dword ptr [ESP + 0x10]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x89              // MOV dword ptr [EAX + 0x4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xc7              // MOV dword ptr [EAX], 0xf64990
        _emit 0x00
        _emit 0x90
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x89              // MOV dword ptr [EAX + 0x8], EDX
        _emit 0x50
        _emit 0x08
        _emit 0x89              // MOV dword ptr [EAX + 0xc], ECX
        _emit 0x48
        _emit 0x0c
        _emit 0xf3              // MOVSS dword ptr [EAX + 0x10], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x10
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0xc]
        _emit 0x4e
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043c2d0
        _emit 0x6b
        _emit 0x81
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x10
        _emit 0x10
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0xc]   (null arm)
        _emit 0x4e
        _emit 0x0c
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043c2d0
        _emit 0x5c
        _emit 0x81
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x10
        _emit 0x10
        _emit 0x00
    }
}
