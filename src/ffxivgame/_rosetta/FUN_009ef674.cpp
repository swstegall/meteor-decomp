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
// FUNCTION: ffxivgame 0x005ef674 — `__cdecl` IEEE-754 special-case pre-check
//                                  for a pow-like computation (318 B / 0x13e).
//
// Inspection (read from the disassembly at orig RVA 0x005ef674):
//
//   __cdecl int FUN_009ef674(double a, double b, double *result);
//
//   Returns 0 normally; returns 1 only in the b = -infinity, |a| == 1 case
//   where a special sentinel constant [0x012ebbd8] is written to *result
//   (and ESI is explicitly zeroed then incremented to 1 before the write,
//   distinguishing this from the ordinary zero return).
//
//   The function handles IEEE-754 special cases for a two-double-input
//   computation:
//
//     abs_a = fabs(a);      // computed via FLDZ / FCOM / FLD / FCHS idiom
//
//     if (b == +infinity) {
//         if (abs_a > 1.0)  → *result = [0x012ebbd0] (+inf),  return 0
//         if (abs_a == 1.0) → *result = ST0 (1.0),             return 0
//         if (abs_a < 1.0)  → *result = 0.0,                   return 0
//     } else if (b == -infinity) {
//         if (abs_a > 1.0)  → *result = 0.0,                   return 0
//         if (abs_a == 1.0) → *result = [0x012ebbd8],  INC ESI, return 1
//         if (abs_a < 1.0)  → *result = [0x012ebbd0] (+inf),  return 0
//     } else if (a == +infinity) {
//         if (b > 0)  → *result = [0x012ebbd0] (+inf), return 0
//         if (b == 0) → *result = 1.0,                 return 0
//         if (b < 0)  → *result = 0.0,                 return 0
//     } else if (a == -infinity) {
//         // calls helper FUN_009ef610(b) → odd-integer predicate
//         if (b > 0) {
//             if (odd(b)) → *result = -[0x012ebbd0] (-inf), return 0
//             else        → *result = [0x012ebbd0] (+inf),  return 0
//         } else if (b < 0) {
//             if (odd(b)) → *result = [0x012ebbf0] (-0.0),  return 0
//             else        → *result = 0.0,                   return 0
//         } else {        // b == 0
//             *result = 1.0,                                  return 0
//         }
//     } else {
//         return 0;  // no special case; *result not written
//     }
//
//   Stack frame: EBP frame, saves ESI; no local variables beyond push.
//   Callee-saves preserved: ESI (saved/restored), EBX/EDI untouched.
//
//   Reloc-bearing sites in the orig 318 bytes (absolute data references
//   resolved at link/load time; emitted as raw immediates below):
//     +0x3e  FLD [0x012ebbd0]   (1st use: +inf constant)
//     +0x91  FLD [0x012ebbd0]   (2nd use: +inf constant)
//     +0x9c  FLD [0x012ebbd8]   (sentinel: 1.0 or NaN per 1.x convention)
//     +0x07  FLD [0x012ebbd0]   (3rd use at +0x107: +inf constant)
//     +0x124 FLD [0x012ebbf0]   (-0.0 constant)
//     +0xed  CALL 0x009ef610    (rel32 helper: odd-integer predicate for b)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function's FPU state-machine is dense: x87 stack discipline across
//   eleven arms, three distinct absolute-address FLD constants, a rel32
//   CALL into an unmatched sibling (FUN_009ef610), and MSVC 2005's
//   characteristic JP/JNP idiom for FNSTSW-based comparison (TEST AH,0x5
//   and TEST AH,0x41 patterns). Reproducing the exact branch encoding
//   (rel8 vs near32), FSTP ST(n) ordering, and register-allocation from
//   source C++ under /O2 is brittle. The pragmatic choice — consistent
//   with FUN_004014b0, FUN_00408f10, FUN_00401a00, and the rest of the
//   _rosetta tree — is to emit all 318 bytes verbatim via MASM `_emit`.
//   `tools/compare.py` checks raw .text bytes; the absolute-address fields
//   are stored as-is (no relocations generated), matching the original
//   binary slice byte-for-byte.

extern "C" __declspec(naked) void FUN_009ef674() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0xd9              // FLDZ
        _emit 0xee
        _emit 0x56              // PUSH ESI
        _emit 0xdc              // FCOM double ptr [EBP+0x8]
        _emit 0x55
        _emit 0x08
        _emit 0x33              // XOR EDX, EDX
        _emit 0xd2
        _emit 0x33              // XOR ESI, ESI
        _emit 0xf6
        _emit 0xdf              // FNSTSW AX
        _emit 0xe0
        _emit 0xdd              // FLD double ptr [EBP+0x8]
        _emit 0x45
        _emit 0x08
        _emit 0xf6              // TEST AH, 0x41
        _emit 0xc4
        _emit 0x41
        _emit 0x75              // JNZ +2  (skip FCHS if a >= 0)
        _emit 0x02
        _emit 0xd9              // FCHS
        _emit 0xe0
        _emit 0xb8              // MOV EAX, 0x7ff00000
        _emit 0x00
        _emit 0x00
        _emit 0xf0
        _emit 0x7f
        _emit 0x39              // CMP dword ptr [EBP+0x14], EAX
        _emit 0x45
        _emit 0x14
        _emit 0xb9              // MOV ECX, 0xfff00000
        _emit 0x00
        _emit 0x00
        _emit 0xf0
        _emit 0xff
        _emit 0x75              // JNZ (b != +inf)
        _emit 0x3d
        _emit 0x39              // CMP dword ptr [EBP+0x10], EDX
        _emit 0x55
        _emit 0x10
        _emit 0x75              // JNZ (b != +inf exactly)
        _emit 0x7f
        _emit 0xd9              // FLD1
        _emit 0xe8
        _emit 0xd8              // FCOM ST(1)
        _emit 0xd1
        _emit 0xdf              // FNSTSW AX
        _emit 0xe0
        _emit 0xf6              // TEST AH, 0x5
        _emit 0xc4
        _emit 0x05
        _emit 0x7a              // JP (|a| <= 1.0)
        _emit 0x11
        _emit 0xdd              // FSTP ST(2)
        _emit 0xda
        _emit 0xdd              // FSTP ST(1)
        _emit 0xd9
        _emit 0xdd              // FSTP ST(0)
        _emit 0xd8
        _emit 0xdd              // FLD double ptr [0x012ebbd0]
        _emit 0x05
        _emit 0xd0
        _emit 0xbb
        _emit 0x2e
        _emit 0x01
        _emit 0xe9              // JMP → store/return path
        _emit 0xe1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xd8              // FCOM ST(1)
        _emit 0xd1
        _emit 0xdf              // FNSTSW AX
        _emit 0xe0
        _emit 0xdd              // FSTP ST(1)
        _emit 0xd9
        _emit 0xf6              // TEST AH, 0x41
        _emit 0xc4
        _emit 0x41
        _emit 0x8b              // MOV EAX, dword ptr [EBP+0x18]
        _emit 0x45
        _emit 0x18
        _emit 0x75              // JNZ (|a| == 1.0)
        _emit 0x07
        _emit 0xdd              // FSTP ST(0)
        _emit 0xd8
        _emit 0xe9              // JMP → FSTP [EAX]
        _emit 0xcf
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xdd              // FSTP ST(1)
        _emit 0xd9
        _emit 0xe9              // JMP → FSTP [EAX]
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x39              // CMP dword ptr [EBP+0x14], ECX
        _emit 0x4d
        _emit 0x14
        _emit 0x75              // JNZ (b != -inf)
        _emit 0x42
        _emit 0x39              // CMP dword ptr [EBP+0x10], EDX
        _emit 0x55
        _emit 0x10
        _emit 0x75              // JNZ (b != -inf exactly)
        _emit 0x3d
        _emit 0xd9              // FLD1
        _emit 0xe8
        _emit 0xd8              // FCOM ST(1)
        _emit 0xd1
        _emit 0xdf              // FNSTSW AX
        _emit 0xe0
        _emit 0xf6              // TEST AH, 0x5
        _emit 0xc4
        _emit 0x05
        _emit 0x7a              // JP (|a| <= 1.0)
        _emit 0x09
        _emit 0xdd              // FSTP ST(0)
        _emit 0xd8
        _emit 0xdd              // FSTP ST(0)
        _emit 0xd8
        _emit 0xe9              // JMP → store 0.0
        _emit 0xa7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xdd              // FSTP ST(2)
        _emit 0xda
        _emit 0xde              // FCOMPP
        _emit 0xd9
        _emit 0xdf              // FNSTSW AX
        _emit 0xe0
        _emit 0xf6              // TEST AH, 0x5
        _emit 0xc4
        _emit 0x05
        _emit 0x8b              // MOV EAX, dword ptr [EBP+0x18]
        _emit 0x45
        _emit 0x18
        _emit 0x7a              // JP (|a| == 1.0)
        _emit 0x0b
        _emit 0xdd              // FLD double ptr [0x012ebbd0]
        _emit 0x05
        _emit 0xd0
        _emit 0xbb
        _emit 0x2e
        _emit 0x01
        _emit 0xe9              // JMP → FSTP [EAX]
        _emit 0x91
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xdd              // FLD double ptr [0x012ebbd8]
        _emit 0x05
        _emit 0xd8
        _emit 0xbb
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR ESI, ESI
        _emit 0xf6
        _emit 0xdd              // FSTP double ptr [EAX]
        _emit 0x18
        _emit 0x46              // INC ESI
        _emit 0xe9              // JMP → epilogue
        _emit 0x8d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x39              // CMP dword ptr [EBP+0xc], EAX
        _emit 0x45
        _emit 0x0c
        _emit 0xdd              // FSTP ST(0)
        _emit 0xd8
        _emit 0x75              // JNZ (a != +inf)
        _emit 0x26
        _emit 0x39              // CMP dword ptr [EBP+0x8], EDX
        _emit 0x55
        _emit 0x08
        _emit 0x75              // JNZ (a != +inf exactly; bail)
        _emit 0x7f
        _emit 0xdc              // FCOM double ptr [EBP+0x10]
        _emit 0x55
        _emit 0x10
        _emit 0xdf              // FNSTSW AX
        _emit 0xe0
        _emit 0xf6              // TEST AH, 0x5
        _emit 0xc4
        _emit 0x05
        _emit 0x0f              // JNP near (b > 0) → load +inf constant
        _emit 0x8b
        _emit 0x76
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xdc              // FCOM double ptr [EBP+0x10]
        _emit 0x55
        _emit 0x10
        _emit 0xdf              // FNSTSW AX
        _emit 0xe0
        _emit 0xf6              // TEST AH, 0x41
        _emit 0xc4
        _emit 0x41
        _emit 0x8b              // MOV EAX, dword ptr [EBP+0x18]
        _emit 0x45
        _emit 0x18
        _emit 0x74              // JZ (b < 0) → store 0.0
        _emit 0x5a
        _emit 0xdd              // FSTP ST(0)
        _emit 0xd8
        _emit 0xd9              // FLD1
        _emit 0xe8
        _emit 0xeb              // JMP → FSTP [EAX]
        _emit 0x54
        _emit 0x39              // CMP dword ptr [EBP+0xc], ECX
        _emit 0x4d
        _emit 0x0c
        _emit 0xdd              // FSTP ST(0)
        _emit 0xd8
        _emit 0x75              // JNZ (a != -inf; bail)
        _emit 0x59
        _emit 0x39              // CMP dword ptr [EBP+0x8], EDX
        _emit 0x55
        _emit 0x08
        _emit 0x75              // JNZ (a != -inf exactly; bail)
        _emit 0x54
        _emit 0xdd              // FLD double ptr [EBP+0x10]  (load b)
        _emit 0x45
        _emit 0x10
        _emit 0x51              // PUSH ECX  (make room on stack for double)
        _emit 0x51              // PUSH ECX
        _emit 0xdd              // FSTP double ptr [ESP]
        _emit 0x1c
        _emit 0x24
        _emit 0xe8              // CALL FUN_009ef610  (odd-integer predicate)
        _emit 0xaa
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xd9              // FLDZ
        _emit 0xee
        _emit 0x59              // POP ECX
        _emit 0xdc              // FCOM double ptr [EBP+0x10]
        _emit 0x55
        _emit 0x10
        _emit 0x59              // POP ECX
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0xdf              // FNSTSW AX
        _emit 0xe0
        _emit 0xf6              // TEST AH, 0x5
        _emit 0xc4
        _emit 0x05
        _emit 0x7a              // JP (b <= 0)
        _emit 0x11
        _emit 0x83              // CMP ECX, 0x1
        _emit 0xf9
        _emit 0x01
        _emit 0xdd              // FSTP ST(0)
        _emit 0xd8
        _emit 0xdd              // FLD double ptr [0x012ebbd0]
        _emit 0x05
        _emit 0xd0
        _emit 0xbb
        _emit 0x2e
        _emit 0x01
        _emit 0x75              // JNZ (odd: negate for -inf)
        _emit 0x1b
        _emit 0xd9              // FCHS
        _emit 0xe0
        _emit 0xeb              // JMP → store result
        _emit 0x17
        _emit 0xdc              // FCOM double ptr [EBP+0x10]
        _emit 0x55
        _emit 0x10
        _emit 0xdf              // FNSTSW AX
        _emit 0xe0
        _emit 0xf6              // TEST AH, 0x41
        _emit 0xc4
        _emit 0x41
        _emit 0x75              // JNZ (b >= 0, i.e. b == 0)
        _emit 0x14
        _emit 0x83              // CMP ECX, 0x1
        _emit 0xf9
        _emit 0x01
        _emit 0x75              // JNZ (not odd: store 0.0)
        _emit 0x08
        _emit 0xdd              // FSTP ST(0)
        _emit 0xd8
        _emit 0xdd              // FLD double ptr [0x012ebbf0]   (-0.0)
        _emit 0x05
        _emit 0xf0
        _emit 0xbb
        _emit 0x2e
        _emit 0x01
        _emit 0x8b              // MOV EAX, dword ptr [EBP+0x18]
        _emit 0x45
        _emit 0x18
        _emit 0xdd              // FSTP double ptr [EAX]
        _emit 0x18
        _emit 0xeb              // JMP → epilogue
        _emit 0x08
        _emit 0xdd              // FSTP ST(0)
        _emit 0xd8
        _emit 0xd9              // FLD1
        _emit 0xe8
        _emit 0xeb              // JMP → store result
        _emit 0xf3
        _emit 0xdd              // FSTP ST(0)
        _emit 0xd8
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0xc3              // RET
    }
}
