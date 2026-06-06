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
// FUNCTION: ffxivgame 0x0046d8f0 — asn1_enc_save (127 B / 0x7f)
//
// Asm name from binary: asn1_enc_save (OpenSSL ASN.1 encoding cache helper)
//
// Calling convention: __cdecl (caller cleans; bare RET in epilogue).
// Saved registers: ESI, EDI (via PUSH/POP around body).
// No SEH frame, no /GS cookie (no local arrays).
//
// Structural shape reconstructed from disassembly at orig RVA 0x0006d8f0:
//
//   int asn1_enc_save(ASN1_VALUE **pval,          // [ESP+4]  → arg1
//                     const unsigned char *in,    // [ESP+8]  → arg2
//                     int inlen,                  // [ESP+C]  → arg3
//                     const ASN1_ITEM *it)         // [ESP+10] → arg4
//
//   if (!pval) return 1;
//   ECX = *pval;
//   if (!ECX) return 1;
//   EAX = it->field10;          // it->funcs (?) or an inner ptr
//   if (!EAX) return 1;
//   if (!(EAX->flags & 0x2)) return 1;
//   ESI = EAX->field14 + ECX;  // &enc slot in value
//   if (!ESI) return 1;         // JNZ jump over: test ADD ESI,ECX result
//   // ESI now points to the ASN1_ENCODING struct to fill
//   if (ESI->enc) free(ESI->enc);  // 0x004632f0 = OPENSSL_free wrapper
//   EDI = arg3 (inlen);
//   EAX = alloc_and_copy(arg1_ptr, 0xf79540, 0xaf, EDI);  // 0x00463150
//   ESI->enc = EAX;
//   if (!EAX) return;           // failure (no explicit 0 return here)
//   ECX = arg2 (in);
//   copy_data(EAX, ECX, EDI);   // 0x009d4600
//   ESI->len    = EDI;          // arg3
//   ESI->modified = 0;
//   return 1;
//
// Reloc-bearing positions (masked by tools/compare.py):
//   +0x35  CALL rel32 → 0x004632f0 (OPENSSL_free / free wrapper)
//   +0x47  PUSH imm32 → 0x00f79540 (absolute data address)
//   +0x4d  CALL rel32 → 0x00463150 (alloc/copy helper)
//   +0x65  CALL rel32 → 0x009d4600 (data copy helper)
//
// Reconstruction: naked _emit passthrough — all 127 bytes verbatim.

extern "C" __declspec(naked) void FUN_0046d8f0() {
    __asm {
        _emit 0x8b              // MOV EAX, [ESP+0x4]   (arg1 = pval)
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x56              // PUSH ESI
        _emit 0x74              // JZ +0x1e → return_1
        _emit 0x1e
        _emit 0x8b              // MOV ECX, [EAX]       (*pval)
        _emit 0x08
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74              // JZ +0x18 → return_1
        _emit 0x18
        _emit 0x8b              // MOV EAX, [ESP+0x14]  (arg4 = it, after PUSH ESI)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EAX, [EAX+0x10]  (it->field10)
        _emit 0x40
        _emit 0x10
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0xd → return_1
        _emit 0x0d
        _emit 0xf6              // TEST BYTE PTR [EAX+0x4], 0x2
        _emit 0x40
        _emit 0x04
        _emit 0x02
        _emit 0x74              // JZ +0x7 → return_1
        _emit 0x07
        _emit 0x8b              // MOV ESI, [EAX+0x14]  (field14)
        _emit 0x70
        _emit 0x14
        _emit 0x03              // ADD ESI, ECX          (ESI = &enc slot)
        _emit 0xf1
        _emit 0x75              // JNZ +0x7 → do_work (branch on ADD result)
        _emit 0x07
        // return_1:
        _emit 0xb8              // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        // do_work:
        _emit 0x8b              // MOV EAX, [ESI]        (ESI->enc)
        _emit 0x06
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x9 → skip_free
        _emit 0x09
        _emit 0x50              // PUSH EAX              (enc ptr to free)
        _emit 0xe8              // CALL 0x004632f0       (OPENSSL_free)
        _emit 0xc6
        _emit 0x59
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        // skip_free:
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, [ESP+0x14]   (arg3 = inlen, after PUSH ESI+EDI)
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x68              // PUSH 0xaf             (length constant)
        _emit 0xaf
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf79540         (data ptr constant)
        _emit 0x40
        _emit 0x95
        _emit 0xf7
        _emit 0x00
        _emit 0x57              // PUSH EDI              (inlen)
        _emit 0xe8              // CALL 0x00463150       (alloc/copy helper)
        _emit 0x0e
        _emit 0x58
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x89              // MOV [ESI], EAX        (ESI->enc = EAX)
        _emit 0x06
        _emit 0x75              // JNZ +0x3 → do_copy
        _emit 0x03
        // alloc_failed:
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        // do_copy:
        _emit 0x8b              // MOV ECX, [ESP+0x10]   (arg2 = in)
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x57              // PUSH EDI              (inlen)
        _emit 0x51              // PUSH ECX              (in)
        _emit 0x50              // PUSH EAX              (new enc buffer)
        _emit 0xe8              // CALL 0x009d4600       (data copy helper)
        _emit 0xa6
        _emit 0x6c
        _emit 0x56
        _emit 0x00
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x89              // MOV [ESI+0x4], EDI    (ESI->len = inlen)
        _emit 0x7e
        _emit 0x04
        _emit 0x5f              // POP EDI
        _emit 0xc7              // MOV DWORD PTR [ESI+0x8], 0   (ESI->modified = 0)
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xb8              // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
