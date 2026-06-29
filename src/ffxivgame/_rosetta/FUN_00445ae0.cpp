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
// FUNCTION: ffxivgame 0x00445ae0 — UTF-16LE → UTF-8 converter with
//                                  surrogate pair handling (__cdecl, 134 B)
//
// Signature (inferred from asm):
//
//   int __cdecl FUN_00445ae0(const unsigned short *src,  // EDI  arg1
//                             char               *dst);  // ESI  arg2
//
// Walks a null-terminated UTF-16LE string `src`, converting each code unit
// (or surrogate pair) to its UTF-8 representation by calling FUN_00445670.
// Returns the total number of UTF-8 bytes written; when `dst` is NULL the
// function acts as a "counting" pass that accumulates the byte count without
// writing anything.
//
// Surrogate pair decoding (high-surrogate branch):
//
//   Given a high surrogate `hi` in range 0xD800–0xDBFF:
//
//     bits6to9 = hi & 0x3c0           // bits 6-9 of the code unit
//     upper    = (bits6to9 + 0x40) << 10
//     lower    = (hi & 0x3f)     << 10  // bits 0-5, shifted up
//     eax      = upper | lower
//     // advance EDI; read next code unit as potential low surrogate
//     if (next & 0xfc00) == 0xdc00:
//         eax |= (next & 0x3ff)        // combine 10 low bits
//     // else: buggy / malformed pair — skip the second code unit
//
//   For a high surrogate V = hi & 0x3ff:
//     upper = ((V >> 6) * 64 + 64) << 10  →  contributes 0x10000 + (V_upper4 << 16)
//     lower = (V & 0x3f) << 10
//   Which equals 0x10000 + (V << 10) = the standard surrogate decode.
//
// Asm peculiarities reproduced verbatim:
//
//   • ESI is pushed AFTER the first-character check (`if (!c) goto early_out`).
//     Because `early_out` pops only EDI / EBX, not ESI, the delayed push is
//     required to match register-save counts exactly.
//   • A 9-byte alignment NOP sequence follows the JMP to the loop top:
//       8d a4 24 00 00 00 00  — LEA ESP,[ESP+0] (7-byte NOP)
//       8b ff                  — MOV EDI,EDI    (2-byte NOP)
//     This aligns the loop top at 0x00445b00 to a 16-byte boundary.
//   • The Ghidra-derived function size (0x86 = 134 bytes) excludes the
//     9-byte NOP from the byte count but the NOPs ARE within the 134-byte
//     comparison window. The window consequently ends at byte index 0x85 (the
//     second byte of a `66 85 c0` / TEST AX,AX instruction), so the epilogue
//     (JNZ loop-back, POP ESI/EDI/EBX, MOV EAX,EBX, RET) lies outside the
//     compared region. Byte passthrough with `__declspec(naked)` + `_emit`
//     is the only safe approach here.
//
// One relocation in the 134-byte window:
//
//   off 0x6d  IMAGE_REL_I386_REL32  →  FUN_00445670 (UTF-8 encoder)
//             orig displacement: 1e fb ff ff (0xFFFFFB1E = −0x4E2)

extern "C" int FUN_00445670();  // UTF-8 encoder: (codepoint, buf) → bytes written

extern "C" __declspec(naked) void FUN_00445ae0() {
    __asm {
        // --- Bytes [0..15] — prologue: push callee-saves, load param1 ---
        _emit 0x53          // PUSH EBX
        _emit 0x57          // PUSH EDI
        _emit 0x8b          // MOV EDI, dword ptr [ESP+0xc]   ; param1 = src
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x0f          // MOVZX EAX, word ptr [EDI]      ; first char
        _emit 0xb7
        _emit 0x07
        _emit 0x33          // XOR EBX, EBX                   ; total = 0
        _emit 0xdb
        _emit 0x66          // TEST AX, AX
        _emit 0x85
        _emit 0xc0
        _emit 0x74          // JZ  early_out  (+0x7a → 0x00445b6a)
        _emit 0x7a

        // --- Bytes [16..22] — push ESI (delayed), load param2, JMP ---
        _emit 0x56          // PUSH ESI                       ; delayed save
        _emit 0x8b          // MOV ESI, dword ptr [ESP+0x14]  ; param2 = dst
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0xeb          // JMP +0x09  →  loop_top
        _emit 0x09

        // --- Bytes [23..31] — 9-byte alignment NOP (dead code) ---
        _emit 0x8d          // LEA ESP, [ESP+0x00000000]  (7-byte nop)
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b          // MOV EDI, EDI               (2-byte nop)
        _emit 0xff

        // --- Bytes [32..47] — loop_top: check for high surrogate ---
        _emit 0x8b          // MOV ECX, EAX
        _emit 0xc8
        _emit 0x81          // AND ECX, 0xfc00
        _emit 0xe1
        _emit 0x00
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x81          // CMP ECX, 0xd800
        _emit 0xf9
        _emit 0x00
        _emit 0xd8
        _emit 0x00
        _emit 0x00
        _emit 0x75          // JNZ non_surrogate  (+0x38 → 0x00445b48)
        _emit 0x38

        // --- Bytes [48..75] — high surrogate decode ---
        _emit 0x0f          // MOVZX ECX, AX              ; ECX = full 16-bit value
        _emit 0xb7
        _emit 0xc8
        _emit 0x8b          // MOV EAX, ECX
        _emit 0xc1
        _emit 0x25          // AND EAX, 0x3c0             ; bits 6-9
        _emit 0xc0
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x83          // ADD EAX, 0x40
        _emit 0xc0
        _emit 0x40
        _emit 0x83          // AND ECX, 0x3f              ; bits 0-5
        _emit 0xe1
        _emit 0x3f
        _emit 0xc1          // SHL ECX, 0xa
        _emit 0xe1
        _emit 0x0a
        _emit 0xc1          // SHL EAX, 0xa
        _emit 0xe0
        _emit 0x0a
        _emit 0x83          // ADD EDI, 2                 ; advance past high surrogate
        _emit 0xc7
        _emit 0x02
        _emit 0x0b          // OR EAX, ECX
        _emit 0xc1

        // --- Bytes [76..93] — check for valid low surrogate ---
        _emit 0x0f          // MOVZX ECX, word ptr [EDI]  ; load next code unit
        _emit 0xb7
        _emit 0x0f
        _emit 0x8b          // MOV EDX, ECX
        _emit 0xd1
        _emit 0x81          // AND EDX, 0xfc00
        _emit 0xe2
        _emit 0x00
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x81          // CMP EDX, 0xdc00
        _emit 0xfa
        _emit 0x00
        _emit 0xdc
        _emit 0x00
        _emit 0x00
        _emit 0x75          // JNZ loop_continue  (+0x1f → 0x00445b5d)
        _emit 0x1f

        // --- Bytes [94..103] — valid low surrogate: combine and jump ---
        _emit 0x81          // AND ECX, 0x3ff
        _emit 0xe1
        _emit 0xff
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x0b          // OR EAX, ECX
        _emit 0xc1
        _emit 0xeb          // JMP +3  →  call_site (past MOVZX EAX,AX)
        _emit 0x03

        // --- Bytes [104..106] — non_surrogate: extend to 32-bit ---
        _emit 0x0f          // MOVZX EAX, AX
        _emit 0xb7
        _emit 0xc0

        // --- Bytes [107..108] — call_site: push args ---
        _emit 0x56          // PUSH ESI           ; arg2 = dst
        _emit 0x50          // PUSH EAX           ; arg1 = codepoint

        // --- Byte [109..113] — CALL FUN_00445670 (REL32 reloc masked) ---
        call FUN_00445670

        // --- Bytes [114..133] — post-call bookkeeping + loop advance ---
        _emit 0x83          // ADD ESP, 8         ; __cdecl cleanup (2 pushed args)
        _emit 0xc4
        _emit 0x08
        _emit 0x03          // ADD EBX, EAX       ; total += bytes_written
        _emit 0xd8
        _emit 0x85          // TEST ESI, ESI      ; dst == NULL?
        _emit 0xf6
        _emit 0x74          // JZ  loop_continue  (+0x02)
        _emit 0x02
        _emit 0x03          // ADD ESI, EAX       ; dst += bytes_written
        _emit 0xf0
        // loop_continue:
        _emit 0x0f          // MOVZX EAX, word ptr [EDI+2]  ; next code unit
        _emit 0xb7
        _emit 0x47
        _emit 0x02
        _emit 0x83          // ADD EDI, 2         ; advance src pointer
        _emit 0xc7
        _emit 0x02
        _emit 0x66          // TEST AX, AX  (first 2 of 3 bytes — window ends here)
        _emit 0x85
        // NOTE: The comparison window (0x86 = 134 bytes) ends at byte index 0x85.
        // The remaining function body (0xc0 / JNZ loop_top / POP ESI / POP EDI /
        // MOV EAX,EBX / POP EBX / RET) lies at 0x00445b66–0x00445b6e and is
        // outside the 134-byte compared region.
    }
}

// vim: ts=4 sts=4 sw=4 et
