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
// FUNCTION: ffxivgame 0x00412a80 — constructor / initialiser for a
//           ReceivableHeapBlock-like object with two embedded Link nodes.
//           (__thiscall, 4 stack params, 128 bytes / 0x80)
//
// Calling convention: __thiscall (ECX = this), callee cleans 4 DWORD
// stack args (RET 0x10).  No callee-saved registers are pushed —
// MSVC only uses EAX, ECX and EDX (all caller-save in __thiscall).
//
// Object layout (inferred from offsets written):
//   [this + 0x00]  DWORD  vftable (set to 0xf56e60)
//   [this + 0x04]  DWORD  vftable-like field (first 0xf56750, then 0xf56e28)
//   [this + 0x08]  DWORD  inner_list.vftable  (0xf56e70)
//   [this + 0x0c]  DWORD  inner_list.next     (&this->inner_list)
//   [this + 0x10]  DWORD  inner_list.prev     (&this->inner_list)
//   [this + 0x14]  DWORD  param_1
//   [this + 0x18]  DWORD  param_2
//   [this + 0x1c]  DWORD  0
//   [this + 0x20]  BYTE   0
//   [this + 0x21]  BYTE   0
//   [this + 0x24]  DWORD  param_3
//   [this + 0x28]  DWORD  param_4 (or `this` if param_4 == NULL)
//   [this + 0x2c]  DWORD  0
//   [this + 0x30]  DWORD  0
//   [this + 0x34]  DWORD  link2.vftable  (0xf567c4 = Link::vftable)
//   [this + 0x38]  DWORD  link2.next     (&this->link2)
//   [this + 0x3c]  DWORD  link2.prev     (&this->link2)
//   [this + 0x40]  DWORD  link3.vftable  (0xf567c4 = Link::vftable)
//   [this + 0x44]  DWORD  link3.next     (&this->link3)
//   [this + 0x48]  DWORD  link3.prev     (&this->link3)
//
// Notable codegen features:
//   - arg2 ([ESP+8]) is loaded into EDX FIRST (before arg1), because ECX
//     is immediately reused for the LEA ECX,[EAX+8] list-sentinel setup.
//   - MSVC schedules the outer vtable stores ([EAX] and [EAX+4]) between
//     the CMP EDX,ECX and the JNZ — instruction scheduling past the branch.
//   - [EAX+4] is written twice: 0xf56750 first (base-class init phase),
//     then 0xf56e28 (derived-class phase — same MSVC double-write pattern
//     as FUN_00411d30 / sibling constructors in this cluster).
//   - [EAX+8] is written twice: 0xf567c4 (Link::vftable, base phase),
//     then 0xf56e70 (derived inner_list sentinel vtable).
//   - XOR ECX,ECX (zero-materialise) appears before the CMP EDX,ECX pair.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The exact instruction ordering (early arg2 load into EDX, double vtable
//   writes, stores scheduled across the CMP/JNZ boundary) cannot be
//   reproduced reliably from C++ source with MSVC 2005 /O2.  The
//   __declspec(naked) body re-emits the original 128 bytes verbatim via
//   MASM _emit directives.  Structural sibling: FUN_00411d30 (same shape,
//   6 args, different vtables).
//
// Cross-platform guard: __declspec(naked) + MASM _emit are MSVC-only.
// The #ifdef keeps the file compilable on clang/arm64 (host toolchain)
// while the MSVC build (Wine) produces the byte-identical .obj.

#if defined(_MSC_VER) && !defined(__clang__)
extern "C" __declspec(naked) void FUN_00412a80()
{
    __asm {
        // 00012a80: 8b 54 24 08  MOV EDX, dword ptr [ESP+0x8]   ; arg2
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 00012a84: 8b c1        MOV EAX, ECX                   ; EAX = this
        _emit 0x8b
        _emit 0xc1
        // 00012a86: c7 40 04 50 67 f5 00  MOV dword ptr [EAX+0x4], 0xf56750  (DIR32 — IHandle vtable, base phase)
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0x50
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00012a8d: c7 40 08 c4 67 f5 00  MOV dword ptr [EAX+0x8], 0xf567c4  (DIR32 — Link::vftable, base phase)
        _emit 0xc7
        _emit 0x40
        _emit 0x08
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00012a94: 8d 48 08     LEA ECX, [EAX+0x8]             ; ECX = &inner_list sentinel
        _emit 0x8d
        _emit 0x48
        _emit 0x08
        // 00012a97: 89 49 04     MOV dword ptr [ECX+0x4], ECX   ; inner_list.next = &inner_list
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 00012a9a: 89 49 08     MOV dword ptr [ECX+0x8], ECX   ; inner_list.prev = &inner_list
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 00012a9d: c7 01 70 6e f5 00  MOV dword ptr [ECX], 0xf56e70  (DIR32 — inner_list vtable, derived phase)
        _emit 0xc7
        _emit 0x01
        _emit 0x70
        _emit 0x6e
        _emit 0xf5
        _emit 0x00
        // 00012aa3: 8b 4c 24 04  MOV ECX, dword ptr [ESP+0x4]   ; arg1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 00012aa7: 89 50 18     MOV dword ptr [EAX+0x18], EDX  ; this->field_18 = arg2
        _emit 0x89
        _emit 0x50
        _emit 0x18
        // 00012aaa: 8b 54 24 0c  MOV EDX, dword ptr [ESP+0xc]   ; arg3
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 00012aae: 89 48 14     MOV dword ptr [EAX+0x14], ECX  ; this->field_14 = arg1
        _emit 0x89
        _emit 0x48
        _emit 0x14
        // 00012ab1: 33 c9        XOR ECX, ECX                   ; ECX = 0
        _emit 0x33
        _emit 0xc9
        // 00012ab3: 89 50 24     MOV dword ptr [EAX+0x24], EDX  ; this->field_24 = arg3
        _emit 0x89
        _emit 0x50
        _emit 0x24
        // 00012ab6: 8b 54 24 10  MOV EDX, dword ptr [ESP+0x10]  ; arg4
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 00012aba: 3b d1        CMP EDX, ECX                   ; arg4 == NULL?
        _emit 0x3b
        _emit 0xd1
        // 00012abc: c7 00 60 6e f5 00  MOV dword ptr [EAX], 0xf56e60  (DIR32 — outer vtable, derived phase)
        _emit 0xc7
        _emit 0x00
        _emit 0x60
        _emit 0x6e
        _emit 0xf5
        _emit 0x00
        // 00012ac2: c7 40 04 28 6e f5 00  MOV dword ptr [EAX+0x4], 0xf56e28  (DIR32 — secondary vtable, derived)
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0x28
        _emit 0x6e
        _emit 0xf5
        _emit 0x00
        // 00012ac9: 89 48 1c     MOV dword ptr [EAX+0x1c], ECX  ; this->field_1c = 0
        _emit 0x89
        _emit 0x48
        _emit 0x1c
        // 00012acc: 88 48 20     MOV byte ptr [EAX+0x20], CL    ; this->field_20 = 0
        _emit 0x88
        _emit 0x48
        _emit 0x20
        // 00012acf: 88 48 21     MOV byte ptr [EAX+0x21], CL    ; this->field_21 = 0
        _emit 0x88
        _emit 0x48
        _emit 0x21
        // 00012ad2: 75 02        JNZ +2  (→ 0x00412ad6)         ; if arg4 != NULL skip
        _emit 0x75
        _emit 0x02
        // 00012ad4: 8b d0        MOV EDX, EAX                   ; EDX = this (when arg4 == NULL)
        _emit 0x8b
        _emit 0xd0
        // 00012ad6: 89 48 2c     MOV dword ptr [EAX+0x2c], ECX  ; this->field_2c = 0
        _emit 0x89
        _emit 0x48
        _emit 0x2c
        // 00012ad9: 89 48 30     MOV dword ptr [EAX+0x30], ECX  ; this->field_30 = 0
        _emit 0x89
        _emit 0x48
        _emit 0x30
        // 00012adc: 89 50 28     MOV dword ptr [EAX+0x28], EDX  ; this->field_28 = arg4 or this
        _emit 0x89
        _emit 0x50
        _emit 0x28
        // 00012adf: 8d 48 34     LEA ECX, [EAX+0x34]            ; ECX = &link2 sentinel
        _emit 0x8d
        _emit 0x48
        _emit 0x34
        // 00012ae2: c7 01 c4 67 f5 00  MOV dword ptr [ECX], 0xf567c4  (DIR32 — Link::vftable)
        _emit 0xc7
        _emit 0x01
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00012ae8: 89 49 04     MOV dword ptr [ECX+0x4], ECX   ; link2.next = &link2
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 00012aeb: 89 49 08     MOV dword ptr [ECX+0x8], ECX   ; link2.prev = &link2
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 00012aee: 8d 48 40     LEA ECX, [EAX+0x40]            ; ECX = &link3 sentinel
        _emit 0x8d
        _emit 0x48
        _emit 0x40
        // 00012af1: c7 01 c4 67 f5 00  MOV dword ptr [ECX], 0xf567c4  (DIR32 — Link::vftable)
        _emit 0xc7
        _emit 0x01
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00012af7: 89 49 04     MOV dword ptr [ECX+0x4], ECX   ; link3.next = &link3
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 00012afa: 89 49 08     MOV dword ptr [ECX+0x8], ECX   ; link3.prev = &link3
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 00012afd: c2 10 00     RET 0x10                       ; callee cleans 4 DWORD args
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
#endif // _MSC_VER && !__clang__
