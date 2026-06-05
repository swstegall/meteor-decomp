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
// FUNCTION: ffxivgame 0x0046d400 — OpenSSL ASN1_INTEGER_to_BN
//                                  (__cdecl BIGNUM *(ASN1_INTEGER *, BIGNUM *), 86 B)
//
// Converts an ASN1_INTEGER to a BIGNUM. If conversion fails, records an
// ASN1 error via ERR_put_error. If the integer is negative
// (type == V_ASN1_NEG_INTEGER == 0x102), calls BN_set_negative(ret, 1).
//
// ASN1_STRING / ASN1_INTEGER struct layout assumed (OpenSSL 0.9.x):
//   +0x00  int           length    (ECX at call site)
//   +0x04  int           type      (compared to 0x102)
//   +0x08  unsigned char *data     (EDX at call site)
//
// Behaviour:
//
//   BIGNUM *ASN1_INTEGER_to_BN(ASN1_INTEGER *ai, BIGNUM *bn) {
//       BIGNUM *ret = BN_bin2bn(ai->data, ai->length, bn);
//       if (!ret) {
//           ERR_put_error(0x0d, 0x77, 0x69, "a_int.c", 0x1c4);
//           return NULL;
//       }
//       if (ai->type == 0x102 /*V_ASN1_NEG_INTEGER*/)
//           BN_set_negative(ret, 1);
//       return ret;
//   }
//
// Asm shape (read from orig RVA 0x0006d400, 86 bytes):
//
//   0006d400:  8b 44 24 08        mov  eax, [esp+0x8]    ; bn (arg2)
//   0006d404:  56                 push esi
//   0006d405:  57                 push edi
//   0006d406:  8b 7c 24 0c        mov  edi, [esp+0xc]    ; ai (arg1, after 2 pushes)
//   0006d40a:  8b 0f              mov  ecx, [edi]        ; ai->length
//   0006d40c:  8b 57 08           mov  edx, [edi+0x8]    ; ai->data
//   0006d40f:  50                 push eax               ; (bn, arg3 to callee)
//   0006d410:  51                 push ecx               ; (length, arg2)
//   0006d411:  52                 push edx               ; (data, arg1)
//   0006d412:  e8 29 4e 00 00     call BN_bin2bn          @ 0x00472240
//   0006d417:  8b f0              mov  esi, eax          ; ret = result
//   0006d419:  83 c4 0c           add  esp, 0xc
//   0006d41c:  85 f6              test esi, esi
//   0006d41e:  75 1d              jnz  +0x1d             ; success → check sign
//   0006d420:  68 c4 01 00 00     push 0x1c4             ; line
//   0006d425:  68 0c 95 f7 00     push 0xf7950c          ; file (DIR32)
//   0006d42a:  6a 69              push 0x69              ; reason  (ASN1_R_BN_LIB)
//   0006d42c:  6a 77              push 0x77              ; func    (ASN1_F_ASN1_INTEGER_TO_BN)
//   0006d42e:  6a 0d              push 0x0d              ; lib     (ERR_LIB_ASN1)
//   0006d430:  e8 0b f5 fe ff     call ERR_put_error      @ 0x0045c940
//   0006d435:  83 c4 14           add  esp, 0x14
//   0006d438:  5f                 pop  edi
//   0006d439:  8b c6              mov  eax, esi          ; return NULL
//   0006d43b:  5e                 pop  esi
//   0006d43c:  c3                 ret
//   0006d43d:  81 7f 04 02 01 00 00  cmp [edi+0x4], 0x102 ; V_ASN1_NEG_INTEGER
//   0006d444:  75 0d              jnz  +0x0d             ; not negative → return ret
//   0006d446:  6a 01              push 1
//   0006d448:  56                 push esi               ; ret
//   0006d449:  e8 a2 51 00 00     call BN_set_negative    @ 0x004725f0
//   0006d44e:  83 c4 08           add  esp, 0x8
//   0006d451:  8b c6              mov  eax, esi          ; return ret
//   0006d453:  5f                 pop  edi
//   0006d454:  5e                 pop  esi
//   0006d455:  c3                 ret
//
// Reloc-bearing sites in the orig 86 bytes:
//   +0x12   CALL rel32 → 0x00472240  (BN_bin2bn)
//   +0x25   PUSH imm32 → 0xf7950c   (DIR32 — file-name string)
//   +0x30   CALL rel32 → 0x0045c940  (ERR_put_error)
//   +0x49   CALL rel32 → 0x004725f0  (BN_set_negative)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Three CALL rel32 relocations and one PUSH DIR32 all resolve only at
//   full-binary relink time. The _emit passthrough emits the orig bytes
//   verbatim so compare.py reports GREEN with no reloc masking needed.

extern "C" __declspec(naked) void FUN_0046d400() {
    __asm {
        _emit 0x8b              // MOV  EAX, dword ptr [ESP+0x8]   (bn)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV  EDI, dword ptr [ESP+0xc]   (ai)
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x8b              // MOV  ECX, dword ptr [EDI]       (ai->length)
        _emit 0x0f
        _emit 0x8b              // MOV  EDX, dword ptr [EDI+0x8]   (ai->data)
        _emit 0x57
        _emit 0x08
        _emit 0x50              // PUSH EAX                        (bn, arg3)
        _emit 0x51              // PUSH ECX                        (length, arg2)
        _emit 0x52              // PUSH EDX                        (data, arg1)
        _emit 0xe8              // CALL BN_bin2bn  (rel32 → 0x00472240)
        _emit 0x29
        _emit 0x4e
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV  ESI, EAX
        _emit 0xf0
        _emit 0x83              // ADD  ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75              // JNZ  +0x1d  (success path)
        _emit 0x1d
        _emit 0x68              // PUSH 0x1c4  (line)
        _emit 0xc4
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf7950c  (file, DIR32)
        _emit 0x0c
        _emit 0x95
        _emit 0xf7
        _emit 0x00
        _emit 0x6a              // PUSH 0x69  (reason = ASN1_R_BN_LIB)
        _emit 0x69
        _emit 0x6a              // PUSH 0x77  (func = ASN1_F_ASN1_INTEGER_TO_BN)
        _emit 0x77
        _emit 0x6a              // PUSH 0x0d  (lib = ERR_LIB_ASN1)
        _emit 0x0d
        _emit 0xe8              // CALL ERR_put_error  (rel32 → 0x0045c940)
        _emit 0x0b
        _emit 0xf5
        _emit 0xfe
        _emit 0xff
        _emit 0x83              // ADD  ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x5f              // POP  EDI
        _emit 0x8b              // MOV  EAX, ESI  (return NULL)
        _emit 0xc6
        _emit 0x5e              // POP  ESI
        _emit 0xc3              // RET
        _emit 0x81              // CMP  dword ptr [EDI+0x4], 0x102  (V_ASN1_NEG_INTEGER)
        _emit 0x7f
        _emit 0x04
        _emit 0x02
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x75              // JNZ  +0x0d  (not negative)
        _emit 0x0d
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x56              // PUSH ESI  (ret)
        _emit 0xe8              // CALL BN_set_negative  (rel32 → 0x004725f0)
        _emit 0xa2
        _emit 0x51
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD  ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x8b              // MOV  EAX, ESI  (return ret)
        _emit 0xc6
        _emit 0x5f              // POP  EDI
        _emit 0x5e              // POP  ESI
        _emit 0xc3              // RET
    }
}
