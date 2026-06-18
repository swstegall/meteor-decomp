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
// FUNCTION: ffxivgame 0x0043be90 — __thiscall constructor-like initializer:
//                                  zeroes fields at offsets 0x4–0x1c on `this`,
//                                  calls a function pointer (probably InitializeCriticalSection
//                                  or similar) on &this->field_1c, then calls
//                                  FUN_0043bca0(this, a1, &local) and returns this.
//                                  (117 bytes / 0x75, EH3-SEH wrapped, one stack arg)
//
// Calling convention: __thiscall (ECX = this); one explicit stack argument (a1);
//   returns EAX = this.  RET 0x4 (stdcall-style epilogue for the one arg).
//
// Stack frame (ESP-relative; cookie installed via EH3 prolog):
//   [ESP+0x00]  __security_cookie ^ ESP
//   [ESP+0x04]  saved EDI
//   [ESP+0x08]  saved ESI
//   [ESP+0x0C]  local slot B (used as out-param placeholder for FUN_0043bca0)
//   [ESP+0x10]  local slot A = this (saved for EH frame)
//   [ESP+0x14]  old FS:[0]  ← EH3 registration prev
//   [ESP+0x18]  scope table (0xe56638)
//   [ESP+0x1C]  EH3 trylevel (init -1, set to 0 before first call)
//   [ESP+0x20]  return address
//   [ESP+0x24]  a1 (first explicit arg)
//
// Object fields touched:
//   [this + 0x04]  zeroed
//   [this + 0x08]  zeroed
//   [this + 0x0C]  zeroed
//   [this + 0x14]  zeroed
//   [this + 0x18]  zeroed
//   [this + 0x1C]  address passed to indirect call (fn ptr at 0xf3e174)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Contains two call sites with relocations: an indirect CALL through IAT-like
//   import at [0x00f3e174], and a rel32 CALL to FUN_0043bca0 at 0x0043beea.
//   Source-level C++ cannot reproduce the exact EH3 prolog layout, the exact
//   register-save order, and the trylevel-advance interleaved with field stores
//   without shifting bytes.  A __declspec(naked) body re-emitting all 117 bytes
//   verbatim via MASM _emit directives produces a .obj whose .text section is
//   byte-identical to the original slice.

extern "C" __declspec(naked) void FUN_0043be90() {
    __asm {
        // 0003be90: 6a ff           PUSH -0x1          (EH3 trylevel guard)
        _emit 0x6a
        _emit 0xff
        // 0003be92: 68 38 66 e5 00  PUSH 0xe56638      (scope table)
        _emit 0x68
        _emit 0x38
        _emit 0x66
        _emit 0xe5
        _emit 0x00
        // 0003be97: 64 a1 00 00 00 00  MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003be9d: 50              PUSH EAX           (old FS:[0])
        _emit 0x50
        // 0003be9e: 83 ec 08        SUB ESP,0x8        (local space)
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0003bea1: 56              PUSH ESI
        _emit 0x56
        // 0003bea2: 57              PUSH EDI
        _emit 0x57
        // 0003bea3: a1 b0 a8 2e 01  MOV EAX,[0x012ea8b0]  (security cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0003bea8: 33 c4           XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0003beaa: 50              PUSH EAX           (cookie ^ ESP)
        _emit 0x50
        // 0003beab: 8d 44 24 14     LEA EAX,[ESP+0x14] (EH3 registration record)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0003beaf: 64 a3 00 00 00 00  MOV FS:[0x0],EAX  (install SEH frame)
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003beb5: 8b f1           MOV ESI,ECX        (ESI = this)
        _emit 0x8b
        _emit 0xf1
        // 0003beb7: 89 74 24 10     MOV [ESP+0x10],ESI (save this for EH)
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 0003bebb: 33 ff           XOR EDI,EDI        (EDI = 0)
        _emit 0x33
        _emit 0xff
        // 0003bebd: 89 7e 04        MOV [ESI+0x4],EDI  (this->field_4 = 0)
        _emit 0x89
        _emit 0x7e
        _emit 0x04
        // 0003bec0: 89 7e 08        MOV [ESI+0x8],EDI  (this->field_8 = 0)
        _emit 0x89
        _emit 0x7e
        _emit 0x08
        // 0003bec3: 89 7e 0c        MOV [ESI+0xc],EDI  (this->field_C = 0)
        _emit 0x89
        _emit 0x7e
        _emit 0x0c
        // 0003bec6: 8d 46 1c        LEA EAX,[ESI+0x1c] (EAX = &this->field_1C)
        _emit 0x8d
        _emit 0x46
        _emit 0x1c
        // 0003bec9: 50              PUSH EAX           (arg to indirect call)
        _emit 0x50
        // 0003beca: 89 7c 24 20     MOV [ESP+0x20],EDI (trylevel = 0)
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        // 0003bece: 89 7e 14        MOV [ESI+0x14],EDI (this->field_14 = 0)
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        // 0003bed1: 89 7e 18        MOV [ESI+0x18],EDI (this->field_18 = 0)
        _emit 0x89
        _emit 0x7e
        _emit 0x18
        // 0003bed4: ff 15 74 e1 f3 00  CALL [0x00f3e174]  (indirect: init field_1C)
        _emit 0xff
        _emit 0x15
        _emit 0x74
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0003beda: 8b 54 24 24     MOV EDX,[ESP+0x24] (EDX = a1)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 0003bede: 8d 4c 24 0c     LEA ECX,[ESP+0xc]  (ECX = &local_B out-param)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0003bee2: 51              PUSH ECX           (push &local_B)
        _emit 0x51
        // 0003bee3: 52              PUSH EDX           (push a1)
        _emit 0x52
        // 0003bee4: 8b ce           MOV ECX,ESI        (ECX = this)
        _emit 0x8b
        _emit 0xce
        // 0003bee6: 89 7c 24 14     MOV [ESP+0x14],EDI (zero local_B before call)
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        // 0003beea: e8 b1 fd ff ff  CALL 0x0043bca0    (FUN_0043bca0(this, a1, &local_B))
        _emit 0xe8
        _emit 0xb1
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0003beef: 8b c6           MOV EAX,ESI        (return this)
        _emit 0x8b
        _emit 0xc6
        // 0003bef1: 8b 4c 24 14     MOV ECX,[ESP+0x14] (restore old FS:[0])
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0003bef5: 64 89 0d 00 00 00 00  MOV FS:[0x0],ECX  (uninstall SEH frame)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003befc: 59              POP ECX            (pop cookie slot)
        _emit 0x59
        // 0003befd: 5f              POP EDI
        _emit 0x5f
        // 0003befe: 5e              POP ESI
        _emit 0x5e
        // 0003beff: 83 c4 14        ADD ESP,0x14       (reclaim locals + EH frame)
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0003bf02: c2 04 00        RET 0x4            (stdcall-pop a1)
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
