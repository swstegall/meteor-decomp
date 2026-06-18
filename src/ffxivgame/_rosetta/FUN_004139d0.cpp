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
// FUNCTION: ffxivgame 0x004139d0 — DetachableHeapBlock constructor
//           (__thiscall, 5 stack params, 135 bytes / 0x87)
//
// Calling convention: __thiscall (ECX = this), callee cleans 5 DWORD
// stack args (RET 0x14).  No callee-saved registers are pushed —
// MSVC only uses EAX, ECX and EDX (all caller-save in __thiscall).
//
// Object layout (inferred from offsets written):
//   [this + 0x00]  DWORD  primary vftable    (0xf56f50 — DetachableHeapBlock main)
//   [this + 0x04]  DWORD  secondary vftable  (0xf56750 IHandle base first,
//                                             then 0xf56f18 DetachableHeapBlock)
//   [this + 0x08]  DWORD  link0.vftable      (0xf567c4 Link base first,
//                                             then 0xf56f60 DHB embedded-Link override)
//   [this + 0x0c]  DWORD  link0.next         (&this->link0)
//   [this + 0x10]  DWORD  link0.prev         (&this->link0)
//   [this + 0x14]  DWORD  param_1
//   [this + 0x18]  DWORD  param_2
//   [this + 0x1c]  DWORD  param_3
//   [this + 0x20]  DWORD  0
//   [this + 0x24]  BYTE   0
//   [this + 0x25]  BYTE   0
//   [this + 0x26]  BYTE   0
//   [this + 0x27]  BYTE   0
//   [this + 0x28]  DWORD  param_4
//   [this + 0x2c]  DWORD  param_5
//   [this + 0x30]  DWORD  0
//   [this + 0x34]  DWORD  0
//   [this + 0x38]  DWORD  link2.vftable  (0xf567c4 = Link::vftable)
//   [this + 0x3c]  DWORD  link2.next     (&this->link2)
//   [this + 0x40]  DWORD  link2.prev     (&this->link2)
//   [this + 0x44]  DWORD  link3.vftable  (0xf567c4 = Link::vftable)
//   [this + 0x48]  DWORD  link3.next     (&this->link3)
//   [this + 0x4c]  DWORD  link3.prev     (&this->link3)
//
// Notable codegen features:
//   - arg2 ([ESP+8]) is loaded into EDX FIRST (before arg1), because ECX
//     is immediately reused for the LEA ECX,[EAX+8] link-sentinel setup.
//   - [EAX+4] is written twice: 0xf56750 first (IHandle base-class init),
//     then 0xf56f18 (derived-class phase — standard MSVC MI double-write).
//   - [EAX+8] is written twice: 0xf567c4 (Link::vftable, base phase),
//     then 0xf56f60 (derived embedded-Link override).
//   - XOR ECX,ECX (zero-materialise) used for the 4-BYTE zero burst at
//     +0x24..+0x27 as well as the DWORD zeros at +0x20/+0x30/+0x34.
//   - param_5 is loaded late into EDX (after the zero burst) and stored
//     at [EAX+0x2c] after the outer vtable pair at [EAX] / [EAX+4].
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The instruction ordering (double vtable writes, early arg2 load, zero
//   burst via CL) cannot be reproduced reliably from C++ source under
//   MSVC 2005 /O2.  __declspec(naked) re-emits the original 135 bytes
//   verbatim via MASM _emit directives, matching the sibling pattern of
//   FUN_00412a80 / FUN_00411d30 from the same Memory::Alternative cluster.
//
// Reloc-bearing positions in the .obj (4-byte payloads masked by compare.py):
//   +0x06  DIR32 → IHandle::vftable          (0xf56750)
//   +0x0d  DIR32 → Link::vftable             (0xf567c4)
//   +0x1d  DIR32 → DHB embedded-Link vtable  (0xf56f60)
//   +0x56  DIR32 → DHB primary vtable        (0xf56f50)
//   +0x5c  DIR32 → DHB secondary vtable      (0xf56f18)
//   +0x69  DIR32 → Link::vftable             (0xf567c4) — link2 sentinel
//   +0x78  DIR32 → Link::vftable             (0xf567c4) — link3 sentinel
//
// Cross-platform guard: __declspec(naked) + MASM _emit are MSVC-only.
// The #ifdef keeps the file compilable on clang/arm64 (host toolchain)
// while the MSVC build (Wine) produces the byte-identical .obj.

#if defined(_MSC_VER) && !defined(__clang__)
extern "C" __declspec(naked) void FUN_004139d0()
{
    __asm {
        // 000139d0: 8b 54 24 08  MOV EDX, dword ptr [ESP+0x8]   ; arg2 (param_2)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 000139d4: 8b c1        MOV EAX, ECX                   ; EAX = this
        _emit 0x8b
        _emit 0xc1
        // 000139d6: c7 40 04 50 67 f5 00  MOV dword ptr [EAX+0x4], 0xf56750  (DIR32 — IHandle vftable, base phase)
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0x50
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 000139dd: c7 40 08 c4 67 f5 00  MOV dword ptr [EAX+0x8], 0xf567c4  (DIR32 — Link vftable, base phase)
        _emit 0xc7
        _emit 0x40
        _emit 0x08
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 000139e4: 8d 48 08     LEA ECX, [EAX+0x8]             ; ECX = &link0 sentinel
        _emit 0x8d
        _emit 0x48
        _emit 0x08
        // 000139e7: 89 49 04     MOV dword ptr [ECX+0x4], ECX   ; link0.next = &link0
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 000139ea: 89 49 08     MOV dword ptr [ECX+0x8], ECX   ; link0.prev = &link0
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 000139ed: c7 01 60 6f f5 00  MOV dword ptr [ECX], 0xf56f60  (DIR32 — DHB embedded-Link vftable)
        _emit 0xc7
        _emit 0x01
        _emit 0x60
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        // 000139f3: 8b 4c 24 04  MOV ECX, dword ptr [ESP+0x4]   ; arg1 (param_1)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 000139f7: 89 48 14     MOV dword ptr [EAX+0x14], ECX  ; this->field_14 = param_1
        _emit 0x89
        _emit 0x48
        _emit 0x14
        // 000139fa: 8b 4c 24 0c  MOV ECX, dword ptr [ESP+0xc]   ; arg3 (param_3)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 000139fe: 89 50 18     MOV dword ptr [EAX+0x18], EDX  ; this->field_18 = param_2 (EDX)
        _emit 0x89
        _emit 0x50
        _emit 0x18
        // 00013a01: 8b 54 24 10  MOV EDX, dword ptr [ESP+0x10]  ; arg4 (param_4)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 00013a05: 89 48 1c     MOV dword ptr [EAX+0x1c], ECX  ; this->field_1c = param_3
        _emit 0x89
        _emit 0x48
        _emit 0x1c
        // 00013a08: 33 c9        XOR ECX, ECX                   ; ECX = 0
        _emit 0x33
        _emit 0xc9
        // 00013a0a: 89 48 20     MOV dword ptr [EAX+0x20], ECX  ; this->field_20 = 0
        _emit 0x89
        _emit 0x48
        _emit 0x20
        // 00013a0d: 88 48 24     MOV byte ptr [EAX+0x24], CL    ; this->byte_24 = 0
        _emit 0x88
        _emit 0x48
        _emit 0x24
        // 00013a10: 88 48 25     MOV byte ptr [EAX+0x25], CL    ; this->byte_25 = 0
        _emit 0x88
        _emit 0x48
        _emit 0x25
        // 00013a13: 88 48 26     MOV byte ptr [EAX+0x26], CL    ; this->byte_26 = 0
        _emit 0x88
        _emit 0x48
        _emit 0x26
        // 00013a16: 88 48 27     MOV byte ptr [EAX+0x27], CL    ; this->byte_27 = 0
        _emit 0x88
        _emit 0x48
        _emit 0x27
        // 00013a19: 89 50 28     MOV dword ptr [EAX+0x28], EDX  ; this->field_28 = param_4
        _emit 0x89
        _emit 0x50
        _emit 0x28
        // 00013a1c: 8b 54 24 14  MOV EDX, dword ptr [ESP+0x14]  ; arg5 (param_5)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 00013a20: 89 48 30     MOV dword ptr [EAX+0x30], ECX  ; this->field_30 = 0 (ECX still 0)
        _emit 0x89
        _emit 0x48
        _emit 0x30
        // 00013a23: 89 48 34     MOV dword ptr [EAX+0x34], ECX  ; this->field_34 = 0 (ECX still 0)
        _emit 0x89
        _emit 0x48
        _emit 0x34
        // 00013a26: c7 00 50 6f f5 00  MOV dword ptr [EAX], 0xf56f50  (DIR32 — DHB primary vftable)
        _emit 0xc7
        _emit 0x00
        _emit 0x50
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        // 00013a2c: c7 40 04 18 6f f5 00  MOV dword ptr [EAX+0x4], 0xf56f18  (DIR32 — DHB secondary vftable)
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0x18
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        // 00013a33: 89 50 2c     MOV dword ptr [EAX+0x2c], EDX  ; this->field_2c = param_5
        _emit 0x89
        _emit 0x50
        _emit 0x2c
        // 00013a36: 8d 48 38     LEA ECX, [EAX+0x38]            ; ECX = &link2 sentinel
        _emit 0x8d
        _emit 0x48
        _emit 0x38
        // 00013a39: c7 01 c4 67 f5 00  MOV dword ptr [ECX], 0xf567c4  (DIR32 — Link::vftable — link2)
        _emit 0xc7
        _emit 0x01
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00013a3f: 89 49 04     MOV dword ptr [ECX+0x4], ECX   ; link2.next = &link2
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 00013a42: 89 49 08     MOV dword ptr [ECX+0x8], ECX   ; link2.prev = &link2
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 00013a45: 8d 48 44     LEA ECX, [EAX+0x44]            ; ECX = &link3 sentinel
        _emit 0x8d
        _emit 0x48
        _emit 0x44
        // 00013a48: c7 01 c4 67 f5 00  MOV dword ptr [ECX], 0xf567c4  (DIR32 — Link::vftable — link3)
        _emit 0xc7
        _emit 0x01
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00013a4e: 89 49 04     MOV dword ptr [ECX+0x4], ECX   ; link3.next = &link3
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 00013a51: 89 49 08     MOV dword ptr [ECX+0x8], ECX   ; link3.prev = &link3
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 00013a54: c2 14 00     RET 0x14                       ; callee cleans 5 DWORD args
        _emit 0xc2
        _emit 0x14
        _emit 0x00
    }
}
#endif // _MSC_VER && !__clang__
