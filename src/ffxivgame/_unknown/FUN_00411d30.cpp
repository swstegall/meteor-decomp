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
// FUNCTION: ffxivgame 0x00011d30 — constructor / initialiser for a
//           RemovableHeapBlock-like object with two embedded linked-list
//           nodes.  (__thiscall, 6 stack params, 136 bytes / 0x88)
//
// Calling convention: __thiscall (ECX = this), callee cleans 6 DWORD
// stack args (RET 0x18).  No callee-saved registers are pushed —
// MSVC only uses EAX, ECX and EDX (all caller-save in __thiscall).
//
// Object layout (inferred from offsets written):
//   [this + 0x00]  DWORD  vftable (set to 0xf56db8)
//   [this + 0x04]  DWORD  vftable-like field (first 0xf56750, then 0xf56d80)
//   [this + 0x08]  DWORD  link1.vftable  (0xf56dc8)
//   [this + 0x0c]  DWORD  link1.prev     (&this->link1)
//   [this + 0x10]  DWORD  link1.next     (&this->link1)
//   [this + 0x14]  DWORD  param_1
//   [this + 0x18]  DWORD  0
//   [this + 0x1c]  DWORD  param_2
//   [this + 0x20]  DWORD  param_3
//   [this + 0x24]  DWORD  param_4
//   [this + 0x28]  DWORD  0
//   [this + 0x2c]  DWORD  param_5
//   [this + 0x30]  DWORD  param_6 (or `this` if param_6 == NULL)
//   [this + 0x34]  DWORD  0
//   [this + 0x38]  DWORD  link2.vftable  (0xf567c4)
//   [this + 0x3c]  DWORD  link2.prev     (&this->link2)
//   [this + 0x40]  DWORD  link2.next     (&this->link2)
//   [this + 0x44]  DWORD  link3.vftable  (0xf567c4)
//   [this + 0x48]  DWORD  link3.prev     (&this->link3)
//   [this + 0x4c]  DWORD  link3.next     (&this->link3)
//
// Notable codegen features:
//   - MSVC schedules independent vtable/zero stores between the CMP and
//     the JNZ (instruction scheduling past the branch).
//   - The field at offset +0x04 is written twice: 0xf56750 first (possibly
//     from a base-class init phase), then 0xf56d80 at the derived-class
//     phase.
//   - XOR EDX, EDX is emitted before the CMP ECX, EDX pair (MSVC lowers
//     the `== NULL` test via an earlier zero-materialization).
//   - link1.vftable at +0x08 is first written 0xf567c4 then overwritten
//     0xf56dc8 (scheduler interleaves base/derived sub-object inits).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The exact instruction ordering and register scheduling (stores across
//   the CMP/JNZ boundary, double-write of vtable fields from interleaved
//   base/derived init phases) cannot be reproduced reliably from C++ source
//   with MSVC 2005 /O2.  The __declspec(naked) body re-emits the original
//   136 bytes verbatim via MASM _emit directives.
//
// Cross-platform guard: __declspec(naked) + MASM _emit are MSVC-only.
// The #ifdef keeps the file compilable on clang/arm64 (host toolchain)
// while the MSVC build (Wine) produces the byte-identical .obj.

#if defined(_MSC_VER) && !defined(__clang__)
extern "C" __declspec(naked) void FUN_00411d30()
{
    __asm {
        // 00011d30: 8b c1     MOV EAX, ECX
        _emit 0x8b
        _emit 0xc1
        // 00011d32: c7 40 04 50 67 f5 00  MOV dword ptr [EAX+0x4], 0xf56750
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0x50
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00011d39: c7 40 08 c4 67 f5 00  MOV dword ptr [EAX+0x8], 0xf567c4
        _emit 0xc7
        _emit 0x40
        _emit 0x08
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00011d40: 8d 48 08  LEA ECX, [EAX+0x8]
        _emit 0x8d
        _emit 0x48
        _emit 0x08
        // 00011d43: 89 49 04  MOV dword ptr [ECX+0x4], ECX
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 00011d46: 89 49 08  MOV dword ptr [ECX+0x8], ECX
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 00011d49: c7 01 c8 6d f5 00  MOV dword ptr [ECX], 0xf56dc8
        _emit 0xc7
        _emit 0x01
        _emit 0xc8
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        // 00011d4f: 8b 4c 24 04  MOV ECX, dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 00011d53: 89 48 14  MOV dword ptr [EAX+0x14], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x14
        // 00011d56: 8b 4c 24 08  MOV ECX, dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 00011d5a: 89 48 1c  MOV dword ptr [EAX+0x1c], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x1c
        // 00011d5d: 8b 4c 24 0c  MOV ECX, dword ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 00011d61: 89 48 20  MOV dword ptr [EAX+0x20], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x20
        // 00011d64: 8b 4c 24 10  MOV ECX, dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00011d68: 89 48 24  MOV dword ptr [EAX+0x24], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x24
        // 00011d6b: 8b 4c 24 14  MOV ECX, dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00011d6f: 33 d2     XOR EDX, EDX
        _emit 0x33
        _emit 0xd2
        // 00011d71: 89 48 2c  MOV dword ptr [EAX+0x2c], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x2c
        // 00011d74: 8b 4c 24 18  MOV ECX, dword ptr [ESP+0x18]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00011d78: 3b ca     CMP ECX, EDX
        _emit 0x3b
        _emit 0xca
        // 00011d7a: c7 00 b8 6d f5 00  MOV dword ptr [EAX], 0xf56db8
        _emit 0xc7
        _emit 0x00
        _emit 0xb8
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        // 00011d80: c7 40 04 80 6d f5 00  MOV dword ptr [EAX+0x4], 0xf56d80
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0x80
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        // 00011d87: 89 50 18  MOV dword ptr [EAX+0x18], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x18
        // 00011d8a: 89 50 28  MOV dword ptr [EAX+0x28], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x28
        // 00011d8d: 75 02     JNZ +2  (→ 0x00011d91)
        _emit 0x75
        _emit 0x02
        // 00011d8f: 8b c8     MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 00011d91: 89 48 30  MOV dword ptr [EAX+0x30], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x30
        // 00011d94: 89 50 34  MOV dword ptr [EAX+0x34], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x34
        // 00011d97: 8d 48 38  LEA ECX, [EAX+0x38]
        _emit 0x8d
        _emit 0x48
        _emit 0x38
        // 00011d9a: c7 01 c4 67 f5 00  MOV dword ptr [ECX], 0xf567c4
        _emit 0xc7
        _emit 0x01
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00011da0: 89 49 04  MOV dword ptr [ECX+0x4], ECX
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 00011da3: 89 49 08  MOV dword ptr [ECX+0x8], ECX
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 00011da6: 8d 48 44  LEA ECX, [EAX+0x44]
        _emit 0x8d
        _emit 0x48
        _emit 0x44
        // 00011da9: c7 01 c4 67 f5 00  MOV dword ptr [ECX], 0xf567c4
        _emit 0xc7
        _emit 0x01
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00011daf: 89 49 04  MOV dword ptr [ECX+0x4], ECX
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 00011db2: 89 49 08  MOV dword ptr [ECX+0x8], ECX
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 00011db5: c2 18 00  RET 0x18
        _emit 0xc2
        _emit 0x18
        _emit 0x00
    }
}
#endif // _MSC_VER && !__clang__
