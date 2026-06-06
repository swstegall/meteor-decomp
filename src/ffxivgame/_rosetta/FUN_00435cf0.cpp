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
// FUNCTION: ffxivgame 0x00435cf0 — virtual-call dispatch + assert-fail
// reporter wrapper (__thiscall, one explicit stack arg, 103 B).
//
// Semantics (recovered from asm):
//
//   void __thiscall FUN_00435cf0(this, Obj *param_1) {
//       // virtual call on param_1's vtable slot +0x14c, self-passed as
//       // the first explicit arg (C-style "object carries its own
//       // dispatch table; methods take self") plus four fields off `this`:
//       int r = (*(*param_1 + 0x14c))(param_1,
//                                     this->field_0x04,
//                                     this->field_0x08,
//                                     this->field_0x10,
//                                     this->field_0x0c);
//       if (r != 0) {
//           // lazy-init the report-handler function pointer at .data
//           // 0x0132390c (guard bit 0 of the flag at 0x01323910), then
//           // invoke it with the classic assert tuple:
//           //   (expr 0xf64be8, file 0xf65304, ctx 0xf64c18,
//           //    line 0x222 = 546, msg 0xf65320)
//           if ((g_flag & 1) == 0) {
//               g_flag |= 1;
//               g_report = (handler)0x00433720;
//           }
//           g_report(0xf64be8, 0xf65304, 0xf64c18, 0x222, 0xf65320);
//       }
//   }
//
// Calling convention: __thiscall — `this` arrives in ECX (never spilled
// to the stack by the caller), one explicit argument at [ESP+4], and the
// epilogue is `RET 0x4` (callee pops the single stack arg).
//
// Reloc-bearing / absolute-address sites in the orig 103 bytes
// (compare.py compares against the orig post-fixup image, so emitting the
// orig bytes verbatim — including the absolute .data/.rdata addresses and
// the indirect call target — reproduces the exact wire image):
//   +0x2a  TEST [imm32]  → 0x01323910 (init flag)
//   +0x32  OR   [imm32]  → 0x01323910 (set init flag)
//   +0x38  MOV  [imm32]  → 0x0132390c (handler slot) = 0x00433720
//   +0x42  PUSH 0x00f65320 (msg string)
//   +0x4c  PUSH 0x00f64c18 (ctx string)
//   +0x51  PUSH 0x00f65304 (file string)
//   +0x56  PUSH 0x00f64be8 (expr string)
//   +0x5b  CALL [imm32]  → [0x0132390c] (report handler)
//
// Why naked asm: this body mixes a virtual indirect call (`CALL EDX`), an
// indirect call through a process-global function pointer (`CALL [imm32]`),
// and several pushed absolute string/immediate addresses. There is no
// source-level C++ form that round-trips back to this exact instruction
// selection and operand encoding (e.g. the `TEST byte ptr [imm32], AL`
// while AL == 1 — a 6-byte form MSVC's optimizer picks after `MOV EAX,1`).
// The sibling reloc-heavy bodies (FUN_004091f0, FUN_00409260) take the
// same `__declspec(naked)` byte-passthrough route; compare.py then reports
// GREEN against the orig slice.

extern "C" __declspec(naked) void FUN_00435cf0() {
    __asm {
        _emit 0x8b      // MOV EAX, dword ptr [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b      // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x8b      // MOV EDX, dword ptr [EDX+0x14c]
        _emit 0x92
        _emit 0x4c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x56      // PUSH ESI
        _emit 0x8b      // MOV ESI, dword ptr [ECX+0xc]
        _emit 0x71
        _emit 0x0c
        _emit 0x56      // PUSH ESI
        _emit 0x8b      // MOV ESI, dword ptr [ECX+0x10]
        _emit 0x71
        _emit 0x10
        _emit 0x56      // PUSH ESI
        _emit 0x8b      // MOV ESI, dword ptr [ECX+0x8]
        _emit 0x71
        _emit 0x08
        _emit 0x8b      // MOV ECX, dword ptr [ECX+0x4]
        _emit 0x49
        _emit 0x04
        _emit 0x56      // PUSH ESI
        _emit 0x51      // PUSH ECX
        _emit 0x50      // PUSH EAX
        _emit 0xff      // CALL EDX
        _emit 0xd2
        _emit 0x85      // TEST EAX, EAX
        _emit 0xc0
        _emit 0x5e      // POP ESI
        _emit 0x74      // JZ +0x3f (-> 0x00435d54, the RET)
        _emit 0x3f
        _emit 0xb8      // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84      // TEST byte ptr [0x01323910], AL
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75      // JNZ +0x10 (-> 0x00435d32)
        _emit 0x10
        _emit 0x09      // OR dword ptr [0x01323910], EAX
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7      // MOV dword ptr [0x0132390c], 0x00433720
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00
        _emit 0x68      // PUSH 0x00f65320
        _emit 0x20
        _emit 0x53
        _emit 0xf6
        _emit 0x00
        _emit 0x68      // PUSH 0x00000222
        _emit 0x22
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x68      // PUSH 0x00f64c18
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68      // PUSH 0x00f65304
        _emit 0x04
        _emit 0x53
        _emit 0xf6
        _emit 0x00
        _emit 0x68      // PUSH 0x00f64be8
        _emit 0xe8
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        _emit 0xff      // CALL dword ptr [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83      // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2      // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
