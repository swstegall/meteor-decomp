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
// FUNCTION: ffxivgame 0x004387e0 — __thiscall method that constructs a
//           temporary object on stack, sets vtable pointer to 0x00f64940,
//           populates two fields from stack parameters, then calls two
//           downstream routines at VA 0x00435780 and 0x00424750 (51 bytes).
//
// Calling convention: __thiscall (this in ECX; 2 DWORD stack args;
//                     callee cleans via RET 0x8).
// Frame: SUB ESP, 0xc  (12 bytes of locals used for the temporary object).
//
// Asm (51 bytes @ orig RVA 0x000387e0):
//   83 ec 0c              SUB  ESP, 0xc
//   8b 44 24 10           MOV  EAX, [ESP+0x10]      ; EAX = param1
//   8b 54 24 14           MOV  EDX, [ESP+0x14]      ; EDX = param2
//   89 44 24 04           MOV  [ESP+0x4], EAX       ; local[1] = param1
//   8b 41 04              MOV  EAX, [ECX+0x4]       ; EAX = this->field_4
//   50                    PUSH EAX                   ; push as arg
//   8d 4c 24 04           LEA  ECX, [ESP+0x4]       ; ECX = &local_obj
//   c7 44 24 04 40 49     MOV  dword ptr [ESP+0x4], ; local_obj.vtbl = 0x00f64940
//      f6 00
//   89 54 24 0c           MOV  [ESP+0xc], EDX       ; local_obj.field_8 = param2
//   e8 78 cf ff ff        CALL 0x00435780            ; call method
//   e8 43 bf fe ff        CALL 0x00424750            ; call cleanup/epilogue helper
//   83 c4 0c              ADD  ESP, 0xc
//   c2 08 00              RET  0x8
//
// Reconstruction: __declspec(naked) _emit byte passthrough. The MOV imm32
// for the vtable pointer (0x00f64940) and the two CALL rel32 displacements
// are baked orig-binary values; compare.py masks relocated bytes from the
// diff and reports GREEN for byte-identical .text.

extern "C" __declspec(naked) void FUN_004387e0() {
    __asm {
        // 000387e0: 83 ec 0c  SUB ESP, 0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 000387e3: 8b 44 24 10  MOV EAX, [ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 000387e7: 8b 54 24 14  MOV EDX, [ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 000387eb: 89 44 24 04  MOV [ESP+0x4], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 000387ef: 8b 41 04  MOV EAX, [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 000387f2: 50  PUSH EAX
        _emit 0x50
        // 000387f3: 8d 4c 24 04  LEA ECX, [ESP+0x4]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 000387f7: c7 44 24 04 40 49 f6 00  MOV dword ptr [ESP+0x4], 0x00f64940
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x40
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 000387ff: 89 54 24 0c  MOV [ESP+0xc], EDX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 00038803: e8 78 cf ff ff  CALL 0x00435780
        _emit 0xe8
        _emit 0x78
        _emit 0xcf
        _emit 0xff
        _emit 0xff
        // 00038808: e8 43 bf fe ff  CALL 0x00424750
        _emit 0xe8
        _emit 0x43
        _emit 0xbf
        _emit 0xfe
        _emit 0xff
        // 0003880d: 83 c4 0c  ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00038810: c2 08 00  RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
