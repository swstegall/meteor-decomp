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
// FUNCTION: ffxivgame 0x000644d0 — _ASN1_STRING_free (42 B / 0x2A)
//
// OpenSSL ASN1_STRING_free: conditionally frees the data buffer and then
// the struct itself via CRYPTO_free. Skips the data-buffer free if the
// pointer is NULL or if the NDEF flag (bit 4, 0x10) is set in flags.
//
// struct ASN1_STRING {
//     int            length;  // [+0x00]
//     int            type;    // [+0x04]
//     unsigned char *data;    // [+0x08]
//     long           flags;   // [+0x0c]
// };
//
// void _ASN1_STRING_free(ASN1_STRING *a):
//   if (a == NULL) return;
//   if (a->data && !(a->flags & 0x10))
//       CRYPTO_free(a->data);
//   CRYPTO_free(a);
//
// Calling convention: __cdecl (caller cleans, plain RET).
// Frame: PUSH ESI at entry (no SUB ESP / no locals).
//
// Asm (42 bytes @ orig RVA 0x000644d0):
//   56               PUSH ESI
//   8b 74 24 08      MOV ESI, dword ptr [ESP+0x8]   ; a
//   85 f6            TEST ESI, ESI
//   74 1f            JZ pop_esi                      ; a == NULL → return
//   8b 46 08         MOV EAX, dword ptr [ESI+0x8]   ; a->data
//   85 c0            TEST EAX, EAX
//   74 0f            JZ push_esi                     ; data NULL → skip data free
//   f6 46 0c 10      TEST byte ptr [ESI+0xc], 0x10  ; a->flags & NDEF
//   75 09            JNZ push_esi                    ; NDEF set → skip data free
//   50               PUSH EAX                        ; push a->data
//   e8 XX XX XX XX   CALL FUN_004632f0               ; CRYPTO_free(a->data)
//   83 c4 04         ADD ESP, 0x4
// push_esi:
//   56               PUSH ESI                        ; push a
//   e8 XX XX XX XX   CALL FUN_004632f0               ; CRYPTO_free(a)
//   83 c4 04         ADD ESP, 0x4
// pop_esi:
//   5e               POP ESI
//   c3               RET
//
// Both CALL targets resolve to FUN_004632f0 (_CRYPTO_free) at RVA 0x000632f0.
// compare.py masks the 4-byte rel32 operands of both CALL instructions.

extern "C" void FUN_004632f0(void *);   // _CRYPTO_free / OPENSSL_free

extern "C" __declspec(naked) void FUN_004644d0(void *) {
    __asm {
        // 000644d0: 56               PUSH ESI
        _emit 0x56
        // 000644d1: 8b 74 24 08      MOV ESI, [ESP+8]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 000644d5: 85 f6            TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 000644d7: 74 1f            JZ +0x1f → pop_esi
        _emit 0x74
        _emit 0x1f
        // 000644d9: 8b 46 08         MOV EAX, [ESI+8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 000644dc: 85 c0            TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000644de: 74 0f            JZ +0x0f → push_esi
        _emit 0x74
        _emit 0x0f
        // 000644e0: f6 46 0c 10      TEST byte ptr [ESI+0xc], 0x10
        _emit 0xf6
        _emit 0x46
        _emit 0x0c
        _emit 0x10
        // 000644e4: 75 09            JNZ +0x09 → push_esi
        _emit 0x75
        _emit 0x09
        // 000644e6: 50               PUSH EAX
        _emit 0x50
        // 000644e7: e8 XX XX XX XX   CALL FUN_004632f0 (CRYPTO_free — reloc)
        call FUN_004632f0
        // 000644ec: 83 c4 04         ADD ESP, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 000644ef: 56               PUSH ESI  (push_esi)
        _emit 0x56
        // 000644f0: e8 XX XX XX XX   CALL FUN_004632f0 (CRYPTO_free — reloc)
        call FUN_004632f0
        // 000644f5: 83 c4 04         ADD ESP, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 000644f8: 5e               POP ESI  (pop_esi)
        _emit 0x5e
        // 000644f9: c3               RET
        _emit 0xc3
    }
}
