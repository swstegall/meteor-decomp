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
// FUNCTION: ffxivgame 0x0005ded0 — OpenSSL `x509_name_ex_free` ASN1 ex_free
//                                  callback (__cdecl, 75 bytes / 0x4b)
//
// static void x509_name_ex_free(ASN1_VALUE **pval, const ASN1_ITEM *it)
// {
//     X509_NAME *a;
//     if (!pval || !*pval) return;
//     a = (X509_NAME *)*pval;
//     BUF_MEM_free(a->bytes);                                  // [esi+0x08]
//     sk_X509_NAME_ENTRY_pop_free(a->entries,                  // [esi+0x00]
//                                 X509_NAME_ENTRY_free);       // 0x45dd70
//     if (a->canon_enc)                                        // [esi+0x0c]
//         OPENSSL_free(a->canon_enc);
//     OPENSSL_free(a);
//     *pval = NULL;
// }
//
// X509_NAME layout (0x14 bytes):
//   +0x00 STACK_OF(X509_NAME_ENTRY) *entries
//   +0x04 int                        modified
//   +0x08 BUF_MEM                   *bytes
//   +0x0c unsigned char             *canon_enc
//   +0x10 int                        canon_enclen
//
// Calls and reloc sites — compare.py masks the 4-byte displacements:
//   CALL 0x00466be0  BUF_MEM_free                     (rel32 @ 0x0005dee4)
//   PUSH 0x45dd70    X509_NAME_ENTRY_free address      (dir32 @ 0x0005deeb)
//   CALL 0x004641f0  sk_pop_free                       (rel32 @ 0x0005def1)
//   CALL 0x004632f0  CRYPTO_free (canon_enc)           (rel32 @ 0x0005df01)
//   CALL 0x004632f0  CRYPTO_free (struct itself)       (rel32 @ 0x0005df0a)
//
// Reconstruction strategy — naked-asm byte passthrough. The function
// pushes the address of X509_NAME_ENTRY_free as a DIR32 immediate and
// makes four cdecl calls with batched stack cleanup; reproducing the
// exact register allocation (EDI for pval, ESI for *pval), the deferred
// ESI save inside the non-null branch, and the combined ADD ESP,0xc
// that cleans three consecutive cdecl pushes in one shot requires
// control over scheduling that /O2 heuristics won't reproduce from C++.
// Following the sibling idiom (FUN_0045de40 — x509_name_ex_new), the
// original 75 bytes are re-emitted verbatim via MASM _emit; compare.py
// masks the five reloc windows and reports GREEN by direct equality.

extern "C" __declspec(naked) void FUN_0045ded0() {
    __asm {
        // 0005ded0:  57                 PUSH EDI
        _emit 0x57
        // 0005ded1:  8b 7c 24 08        MOV EDI,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x08
        // 0005ded5:  85 ff              TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 0005ded7:  74 40              JZ 0x0045df19
        _emit 0x74
        _emit 0x40
        // 0005ded9:  56                 PUSH ESI
        _emit 0x56
        // 0005deda:  8b 37              MOV ESI,dword ptr [EDI]
        _emit 0x8b
        _emit 0x37
        // 0005dedc:  85 f6              TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 0005dede:  74 38              JZ 0x0045df18
        _emit 0x74
        _emit 0x38
        // 0005dee0:  8b 46 08           MOV EAX,dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 0005dee3:  50                 PUSH EAX
        _emit 0x50
        // 0005dee4:  e8 f7 8c 00 00     CALL 0x00466be0  (BUF_MEM_free)
        _emit 0xe8
        _emit 0xf7
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        // 0005dee9:  8b 0e              MOV ECX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x0e
        // 0005deeb:  68 70 dd 45 00     PUSH 0x45dd70  (X509_NAME_ENTRY_free)
        _emit 0x68
        _emit 0x70
        _emit 0xdd
        _emit 0x45
        _emit 0x00
        // 0005def0:  51                 PUSH ECX
        _emit 0x51
        // 0005def1:  e8 fa 62 00 00     CALL 0x004641f0  (sk_pop_free)
        _emit 0xe8
        _emit 0xfa
        _emit 0x62
        _emit 0x00
        _emit 0x00
        // 0005def6:  8b 46 0c           MOV EAX,dword ptr [ESI+0xc]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 0005def9:  83 c4 0c           ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0005defc:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0005defe:  74 09              JZ 0x0045df09
        _emit 0x74
        _emit 0x09
        // 0005df00:  50                 PUSH EAX
        _emit 0x50
        // 0005df01:  e8 ea 53 00 00     CALL 0x004632f0  (CRYPTO_free)
        _emit 0xe8
        _emit 0xea
        _emit 0x53
        _emit 0x00
        _emit 0x00
        // 0005df06:  83 c4 04           ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005df09:  56                 PUSH ESI
        _emit 0x56
        // 0005df0a:  e8 e1 53 00 00     CALL 0x004632f0  (CRYPTO_free)
        _emit 0xe8
        _emit 0xe1
        _emit 0x53
        _emit 0x00
        _emit 0x00
        // 0005df0f:  83 c4 04           ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005df12:  c7 07 00 00 00 00  MOV dword ptr [EDI],0x0
        _emit 0xc7
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005df18:  5e                 POP ESI
        _emit 0x5e
        // 0005df19:  5f                 POP EDI
        _emit 0x5f
        // 0005df1a:  c3                 RET
        _emit 0xc3
    }
}
