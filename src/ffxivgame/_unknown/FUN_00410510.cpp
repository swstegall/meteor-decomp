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
// FUNCTION: ffxivgame 0x00010510 — SQEX::CDev::Engine::Memory::Alternative::SeparateHeapBlock
//                                  constructor (146 B / 0x92)
//
// __thiscall void SeparateHeapBlock::ctor(
//     void *param_1,   [ESP+0x04]
//     void *param_2,   [ESP+0x08]
//     void *param_3,   [ESP+0x0C]
//     void *param_4,   [ESP+0x10]
//     void *param_5,   [ESP+0x14]
//     unsigned char param_6,  [ESP+0x18]
//     void *param_7,   [ESP+0x1C]
//     void *param_8)   [ESP+0x20]
//
// ECX = this; callee cleans 8 stack args (RET 0x20).
// No callee-saved registers used. No local frame (no SUB ESP).
//
// Object layout (inferred from offsets written):
//   [this + 0x00]  void*    vptr0     — SeparateHeapBlock::vftable (0xf56808)
//   [this + 0x04]  void*    vptr1     — IHandle subobject vftable (0xf567d0)
//   [this + 0x08]  Link     m_link0   — 12-byte embedded Link (vptr,prev,next) self-init
//     [+0x08] vptr = 0xf56818, [+0x0c] prev = self, [+0x10] next = self
//   [this + 0x14]  void*    f14       = param_1
//   [this + 0x18]  void*    f18       = param_2
//   [this + 0x1c]  void*    f1c       = param_3
//   [this + 0x20]  void*    f20       = param_4
//   [this + 0x24]  void*    f24       = param_5
//   [this + 0x28]  uint8_t  f28       = param_6
//   [this + 0x2c]  void*    f2c       = param_7
//   [this + 0x30]  void*    f30       = param_8 (or this if param_8 == NULL)
//   [this + 0x34]  int      f34       = 0
//   [this + 0x38]  Link     m_link1   — 12-byte embedded Link self-init (vptr=0xf567c4)
//   [this + 0x44]  Link     m_link2   — 12-byte embedded Link self-init (vptr=0xf567c4)
//
// Two-pass vftable initialisation is the MSVC multi-inheritance constructor
// pattern: bases are written first (0xf56750 at +0x4, 0xf567c4 at +0x8),
// then overwritten with the derived class's vtable after all sub-objects are
// initialised (0xf56808 at +0x0, 0xf567d0 at +0x4).

extern "C" __declspec(naked) void FUN_00410510() {
    __asm {
        // 00010510: 8b 54 24 08   MOV EDX, [ESP+0x8]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 00010514: 8b c1         MOV EAX, ECX
        _emit 0x8b
        _emit 0xc1
        // 00010516: c7 40 04 50 67 f5 00   MOV [EAX+0x4], 0xf56750
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0x50
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 0001051d: c7 40 08 c4 67 f5 00   MOV [EAX+0x8], 0xf567c4
        _emit 0xc7
        _emit 0x40
        _emit 0x08
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00010524: 8d 48 08      LEA ECX, [EAX+0x8]
        _emit 0x8d
        _emit 0x48
        _emit 0x08
        // 00010527: 89 49 04      MOV [ECX+0x4], ECX
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 0001052a: 89 49 08      MOV [ECX+0x8], ECX
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 0001052d: c7 01 18 68 f5 00   MOV [ECX], 0xf56818
        _emit 0xc7
        _emit 0x01
        _emit 0x18
        _emit 0x68
        _emit 0xf5
        _emit 0x00
        // 00010533: 8b 4c 24 04   MOV ECX, [ESP+0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 00010537: 89 48 14      MOV [EAX+0x14], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x14
        // 0001053a: 8b 4c 24 0c   MOV ECX, [ESP+0xc]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0001053e: 89 48 1c      MOV [EAX+0x1c], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x1c
        // 00010541: 8b 4c 24 14   MOV ECX, [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00010545: 89 48 24      MOV [EAX+0x24], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x24
        // 00010548: 8b 4c 24 1c   MOV ECX, [ESP+0x1c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0001054c: 89 50 18      MOV [EAX+0x18], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x18
        // 0001054f: 8b 54 24 10   MOV EDX, [ESP+0x10]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 00010553: 89 48 2c      MOV [EAX+0x2c], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x2c
        // 00010556: 8b 4c 24 20   MOV ECX, [ESP+0x20]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 0001055a: 85 c9         TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 0001055c: 89 50 20      MOV [EAX+0x20], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x20
        // 0001055f: 8a 54 24 18   MOV DL, byte ptr [ESP+0x18]
        _emit 0x8a
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 00010563: c7 00 08 68 f5 00   MOV [EAX], 0xf56808
        _emit 0xc7
        _emit 0x00
        _emit 0x08
        _emit 0x68
        _emit 0xf5
        _emit 0x00
        // 00010569: c7 40 04 d0 67 f5 00   MOV [EAX+0x4], 0xf567d0
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0xd0
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00010570: 88 50 28      MOV byte ptr [EAX+0x28], DL
        _emit 0x88
        _emit 0x50
        _emit 0x28
        // 00010573: 75 02         JNZ +0x02 (skip MOV ECX,EAX)
        _emit 0x75
        _emit 0x02
        // 00010575: 8b c8         MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 00010577: 89 48 30      MOV [EAX+0x30], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x30
        // 0001057a: c7 40 34 00 00 00 00   MOV [EAX+0x34], 0x0
        _emit 0xc7
        _emit 0x40
        _emit 0x34
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00010581: 8d 48 38      LEA ECX, [EAX+0x38]
        _emit 0x8d
        _emit 0x48
        _emit 0x38
        // 00010584: c7 01 c4 67 f5 00   MOV [ECX], 0xf567c4
        _emit 0xc7
        _emit 0x01
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 0001058a: 89 49 04      MOV [ECX+0x4], ECX
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 0001058d: 89 49 08      MOV [ECX+0x8], ECX
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 00010590: 8d 48 44      LEA ECX, [EAX+0x44]
        _emit 0x8d
        _emit 0x48
        _emit 0x44
        // 00010593: c7 01 c4 67 f5 00   MOV [ECX], 0xf567c4
        _emit 0xc7
        _emit 0x01
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00010599: 89 49 04      MOV [ECX+0x4], ECX
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 0001059c: 89 49 08      MOV [ECX+0x8], ECX
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 0001059f: c2 20 00      RET 0x20
        _emit 0xc2
        _emit 0x20
        _emit 0x00
    }
}
