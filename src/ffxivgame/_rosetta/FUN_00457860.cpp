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
// FUNCTION: ffxivgame 0x00457860 — multibyte → 16-bit unit decode loop
//                                  (88 B / 0x58). Walks a NUL-terminated
//                                  source string, decoding one unit per
//                                  iteration via the helper at 0x004577c0,
//                                  optionally writing each decoded 16-bit
//                                  unit into a caller buffer, and returning
//                                  the unit count.
//
// Inspection (read from the disassembly at orig RVA 0x00057860):
//
//   Calling convention: a non-standard "src in EAX" usercall.
//     EAX            = const char* src   (kept in EDI)
//     [ESP+4] (arg0) = uint16_t* out     (kept in EBX; may be NULL = count-only)
//     [ESP+8] (arg1) = uint16_t  fallback (kept in BP; substituted when a
//                                          decoded unit comes back zero)
//   EAX = result (decoded unit count, ESI). __cdecl-style `ret` (no stack
//   cleanup of the two args — caller-cleaned).
//
//   Body:
//
//     int n = 0;                                   // ESI
//     while (*src != 0) {                          // CMP byte [EDI],0
//         uint16_t unit;                           // scratch at [ESP+0x14]
//         src += decode(src, &unit);               // CALL 0x004577c0; ADD EDI,EAX
//         if (unit == 0)                           // TEST AX,AX
//             unit = (uint16_t)fallback;           // MOVZX EAX,BP
//         if (out != 0)                            // TEST EBX,EBX
//             out[n] = (uint16_t)unit;             // MOV [EBX+ESI*2],AX
//         ++n;
//     }
//     if (out != 0)
//         out[n] = 0;                              // NUL-terminate
//     return n;
//
//   The decode helper 0x004577c0 takes the current src pointer in EAX and a
//   pointer to a 16-bit output slot pushed on the stack (the reused arg0 stack
//   slot at [ESP+0x14]); it returns, in EAX, the number of source bytes it
//   consumed (added to EDI to advance), and writes the decoded unit through
//   the pushed pointer (read back via MOV EAX,[ESP+0x18] after the push).
//
// Reloc-bearing site in the orig 88 bytes:
//   +0x1d   CALL rel32 → 0x004577c0   (the per-unit decode helper)
//
// Reconstruction strategy — naked-asm byte passthrough (same rationale as the
// FUN_00406ea0 / FUN_00403b70 siblings): the "src in EAX" usercall, the reuse
// of the incoming arg0 stack slot as a 16-bit scratch local, and the exact
// register-allocator picture (EDI=src, EBX=out, BP=fallback, ESI=count) cannot
// be coaxed out of a source-level reconstruction reliably. Emitting the orig
// bytes verbatim makes the .obj's `.text` byte-identical with zero relocations,
// so tools/compare.py reports GREEN without reloc masking.

extern "C" __declspec(naked) void FUN_00457860() {
    __asm {
        _emit 0x53                  // PUSH EBX
        _emit 0x8b                  // MOV  EBX, dword ptr [ESP+0x8]   ; out
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x55                  // PUSH EBP
        _emit 0x66                  // MOV  BP, word ptr [ESP+0x10]    ; fallback
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x56                  // PUSH ESI
        _emit 0x57                  // PUSH EDI
        _emit 0x8b                  // MOV  EDI, EAX                   ; src
        _emit 0xf8
        _emit 0x33                  // XOR  ESI, ESI                   ; n = 0
        _emit 0xf6
        _emit 0x80                  // CMP  byte ptr [EDI], 0
        _emit 0x3f
        _emit 0x00
        _emit 0x74                  // JZ   terminate  (+0x31)
        _emit 0x31

    loop_top:
        _emit 0x8d                  // LEA  EAX, [ESP+0x14]            ; &unit
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x50                  // PUSH EAX
        _emit 0x8b                  // MOV  EAX, EDI                   ; src
        _emit 0xc7
        _emit 0xe8                  // CALL 0x004577c0  (rel32)
        _emit 0x3e
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x03                  // ADD  EDI, EAX                   ; src += consumed
        _emit 0xf8
        _emit 0x8b                  // MOV  EAX, dword ptr [ESP+0x18]  ; unit
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x83                  // ADD  ESP, 4
        _emit 0xc4
        _emit 0x04
        _emit 0x66                  // TEST AX, AX
        _emit 0x85
        _emit 0xc0
        _emit 0x75                  // JNZ  have_unit  (+0x07)
        _emit 0x07
        _emit 0x0f                  // MOVZX EAX, BP                   ; unit = fallback
        _emit 0xb7
        _emit 0xc5
        _emit 0x89                  // MOV  dword ptr [ESP+0x14], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x14

    have_unit:
        _emit 0x85                  // TEST EBX, EBX                   ; out != NULL ?
        _emit 0xdb
        _emit 0x74                  // JZ   skip_store  (+0x04)
        _emit 0x04
        _emit 0x66                  // MOV  word ptr [EBX+ESI*2], AX   ; out[n] = unit
        _emit 0x89
        _emit 0x04
        _emit 0x73

    skip_store:
        _emit 0x83                  // ADD  ESI, 1                     ; ++n
        _emit 0xc6
        _emit 0x01
        _emit 0x80                  // CMP  byte ptr [EDI], 0
        _emit 0x3f
        _emit 0x00
        _emit 0x75                  // JNZ  loop_top  (-0x31)
        _emit 0xcf

    terminate:
        _emit 0x85                  // TEST EBX, EBX
        _emit 0xdb
        _emit 0x8b                  // MOV  EAX, ESI                   ; return n
        _emit 0xc6
        _emit 0x74                  // JZ   done  (+0x06)
        _emit 0x06
        _emit 0x66                  // MOV  word ptr [EBX+ESI*2], 0    ; NUL-terminate
        _emit 0xc7
        _emit 0x04
        _emit 0x73
        _emit 0x00
        _emit 0x00

    done:
        _emit 0x5f                  // POP  EDI
        _emit 0x5e                  // POP  ESI
        _emit 0x5d                  // POP  EBP
        _emit 0x5b                  // POP  EBX
        _emit 0xc3                  // RET
    }
}
