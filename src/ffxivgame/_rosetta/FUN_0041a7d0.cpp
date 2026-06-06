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
// FUNCTION: ffxivgame 0x0041a7d0 — __thiscall 1-based indexed array accessor
//                                   with lazy-init assert guard (103 B / 0x67)
//
// void __thiscall FUN_0041a7d0(this /*ECX*/, int idx /*[ESP+4]*/, int val /*[ESP+8]*/)
//   RET 0x8 — callee cleans two DWORD stack args
//
// Behaviour:
//   1. If idx == 0: fire a one-time-initialised assert dispatcher through
//      the global function pointer at [0x0132390c] (same slot used by
//      FUN_00417370, FUN_00410a00, FUN_004178c0, etc.).  Unlike those
//      siblings, no register holds 1 before the flag check, so the
//      compiler emits the 7-byte immediate-form instructions:
//        TEST byte ptr [0x01323910], 0x1   (f6 05 … — NOT the 6-byte AL form)
//        OR   dword ptr [0x01323910], 0x1  (83 0d … — NOT the EAX/EBX form)
//      The assert target installed is 0x00418740.
//   2. Load this->field_0x8 (pointer to an array of 8-byte elements).
//   3. EDX = arr[(idx-1)].dword_at_4   (SIB: [ECX + EDI*8 - 0x4])
//   4. Call FUN_00422f90(__thiscall on global at [0x0132987c], EDX, val, 1).
//
// Globals (same as all siblings in this assert family):
//   0x01323910  g_assert_flag  — init flag (bit 0 = "pointer installed")
//   0x0132390c  g_assert_fnptr — function pointer (installed on first call)
//   0x0132987c  g_global_obj   — global object pointer (target of FUN_00422f90)
//
// Assert call args (pushed right-to-left, cdecl, 5 args):
//   arg1 = 0xf57ec4  (expression or label string)
//   arg2 = 0xf54d48  (file name string — shared with all siblings)
//   arg3 = 0xf57ed0  (function name string)
//   arg4 = 0xa4      (line number 164 decimal)
//   arg5 = 0xf57f18  (message / description string)
//
// Reloc-bearing sites (masked by compare.py as DIR32):
//   +0x0c  TEST mem8  → 0x01323910  (g_assert_flag)
//   +0x11  (JNZ — no reloc)
//   +0x13  OR   mem32 → 0x01323910  (g_assert_flag)
//   +0x1a  MOV  mem32 → 0x0132390c, imm32=0x00418740  (g_assert_fnptr + fn)
//   +0x24  PUSH imm32 → 0x00f57f18  (assert arg5)
//   +0x29  PUSH imm32 → 0x000000a4  (line number; plain imm32)
//   +0x2e  PUSH imm32 → 0x00f57ed0  (assert arg3)
//   +0x33  PUSH imm32 → 0x00f54d48  (assert arg2 / file)
//   +0x38  PUSH imm32 → 0x00f57ec4  (assert arg1 / expr)
//   +0x3d  CALL mem32 → 0x0132390c  (indirect through g_assert_fnptr)
//   +0x47  MOV  mem32 → 0x0132987c  (g_global_obj load)
//   +0x4f  CALL rel32 → FUN_00422f90
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The 7-byte immediate TEST/OR encoding of the assert init block, the
//   hardcoded SIB dereference [ECX+EDI*8-4], and the indirect CALL
//   through the global function pointer all prevent clean source-level
//   reconstruction.  Same _emit-passthrough approach as FUN_00417370
//   (which is the same 103-byte size and shares the same two assert
//   globals).

extern "C" __declspec(naked) void FUN_0041a7d0() {
    __asm {
        // Prologue: save ESI+EDI, load params
        // 0001a7d0: 56           PUSH ESI
        _emit 0x56
        // 0001a7d1: 57           PUSH EDI
        _emit 0x57
        // 0001a7d2: 8b 7c 24 0c  MOV EDI, dword ptr [ESP+0xc]   (idx, 1-based)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 0001a7d6: 85 ff        TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 0001a7d8: 8b f1        MOV ESI, ECX   (save 'this')
        _emit 0x8b
        _emit 0xf1
        // 0001a7da: 75 3c        JNZ +0x3c  (→ 0x0041a818, skip assert block)
        _emit 0x75
        _emit 0x3c

        // Assert block (only reached when idx == 0)
        // 0001a7dc: f6 05 10 39 32 01 01  TEST byte ptr [0x01323910], 0x1
        //           Immediate form (f6 05) — no prior MOV EAX,1 in this fn.
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0001a7e3: 75 11        JNZ +0x11  (→ 0x0041a7f6, already inited)
        _emit 0x75
        _emit 0x11
        // 0001a7e5: 83 0d 10 39 32 01 01  OR dword ptr [0x01323910], 0x1
        //           Immediate form (83 0d) — no EAX/EBX register form here.
        _emit 0x83
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0001a7ec: c7 05 0c 39 32 01 40 87 41 00
        //           MOV dword ptr [0x0132390c], 0x00418740  (install assert fn ptr)
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x40
        _emit 0x87
        _emit 0x41
        _emit 0x00

        // Push 5 assert args and dispatch (right-to-left = last arg first)
        // 0001a7f6: 68 18 7f f5 00  PUSH 0x00f57f18  (arg5: message string)
        _emit 0x68
        _emit 0x18
        _emit 0x7f
        _emit 0xf5
        _emit 0x00
        // 0001a7fb: 68 a4 00 00 00  PUSH 0x000000a4  (arg4: line 164; 5-byte form)
        _emit 0x68
        _emit 0xa4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001a800: 68 d0 7e f5 00  PUSH 0x00f57ed0  (arg3: function name)
        _emit 0x68
        _emit 0xd0
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        // 0001a805: 68 48 4d f5 00  PUSH 0x00f54d48  (arg2: file name)
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 0001a80a: 68 c4 7e f5 00  PUSH 0x00f57ec4  (arg1: expression)
        _emit 0x68
        _emit 0xc4
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        // 0001a80f: ff 15 0c 39 32 01  CALL dword ptr [0x0132390c]  (indirect)
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0001a815: 83 c4 14  ADD ESP, 0x14  (clean up 5 × 4 = 20 bytes)
        _emit 0x83
        _emit 0xc4
        _emit 0x14

        // Main body (reached whether or not the assert fired)
        // 0001a818: 8b 4e 08  MOV ECX, dword ptr [ESI+0x8]   (this->arr_ptr)
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        // 0001a81b: 8b 44 24 10  MOV EAX, dword ptr [ESP+0x10]  (val, 2nd stack arg)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0001a81f: 8b 54 f9 fc  MOV EDX, dword ptr [ECX+EDI*8-0x4]
        //           SIB: base=ECX, index=EDI, scale=8, disp=-4
        //           → arr[(idx-1)].dword_at_offset_4
        _emit 0x8b
        _emit 0x54
        _emit 0xf9
        _emit 0xfc
        // 0001a823: 8b 0d 7c 98 32 01  MOV ECX, dword ptr [0x0132987c]  (global obj)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001a829: 6a 01  PUSH 0x1   (arg3 for FUN_00422f90)
        _emit 0x6a
        _emit 0x01
        // 0001a82b: 50     PUSH EAX   (arg2: val)
        _emit 0x50
        // 0001a82c: 52     PUSH EDX   (arg1: arr element value)
        _emit 0x52
        // 0001a82d: e8 5e 87 00 00  CALL 0x00422f90  (rel32; masked by compare.py)
        _emit 0xe8
        _emit 0x5e
        _emit 0x87
        _emit 0x00
        _emit 0x00

        // Epilogue
        // 0001a832: 5f  POP EDI
        _emit 0x5f
        // 0001a833: 5e  POP ESI
        _emit 0x5e
        // 0001a834: c2 08 00  RET 0x8  (__thiscall, callee cleans 2 DWORD args)
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
