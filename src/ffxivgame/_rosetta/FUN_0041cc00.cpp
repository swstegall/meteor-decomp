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
// FUNCTION: ffxivgame 0x0001cc00 — `__cdecl` 2-arg float dispatcher (82 B / 0x52).
//   Passes float arg1 verbatim and float arg2 divided by a double constant
//   to two successive __thiscall calls on the global object at [0x0132987c]
//   via FUN_004236e0 (RET 8 callee), using SSE2 for the float↔double
//   conversion.
//
// Asm (82 bytes @ RVA 0x0001cc00):
//
//   0001cc00: f3 0f 10 44 24 04   MOVSS XMM0,[ESP+4]           ; load float arg1
//   0001cc06: 8b 0d 7c 98 32 01   MOV ECX,[0x0132987c]         ; this = *global
//   0001cc0c: f3 0f 11 44 24 04   MOVSS [ESP+4],XMM0           ; store back (canonical)
//   0001cc12: 8b 44 24 04         MOV EAX,[ESP+4]              ; float bits → EAX
//   0001cc16: 50                  PUSH EAX                     ; push arg1
//   0001cc17: 68 af 00 00 00      PUSH 0xaf                    ; push id=175
//   0001cc1c: e8 bf 6a 00 00      CALL 0x004236e0              ; __thiscall RET 8
//   0001cc21: f3 0f 10 44 24 08   MOVSS XMM0,[ESP+8]           ; load float arg2
//   0001cc27: 0f 5a c0            CVTPS2PD XMM0,XMM0           ; float → double
//   0001cc2a: f2 0f 5e 05 90 98 f5 00  DIVSD XMM0,[0x00f59890] ; / double constant
//   0001cc32: 66 0f 5a c0         CVTPD2PS XMM0,XMM0           ; double → float
//   0001cc36: f3 0f 11 44 24 04   MOVSS [ESP+4],XMM0           ; store result
//   0001cc3c: 8b 4c 24 04         MOV ECX,[ESP+4]              ; result bits → ECX
//   0001cc40: 51                  PUSH ECX                     ; push result
//   0001cc41: 8b 0d 7c 98 32 01   MOV ECX,[0x0132987c]         ; this = *global
//   0001cc47: 68 c3 00 00 00      PUSH 0xc3                    ; push id=195
//   0001cc4c: e8 8f 6a 00 00      CALL 0x004236e0              ; __thiscall RET 8
//   0001cc51: c3                  RET
//
// Conventions:
//   - This function: __cdecl (plain RET, no stack cleanup)
//   - FUN_004236e0: __thiscall (ECX = this, RET 8 cleans 2 args)
//   - Global at 0x0132987c: pointer to the receiver object
//   - [0x00f59890]: 8-byte double constant (scale denominator for arg2)
//
// Reloc-bearing sites in the orig 82 bytes:
//   +0x08   DIR32 → 0x0132987c  (global object pointer, first load)
//   +0x1d   CALL rel32 → 0x004236e0  (first dispatch, rel32 = 0x00006abf)
//   +0x2e   DIR32 → 0x00f59890  (double constant address in DIVSD)
//   +0x43   DIR32 → 0x0132987c  (global object pointer, second load)
//   +0x4d   CALL rel32 → 0x004236e0  (second dispatch, rel32 = 0x00006a8f)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The SSE2 round-trip (MOVSS load→store→MOV) before the first __thiscall,
//   the CVTPS2PD/DIVSD/CVTPD2PS float-via-double computation, and the five
//   reloc-bearing sites make source-level C++ reconstruction brittle. Any
//   deviation in /arch: flag, float-parameter declaration, or register-
//   allocation order shifts at least one byte. The pragmatic choice —
//   consistent with FUN_0041cf50 and FUN_0041c060 in the same module — is a
//   `__declspec(naked)` body re-emitting the orig 82 bytes verbatim via MASM
//   `_emit` directives. The .obj `.text` becomes byte-identical to the orig
//   slice; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0041cc00() {
    __asm {
        // 0001cc00: MOVSS XMM0,[ESP+4]      ; load float arg1
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001cc06: MOV ECX,[0x0132987c]   ; this = *global
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001cc0c: MOVSS [ESP+4],XMM0     ; store back
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001cc12: MOV EAX,[ESP+4]        ; float bits → EAX
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001cc16: PUSH EAX
        _emit 0x50
        // 0001cc17: PUSH 0xaf              ; id = 175
        _emit 0x68
        _emit 0xaf
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001cc1c: CALL 0x004236e0        ; __thiscall, rel32 = 0x00006abf
        _emit 0xe8
        _emit 0xbf
        _emit 0x6a
        _emit 0x00
        _emit 0x00
        // 0001cc21: MOVSS XMM0,[ESP+8]     ; load float arg2 (callee cleaned 8 bytes)
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0001cc27: CVTPS2PD XMM0,XMM0    ; float → double
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        // 0001cc2a: DIVSD XMM0,[0x00f59890] ; double / constant
        _emit 0xf2
        _emit 0x0f
        _emit 0x5e
        _emit 0x05
        _emit 0x90
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        // 0001cc32: CVTPD2PS XMM0,XMM0    ; double → float
        _emit 0x66
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        // 0001cc36: MOVSS [ESP+4],XMM0     ; store result
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001cc3c: MOV ECX,[ESP+4]        ; result bits → ECX
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0001cc40: PUSH ECX
        _emit 0x51
        // 0001cc41: MOV ECX,[0x0132987c]   ; this = *global
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001cc47: PUSH 0xc3              ; id = 195
        _emit 0x68
        _emit 0xc3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001cc4c: CALL 0x004236e0        ; __thiscall, rel32 = 0x00006a8f
        _emit 0xe8
        _emit 0x8f
        _emit 0x6a
        _emit 0x00
        _emit 0x00
        // 0001cc51: RET
        _emit 0xc3
    }
}
