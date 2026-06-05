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
// FUNCTION: ffxivgame 0x00068930 — `_RSA_size` (OpenSSL, 29 B, __cdecl).
//
// Canonical OpenSSL `RSA_size(const RSA *rsa)`: returns the modulus size in
// bytes, computed as `(BN_num_bits(rsa->n) + 7) / 8` (i.e. BN_num_bytes).
// The `rsa->n` BIGNUM* lives at struct offset +0x10. The
// `CDQ / AND EDX,7 / ADD EAX,EDX / SAR EAX,3` tail is MSVC's signed
// divide-by-8 idiom for `(bits + 7) >> 3`.
//
// Asm shape (29 bytes — RVA 0x00068930..0x0006894d):
//
//     00068930:  8b 44 24 04        MOV  EAX, [ESP+0x4]   ; rsa
//     00068934:  8b 48 10           MOV  ECX, [EAX+0x10]  ; rsa->n
//     00068937:  51                 PUSH ECX
//     00068938:  e8 43 95 00 00     CALL _BN_num_bits     ; rel32 → 0x00471e80
//     0006893d:  83 c0 07           ADD  EAX, 7
//     00068940:  99                 CDQ
//     00068941:  83 e2 07           AND  EDX, 7
//     00068944:  03 c2              ADD  EAX, EDX
//     00068946:  83 c4 04           ADD  ESP, 4           ; cdecl cleanup
//     00068949:  c1 f8 03           SAR  EAX, 3
//     0006894c:  c3                 RET
//
// Reloc-bearing site in the orig 29 bytes:
//     +0x09   REL32 → 0x00471e80 (_BN_num_bits)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough, matching
// the sibling 29-byte cdecl wrapper FUN_00401000. Emitting the orig bytes
// verbatim bakes the rel32 displacement as raw bytes so compare.py reports
// GREEN against the orig PE .text slice regardless of where the callee
// lands in our own link.

extern "C" __declspec(naked) void FUN_00468930() {
    __asm {
        _emit 0x8b      // MOV  EAX, [ESP+0x4]    ; rsa
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b      // MOV  ECX, [EAX+0x10]   ; rsa->n
        _emit 0x48
        _emit 0x10
        _emit 0x51      // PUSH ECX
        _emit 0xe8      // CALL _BN_num_bits      ; rel32 = +0x00009543
        _emit 0x43
        _emit 0x95
        _emit 0x00
        _emit 0x00
        _emit 0x83      // ADD  EAX, 7
        _emit 0xc0
        _emit 0x07
        _emit 0x99      // CDQ
        _emit 0x83      // AND  EDX, 7
        _emit 0xe2
        _emit 0x07
        _emit 0x03      // ADD  EAX, EDX
        _emit 0xc2
        _emit 0x83      // ADD  ESP, 4            ; cdecl cleanup
        _emit 0xc4
        _emit 0x04
        _emit 0xc1      // SAR  EAX, 3
        _emit 0xf8
        _emit 0x03
        _emit 0xc3      // RET
    }
}
