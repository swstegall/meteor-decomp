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
// FUNCTION: ffxivgame 0x00438950 — thiscall shim: construct a 12-byte
//           stack object (vtable=0x00f64978, +param1, +param2) and
//           dispatch into FUN_00435a30 (46 B / 0x2E)
//
// Calling convention: __thiscall (ECX = this on entry; two additional
//   DWORD stack params; callee cleans 8 bytes via `ret 8`).
//
// Stack frame (after SUB ESP,0xc):
//   [ESP+0x00..0x0B]  12 bytes of local scratch (becomes the temp object)
//   [ESP+0x0C]        saved return address
//   [ESP+0x10]        param1  (stack arg 0)
//   [ESP+0x14]        param2  (stack arg 1)
//
// Sequence:
//   1. SUB ESP,0xc           ; allocate 12 bytes
//   2. MOV EAX,[ESP+0x10]    ; EAX = param1
//   3. MOV EDX,[ESP+0x14]    ; EDX = param2
//   4. MOV [ESP+0x04],EAX    ; temp[0x04] = param1
//   5. MOV EAX,[ECX+0x04]    ; EAX = this->field_0x4
//   6. PUSH EAX              ; push this->field_0x4 as arg to inner call
//      (ESP decreases by 4; temp object is now at new ESP+0x04)
//   7. LEA ECX,[ESP+0x04]    ; ECX = &temp (vtable slot)
//   8. MOV [ESP+0x04],0xf64978 ; temp.vtbl = 0x00f64978
//   9. MOV [ESP+0x0C],EDX    ; temp[0x08] = param2
//  10. CALL FUN_00435a30     ; dispatch (thiscall: ECX=&temp,
//                            ;            [ESP]=this->field_0x4)
//  11. ADD ESP,0xc           ; tear down scratch + pushed arg
//  12. RET 0x8               ; return, callee-clean 8 bytes
//
// Reconstruction: __declspec(naked) _emit passthrough.  The imm32 at
// instruction 8 (0x00f64978 — vtable pointer) and the rel32 at
// instruction 10 are reloc-bearing in the original binary; compare.py
// masks those bytes so we emit the original wire values verbatim.

extern "C" __declspec(naked) void FUN_00438950() {
    __asm {
        // 00038950: 83 ec 0c          SUB ESP, 0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00038953: 8b 44 24 10       MOV EAX, dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00038957: 8b 54 24 14       MOV EDX, dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0003895b: 89 44 24 04       MOV dword ptr [ESP+0x04], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0003895f: 8b 41 04          MOV EAX, dword ptr [ECX+0x04]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00038962: 50                PUSH EAX
        _emit 0x50
        // 00038963: 8d 4c 24 04       LEA ECX, [ESP+0x04]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 00038967: c7 44 24 04 78 49 f6 00  MOV dword ptr [ESP+0x04], 0x00f64978
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x78
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 0003896f: 89 54 24 0c       MOV dword ptr [ESP+0x0c], EDX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 00038973: e8 b8 d0 ff ff    CALL FUN_00435a30
        _emit 0xe8
        _emit 0xb8
        _emit 0xd0
        _emit 0xff
        _emit 0xff
        // 00038978: 83 c4 0c          ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0003897b: c2 08 00          RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
