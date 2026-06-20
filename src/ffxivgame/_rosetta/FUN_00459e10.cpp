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
// FUNCTION: ffxivgame 0x00059e10 — binary search over a sorted byte array
//                                  (50 B / 0x32).
//
// Custom register-based calling convention (compiler-internal, not a named
// MSVC cc):
//   EAX  = count (n) — length of the sorted byte array
//   EDI  = base pointer to the sorted byte array
//   [ESP+4] (before any intra-function pushes) = key byte to search for
//   Returns EAX = found index, or -1 (0xFFFFFFFF) if not found.
//
// Algorithm — classic binary search:
//   low  = EDX = 0
//   high = ESI = n - 1
//   loop while high >= low (unsigned):
//     mid = (high + low) >> 1
//     diff = key - array[mid]
//     if diff == 0  → return mid            (found)
//     if diff <  0  → high = mid - 1
//     else          → low  = mid + 1
//   return -1                               (not found)
//
// Frame:
//   PUSH EBX ; PUSH ESI     (callee-saves; no ESP adjustment, no frame ptr)
//
// Encoding notes:
//   +0x0D  8d 49 00  LEA ECX,[ECX+0]  — 3-byte NOP for loop alignment.
//          MASM would canonicalize to the 2-byte form (8d 09), so we emit
//          raw bytes for the whole function to guarantee byte identity.
//   No external relocations — all branch targets are intra-function
//   PC-relative immediates, fully determined at assembly time.
//
// Asm (50 bytes @ RVA 0x00059e10):
//   53               PUSH EBX
//   8a 5c 24 08      MOV  BL, [ESP+8]
//   56               PUSH ESI
//   8b f0            MOV  ESI, EAX
//   33 d2            XOR  EDX, EDX
//   83 c6 ff         ADD  ESI, -1
//   8d 49 00         LEA  ECX, [ECX+0]          ; 3-byte loop-alignment NOP
// loop:
//   8d 04 16         LEA  EAX, [ESI+EDX*1]
//   d1 e8            SHR  EAX, 1
//   8a cb            MOV  CL, BL
//   2a 0c 38         SUB  CL, [EAX+EDI*1]
//   74 13            JZ   found
//   84 c9            TEST CL, CL
//   7d 05            JGE  go_high
//   8d 70 ff         LEA  ESI, [EAX-1]          ; high = mid - 1
//   eb 03            JMP  check
// go_high:
//   8d 50 01         LEA  EDX, [EAX+1]          ; low = mid + 1
// check:
//   3b f2            CMP  ESI, EDX
//   73 e4            JNC  loop                  ; while high >= low (unsigned)
//   83 c8 ff         OR   EAX, 0xFFFFFFFF       ; not found → EAX = -1
// found:
//   5e               POP  ESI
//   5b               POP  EBX
//   c3               RET

extern "C" __declspec(naked) void FUN_00459e10() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8a              // MOV BL, byte ptr [ESP+8]
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x33              // XOR EDX, EDX
        _emit 0xd2
        _emit 0x83              // ADD ESI, -1
        _emit 0xc6
        _emit 0xff
        _emit 0x8d              // LEA ECX, [ECX+0]  (3-byte NOP)
        _emit 0x49
        _emit 0x00
        // loop:
        _emit 0x8d              // LEA EAX, [ESI+EDX*1]
        _emit 0x04
        _emit 0x16
        _emit 0xd1              // SHR EAX, 1
        _emit 0xe8
        _emit 0x8a              // MOV CL, BL
        _emit 0xcb
        _emit 0x2a              // SUB CL, byte ptr [EAX+EDI*1]
        _emit 0x0c
        _emit 0x38
        _emit 0x74              // JZ found (+0x13)
        _emit 0x13
        _emit 0x84              // TEST CL, CL
        _emit 0xc9
        _emit 0x7d              // JGE go_high (+0x05)
        _emit 0x05
        _emit 0x8d              // LEA ESI, [EAX-1]
        _emit 0x70
        _emit 0xff
        _emit 0xeb              // JMP check (+0x03)
        _emit 0x03
        // go_high:
        _emit 0x8d              // LEA EDX, [EAX+1]
        _emit 0x50
        _emit 0x01
        // check:
        _emit 0x3b              // CMP ESI, EDX
        _emit 0xf2
        _emit 0x73              // JNC loop (-0x1c)
        _emit 0xe4
        _emit 0x83              // OR EAX, 0xFFFFFFFF
        _emit 0xc8
        _emit 0xff
        // found:
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
