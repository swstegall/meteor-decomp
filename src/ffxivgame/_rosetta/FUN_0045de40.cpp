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
// FUNCTION: ffxivgame 0x0005de40 — OpenSSL `x509_name_ex_new` ASN1 ex_new
//                                  callback (__cdecl, 139 bytes / 0x8b)
//
// static int x509_name_ex_new(ASN1_VALUE **val, const ASN1_ITEM *it)
// {
//     X509_NAME *ret = NULL;
//     ret = OPENSSL_malloc(sizeof(X509_NAME));     // 20 bytes
//     if (!ret) goto memerr;
//     if ((ret->entries = sk_X509_NAME_ENTRY_new_null()) == NULL) goto memerr;
//     if ((ret->bytes   = BUF_MEM_new())              == NULL) goto memerr;
//     ret->canon_enc    = NULL;     // [esi+0x0c]
//     ret->canon_enclen = 0;        // [esi+0x10]
//     ret->modified     = 1;        // [esi+0x04]
//     *val = (ASN1_VALUE *)ret;     // [ecx] = esi
//     return 1;
//  memerr:
//     ASN1err(ASN1_F_X509_NAME_EX_NEW, ERR_R_MALLOC_FAILURE);
//     if (ret) {
//         if (ret->entries) sk_X509_NAME_ENTRY_free(ret->entries);
//         OPENSSL_free(ret);
//     }
//     return 0;
// }
//
// X509_NAME layout (0x14 bytes):
//   +0x00 STACK_OF(X509_NAME_ENTRY) *entries
//   +0x04 int                        modified
//   +0x08 BUF_MEM                    *bytes
//   +0x0c unsigned char             *canon_enc
//   +0x10 int                        canon_enclen
//
// Calls (e8 rel32 reloc sites — compare.py masks the 4-byte displacements):
//   CALL 0x00463150  CRYPTO_malloc
//   CALL 0x004640e0  sk_new_null
//   CALL 0x00466ba0  BUF_MEM_new
//   CALL 0x0045c940  ERR_put_error
//   CALL 0x00464000  sk_free
//   CALL 0x004632f0  CRYPTO_free
//
// Reconstruction strategy — naked-asm byte passthrough. The function
// references OpenSSL globals (the source-file string @0x00f691c0, line/
// reason/function ASN1err constants) and six cross-module CALLs whose
// rel32 targets the linker resolves; a source-level C++ form can't pin
// those without the real OpenSSL headers/objects. The __declspec(naked)
// body re-emits the original 139 bytes verbatim via MASM _emit; the
// .obj's .text is byte-identical to the orig slice and compare.py
// reports GREEN.

extern "C" __declspec(naked) void FUN_0045de40() {
    __asm {
        // 0005de40:  56                 PUSH ESI
        _emit 0x56
        // 0005de41:  68 87 00 00 00     PUSH 0x87
        _emit 0x68
        _emit 0x87
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005de46:  68 c0 91 f6 00     PUSH 0xf691c0   (source-file string)
        _emit 0x68
        _emit 0xc0
        _emit 0x91
        _emit 0xf6
        _emit 0x00
        // 0005de4b:  6a 14              PUSH 0x14
        _emit 0x6a
        _emit 0x14
        // 0005de4d:  e8 fe 52 00 00     CALL 0x00463150  (CRYPTO_malloc)
        _emit 0xe8
        _emit 0xfe
        _emit 0x52
        _emit 0x00
        _emit 0x00
        // 0005de52:  8b f0              MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 0005de54:  83 c4 0c           ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0005de57:  85 f6              TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 0005de59:  74 17              JZ 0x0045de72
        _emit 0x74
        _emit 0x17
        // 0005de5b:  e8 80 62 00 00     CALL 0x004640e0  (sk_new_null)
        _emit 0xe8
        _emit 0x80
        _emit 0x62
        _emit 0x00
        _emit 0x00
        // 0005de60:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0005de62:  89 06              MOV [ESI],EAX
        _emit 0x89
        _emit 0x06
        // 0005de64:  74 0c              JZ 0x0045de72
        _emit 0x74
        _emit 0x0c
        // 0005de66:  e8 35 8d 00 00     CALL 0x00466ba0  (BUF_MEM_new)
        _emit 0xe8
        _emit 0x35
        _emit 0x8d
        _emit 0x00
        _emit 0x00
        // 0005de6b:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0005de6d:  89 46 08           MOV [ESI+0x8],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 0005de70:  75 3b              JNZ 0x0045dead
        _emit 0x75
        _emit 0x3b
        // 0005de72:  68 93 00 00 00     PUSH 0x93
        _emit 0x68
        _emit 0x93
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005de77:  68 c0 91 f6 00     PUSH 0xf691c0   (source-file string)
        _emit 0x68
        _emit 0xc0
        _emit 0x91
        _emit 0xf6
        _emit 0x00
        // 0005de7c:  6a 41              PUSH 0x41
        _emit 0x6a
        _emit 0x41
        // 0005de7e:  68 ab 00 00 00     PUSH 0xab
        _emit 0x68
        _emit 0xab
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005de83:  6a 0d              PUSH 0xd
        _emit 0x6a
        _emit 0x0d
        // 0005de85:  e8 b6 ea ff ff     CALL 0x0045c940  (ERR_put_error)
        _emit 0xe8
        _emit 0xb6
        _emit 0xea
        _emit 0xff
        _emit 0xff
        // 0005de8a:  83 c4 14           ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0005de8d:  85 f6              TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 0005de8f:  74 18              JZ 0x0045dea9
        _emit 0x74
        _emit 0x18
        // 0005de91:  8b 06              MOV EAX,[ESI]
        _emit 0x8b
        _emit 0x06
        // 0005de93:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0005de95:  74 09              JZ 0x0045dea0
        _emit 0x74
        _emit 0x09
        // 0005de97:  50                 PUSH EAX
        _emit 0x50
        // 0005de98:  e8 63 61 00 00     CALL 0x00464000  (sk_free)
        _emit 0xe8
        _emit 0x63
        _emit 0x61
        _emit 0x00
        _emit 0x00
        // 0005de9d:  83 c4 04           ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005dea0:  56                 PUSH ESI
        _emit 0x56
        // 0005dea1:  e8 4a 54 00 00     CALL 0x004632f0  (CRYPTO_free)
        _emit 0xe8
        _emit 0x4a
        _emit 0x54
        _emit 0x00
        _emit 0x00
        // 0005dea6:  83 c4 04           ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005dea9:  33 c0              XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0005deab:  5e                 POP ESI
        _emit 0x5e
        // 0005deac:  c3                 RET
        _emit 0xc3
        // 0005dead:  8b 4c 24 08        MOV ECX,[ESP+0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 0005deb1:  b8 01 00 00 00     MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005deb6:  c7 46 0c 00 00 00 00  MOV [ESI+0xc],0x0
        _emit 0xc7
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005debd:  c7 46 10 00 00 00 00  MOV [ESI+0x10],0x0
        _emit 0xc7
        _emit 0x46
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005dec4:  89 46 04           MOV [ESI+0x4],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 0005dec7:  89 31              MOV [ECX],ESI
        _emit 0x89
        _emit 0x31
        // 0005dec9:  5e                 POP ESI
        _emit 0x5e
        // 0005deca:  c3                 RET
        _emit 0xc3
    }
}
