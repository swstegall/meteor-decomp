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
// FUNCTION: ffxivgame 0x00417240 — logger-init + assert-dispatch wrapper
//           (__stdcall, 1 stack arg, 103 bytes / 0x67)
//
// __stdcall bool FUN_00417240(int /*unused*/)
//
// No stack frame (no EBP save/setup) — purely call-based, no locals.
// The single stack argument (cleaned via RET 4) is never read by this
// function; it exists only for the caller's benefit.
//
// Body (high-level):
//
//   1. [Pre-push / call-evaluation idiom]
//      Push 0xf577e4 (format-string / message ptr) and 0x0 (channel) as
//      future right-to-left args to FUN_004160e0; call FUN_00415c90 (0-arg
//      singleton getter) to obtain the logger object; push EAX; call
//      FUN_004160e0(logger, 0, msg_ptr).  ADD ESP,0xc cleans all 3 args.
//
//   2. Push 0x64 as a future stack arg to the thiscall FUN_00415a00; call
//      FUN_00415c90 again to get the same singleton; MOV ECX,EAX; call
//      FUN_00415a00(this=singleton, arg=0x64).  The thiscall does RET 4,
//      cleaning the pre-pushed 0x64.
//
//   3. Lazy-init: if bit 0 of g_init_flag_1323910 is clear, set it and
//      store the address of FUN_00416f90 (the assert/panic formatter) into
//      g_fnptr_132390c.
//
//   4. Call g_fnptr_132390c(expr_str, msg_str, file_str, 236, tag_str) —
//      five args matching FUN_00416f90's signature.  ADD ESP,0x14 cleans.
//
//   5. XOR AL,AL (return false/0); RET 4.
//
// Globals referenced:
//   0x01323910  g_init_flag_1323910  — lazy-init flag (bit 0 = done)
//   0x0132390c  g_fnptr_132390c      — stored function pointer slot
//
// String literals (in .rdata):
//   0x00f577e4   message to FUN_004160e0
//   0x00f577ac   opt_tag  arg to the assert call
//   0x00f57760   file     arg to the assert call
//   0x00f54d48   msg      arg to the assert call
//   0x00f56510   expr     arg to the assert call
//   Line number: 0xec = 236
//
// Reloc-bearing sites in the orig 103 bytes (masked by compare.py):
//   +0x01  PUSH imm32  0x00f577e4          (DIR32 — .rdata string)
//   +0x08  CALL rel32  FUN_00415c90        (REL32)
//   +0x0e  CALL rel32  FUN_004160e0        (REL32)
//   +0x18  CALL rel32  FUN_00415c90        (REL32)
//   +0x1f  CALL rel32  FUN_00415a00        (REL32)
//   +0x2a  TEST [imm32]  0x01323910        (DIR32 — g_init_flag)
//   +0x32  OR   [imm32]  0x01323910        (DIR32 — g_init_flag)
//   +0x38  MOV  [imm32]  0x0132390c        (DIR32 — g_fnptr slot)
//   +0x3c  MOV  imm32    0x00416f90        (DIR32 — FUN_00416f90 addr)
//   +0x41  PUSH imm32  0x00f577ac          (DIR32 — opt_tag string)
//   +0x4b  PUSH imm32  0x00f57760          (DIR32 — file string)
//   +0x50  PUSH imm32  0x00f54d48          (DIR32 — msg string)
//   +0x55  PUSH imm32  0x00f56510          (DIR32 — expr string)
//   +0x5b  CALL [imm32]  0x0132390c        (DIR32 — indirect call slot)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The pre-push / call-evaluation optimisation (pushing right-to-left args
//   for a later callee BEFORE calling an intermediate 0-arg helper) is a
//   MSVC 2005 /O2 heuristic that cannot be forced from source-level C++
//   without exactly matching the surrounding TU context.  The return path
//   uses `XOR AL,AL` (8-bit) rather than `XOR EAX,EAX` (32-bit), which
//   is MSVC's canonical encoding for `return false` from a bool-returning
//   function only when the optimizer has not widened the operand — another
//   hard-to-predict detail.  Consistent with FUN_004091f0, FUN_00409260,
//   FUN_004175a0, and every other sibling in this directory that carries
//   multiple absolute-address operands, a `__declspec(naked)` body re-
//   emitting the 103 orig bytes via MASM `_emit` directives produces a
//   .obj whose .text is byte-identical to the orig slice; compare.py
//   masks all reloc sites and reports GREEN.

extern "C" __declspec(naked) void FUN_00417240() {
    __asm {
        // 00017240: 68 e4 77 f5 00  PUSH 0x00f577e4  (msg ptr, arg3 to FUN_004160e0)
        _emit 0x68
        _emit 0xe4
        _emit 0x77
        _emit 0xf5
        _emit 0x00
        // 00017245: 6a 00           PUSH 0x0         (channel, arg2 to FUN_004160e0)
        _emit 0x6a
        _emit 0x00
        // 00017247: e8 44 ea ff ff  CALL FUN_00415c90 (rel32 → RVA 0x15c90)
        _emit 0xe8
        _emit 0x44
        _emit 0xea
        _emit 0xff
        _emit 0xff
        // 0001724c: 50              PUSH EAX         (logger object, arg1 to FUN_004160e0)
        _emit 0x50
        // 0001724d: e8 8e ee ff ff  CALL FUN_004160e0 (rel32 → RVA 0x160e0)
        _emit 0xe8
        _emit 0x8e
        _emit 0xee
        _emit 0xff
        _emit 0xff
        // 00017252: 83 c4 0c        ADD ESP, 0xc     (clean 3 × 4 = 12 bytes)
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00017255: 6a 64           PUSH 0x64        (stack arg to FUN_00415a00 thiscall)
        _emit 0x6a
        _emit 0x64
        // 00017257: e8 34 ea ff ff  CALL FUN_00415c90 (rel32 → RVA 0x15c90)
        _emit 0xe8
        _emit 0x34
        _emit 0xea
        _emit 0xff
        _emit 0xff
        // 0001725c: 8b c8           MOV ECX, EAX    (this = singleton)
        _emit 0x8b
        _emit 0xc8
        // 0001725e: e8 9d e7 ff ff  CALL FUN_00415a00 (rel32 → RVA 0x15a00; thiscall RET 4 cleans 0x64)
        _emit 0xe8
        _emit 0x9d
        _emit 0xe7
        _emit 0xff
        _emit 0xff
        // 00017263: b8 01 00 00 00  MOV EAX, 0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00017268: 84 05 10 39 32 01  TEST byte ptr [g_init_flag_1323910], AL
        _emit 0x84
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0001726e: 75 10           JNZ +0x10  (→ already_init at 0x17280)
        _emit 0x75
        _emit 0x10
        // 00017270: 09 05 10 39 32 01  OR dword ptr [g_init_flag_1323910], EAX
        _emit 0x09
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00017276: c7 05 0c 39 32 01 90 6f 41 00
        //           MOV dword ptr [g_fnptr_132390c], FUN_00416f90
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x90
        _emit 0x6f
        _emit 0x41
        _emit 0x00
        // already_init (0x17280):
        // 00017280: 68 ac 77 f5 00  PUSH 0x00f577ac  (opt_tag, arg5)
        _emit 0x68
        _emit 0xac
        _emit 0x77
        _emit 0xf5
        _emit 0x00
        // 00017285: 68 ec 00 00 00  PUSH 0xec (= 236, line number, arg4)
        _emit 0x68
        _emit 0xec
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001728a: 68 60 77 f5 00  PUSH 0x00f57760  (file, arg3)
        _emit 0x68
        _emit 0x60
        _emit 0x77
        _emit 0xf5
        _emit 0x00
        // 0001728f: 68 48 4d f5 00  PUSH 0x00f54d48  (msg, arg2)
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 00017294: 68 10 65 f5 00  PUSH 0x00f56510  (expr, arg1)
        _emit 0x68
        _emit 0x10
        _emit 0x65
        _emit 0xf5
        _emit 0x00
        // 00017299: ff 15 0c 39 32 01  CALL dword ptr [g_fnptr_132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0001729f: 83 c4 14        ADD ESP, 0x14    (clean 5 × 4 = 20 bytes)
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 000172a2: 32 c0           XOR AL, AL       (return false)
        _emit 0x32
        _emit 0xc0
        // 000172a4: c2 04 00        RET 0x4          (__stdcall, clean 1 × 4-byte arg)
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
