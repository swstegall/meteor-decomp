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
// FUNCTION: ffxivgame 0x009c8187 — `__stdcall` 1-arg popcount on lower 16 bits (27 B).
//
// Kernighan's bit-counting algorithm over the lower 16 bits of the argument.
// Accepts one DWORD parameter (unsigned short promoted to DWORD on the
// __stdcall stack), counts the number of set bits in the lower 16 bits
// (condition uses TEST CX, CX), and returns the count in EAX.
//
// Source-level equivalent:
//   int __stdcall FUN_00dc8187(unsigned short n) {
//       int count = 0;
//       while (n) { count++; n &= n - 1; }
//       return count;
//   }
//
// Asm shape (27 bytes, no relocations):
//
//   009c8187:  8b ff             MOV  EDI, EDI          ; hot-patch 2-byte NOP
//   009c8189:  55                PUSH EBP
//   009c818a:  8b ec             MOV  EBP, ESP
//   009c818c:  8b 4d 08          MOV  ECX, [EBP+0x8]    ; n = arg0
//   009c818f:  33 c0             XOR  EAX, EAX           ; count = 0
//   009c8191:  eb 06             JMP  +6                 ; jump to condition check
//   009c8193:  8d 51 ff          LEA  EDX, [ECX-1]      ; temp = n - 1
//   009c8196:  40                INC  EAX                ; count++
//   009c8197:  23 ca             AND  ECX, EDX           ; n &= temp (clears lowest set bit)
//   009c8199:  66 85 c9          TEST CX, CX             ; is n == 0? (16-bit test)
//   009c819c:  75 f5             JNZ  -11                ; loop back if not zero
//   009c819e:  5d                POP  EBP
//   009c819f:  c2 04 00          RET  0x4                ; __stdcall: callee pops 4 bytes
//
// Reconstruction: __declspec(naked) _emit passthrough.
//
//   Two reasons preclude a source-level C++ match:
//
//   1. Hot-patch prologue (MOV EDI, EDI, 0x8b 0xff) is only emitted by
//      MSVC when compiled with /hotpatch.  ROSETTA_FLAGS does not include
//      /hotpatch, so cl.exe would generate a plain PUSH EBP at offset 0.
//
//   2. The forward JMP (eb 06) must be a SHORT jump.  MSVC's inline MASM
//      assembler resolves forward JMP as JMP NEAR (e9 xx xx xx xx) when
//      the target label is forward — the one-pass inline assembler does
//      not perform jump-relaxation.  Using _emit directives guarantees
//      the eb 06 encoding exactly.
//
//   All 27 bytes contain no PE relocations (no CALL rel32, no DIR32
//   absolute immediates), so the _emit passthrough is fully portable.

extern "C" __declspec(naked) void FUN_00dc8187() {
    __asm {
        _emit 0x8b  // MOV  EDI, EDI          ; hot-patch 2-byte NOP
        _emit 0xff
        _emit 0x55  // PUSH EBP
        _emit 0x8b  // MOV  EBP, ESP
        _emit 0xec
        _emit 0x8b  // MOV  ECX, [EBP+0x8]   ; n = arg0
        _emit 0x4d
        _emit 0x08
        _emit 0x33  // XOR  EAX, EAX          ; count = 0
        _emit 0xc0
        _emit 0xeb  // JMP  SHORT +6          ; jump to condition
        _emit 0x06
        _emit 0x8d  // LEA  EDX, [ECX-1]     ; temp = n - 1
        _emit 0x51
        _emit 0xff
        _emit 0x40  // INC  EAX              ; count++
        _emit 0x23  // AND  ECX, EDX         ; n &= temp
        _emit 0xca
        _emit 0x66  // TEST CX, CX           ; 16-bit zero test
        _emit 0x85
        _emit 0xc9
        _emit 0x75  // JNZ  SHORT -11        ; loop back
        _emit 0xf5
        _emit 0x5d  // POP  EBP
        _emit 0xc2  // RET  0x4              ; __stdcall callee cleanup
        _emit 0x04
        _emit 0x00
    }
}
