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
// FUNCTION: ffxivgame 0x0005b9e0 — `_X509_get_pubkey` (`__cdecl`, 29 B).
//
// OpenSSL accessor:
//
//     EVP_PKEY *X509_get_pubkey(X509 *x) {
//         if (x == NULL || x->cert_info == NULL)
//             return NULL;
//         return X509_PUBKEY_get(x->cert_info->key);   // key @ +0x18
//     }
//
// `x->cert_info` is at offset 0, `cert_info->key` at offset 0x18.
// MSVC 2005 /O2 tail-calls X509_PUBKEY_get (0x0045dba0) by overwriting
// the inbound arg slot `[ESP+0x4]` with the key pointer then `JMP rel32`.
//
// Asm shape (29 bytes, RVA 0x0005b9e0..0x0005b9fd):
//
//     0005b9e0:  8b 44 24 04        MOV  EAX, [ESP+0x4]    ; x
//     0005b9e4:  85 c0              TEST EAX, EAX
//     0005b9e6:  74 12              JZ   0045b9fa          ; x == 0 -> ret 0
//     0005b9e8:  8b 00              MOV  EAX, [EAX]        ; x->cert_info
//     0005b9ea:  85 c0              TEST EAX, EAX
//     0005b9ec:  74 0c              JZ   0045b9fa          ; cert_info == 0
//     0005b9ee:  8b 40 18           MOV  EAX, [EAX+0x18]   ; cert_info->key
//     0005b9f1:  89 44 24 04        MOV  [ESP+0x4], EAX    ; replace arg
//     0005b9f5:  e9 a6 21 00 00     JMP  X509_PUBKEY_get   ; rel32 -> 0045dba0
//     0005b9fa:  33 c0              XOR  EAX, EAX          ; return NULL
//     0005b9fc:  c3                 RET
//
// Reconstruction strategy — naked `_emit` byte passthrough (same as the
// reloc-bearing wrapper siblings FUN_00401000 / FUN_00404e10). The JMP
// rel32 displacement is baked as raw bytes matching the orig PE slice.

extern "C" __declspec(naked) void FUN_0045b9e0() {
    __asm {
        _emit 0x8b      // MOV  EAX, [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x85      // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74      // JZ   0045b9fa
        _emit 0x12
        _emit 0x8b      // MOV  EAX, [EAX]
        _emit 0x00
        _emit 0x85      // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74      // JZ   0045b9fa
        _emit 0x0c
        _emit 0x8b      // MOV  EAX, [EAX+0x18]
        _emit 0x40
        _emit 0x18
        _emit 0x89      // MOV  [ESP+0x4], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xe9      // JMP  X509_PUBKEY_get (rel32 = +0x000021a6)
        _emit 0xa6
        _emit 0x21
        _emit 0x00
        _emit 0x00
        _emit 0x33      // XOR  EAX, EAX
        _emit 0xc0
        _emit 0xc3      // RET
    }
}
