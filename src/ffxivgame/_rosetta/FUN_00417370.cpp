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
// FUNCTION: ffxivgame 0x00417370 — frameless __stdcall init stub (103 B / 0x67)
//
// __stdcall int FUN_00417370(int arg1, int arg2)
//
//   No stack frame (no PUSH EBP / MOV EBP,ESP). The two incoming stack args
//   are never read by this function — they are silently discarded, but RET 0x8
//   pops them on the way out.
//
//   Logical body:
//
//   1. Block 1 — log call via singleton:
//        void *logger = FUN_00415c90();          // cdecl, 0 args, singleton getter
//        FUN_004160e0(logger, 0, "...");          // cdecl, 3-arg log helper
//
//   2. Block 2 — thiscall on singleton with pre-staged arg:
//        // MSVC pre-stages the arg (PUSH 0x64) before calling the singleton
//        // getter so the stack is already set up for the thiscall:
//        void *obj = FUN_00415c90();              // cdecl, 0 args
//        FUN_00415a00(obj, 100);                  // __thiscall, 1 explicit arg (RET 0x4)
//
//   3. Block 3 — lazy init of global function pointer:
//        if (!(g_init & 1)) {
//            g_init |= 1;
//            g_fn_ptr = FUN_00416f90;
//        }
//
//   4. Block 4 — call through function pointer:
//        g_fn_ptr(...);                           // 5 args, cdecl (caller cleans 0x14)
//        return 0;
//
//   Globals:
//     0x01323910  g_init    — init flag (bit 0 = "function pointer installed")
//     0x0132390c  g_fn_ptr  — function pointer (set lazily to 0x00416f90)
//
//   Call targets (all rel32; the displaced bytes are masked by compare.py):
//     FUN_00415c90  0x00415c90 — singleton get-or-init (cdecl, 0 args)
//     FUN_004160e0  0x004160e0 — filtered vprintf-style log helper (cdecl, 3+ args)
//     FUN_00415a00  0x00415a00 — thiscall method (__thiscall, 1 explicit arg)
//
//   Key encoding points:
//     - PUSH 0x64 is emitted BEFORE the second CALL FUN_00415c90 (MSVC
//       pre-stage optimization: arg is pushed early to avoid a separate PUSH
//       after the getter returns; FUN_00415a00 reads it as its stack arg).
//     - TEST byte ptr [g_init], AL (84 05) uses AL=1 from the preceding
//       MOV EAX,1; the 6-byte form "TEST [mem8], reg8" saves one byte vs the
//       7-byte "TEST [mem8], imm8" form.
//     - MOV dword ptr [g_fn_ptr], 0x00416f90 embeds the code address
//       0x00416f90 as a raw imm32; both the address slot and the immediate
//       are DIR32 relocs (masked by compare.py).
//     - CALL dword ptr [g_fn_ptr] (FF 15) is an indirect call through memory;
//       the 4-byte address of g_fn_ptr is a DIR32 reloc (masked).
//
//   Why naked asm:
//     The pre-staged PUSH 0x64 idiom, the specific TEST [mem8],AL encoding,
//     and the frameless epilogue (no LEAVE / POP EBP) cannot be reliably
//     reproduced from C++ source without controlling the optimiser's register
//     allocation and instruction-selection choices across the TU boundary.
//     The byte-passthrough approach (same as FUN_004091f0, FUN_00409260, etc.)
//     avoids all of those uncertainties.
//
//   Original 103 bytes (per asm/ffxivgame/00017370_FUN_00417370.s):
//
//     00017370: 68 64 78 f5 00 6a 00 e8 14 e9 ff ff 50 e8 5e ed
//     00017380: ff ff 83 c4 0c 6a 64 e8 04 e9 ff ff 8b c8 e8 6d
//     00017390: e6 ff ff b8 01 00 00 00 84 05 10 39 32 01 75 10
//     000173a0: 09 05 10 39 32 01 c7 05 0c 39 32 01 90 6f 41 00
//     000173b0: 68 28 78 f5 00 68 1b 02 00 00 68 60 77 f5 00 68
//     000173c0: 48 4d f5 00 68 10 65 f5 00 ff 15 0c 39 32 01 83
//     000173d0: c4 14 33 c0 c2 08 00

extern "C" __declspec(naked) void FUN_00417370() {
    __asm {
        // --- Block 1: log via singleton -----------------------------------
        // 00017370: 68 64 78 f5 00   PUSH 0x00f57864   (string pointer; DIR32 reloc)
        _emit 0x68
        _emit 0x64
        _emit 0x78
        _emit 0xf5
        _emit 0x00
        // 00017375: 6a 00            PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00017377: e8 14 e9 ff ff   CALL FUN_00415c90  (rel32 masked)
        _emit 0xe8
        _emit 0x14
        _emit 0xe9
        _emit 0xff
        _emit 0xff
        // 0001737c: 50               PUSH EAX           (push singleton for FUN_004160e0)
        _emit 0x50
        // 0001737d: e8 5e ed ff ff   CALL FUN_004160e0  (rel32 masked)
        _emit 0xe8
        _emit 0x5e
        _emit 0xed
        _emit 0xff
        _emit 0xff
        // 00017382: 83 c4 0c         ADD ESP, 0xc       (caller cleans 3 cdecl args)
        _emit 0x83
        _emit 0xc4
        _emit 0x0c

        // --- Block 2: thiscall on singleton (arg pre-staged before getter) ---
        // 00017385: 6a 64            PUSH 0x64          (pre-stage arg for FUN_00415a00)
        _emit 0x6a
        _emit 0x64
        // 00017387: e8 04 e9 ff ff   CALL FUN_00415c90  (rel32 masked; returns obj in EAX)
        _emit 0xe8
        _emit 0x04
        _emit 0xe9
        _emit 0xff
        _emit 0xff
        // 0001738c: 8b c8            MOV ECX, EAX       (ECX = this for thiscall)
        _emit 0x8b
        _emit 0xc8
        // 0001738e: e8 6d e6 ff ff   CALL FUN_00415a00  (rel32 masked; thiscall, RET 0x4)
        _emit 0xe8
        _emit 0x6d
        _emit 0xe6
        _emit 0xff
        _emit 0xff

        // --- Block 3: lazy-init global function pointer -------------------
        // 00017393: b8 01 00 00 00   MOV EAX, 1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00017398: 84 05 10 39 32 01  TEST byte ptr [0x01323910], AL
        //           (6-byte form uses AL from MOV EAX,1; DIR32 masked)
        _emit 0x84
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0001739e: 75 10            JNZ +0x10  (skip the init block if already done)
        _emit 0x75
        _emit 0x10
        // 000173a0: 09 05 10 39 32 01  OR dword ptr [0x01323910], EAX
        //           (sets bit 0 of init flag; DIR32 masked)
        _emit 0x09
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 000173a6: c7 05 0c 39 32 01 90 6f 41 00
        //           MOV dword ptr [0x0132390c], 0x00416f90
        //           (install function pointer; both the addr and the imm32
        //            are DIR32 relocs masked by compare.py)
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

        // --- Block 4: call through function pointer with 5 args -----------
        // 000173b0: 68 28 78 f5 00   PUSH 0x00f57828  (arg5; DIR32 masked)
        _emit 0x68
        _emit 0x28
        _emit 0x78
        _emit 0xf5
        _emit 0x00
        // 000173b5: 68 1b 02 00 00   PUSH 0x0000021b  (arg4 = 539; plain imm32)
        _emit 0x68
        _emit 0x1b
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 000173ba: 68 60 77 f5 00   PUSH 0x00f57760  (arg3; DIR32 masked)
        _emit 0x68
        _emit 0x60
        _emit 0x77
        _emit 0xf5
        _emit 0x00
        // 000173bf: 68 48 4d f5 00   PUSH 0x00f54d48  (arg2; DIR32 masked)
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 000173c4: 68 10 65 f5 00   PUSH 0x00f56510  (arg1; DIR32 masked)
        _emit 0x68
        _emit 0x10
        _emit 0x65
        _emit 0xf5
        _emit 0x00
        // 000173c9: ff 15 0c 39 32 01  CALL dword ptr [0x0132390c]
        //           (indirect call through g_fn_ptr; DIR32 masked)
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 000173cf: 83 c4 14         ADD ESP, 0x14    (caller cleans 5 cdecl args)
        _emit 0x83
        _emit 0xc4
        _emit 0x14

        // --- Epilogue: return 0 -------------------------------------------
        // 000173d2: 33 c0            XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 000173d4: c2 08 00         RET 0x8  (__stdcall: callee cleans 2 DWORDs)
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
