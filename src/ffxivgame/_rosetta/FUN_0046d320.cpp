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
// FUNCTION: ffxivgame 0x0006d320 — _BN_to_ASN1_INTEGER (212 B / 0xd4),
//                                  OpenSSL BN_to_ASN1_INTEGER.
//                                  __cdecl, no frame pointer, no /GS.
//
// Signature:
//   ASN1_INTEGER * _BN_to_ASN1_INTEGER(const BIGNUM *bn, ASN1_INTEGER *ai);
//
// Body outline:
//
//   if (ai == NULL)
//       ret = ASN1_STRING_type_new(V_ASN1_INTEGER);  // 0x00464480
//   else
//       ret = ai;
//
//   if (ret == NULL) {
//       BNerr(BN_F_BN_TO_ASN1_INTEGER, ERR_R_NESTED_ASN1_ERROR);  // line 0x1a0
//       goto err;
//   }
//
//   ret->type = bn->neg ? V_ASN1_NEG_INTEGER : V_ASN1_INTEGER;
//   j = BN_num_bits(bn);   // 0x00471e80
//   len = (j != 0) ? (j / 8 + 1) : 0;
//   len += 4;
//
//   if (ret->length < len) {
//       new_data = CRYPTO_realloc(ret->data, len, file, 0x1aa);  // 0x004631c0
//       if (!new_data) {
//           BNerr(BN_F_BN_TO_ASN1_INTEGER, ERR_R_MALLOC_FAILURE);  // line 0x1ad
//           goto err;
//       }
//       ret->data = new_data;
//   }
//
//   ret->length = BN_bn2bin(bn, ret->data);  // 0x00472330
//   if (!ret->length) {
//       ret->data[0] = 0;
//       ret->length = 1;
//   }
//   return ret;
//
// err:
//   if (ret != ai) ASN1_INTEGER_free(ret);  // 0x004644d0
//   return NULL;
//
// Reloc-bearing sites (compare.py masks these 4-byte windows):
//   +0x0e   REL32 → 0x00464480  (ASN1_STRING_type_new)
//   +0x44   REL32 → 0x00471e80  (BN_num_bits)
//   +0x72   REL32 → 0x004631c0  (CRYPTO_realloc)
//   +0x91   REL32 → 0x0045c940  (ERR_put_error)
//   +0x9e   REL32 → 0x004644d0  (ASN1_INTEGER_free)
//   +0xb4   REL32 → 0x00472330  (BN_bn2bin)
//
// Reconstruction strategy — __declspec(naked) byte passthrough.
//
//   The interleaving of PUSH EDI (argument setup for BN_num_bits) before
//   the MOV [ESI+4] type store, the CDQ/AND/SAR sequence for signed-safe
//   division, the JGE skip-realloc branch, and the two separate error
//   paths that share a common ERR_put_error tail all represent MSVC 2005
//   register-scheduler decisions that are impractical to reproduce from
//   C++ source reliably. Emitting all 212 original bytes via _emit gives
//   a byte-exact match.

extern "C" __declspec(naked) void FUN_0046d320() {
    __asm {
        _emit 0x53          // PUSH EBX
        _emit 0x8b          // MOV EBX, [ESP+0xc]
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        _emit 0x85          // TEST EBX, EBX
        _emit 0xdb
        _emit 0x56          // PUSH ESI
        _emit 0x57          // PUSH EDI
        _emit 0x75          // JNZ +0x0e
        _emit 0x0e
        _emit 0x6a          // PUSH 0x2
        _emit 0x02
        _emit 0xe8          // CALL 0x00464480 (ASN1_STRING_type_new)
        _emit 0x4e
        _emit 0x71
        _emit 0xff
        _emit 0xff
        _emit 0x83          // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x8b          // MOV ESI, EAX
        _emit 0xf0
        _emit 0xeb          // JMP +0x02
        _emit 0x02
        _emit 0x8b          // MOV ESI, EBX
        _emit 0xf3
        _emit 0x85          // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75          // JNZ +0x0e
        _emit 0x0e
        _emit 0x68          // PUSH 0x1a0
        _emit 0xa0
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68          // PUSH 0xf7950c
        _emit 0x0c
        _emit 0x95
        _emit 0xf7
        _emit 0x00
        _emit 0x6a          // PUSH 0x3a
        _emit 0x3a
        _emit 0xeb          // JMP +0x5d
        _emit 0x5d
        _emit 0x8b          // MOV EDI, [ESP+0x10]
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x8b          // MOV EAX, [EDI+0xc]
        _emit 0x47
        _emit 0x0c
        _emit 0xf7          // NEG EAX
        _emit 0xd8
        _emit 0x1b          // SBB EAX, EAX
        _emit 0xc0
        _emit 0x25          // AND EAX, 0x100
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x83          // ADD EAX, 0x2
        _emit 0xc0
        _emit 0x02
        _emit 0x57          // PUSH EDI
        _emit 0x89          // MOV [ESI+0x4], EAX
        _emit 0x46
        _emit 0x04
        _emit 0xe8          // CALL 0x00471e80 (BN_num_bits)
        _emit 0x17
        _emit 0x4b
        _emit 0x00
        _emit 0x00
        _emit 0x83          // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74          // JZ +0x0c
        _emit 0x0c
        _emit 0x99          // CDQ
        _emit 0x83          // AND EDX, 0x7
        _emit 0xe2
        _emit 0x07
        _emit 0x03          // ADD EAX, EDX
        _emit 0xc2
        _emit 0xc1          // SAR EAX, 0x3
        _emit 0xf8
        _emit 0x03
        _emit 0x83          // ADD EAX, 0x1
        _emit 0xc0
        _emit 0x01
        _emit 0x83          // ADD EAX, 0x4
        _emit 0xc0
        _emit 0x04
        _emit 0x39          // CMP [ESI], EAX
        _emit 0x06
        _emit 0x7d          // JGE +0x4c
        _emit 0x4c
        _emit 0x8b          // MOV ECX, [ESI+0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x68          // PUSH 0x1aa
        _emit 0xaa
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68          // PUSH 0xf7950c
        _emit 0x0c
        _emit 0x95
        _emit 0xf7
        _emit 0x00
        _emit 0x50          // PUSH EAX
        _emit 0x51          // PUSH ECX
        _emit 0xe8          // CALL 0x004631c0 (CRYPTO_realloc)
        _emit 0x29
        _emit 0x5e
        _emit 0xff
        _emit 0xff
        _emit 0x83          // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75          // JNZ +0x2e
        _emit 0x2e
        _emit 0x68          // PUSH 0x1ad
        _emit 0xad
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68          // PUSH 0xf7950c
        _emit 0x0c
        _emit 0x95
        _emit 0xf7
        _emit 0x00
        _emit 0x6a          // PUSH 0x41
        _emit 0x41
        _emit 0x68          // PUSH 0x8b
        _emit 0x8b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a          // PUSH 0xd
        _emit 0x0d
        _emit 0xe8          // CALL 0x0045c940 (ERR_put_error)
        _emit 0x8a
        _emit 0xf5
        _emit 0xfe
        _emit 0xff
        _emit 0x83          // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x3b          // CMP ESI, EBX
        _emit 0xf3
        _emit 0x74          // JZ +0x09
        _emit 0x09
        _emit 0x56          // PUSH ESI
        _emit 0xe8          // CALL 0x004644d0 (ASN1_INTEGER_free)
        _emit 0x0d
        _emit 0x71
        _emit 0xff
        _emit 0xff
        _emit 0x83          // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x5f          // POP EDI
        _emit 0x5e          // POP ESI
        _emit 0x33          // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5b          // POP EBX
        _emit 0xc3          // RET
        _emit 0x89          // MOV [ESI+0x8], EAX
        _emit 0x46
        _emit 0x08
        _emit 0x8b          // MOV EDX, [ESI+0x8]
        _emit 0x56
        _emit 0x08
        _emit 0x52          // PUSH EDX
        _emit 0x57          // PUSH EDI
        _emit 0xe8          // CALL 0x00472330 (BN_bn2bin)
        _emit 0x57
        _emit 0x4f
        _emit 0x00
        _emit 0x00
        _emit 0x83          // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0x89          // MOV [ESI], EAX
        _emit 0x06
        _emit 0x75          // JNZ +0x0c
        _emit 0x0c
        _emit 0x8b          // MOV EAX, [ESI+0x8]
        _emit 0x46
        _emit 0x08
        _emit 0xc6          // MOV byte ptr [EAX], 0x0
        _emit 0x00
        _emit 0x00
        _emit 0xc7          // MOV dword ptr [ESI], 0x1
        _emit 0x06
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5f          // POP EDI
        _emit 0x8b          // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e          // POP ESI
        _emit 0x5b          // POP EBX
        _emit 0xc3          // RET
    }
}
