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
// FUNCTION: ffxivgame 0x0005dba0 — _X509_PUBKEY_get  (237 B / 0xed)
//                                  OpenSSL X509_PUBKEY_get, __cdecl, no SEH.
//
// Behaviour read from asm/ffxivgame/0005dba0__X509_PUBKEY_get.s:
//
//   EVP_PKEY *X509_PUBKEY_get(X509_PUBKEY *key)   — arg at [ESP+0xc] after
//                                                    PUSH ESI / PUSH EDI.
//
//   EDI = key, ESI = ret (EVP_PKEY*).
//
//   if (key == NULL)                           → goto error_return (XOR EAX,EAX)
//   if (key->pkey != NULL) {                   ; [EDI+0x08] == pkey
//       CRYPTO_add_lock(&key->pkey->references,// [EDI+0x08]+0x08 = references
//                       1, CRYPTO_LOCK_EVP_PKEY,
//                       file, /*line=*/141);
//       return key->pkey;
//   }
//   if (key->public_key == NULL)               ; [EDI+0x04] == public_key
//       goto error_return;
//   ret = EVP_PKEY_new();                      ; CALL 0x0045cb70
//   if (ret == NULL) {
//       X509err(X509_F_X509_PUBKEY_GET,        ; func=0x77, reason=0x41=ERR_R_MALLOC_FAILURE
//               ERR_R_MALLOC_FAILURE, file, 149);
//       goto error;
//   }
//   if (!EVP_PKEY_set_type(ret,                ; CALL 0x0045cdc0
//                          OBJ_obj2nid(key->algor->algorithm))) {  ; CALL 0x00464d80
//       X509err(X509_F_X509_PUBKEY_GET,        ; reason=0x6f
//               X509_R_UNSUPPORTED_ALGORITHM, file, 155);
//       goto error;
//   }
//   ameth = ret->ameth;                        ; [ESI+0x0c]
//   pub_decode = ameth->pub_decode;            ; [ameth+0x14]
//   if (pub_decode != NULL) {
//       if (!pub_decode(ret, key)) {           ; CALL EAX (indirect)
//           X509err(X509_F_X509_PUBKEY_GET,    ; reason=0x7d
//                   X509_R_PUBLIC_KEY_DECODE_ERROR, file, 164);
//           goto error;
//       }
//   } else {
//       X509err(X509_F_X509_PUBKEY_GET,        ; reason=0x7c
//               X509_R_METHOD_NOT_SUPPORTED, file, 170);
//       goto error;
//   }
//   key->pkey = ret;                           ; MOV [EDI+0x08], ESI
//   CRYPTO_add_lock(&ret->references,          ; [ESI+0x08]
//                   1, CRYPTO_LOCK_EVP_PKEY, file, /*line=*/175);
//   return ret;
// error:
//   if (ret != NULL) EVP_PKEY_free(ret);       ; CALL 0x0045ce50
// error_return:
//   return NULL;
//
// X509_PUBKEY layout (recovered from field offsets):
//   +0x00  X509_ALGOR  *algor;          (algor->algorithm at [algor+0x00])
//   +0x04  ASN1_BIT_STRING *public_key;
//   +0x08  EVP_PKEY    *pkey;
//
// EVP_PKEY layout (field offsets used here):
//   +0x08  int references;
//   +0x0c  EVP_PKEY_ASN1_METHOD *ameth;
//
// EVP_PKEY_ASN1_METHOD layout:
//   +0x14  int (*pub_decode)(EVP_PKEY*, X509_PUBKEY*);
//
// ERR_put_error args:  lib=0x0b (ERR_LIB_X509), func=0x77
//                      (X509_F_X509_PUBKEY_GET=119), reason, file, line.
// CRYPTO_add_lock  args: pointer, amount=1, type=0x0a
//                        (CRYPTO_LOCK_EVP_PKEY=10), file, line.
// Filename string  baked at binary VA 0x00f6900c ("crypto/x509/x_pubkey.c").
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function embeds the OpenSSL filename string as an absolute immediate
//   (0xf6900c, pushed five times as PUSH imm32 without a relocation entry)
//   and interleaves argument-evaluation with PUSH sequences in a way that
//   is brittle under /O2 register allocation (e.g. ADD EAX,8 inserted
//   between PUSH imm32 stubs while EAX still holds key->pkey).  A naked
//   byte passthrough reproduces the 237 bytes exactly without needing the
//   full OpenSSL header tree or per-line source alignment.

extern "C" __declspec(naked) void FUN_0045dba0() {
    __asm {
        // 0005dba0  PUSH ESI
        _emit 0x56
        // 0005dba1  PUSH EDI
        _emit 0x57
        // 0005dba2  MOV EDI, [ESP+0xc]    ; key
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 0005dba6  TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 0005dba8  JZ 0x0045dc88 (near)
        _emit 0x0f
        _emit 0x84
        _emit 0xda
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005dbae  MOV EAX, [EDI+0x8]   ; key->pkey
        _emit 0x8b
        _emit 0x47
        _emit 0x08
        // 0005dbb1  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005dbb3  JZ 0x0045dbd5 (short)
        _emit 0x74
        _emit 0x20
        // 0005dbb5  PUSH 0x8d (line=141)
        _emit 0x68
        _emit 0x8d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005dbba  PUSH 0xf6900c (file)
        _emit 0x68
        _emit 0x0c
        _emit 0x90
        _emit 0xf6
        _emit 0x00
        // 0005dbbf  PUSH 0xa (CRYPTO_LOCK_EVP_PKEY)
        _emit 0x6a
        _emit 0x0a
        // 0005dbc1  ADD EAX, 0x8         ; &key->pkey->references
        _emit 0x83
        _emit 0xc0
        _emit 0x08
        // 0005dbc4  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005dbc6  PUSH EAX
        _emit 0x50
        // 0005dbc7  CALL 0x00466000 (CRYPTO_add_lock)
        _emit 0xe8
        _emit 0x34
        _emit 0x84
        _emit 0x00
        _emit 0x00
        // 0005dbcc  MOV EAX, [EDI+0x8]   ; return key->pkey
        _emit 0x8b
        _emit 0x47
        _emit 0x08
        // 0005dbcf  ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0005dbd2  POP EDI
        _emit 0x5f
        // 0005dbd3  POP ESI
        _emit 0x5e
        // 0005dbd4  RET
        _emit 0xc3
        // 0005dbd5  CMP dword ptr [EDI+4], 0x0  ; key->public_key
        _emit 0x83
        _emit 0x7f
        _emit 0x04
        _emit 0x00
        // 0005dbd9  JZ 0x0045dc88 (near)
        _emit 0x0f
        _emit 0x84
        _emit 0xa9
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005dbdf  CALL 0x0045cb70 (EVP_PKEY_new)
        _emit 0xe8
        _emit 0x8c
        _emit 0xef
        _emit 0xff
        _emit 0xff
        // 0005dbe4  MOV ESI, EAX         ; ret
        _emit 0x8b
        _emit 0xf0
        // 0005dbe6  TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0005dbe8  JNZ 0x0045dbf8 (short)
        _emit 0x75
        _emit 0x0e
        // 0005dbea  PUSH 0x95 (line=149)
        _emit 0x68
        _emit 0x95
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005dbef  PUSH 0xf6900c (file)
        _emit 0x68
        _emit 0x0c
        _emit 0x90
        _emit 0xf6
        _emit 0x00
        // 0005dbf4  PUSH 0x41 (ERR_R_MALLOC_FAILURE)
        _emit 0x6a
        _emit 0x41
        // 0005dbf6  JMP 0x0045dc6f (short)
        _emit 0xeb
        _emit 0x77
        // 0005dbf8  MOV EAX, [EDI]       ; key->algor
        _emit 0x8b
        _emit 0x07
        // 0005dbfa  MOV ECX, [EAX]       ; algor->algorithm
        _emit 0x8b
        _emit 0x08
        // 0005dbfc  PUSH ECX
        _emit 0x51
        // 0005dbfd  CALL 0x00464d80 (OBJ_obj2nid)
        _emit 0xe8
        _emit 0x7e
        _emit 0x71
        _emit 0x00
        _emit 0x00
        // 0005dc02  PUSH EAX (nid)
        _emit 0x50
        // 0005dc03  PUSH ESI (ret)
        _emit 0x56
        // 0005dc04  CALL 0x0045cdc0 (EVP_PKEY_set_type)
        _emit 0xe8
        _emit 0xb7
        _emit 0xf1
        _emit 0xff
        _emit 0xff
        // 0005dc09  ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0005dc0c  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005dc0e  JNZ 0x0045dc1e (short)
        _emit 0x75
        _emit 0x0e
        // 0005dc10  PUSH 0x9b (line=155)
        _emit 0x68
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005dc15  PUSH 0xf6900c (file)
        _emit 0x68
        _emit 0x0c
        _emit 0x90
        _emit 0xf6
        _emit 0x00
        // 0005dc1a  PUSH 0x6f (X509_R_UNSUPPORTED_ALGORITHM)
        _emit 0x6a
        _emit 0x6f
        // 0005dc1c  JMP 0x0045dc6f (short)
        _emit 0xeb
        _emit 0x51
        // 0005dc1e  MOV EDX, [ESI+0xc]   ; ret->ameth
        _emit 0x8b
        _emit 0x56
        _emit 0x0c
        // 0005dc21  MOV EAX, [EDX+0x14]  ; ameth->pub_decode
        _emit 0x8b
        _emit 0x42
        _emit 0x14
        // 0005dc24  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005dc26  JZ 0x0045dc63 (short)
        _emit 0x74
        _emit 0x3b
        // 0005dc28  PUSH EDI             ; key
        _emit 0x57
        // 0005dc29  PUSH ESI             ; ret
        _emit 0x56
        // 0005dc2a  CALL EAX             ; pub_decode(ret, key)
        _emit 0xff
        _emit 0xd0
        // 0005dc2c  ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0005dc2f  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005dc31  JNZ 0x0045dc41 (short)
        _emit 0x75
        _emit 0x0e
        // 0005dc33  PUSH 0xa4 (line=164)
        _emit 0x68
        _emit 0xa4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005dc38  PUSH 0xf6900c (file)
        _emit 0x68
        _emit 0x0c
        _emit 0x90
        _emit 0xf6
        _emit 0x00
        // 0005dc3d  PUSH 0x7d (X509_R_PUBLIC_KEY_DECODE_ERROR)
        _emit 0x6a
        _emit 0x7d
        // 0005dc3f  JMP 0x0045dc6f (short)
        _emit 0xeb
        _emit 0x2e
        // 0005dc41  PUSH 0xaf (line=175)
        _emit 0x68
        _emit 0xaf
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005dc46  PUSH 0xf6900c (file)
        _emit 0x68
        _emit 0x0c
        _emit 0x90
        _emit 0xf6
        _emit 0x00
        // 0005dc4b  PUSH 0xa (CRYPTO_LOCK_EVP_PKEY)
        _emit 0x6a
        _emit 0x0a
        // 0005dc4d  LEA EAX, [ESI+0x8]   ; &ret->references
        _emit 0x8d
        _emit 0x46
        _emit 0x08
        // 0005dc50  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005dc52  PUSH EAX
        _emit 0x50
        // 0005dc53  MOV [EDI+0x8], ESI   ; key->pkey = ret
        _emit 0x89
        _emit 0x77
        _emit 0x08
        // 0005dc56  CALL 0x00466000 (CRYPTO_add_lock)
        _emit 0xe8
        _emit 0xa5
        _emit 0x83
        _emit 0x00
        _emit 0x00
        // 0005dc5b  ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0005dc5e  POP EDI
        _emit 0x5f
        // 0005dc5f  MOV EAX, ESI         ; return ret
        _emit 0x8b
        _emit 0xc6
        // 0005dc61  POP ESI
        _emit 0x5e
        // 0005dc62  RET
        _emit 0xc3
        // 0005dc63  PUSH 0xaa (line=170)
        _emit 0x68
        _emit 0xaa
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005dc68  PUSH 0xf6900c (file)
        _emit 0x68
        _emit 0x0c
        _emit 0x90
        _emit 0xf6
        _emit 0x00
        // 0005dc6d  PUSH 0x7c (X509_R_METHOD_NOT_SUPPORTED)
        _emit 0x6a
        _emit 0x7c
        // 0005dc6f  PUSH 0x77 (X509_F_X509_PUBKEY_GET)
        _emit 0x6a
        _emit 0x77
        // 0005dc71  PUSH 0x0b (ERR_LIB_X509)
        _emit 0x6a
        _emit 0x0b
        // 0005dc73  CALL 0x0045c940 (ERR_put_error)
        _emit 0xe8
        _emit 0xc8
        _emit 0xec
        _emit 0xff
        _emit 0xff
        // 0005dc78  ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0005dc7b  TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0005dc7d  JZ 0x0045dc88 (short)
        _emit 0x74
        _emit 0x09
        // 0005dc7f  PUSH ESI             ; ret
        _emit 0x56
        // 0005dc80  CALL 0x0045ce50 (EVP_PKEY_free)
        _emit 0xe8
        _emit 0xcb
        _emit 0xf1
        _emit 0xff
        _emit 0xff
        // 0005dc85  ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005dc88  POP EDI
        _emit 0x5f
        // 0005dc89  XOR EAX, EAX         ; return NULL
        _emit 0x33
        _emit 0xc0
        // 0005dc8b  POP ESI
        _emit 0x5e
        // 0005dc8c  RET
        _emit 0xc3
    }
}
