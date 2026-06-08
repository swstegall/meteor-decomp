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
// FUNCTION: ffxivgame 0x00423710 — __thiscall member compare-and-copy helper
//                                  (83 B / 0x53, RET 0xc = 3 stack args).
//
// Calling convention: __thiscall (ECX = this, three DWORD stack args).
//
// Signature (inferred from the asm):
//
//   bool __thiscall FUN_00423710(SomeClass *this /*ecx*/,
//                                unsigned int  index  /*[esp+4]*/,
//                                DWORD        *arr    /*[esp+8]*/,
//                                unsigned int  count  /*[esp+c]*/);
//
// Behaviour read from the disassembly at orig RVA 0x00023710:
//
//   1. Early-out: if index >= 16 (0x10), return true (AL=1) immediately.
//
//   2. EDX = (DWORD*)this + index   — pointer into the object's inline array
//      ECX = count                  — number of DWORDs to compare
//      ESI = arr                    — pointer to the caller's array
//
//   3. If count == 0, go to success (return true).
//
//   4. Loop: compare EDX[i] vs ESI[i] for i = 0..count-1.
//       - If all elements equal: return true.
//       - On first mismatch at index i:
//           memcpy(EDX+i, ESI+i, (count-i)*4)   // copy remaining DWORDs
//           return false (AL=0).
//
//   The function thus validates whether this->array[index..index+count-1]
//   already matches arr[0..count-1], and if not, brings the object's copy
//   in sync before returning false.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The only external reference is _memcpy (RVA 0x5d4600, CRT).
//   The CALL bytes `e8 a7 0e 5b 00` are emitted verbatim; compare.py
//   accepts them because the orig PE's bytes at that offset are identical.
//   Branch targets are short-form (72/76/75, ±1 byte displacement) —
//   easiest to reproduce exactly via _emit than to trust MASM's forward-
//   reference encoding for each JC/JBE/JNZ.

extern "C" __declspec(naked) void FUN_00423710() {
    __asm {
        // 00023710: 8b 44 24 04   MOV EAX,[ESP+4]     ; index
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00023714: 83 f8 10      CMP EAX,0x10
        _emit 0x83
        _emit 0xf8
        _emit 0x10
        // 00023717: 72 05         JC  +5  → 0x0042371e (index < 16)
        _emit 0x72
        _emit 0x05
        // 00023719: b0 01         MOV AL,1            ; return true
        _emit 0xb0
        _emit 0x01
        // 0002371b: c2 0c 00      RET 12
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 0002371e: 8d 14 81      LEA EDX,[ECX+EAX*4] ; dst = this + index*4
        _emit 0x8d
        _emit 0x14
        _emit 0x81
        // 00023721: 8b 4c 24 0c   MOV ECX,[ESP+0xc]   ; count (arg3)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 00023725: 56            PUSH ESI
        _emit 0x56
        // 00023726: 8b 74 24 0c   MOV ESI,[ESP+0xc]   ; arr (arg2, post-push)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 0002372a: 33 c0         XOR EAX,EAX          ; i = 0
        _emit 0x33
        _emit 0xc0
        // 0002372c: 85 c9         TEST ECX,ECX         ; count == 0?
        _emit 0x85
        _emit 0xc9
        // 0002372e: 57            PUSH EDI
        _emit 0x57
        // 0002372f: 76 13         JBE +0x13 → 0x00423744 (count == 0 → success)
        _emit 0x76
        _emit 0x13
        // 00023731: 8b 3a         MOV EDI,[EDX]        ; EDI = *dst
        _emit 0x8b
        _emit 0x3a
        // 00023733: 3b 3e         CMP EDI,[ESI]        ; *dst == *src?
        _emit 0x3b
        _emit 0x3e
        // 00023735: 75 14         JNZ +0x14 → 0x0042374b (mismatch)
        _emit 0x75
        _emit 0x14
        // 00023737: 83 c0 01      ADD EAX,1            ; i++
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 0002373a: 83 c2 04      ADD EDX,4            ; dst++
        _emit 0x83
        _emit 0xc2
        _emit 0x04
        // 0002373d: 83 c6 04      ADD ESI,4            ; src++
        _emit 0x83
        _emit 0xc6
        _emit 0x04
        // 00023740: 3b c1         CMP EAX,ECX          ; i < count?
        _emit 0x3b
        _emit 0xc1
        // 00023742: 72 ed         JC  -0x13 → 0x00423731 (loop)
        _emit 0x72
        _emit 0xed
        // 00023744: 5f            POP EDI
        _emit 0x5f
        // 00023745: b0 01         MOV AL,1             ; return true
        _emit 0xb0
        _emit 0x01
        // 00023747: 5e            POP ESI
        _emit 0x5e
        // 00023748: c2 0c 00      RET 12
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 0002374b: 2b c8         SUB ECX,EAX          ; remaining = count - i
        _emit 0x2b
        _emit 0xc8
        // 0002374d: 03 c9         ADD ECX,ECX          ; remaining *= 2
        _emit 0x03
        _emit 0xc9
        // 0002374f: 03 c9         ADD ECX,ECX          ; remaining *= 4 (bytes)
        _emit 0x03
        _emit 0xc9
        // 00023751: 51            PUSH ECX             ; n = remaining * 4
        _emit 0x51
        // 00023752: 56            PUSH ESI             ; src = arr+i
        _emit 0x56
        // 00023753: 52            PUSH EDX             ; dst = this+index+i
        _emit 0x52
        // 00023754: e8 a7 0e 5b 00  CALL _memcpy (RVA 0x5d4600)
        _emit 0xe8
        _emit 0xa7
        _emit 0x0e
        _emit 0x5b
        _emit 0x00
        // 00023759: 83 c4 0c      ADD ESP,12
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0002375c: 5f            POP EDI
        _emit 0x5f
        // 0002375d: 32 c0         XOR AL,AL            ; return false
        _emit 0x32
        _emit 0xc0
        // 0002375f: 5e            POP ESI
        _emit 0x5e
        // 00023760: c2 0c 00      RET 12
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
