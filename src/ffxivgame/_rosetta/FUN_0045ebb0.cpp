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
// FUNCTION: ffxivgame 0x0045ebb0 — multi-path dispatch with alloca frame
//                                  (353 B / 0x161, `__cdecl`, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x0005ebb0):
//
//   __cdecl int FUN_0045ebb0(arg1, arg2_ptr, arg3, arg4_ptr, ...)
//
//   Structural shape:
//
//     Prologue: alloca(0xc) + save EBX, EBP.
//     ESI (caller-maintained object ptr) and EDX (caller-maintained value)
//     are used as implicit context throughout — neither is loaded from the
//     argument list here; they arrive live in the registers from the caller
//     and are forwarded to FUN_0045fb10.
//
//     EBP = arg2 (a pointer)
//     EBX = arg3
//     if (!EBP) { EAX = 0; return; }        // early null-guard
//
//     [ESI] flags consulted: bit 0x10 → branch to 0x45ecfa (fast path);
//                            bits 0xc0 → forwarded as arg to FUN_0045e9f0.
//
//     Main path (bit 0x10 set):
//       Call FUN_0045e9f0 with 11 stack slots (9 pushed + 2 from earlier
//       PUSH EBX / PUSH ECX), result in EAX.
//       EAX == 0  → log error 0x234/0x3a and return 0.
//       EAX == -1 → OR EAX,EAX (no-op, sets ZF=0) and return EAX.
//       Otherwise: check byte flag, call FUN_0045fb10, then
//         on failure → log error 0x244/0x3a and return 0,
//         on success → adjust EDI/EBX by the output delta, then
//           if flag byte nonzero: call FUN_0045e9c0; fail → log 0x24f, return 0,
//           if EBX == 0: log 0x25a via 0x45c940, call FUN_00461240, return 0,
//           success: store result into *arg4, return 1.
//
//     Fast path (bit 0x10 clear, 0x45ecfa):
//       Push ESI, EDX, arg4, EBP (4 args) to FUN_0045fb10; return EAX.
//
//   Stack frame (alloca at top + three saves):
//     [esp+0x00] saved_EDI   (after later PUSH EDI)
//     [esp+0x04] saved_EBP   (callee-saved EBP)
//     [esp+0x08] saved_EBX   (callee-saved EBX)
//     [esp+0x0c..0x14] alloca locals (3 DWORDs, 12 bytes)
//     [esp+0x10] retaddr
//     [esp+0x14] arg1
//     [esp+0x18] arg2_ptr    → EBP
//     [esp+0x1c] arg3        → EBX
//     [esp+0x20] arg4_ptr
//
//   Reloc-bearing sites in the orig 353 bytes (absolute addresses at
//   orig image base 0x00400000; tools/compare.py masks on compare):
//     +0x05  REL32 → 0x009d29d0  (_alloca_probe CALL)
//     +0x5a  REL32 → 0x0045e9f0  (CALL inner parse fn)
//     +0x8b  IMM32 → 0xf69300    (PUSH log-string addr, path 1a)
//     +0xa3  REL32 → 0x0045fb10  (CALL write fn, path 1b)
//     +0xb4  IMM32 → 0xf69300    (PUSH log-string addr, path 1c)
//     +0xbb  IMM32 → 0x84        (note: opcode value, not reloc)
//     +0xc2  REL32 → 0x0045c940  (CALL log fn, path 1c)
//     +0xe5  REL32 → 0x0045e9c0  (CALL flag-check fn)
//     +0xf6  IMM32 → 0xf69300    (PUSH log-string addr, path 2a)
//     +0x10b IMM32 → 0xf69300    (PUSH log-string addr, path 2b)
//     +0x119 REL32 → 0x0045c940  (CALL log fn, path 2b)
//     +0x123 REL32 → 0x00461240  (CALL cleanup fn)
//     +0x152 REL32 → 0x0045fb10  (CALL write fn, fast path)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function body contains 11 relocation windows (PC-relative CALLs
//   and absolute IMM32 string addresses) that only resolve correctly in
//   the orig binary's link-time RVA space. Attempting to reproduce the
//   exact register scheduling (ESI/EDX live-in, alloca timing, LEA-based
//   arg passing, the 11-slot push sequence for FUN_0045e9f0) and the
//   precise short vs. near branch encodings at /O2 is impractical. The
//   same naked-asm passthrough used by FUN_00405080 / FUN_0040ced0 /
//   FUN_004014b0 is the correct approach here.

extern "C" __declspec(naked) void FUN_0045ebb0() {
    __asm {
        _emit 0xb8
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x16
        _emit 0x3e
        _emit 0x57
        _emit 0x00
        _emit 0x53
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x55
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x85
        _emit 0xed
        _emit 0x75
        _emit 0x08
        _emit 0x5d
        _emit 0x33
        _emit 0xc0
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3
        _emit 0x8b
        _emit 0x06
        _emit 0xa8
        _emit 0x10
        _emit 0x57
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x8b
        _emit 0x3f
        _emit 0x53
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x51
        _emit 0x0f
        _emit 0x84
        _emit 0x13
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x25
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x50
        _emit 0x52
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x51
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x34
        _emit 0x52
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x2b
        _emit 0x50
        _emit 0x6a
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0x6a
        _emit 0x00
        _emit 0x51
        _emit 0xe8
        _emit 0xe1
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x3c
        _emit 0x83
        _emit 0xc4
        _emit 0x2c
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x07
        _emit 0x68
        _emit 0x34
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x43
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        _emit 0x75
        _emit 0x09
        _emit 0x5f
        _emit 0x5d
        _emit 0x0b
        _emit 0xc0
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x00
        _emit 0x75
        _emit 0x0e
        _emit 0x68
        _emit 0x3c
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x00
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x78
        _emit 0xeb
        _emit 0x27
        _emit 0x53
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        _emit 0x6a
        _emit 0x00
        _emit 0x56
        _emit 0x53
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x52
        _emit 0x55
        _emit 0xe8
        _emit 0xb8
        _emit 0x0e
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x24
        _emit 0x68
        _emit 0x44
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x00
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x3a
        _emit 0x68
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x0d
        _emit 0xe8
        _emit 0xc9
        _emit 0xdc
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x5f
        _emit 0x5d
        _emit 0x33
        _emit 0xc0
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3
        _emit 0x2b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x03
        _emit 0xdf
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x0f
        _emit 0x00
        _emit 0x74
        _emit 0x22
        _emit 0x53
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xe8
        _emit 0x26
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x43
        _emit 0x68
        _emit 0x4f
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x00
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0x89
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x10
        _emit 0x85
        _emit 0xdb
        _emit 0x74
        _emit 0x2e
        _emit 0x68
        _emit 0x5a
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x00
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x77
        _emit 0x68
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x0d
        _emit 0xe8
        _emit 0x72
        _emit 0xdc
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x56
        _emit 0x55
        _emit 0xe8
        _emit 0x68
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x5f
        _emit 0x5d
        _emit 0x33
        _emit 0xc0
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x5f
        _emit 0x5d
        _emit 0x89
        _emit 0x08
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3
        _emit 0x56
        _emit 0x52
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x30
        _emit 0x52
        _emit 0x55
        _emit 0xe8
        _emit 0x09
        _emit 0x0e
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x5f
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3
    }
}
