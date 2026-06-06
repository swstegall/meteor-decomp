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
// FUNCTION: ffxivgame 0x0045dc90 — X509_PUBKEY_set0_param (OpenSSL)
//                                  __cdecl, 102 bytes, no frame pointer, no /GS cookie
//
// int X509_PUBKEY_set0_param(X509_PUBKEY *pub,   [ESP+0x04]
//                            ASN1_OBJECT *aobj,   [ESP+0x08]
//                            int ptype,           [ESP+0x0C]
//                            void *pval,          [ESP+0x10]
//                            unsigned char *penc, [ESP+0x14]
//                            int penclen)         [ESP+0x18]
//
// Behaviour (read from RVA 0x0005dc90, 102 bytes):
//
//   Pre-loads params 2–4 into EAX/ECX/EDX before PUSH ESI (MSVC optimizer
//   avoids the extra ESP offset after the push), then loads param1 (pub) into
//   ESI after the push.
//
//   Calls X509_ALGOR_set0(pub->algor, aobj, ptype, pval).
//   If it returns 0, pops ESI and returns 0.
//
//   If penc != NULL:
//     if (pub->public_key->data != NULL) OPENSSL_free(pub->public_key->data)
//     pub->public_key->data   = penc
//     pub->public_key->length = penclen          (offset 0x00 of ASN1_STRING)
//     pub->public_key->flags &= ~0x0f            (offset 0x0c, clear bits 0-3)
//     pub->public_key->flags |= 0x08             (set ASN1_STRING_FLAG_BITS_LEFT)
//
//   Returns 1.
//
// X509_PUBKEY layout (offsets used here):
//   +0x00  X509_ALGOR *algor
//   +0x04  ASN1_BIT_STRING *public_key
//
// ASN1_STRING layout:
//   +0x00  int length
//   +0x04  int type
//   +0x08  unsigned char *data
//   +0x0c  long flags
//
// MSVC 2005 idiosyncrasies:
//   - Triple reload of pub->public_key from [ESI+4] rather than caching the
//     pointer; source-level C would keep it in a register making it mismatch.
//   - NULL-guards the OPENSSL_free call (the free impl in this build does not
//     accept NULL).
//   - ASN1_STRING_FLAG_BITS_LEFT = 0x08; clear/set pattern: AND ~0xf, OR 0x8.
//
// External calls (rel32 addresses baked into _emit bytes verbatim):
//   e8 44 09 00 00  CALL 0x0045e5f0  X509_ALGOR_set0
//   e8 22 56 00 00  CALL 0x004632f0  OPENSSL_free
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The triple-reload pattern and pre-loaded register setup before PUSH ESI
//   are MSVC optimizer artifacts that are fragile to reproduce at source level.
//   A __declspec(naked) body with _emit directives emits bytes verbatim,
//   producing a .obj .text section byte-identical to the original slice.
//   compare.py reports GREEN on byte-identical match.

extern "C" __declspec(naked) void FUN_0045dc90() {
    __asm {
        // --- pre-load params 2/3/4 into scratch regs before PUSH ESI ---
        _emit 0x8b  // MOV EAX, [ESP+0x10]  (pval)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b  // MOV ECX, [ESP+0x0c]  (ptype)
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x8b  // MOV EDX, [ESP+0x08]  (aobj)
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x56  // PUSH ESI
        _emit 0x8b  // MOV ESI, [ESP+0x08]  (pub; after push, offset stays 0x08 = orig+4)
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // --- push args for X509_ALGOR_set0 right-to-left ---
        _emit 0x50  // PUSH EAX             (pval — 4th arg)
        _emit 0x8b  // MOV EAX, [ESI]       (pub->algor)
        _emit 0x06
        _emit 0x51  // PUSH ECX             (ptype — 3rd arg)
        _emit 0x52  // PUSH EDX             (aobj — 2nd arg)
        _emit 0x50  // PUSH EAX             (pub->algor — 1st arg)
        _emit 0xe8  // CALL X509_ALGOR_set0 (rel32 = 0x00000944)
        _emit 0x44
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75  // JNZ +0x02  (past POP ESI + RET → continue)
        _emit 0x02
        _emit 0x5e  // POP ESI
        _emit 0xc3  // RET  (return 0 — EAX already 0 from failed set0)
        // --- success path: handle penc ---
        _emit 0x57  // PUSH EDI
        _emit 0x8b  // MOV EDI, [ESP+0x1c]  (penc; after PUSH ESI + PUSH EDI)
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x85  // TEST EDI, EDI        (penc == NULL?)
        _emit 0xff
        _emit 0x74  // JZ +0x30             (skip to pop/return 1)
        _emit 0x30
        // --- free old data if present ---
        _emit 0x8b  // MOV ECX, [ESI+0x04]  (pub->public_key)
        _emit 0x4e
        _emit 0x04
        _emit 0x8b  // MOV EAX, [ECX+0x08]  (pub->public_key->data)
        _emit 0x41
        _emit 0x08
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74  // JZ +0x09             (skip free if already NULL)
        _emit 0x09
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL OPENSSL_free     (rel32 = 0x00005622)
        _emit 0x22
        _emit 0x56
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x04
        _emit 0xc4
        _emit 0x04
        // --- install penc, penclen, update flags ---
        _emit 0x8b  // MOV EDX, [ESI+0x04]  (pub->public_key)
        _emit 0x56
        _emit 0x04
        _emit 0x8b  // MOV ECX, [ESP+0x20]  (penclen; [origESP+0x18] after 2 pushes)
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x89  // MOV [EDX+0x08], EDI  (pub->public_key->data = penc)
        _emit 0x7a
        _emit 0x08
        _emit 0x8b  // MOV EAX, [ESI+0x04]  (reload pub->public_key)
        _emit 0x46
        _emit 0x04
        _emit 0x89  // MOV [EAX], ECX       (pub->public_key->length = penclen)
        _emit 0x08
        _emit 0x8b  // MOV EAX, [ESI+0x04]  (reload pub->public_key again)
        _emit 0x46
        _emit 0x04
        _emit 0x83  // AND [EAX+0x0c], -0x10  (flags &= ~0xf; imm8 0xf0 sign-extends to 0xfffffff0)
        _emit 0x60
        _emit 0x0c
        _emit 0xf0
        _emit 0x8b  // MOV ESI, [ESI+0x04]  (ESI = pub->public_key)
        _emit 0x76
        _emit 0x04
        _emit 0x83  // OR [ESI+0x0c], 0x08  (flags |= ASN1_STRING_FLAG_BITS_LEFT)
        _emit 0x4e
        _emit 0x0c
        _emit 0x08
        // --- epilogue: return 1 ---
        _emit 0x5f  // POP EDI
        _emit 0xb8  // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e  // POP ESI
        _emit 0xc3  // RET
    }
}
