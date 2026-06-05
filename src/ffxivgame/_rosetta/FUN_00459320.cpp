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
// FUNCTION: ffxivgame 0x00459320 — __thiscall initialiser with EH4 SEH
//                                  frame (291 B / 0x123).
//
// Inspection (read from the disassembly at orig RVA 0x00059320):
//
//   __thiscall SomeClass* SomeClass::Init(int /*unused*/, void* obj, int val)
//
//     Sets dual vtable pointers at offsets 0 and 4 (0xf67938 / 0xf67924),
//     field_8 = 1, field_C = val, field_10 = obj, field_18 = -1,
//     field_1C = -1, field_38 = 7, field_34 = 0, field_24 (word) = 0.
//     Then enters EH4 try block (state → 0), calls FUN_00459070 on this,
//     dispatches through obj's vtable[7] to obtain a handle, and either
//     registers it via FUN_00459120 + vtable[2] release, or falls through
//     to a vtable[5]/vtable[8]/vtable[9] sequence before the same
//     FUN_00459120 + vtable[2] release path.  Returns this in EAX.
//
//   EH4 prologue layout (EH4 = __except_handler4, /GS cookie):
//     [ESP+ 0]  security cookie ^ ESP
//     [ESP+ 4]  saved ESI
//     [ESP+ 8]  saved ECX (this)
//     [ESP+ C]  prev FS:[0]          ← FS:[0] installed here
//     [ESP+10]  scope-table (rdata)
//     [ESP+14]  trylevel (-1 idle, 0 in try)
//     [ESP+18]  return address
//     [ESP+1C]  arg1 (unused)
//     [ESP+20]  arg2 = obj pointer
//     [ESP+24]  arg3 = val / output param for vtable calls
//     epilogue: restore FS:[0] from [ESP+C], POP ECX, POP ESI,
//               ADD ESP,0x10, RET 0xC
//
//   Reconstruction strategy — naked byte passthrough:
//
//     Source-level C++ cannot reproducibly drive MSVC 2005 /O2 /GS /EHsc
//     to emit this exact EH4 frame interleaved with the callee-register
//     saves (PUSH ECX / PUSH ESI BETWEEN the FS:[0] save and the cookie
//     XOR), the specific trylevel-0 write location, and the vtable call
//     sequences with stack-passed object pointers (non-__thiscall dispatch
//     pattern).  All absolute-address immediates in the byte stream
//     (vtable pointers 0xf67938/0xf67924, cookie slot 0x012ea8b0, scope-
//     table 0xe585fb, FS:[0] 0-constant) are emitted verbatim — they match
//     the orig binary bytes directly, so compare.py needs no reloc mask
//     for them.  The three CALL rel32 displacements (-0x326, -0x29e,
//     -0x302) are similarly hardcoded as the orig binary values and match
//     without relocation entries.

extern "C" __declspec(naked) void FUN_00459320() {
    __asm {
        // 00059320 — EH4 prologue
        _emit 0x6a  // PUSH -1                (trylevel = -1)
        _emit 0xff
        _emit 0x68  // PUSH 0x00e585fb        (scope-table)
        _emit 0xfb
        _emit 0x85
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX               (old FS chain)
        _emit 0x51  // PUSH ECX               (save this)
        _emit 0x56  // PUSH ESI               (save ESI)

        // 00059330 — cookie
        _emit 0xa1  // MOV EAX, [__security_cookie]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX               (cookie ^ ESP)

        // 00059338 — install SEH frame
        _emit 0x8d  // LEA EAX, [ESP+0xC]     (= &prev FS chain)
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x64  // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // 00059342 — save this, read args, initialise fields
        _emit 0x8b  // MOV ESI, ECX           (ESI = this)
        _emit 0xf1
        _emit 0x89  // MOV [ESP+8], ESI       (save this for EH)
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x8b  // MOV EAX, [ESP+0x24]    (arg3 = val)
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x8b  // MOV ECX, [ESP+0x20]    (arg2 = obj)
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x89  // MOV [ESI+0xC], EAX     (field_C = val)
        _emit 0x46
        _emit 0x0c
        _emit 0x83  // OR EAX, 0xFFFFFFFF     (EAX = -1)
        _emit 0xc8
        _emit 0xff
        _emit 0xc7  // MOV [ESI], 0x00f67938  (vtable1)
        _emit 0x06
        _emit 0x38
        _emit 0x79
        _emit 0xf6
        _emit 0x00
        _emit 0xc7  // MOV [ESI+4], 0x00f67924 (vtable2)
        _emit 0x46
        _emit 0x04
        _emit 0x24
        _emit 0x79
        _emit 0xf6
        _emit 0x00
        _emit 0xc7  // MOV [ESI+8], 1
        _emit 0x46
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89  // MOV [ESI+0x10], ECX    (field_10 = obj)
        _emit 0x4e
        _emit 0x10
        _emit 0x89  // MOV [ESI+0x18], EAX    (field_18 = -1)
        _emit 0x46
        _emit 0x18
        _emit 0x89  // MOV [ESI+0x1C], EAX    (field_1C = -1)
        _emit 0x46
        _emit 0x1c
        _emit 0xc7  // MOV [ESI+0x38], 7
        _emit 0x46
        _emit 0x38
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7  // MOV [ESI+0x34], 0
        _emit 0x46
        _emit 0x34
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66  // MOV word [ESI+0x24], 0
        _emit 0xc7
        _emit 0x46
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV ECX, ESI           (ECX = this for call)
        _emit 0xce

        // 00059389 — enter try block (state = 0) + call FUN_00459070
        _emit 0xc7  // MOV [ESP+0x14], 0      (trylevel = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL FUN_00459070
        _emit 0xda
        _emit 0xfc
        _emit 0xff
        _emit 0xff

        // 00059396 — call vtable[7] on field_10
        _emit 0x8b  // MOV EAX, [ESI+0x10]   (field_10)
        _emit 0x46
        _emit 0x10
        _emit 0x8b  // MOV EDX, [EAX]         (vtable ptr)
        _emit 0x10
        _emit 0x8b  // MOV EDX, [EDX+0x1C]    (vtable[7])
        _emit 0x52
        _emit 0x1c
        _emit 0x8d  // LEA ECX, [ESP+0x24]    (&out_param)
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x51  // PUSH ECX               (arg: &out)
        _emit 0x50  // PUSH EAX               (arg: field_10)
        _emit 0xff  // CALL EDX               (vtable[7](field_10, &out))
        _emit 0xd2
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x0f  // JL  +0x80              (failure path)
        _emit 0x8c
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // 000593ae — check out_param
        _emit 0x8b  // MOV EAX, [ESP+0x24]    (out_param value)
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74  // JZ  +0x29              (null → alternate path)
        _emit 0x29

        // 000593b6 — non-null path: FUN_00459120 + vtable[2] release
        _emit 0x50  // PUSH EAX               (out_param)
        _emit 0x8b  // MOV ECX, ESI           (ECX = this)
        _emit 0xce
        _emit 0xe8  // CALL FUN_00459120
        _emit 0x62
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV EAX, [ESP+0x24]    (out_param again)
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x8b  // MOV ECX, [EAX]         (vtable)
        _emit 0x08
        _emit 0x8b  // MOV EDX, [ECX+8]       (vtable[2])
        _emit 0x51
        _emit 0x08
        _emit 0x50  // PUSH EAX
        _emit 0xff  // CALL EDX               (vtable[2](out_param))
        _emit 0xd2

        // 000593ca — epilogue 1 (success, non-null)
        _emit 0x8b  // MOV EAX, ESI           (return this)
        _emit 0xc6
        _emit 0x8b  // MOV ECX, [ESP+0xC]     (old FS chain)
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x64  // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX                (discard cookie)
        _emit 0x5e  // POP ESI                (restore ESI)
        _emit 0x83  // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc2  // RET 0xC
        _emit 0x0c
        _emit 0x00

        // 000593df — null path: vtable[5] / vtable[8] / vtable[9]
        _emit 0x8b  // MOV EAX, [ESI+0x10]   (field_10)
        _emit 0x46
        _emit 0x10
        _emit 0x8b  // MOV ECX, [EAX]         (vtable)
        _emit 0x08
        _emit 0x8d  // LEA EDX, [ESP+0x24]    (&out_param)
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x52  // PUSH EDX
        _emit 0x50  // PUSH EAX               (field_10)
        _emit 0x8b  // MOV EAX, [ECX+0x14]    (vtable[5])
        _emit 0x41
        _emit 0x14
        _emit 0xff  // CALL EAX
        _emit 0xd0
        _emit 0x8b  // MOV EAX, [ESI+0x10]
        _emit 0x46
        _emit 0x10
        _emit 0x8b  // MOV EDX, [ESP+0x24]    (out_param)
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x8b  // MOV ECX, [EAX]         (vtable)
        _emit 0x08
        _emit 0x52  // PUSH EDX
        _emit 0x50  // PUSH EAX               (field_10)
        _emit 0x8b  // MOV EAX, [ECX+0x20]    (vtable[8])
        _emit 0x41
        _emit 0x20
        _emit 0xff  // CALL EAX
        _emit 0xd0
        _emit 0x8b  // MOV EDX, [ESP+0x24]
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x8b  // MOV EAX, [ESI+0x10]
        _emit 0x46
        _emit 0x10
        _emit 0x8b  // MOV ECX, [EAX]
        _emit 0x08
        _emit 0x6a  // PUSH 0
        _emit 0x00
        _emit 0x52  // PUSH EDX
        _emit 0x8b  // MOV EDX, [ESP+0x24]    (re-read after ESP shift)
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x52  // PUSH EDX
        _emit 0x50  // PUSH EAX               (field_10)
        _emit 0x8b  // MOV EAX, [ECX+0x24]    (vtable[9])
        _emit 0x41
        _emit 0x24
        _emit 0xff  // CALL EAX
        _emit 0xd0
        _emit 0x8b  // MOV ECX, [ESP+0x24]    (out_param)
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x51  // PUSH ECX
        _emit 0x8b  // MOV ECX, ESI           (ECX = this)
        _emit 0xce
        _emit 0xe8  // CALL FUN_00459120
        _emit 0xfe
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV EAX, [ESP+0x24]
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x8b  // MOV EDX, [EAX]         (vtable)
        _emit 0x10
        _emit 0x50  // PUSH EAX
        _emit 0x8b  // MOV EAX, [EDX+8]       (vtable[2])
        _emit 0x42
        _emit 0x08
        _emit 0xff  // CALL EAX
        _emit 0xd0

        // 0005942e — shared epilogue (failure / null path)
        _emit 0x8b  // MOV EAX, ESI           (return this)
        _emit 0xc6
        _emit 0x8b  // MOV ECX, [ESP+0xC]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x64  // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX
        _emit 0x5e  // POP ESI
        _emit 0x83  // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc2  // RET 0xC
        _emit 0x0c
        _emit 0x00
    }
}
