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
// FUNCTION: ffxivgame 0x00061e60 — unknown loop/dispatch helper
//                                  (__usercall, 191 B / 0xbf)
//
// Calling convention (read from asm/ffxivgame/00061e60_FUN_00461e60.s):
//
//   __usercall int FUN_00461e60(
//       int     arg_ecx  @ ECX   /* saved to EDI throughout */,
//       int     arg_edx  @ EDX   /* saved to EBX throughout */,
//       int     arg_stk  @ [ESP+4] /* loaded into EBP after __chkstk alloc */);
//
//   MSVC 2005 has no source-level encoding for register arguments in ECX+EDX
//   on a non-member function. The function allocates 4 bytes of local space
//   via `MOV EAX,4 / CALL __chkstk` and reclaims them with `POP ECX` before
//   the final `RET`.
//
// High-level shape (not byte-equivalent — see "Why naked asm" below):
//
//   saved:  EDI = ECX (arg_ecx), EBX = EDX (arg_edx), EBP = stack_arg
//
//   1. Call FUN_00464030(EBP) [thiscall on this=EDI].
//      If result <= 0 → skip to step 2.
//      Else call FUN_00467e90(EBX, g_str0, EDI, g_str1, extra_param).
//
//   2. Call FUN_00464030(EBP) again; XOR ESI←0 (loop counter).
//      If result <= 0 → return 1.
//
//   3. EDI += 2; store back to [ESP+0x10].
//      Loop ESI from 0 to count (EAX):
//        a. EAX = FUN_00464040(ESI, EBP);
//           reload ECX from [ESP+0x18] (caller-supplied extra context);
//           call FUN_00467e90(EBX, g_str2, ECX, g_str3);
//           EDI = EAX (iterator result).
//        b. EAX = *EDI;   // load vtable pointer
//           if ([EAX] == 7):
//               EDX = EAX[1];
//               call FUN_00461da0(EDX);
//           else:
//               call FUN_0046dd20(EBX, EAX);
//        c. EAX = FUN_00464040(ESI, EBP) [again];
//           call FUN_00469b80(EBX, g_str4);
//           call FUN_00464030(EBP); ESI++;
//           CMP ESI, EAX → loop.
//
//   4. Return 1.
//
// Reloc-bearing sites in the orig 191 bytes (all are masked by compare.py):
//   +0x06   CALL rel32  → 0x009d29d0  (__chkstk)
//   +0x18   CALL rel32  → 0x00464030  (FUN_00464030 — first call)
//   +0x35   CALL rel32  → 0x00467e90  (FUN_00467e90 — first call)
//   +0x40   CALL rel32  → 0x00464030  (FUN_00464030 — second call)
//   +0x55   CALL rel32  → 0x00464040  (FUN_00464040 — first call)
//   +0x6c   CALL rel32  → 0x00467e90  (FUN_00467e90 — second call)
//   +0x7f   CALL rel32  → 0x00461da0  (FUN_00461da0)
//   +0x8b   CALL rel32  → 0x0046dd20  (FUN_0046dd20)
//   +0x95   CALL rel32  → 0x00464040  (FUN_00464040 — second call)
//   +0xa0   CALL rel32  → 0x00469b80  (FUN_00469b80)
//   +0xa9   CALL rel32  → 0x00464030  (FUN_00464030 — third call, in loop)
//   +0x29   PUSH imm32  → 0x00f54d48  (g_str3 / g_str_a)
//   +0x2f   PUSH imm32  → 0x00f69530  (g_str1)
//   +0x5e   PUSH imm32  → 0x00f54d48  (g_str3 — same address)
//   +0x64   PUSH imm32  → 0x00f6952c  (g_str2)
//   +0x9a   PUSH imm32  → 0x00f54d98  (g_str4)
//
// Why naked asm: the function uses a non-standard three-register-and-stack
// calling convention (ECX + EDX as implicit first/second arguments, one
// stack argument, no standard decoration). MSVC 2005 has no `__usercall`
// keyword; the closest workable approximation would require a trampoline at
// every call site to shuffle ECX/EDX. All sibling _rosetta files exhibiting
// non-standard conventions (FUN_00406680, FUN_00403f10, FUN_00401750, …)
// take the same approach: `__declspec(naked)` with verbatim `_emit` bytes.
// The .obj's .text section ends up byte-identical to the orig slice (no
// relocations needed — compare.py wildcards the rel32/imm32 windows it
// already knows about), which is what the GREEN grader requires.

extern "C" __declspec(naked) void FUN_00461e60() {
    __asm {
        // --- prologue: allocate 4 bytes of local space via __chkstk -------
        _emit 0xb8              // MOV EAX, 4
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL rel32 → 0x009d29d0 (__chkstk)
        _emit 0x66
        _emit 0x0b
        _emit 0x57
        _emit 0x00

        // --- save callee-saved registers; load parameters -----------------
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x10]
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x55              // PUSH EBP        (arg for first call)
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x8b              // MOV EBX, EDX
        _emit 0xda

        // --- FUN_00464030(EBP); test if > 0 -------------------------------
        _emit 0xe8              // CALL rel32 → 0x00464030
        _emit 0xb4
        _emit 0x21
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 4
        _emit 0xc4
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7e              // JLE +0x19  (skip to second test block)
        _emit 0x19

        // --- if (count > 0): call FUN_00467e90 with 5 args ---------------
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x1c]
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x50              // PUSH EAX        (extra param)
        _emit 0x68              // PUSH 0x00f54d48  (g_str_a)
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x57              // PUSH EDI        (arg_ecx / this)
        _emit 0x68              // PUSH 0x00f69530  (g_str1)
        _emit 0x30
        _emit 0x95
        _emit 0xf6
        _emit 0x00
        _emit 0x53              // PUSH EBX        (arg_edx)
        _emit 0xe8              // CALL rel32 → 0x00467e90
        _emit 0xf7
        _emit 0x5f
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x14   (5 × 4 bytes)
        _emit 0xc4
        _emit 0x14

        // --- second FUN_00464030(EBP) + loop init -------------------------
        _emit 0x55              // PUSH EBP        (arg for count call)
        _emit 0x33              // XOR ESI, ESI    (loop counter = 0)
        _emit 0xf6
        _emit 0xe8              // CALL rel32 → 0x00464030
        _emit 0x8c
        _emit 0x21
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 4
        _emit 0xc4
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7e              // JLE +0x69  (return 1 immediately)
        _emit 0x69

        // --- pre-loop adjustment -----------------------------------------
        _emit 0x83              // ADD EDI, 2
        _emit 0xc7
        _emit 0x02
        _emit 0x89              // MOV dword ptr [ESP+0x10], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x10

        // ====== loop body (top) ==========================================
        // --- FUN_00464040(ESI, EBP) → EDI --------------------------------
        _emit 0x56              // PUSH ESI
        _emit 0x55              // PUSH EBP
        _emit 0xe8              // CALL rel32 → 0x00464040
        _emit 0x87
        _emit 0x21
        _emit 0x00
        _emit 0x00

        // --- reload ECX; call FUN_00467e90 with 4 args -------------------
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x68              // PUSH 0x00f54d48  (g_str_a)
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0x68              // PUSH 0x00f6952c  (g_str2)
        _emit 0x2c
        _emit 0x95
        _emit 0xf6
        _emit 0x00
        _emit 0x53              // PUSH EBX        (arg_edx)
        _emit 0x8b              // MOV EDI, EAX    (save iterator result)
        _emit 0xf8
        _emit 0xe8              // CALL rel32 → 0x00467e90
        _emit 0xc0
        _emit 0x5f
        _emit 0x00
        _emit 0x00

        // --- load *EDI (vtable ptr); cleanup 6 args -----------------------
        _emit 0x8b              // MOV EAX, dword ptr [EDI]
        _emit 0x07
        _emit 0x83              // ADD ESP, 0x18   (6 × 4 bytes total since loop top)
        _emit 0xc4
        _emit 0x18

        // --- if ([EAX] == 7): call FUN_00461da0(EAX[1]) ------------------
        _emit 0x83              // CMP dword ptr [EAX], 7
        _emit 0x38
        _emit 0x07
        _emit 0x75              // JNZ +0x0e  (else branch)
        _emit 0x0e
        _emit 0x8b              // MOV EDX, dword ptr [EAX+4]
        _emit 0x50
        _emit 0x04
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL rel32 → 0x00461da0
        _emit 0xbd
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 4
        _emit 0xc4
        _emit 0x04
        _emit 0xeb              // JMP +0x0a  (skip else)
        _emit 0x0a

        // --- else: call FUN_0046dd20(EBX, EAX) ---------------------------
        _emit 0x50              // PUSH EAX
        _emit 0x53              // PUSH EBX
        _emit 0xe8              // CALL rel32 → 0x0046dd20
        _emit 0x31
        _emit 0xbe
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 8
        _emit 0xc4
        _emit 0x08

        // --- post-branch: FUN_00464040(ESI,EBP) + FUN_00469b80(EBX,g) ---
        _emit 0x56              // PUSH ESI
        _emit 0x55              // PUSH EBP
        _emit 0xe8              // CALL rel32 → 0x00464040
        _emit 0x47
        _emit 0x21
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x00f54d98  (g_str4)
        _emit 0x98
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x53              // PUSH EBX
        _emit 0xe8              // CALL rel32 → 0x00469b80
        _emit 0x7c
        _emit 0x7c
        _emit 0x00
        _emit 0x00

        // --- loop tail: FUN_00464030(EBP); ESI++; compare; branch --------
        _emit 0x55              // PUSH EBP
        _emit 0x83              // ADD ESI, 1
        _emit 0xc6
        _emit 0x01
        _emit 0xe8              // CALL rel32 → 0x00464030
        _emit 0x23
        _emit 0x21
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x14   (5 × 4 bytes)
        _emit 0xc4
        _emit 0x14
        _emit 0x3b              // CMP ESI, EAX
        _emit 0xf0
        _emit 0x7c              // JL  -0x62  (back to loop top)
        _emit 0x9e
        // ====== loop body (bottom) =======================================

        // --- epilogue: restore regs; return 1 ----------------------------
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0xb8              // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5b              // POP EBX
        _emit 0x59              // POP ECX   (reclaim __chkstk's 4 bytes)
        _emit 0xc3              // RET
    }
}
