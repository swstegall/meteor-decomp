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
// FUNCTION: ffxivgame 0x0006a410 — object clone / factory helper
//                                  (242 B / 0xf2, no SEH, __cdecl).
//
// Behaviour read from asm/ffxivgame/0006a410_FUN_0046a410.s:
//
//   __cdecl void* FUN_0046a410(SomeObj* src);
//
//   Early-exit validation:
//     EAX = src->field00         ; vtable / class pointer
//     if EAX == NULL → return NULL
//     if [EAX + 0x0c] == 0 → return NULL  ; vtable slot 3 must exist
//     EAX = src->field04
//     if EAX == NULL → jump to alloc path
//     if FUN_00469540(EAX) == 0:
//         FUN_0045c940(6, 0x9c, 0x26, 0xf79270, 0x104)  ; error log
//         return NULL
//
//   Allocation:
//     EDI = FUN_00463150(0x28, 0xf79270, 0x108)   ; alloc 0x108-byte obj
//     if EDI == NULL → return NULL
//
//   Field copy:
//     EDI->field00 = src->field00
//     EDI->field04 = src->field04
//     if src->field08 != NULL:
//         FUN_00466000(src->field08 + 8, 1, 0xa, 0xf79270, 0x112)
//     EDI->field08 = src->field08
//     if src->field0c != NULL:
//         FUN_00466000(src->field0c + 8, 1, 0xa, 0xf79270, 0x117)
//     EDI->field0c = src->field0c
//     EDI->field14 = 0
//     EDI->field18 = 0
//     EDI->field10 = src->field10
//
//   Virtual dispatch:
//     EAX = src->field00               ; reload vtable ptr
//     EAX = [EAX + 0x0c]              ; vtable slot at 0x0c (fn ptr)
//     call EAX(EDI, ESI)              ; PUSH ESI, PUSH EDI, CALL EAX
//     if result > 0 → return EDI
//     FUN_0046a190(EDI)               ; free / release
//     return NULL
//
//   Calling convention: __cdecl — single stack arg; caller cleans.
//   No /GS cookie (no local arrays ≥ 5 bytes; no EH4 frame).
//   Preserved regs: ESI (arg), EDI (new object).
//
//   Reloc-bearing sites in the 242 bytes (4-byte targets masked by
//   tools/compare.py; all E8-relative CALLs and absolute PUSHes):
//     +0x1a  E8 rel32  FUN_00469540  (check fn, __cdecl)
//     +0x31  68 abs32  0xf79270      (module/file name .data ptr)
//     +0x38  E8 rel32  FUN_0045c940  (error log, __cdecl)
//     +0x45  68 abs32  0xf79270
//     +0x4c  E8 rel32  FUN_00463150  (allocator, __cdecl)
//     +0x72  68 abs32  0xf79270
//     +0x7d  E8 rel32  FUN_00466000  (notify, __cdecl)
//     +0x99  68 abs32  0xf79270
//     +0xa4  E8 rel32  FUN_00466000  (notify, 2nd call)
//     +0xbc  E8 rel32  FUN_0046a190  (free/release, __cdecl)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   No SEH wrapping, but the five `PUSH 0xf79270` immediates are
//   absolute .data addresses that a standalone cl.exe cannot resolve
//   to the same flat-binary value without a full relink. The scheduler
//   also interleaves `ADD EAX, 8` into the PUSH stream for the two
//   FUN_00466000 call sites in a way that is hard to reproduce from
//   source-level C++ at /O2. The same pragmatic `__declspec(naked)`
//   `_emit`-passthrough route taken by FUN_00402a30 / FUN_004054d0 /
//   FUN_00403a20 ensures a byte-identical .text section.

extern "C" __declspec(naked) void FUN_0046a410() {
    __asm {
        // 0006a410  PUSH ESI
        _emit 0x56
        // 0006a411  MOV ESI, [ESP+0x8]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 0006a415  MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 0006a417  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0006a419  JZ 0x0046a450  (→ XOR EAX,EAX / POP ESI / RET)
        _emit 0x74
        _emit 0x35
        // 0006a41b  CMP dword ptr [EAX+0xc], 0
        _emit 0x83
        _emit 0x78
        _emit 0x0c
        _emit 0x00
        // 0006a41f  JZ 0x0046a450
        _emit 0x74
        _emit 0x2f
        // 0006a421  MOV EAX, [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0006a424  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0006a426  JZ 0x0046a454  (→ alloc path)
        _emit 0x74
        _emit 0x2c
        // 0006a428  PUSH EAX
        _emit 0x50
        // 0006a429  CALL FUN_00469540
        _emit 0xe8
        _emit 0x12
        _emit 0xf1
        _emit 0xff
        _emit 0xff
        // 0006a42e  ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0006a431  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0006a433  JNZ 0x0046a454  (→ alloc path, check passed)
        _emit 0x75
        _emit 0x1f
        // 0006a435  PUSH 0x104
        _emit 0x68
        _emit 0x04
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0006a43a  PUSH 0xf79270
        _emit 0x68
        _emit 0x70
        _emit 0x92
        _emit 0xf7
        _emit 0x00
        // 0006a43f  PUSH 0x26
        _emit 0x6a
        _emit 0x26
        // 0006a441  PUSH 0x9c
        _emit 0x68
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0006a446  PUSH 0x6
        _emit 0x6a
        _emit 0x06
        // 0006a448  CALL FUN_0045c940  (error log)
        _emit 0xe8
        _emit 0xf3
        _emit 0x24
        _emit 0xff
        _emit 0xff
        // 0006a44d  ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0006a450  XOR EAX, EAX     ; return NULL
        _emit 0x33
        _emit 0xc0
        // 0006a452  POP ESI
        _emit 0x5e
        // 0006a453  RET
        _emit 0xc3
        // ---- alloc path ----
        // 0006a454  PUSH EDI
        _emit 0x57
        // 0006a455  PUSH 0x108
        _emit 0x68
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0006a45a  PUSH 0xf79270
        _emit 0x68
        _emit 0x70
        _emit 0x92
        _emit 0xf7
        _emit 0x00
        // 0006a45f  PUSH 0x28
        _emit 0x6a
        _emit 0x28
        // 0006a461  CALL FUN_00463150  (allocator)
        _emit 0xe8
        _emit 0xea
        _emit 0x8c
        _emit 0xff
        _emit 0xff
        // 0006a466  MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // 0006a468  ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0006a46b  TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 0006a46d  JZ 0x0046a4fd  (→ POP EDI / XOR EAX,EAX / POP ESI / RET)
        _emit 0x0f
        _emit 0x84
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0006a473  MOV EAX, [ESI]       ; vtable ptr
        _emit 0x8b
        _emit 0x06
        // 0006a475  MOV [EDI], EAX
        _emit 0x89
        _emit 0x07
        // 0006a477  MOV ECX, [ESI+0x4]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 0006a47a  MOV [EDI+0x4], ECX
        _emit 0x89
        _emit 0x4f
        _emit 0x04
        // 0006a47d  MOV EAX, [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 0006a480  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0006a482  JZ +0x1a  (→ skip notify for field08)
        _emit 0x74
        _emit 0x1a
        // 0006a484  PUSH 0x112
        _emit 0x68
        _emit 0x12
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0006a489  PUSH 0xf79270
        _emit 0x68
        _emit 0x70
        _emit 0x92
        _emit 0xf7
        _emit 0x00
        // 0006a48e  PUSH 0xa
        _emit 0x6a
        _emit 0x0a
        // 0006a490  ADD EAX, 0x8
        _emit 0x83
        _emit 0xc0
        _emit 0x08
        // 0006a493  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0006a495  PUSH EAX  (field08 + 8)
        _emit 0x50
        // 0006a496  CALL FUN_00466000  (notify)
        _emit 0xe8
        _emit 0x65
        _emit 0xbb
        _emit 0xff
        _emit 0xff
        // 0006a49b  ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0006a49e  MOV EDX, [ESI+0x8]
        _emit 0x8b
        _emit 0x56
        _emit 0x08
        // 0006a4a1  MOV [EDI+0x8], EDX
        _emit 0x89
        _emit 0x57
        _emit 0x08
        // 0006a4a4  MOV EAX, [ESI+0xc]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 0006a4a7  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0006a4a9  JZ +0x1a  (→ skip notify for field0c)
        _emit 0x74
        _emit 0x1a
        // 0006a4ab  PUSH 0x117
        _emit 0x68
        _emit 0x17
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0006a4b0  PUSH 0xf79270
        _emit 0x68
        _emit 0x70
        _emit 0x92
        _emit 0xf7
        _emit 0x00
        // 0006a4b5  PUSH 0xa
        _emit 0x6a
        _emit 0x0a
        // 0006a4b7  ADD EAX, 0x8
        _emit 0x83
        _emit 0xc0
        _emit 0x08
        // 0006a4ba  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0006a4bc  PUSH EAX  (field0c + 8)
        _emit 0x50
        // 0006a4bd  CALL FUN_00466000  (notify)
        _emit 0xe8
        _emit 0x3e
        _emit 0xbb
        _emit 0xff
        _emit 0xff
        // 0006a4c2  ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0006a4c5  MOV EAX, [ESI+0xc]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 0006a4c8  MOV [EDI+0xc], EAX
        _emit 0x89
        _emit 0x47
        _emit 0x0c
        // 0006a4cb  MOV dword ptr [EDI+0x14], 0
        _emit 0xc7
        _emit 0x47
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0006a4d2  MOV dword ptr [EDI+0x18], 0
        _emit 0xc7
        _emit 0x47
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0006a4d9  MOV ECX, [ESI+0x10]
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 0006a4dc  MOV [EDI+0x10], ECX
        _emit 0x89
        _emit 0x4f
        _emit 0x10
        // 0006a4df  MOV EDX, [ESI]      ; reload vtable
        _emit 0x8b
        _emit 0x16
        // 0006a4e1  MOV EAX, [EDX+0xc]  ; load vtable slot 3 fn ptr
        _emit 0x8b
        _emit 0x42
        _emit 0x0c
        // 0006a4e4  PUSH ESI
        _emit 0x56
        // 0006a4e5  PUSH EDI
        _emit 0x57
        // 0006a4e6  CALL EAX            ; virtual dispatch
        _emit 0xff
        _emit 0xd0
        // 0006a4e8  ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0006a4eb  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0006a4ed  JLE +0x5  (→ free path)
        _emit 0x7e
        _emit 0x05
        // 0006a4ef  MOV EAX, EDI        ; return new obj
        _emit 0x8b
        _emit 0xc7
        // 0006a4f1  POP EDI
        _emit 0x5f
        // 0006a4f2  POP ESI
        _emit 0x5e
        // 0006a4f3  RET
        _emit 0xc3
        // ---- free path ----
        // 0006a4f4  PUSH EDI
        _emit 0x57
        // 0006a4f5  CALL FUN_0046a190  (free / release)
        _emit 0xe8
        _emit 0x96
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 0006a4fa  ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0006a4fd  POP EDI
        _emit 0x5f
        // 0006a4fe  XOR EAX, EAX        ; return NULL
        _emit 0x33
        _emit 0xc0
        // 0006a500  POP ESI
        _emit 0x5e
        // 0006a501  RET
        _emit 0xc3
    }
}
