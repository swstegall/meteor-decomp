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
// FUNCTION: ffxivgame 0x00464440 — _ASN1_STRING_set0 (51 B / 0x33)
//
// OpenSSL ASN1_STRING_set0: replace the data buffer of an ASN1_STRING with
// a pre-allocated block (caller is responsible for allocating `data`).
// If the string already has a buffer, it is freed via CRYPTO_free before
// the new pointer is installed.
//
// struct ASN1_STRING {
//     int            length;  // [+0x00]
//     int            type;    // [+0x04]
//     unsigned char *data;    // [+0x08]
//     long           flags;   // [+0x0c]
// };
//
// void ASN1_STRING_set0(ASN1_STRING *str, void *data, int len):
//   if (str->data)
//       CRYPTO_free(str->data);
//   str->data   = (unsigned char *)data;
//   str->length = len;
//
// Calling convention: __cdecl (caller cleans, plain RET).
// Frame: PUSH ESI only — no EBP frame, no SUB ESP, no other callee-saves.
//
// Asm (51 bytes @ orig RVA 0x00064440):
//   56               PUSH ESI
//   8b 74 24 08      MOV ESI, [ESP+0x8]           ; str  (arg1)
//   8b 46 08         MOV EAX, [ESI+0x8]           ; str->data
//   85 c0            TEST EAX, EAX
//   74 18            JZ null_data                  ; no existing buffer → skip free
//   50               PUSH EAX                      ; push old str->data
//   e8 XX XX XX XX   CALL FUN_004632f0             ; CRYPTO_free(str->data)
//   8b 44 24 10      MOV EAX, [ESP+0x10]           ; data  (arg2, after PUSH EAX)
//   8b 4c 24 14      MOV ECX, [ESP+0x14]           ; len   (arg3, after PUSH EAX)
//   83 c4 04         ADD ESP, 0x4                  ; caller cleans PUSH EAX
//   89 46 08         MOV [ESI+0x8], EAX            ; str->data = data
//   89 0e            MOV [ESI], ECX                ; str->length = len
//   5e               POP ESI
//   c3               RET
// null_data:
//   8b 54 24 0c      MOV EDX, [ESP+0xC]            ; data  (arg2)
//   8b 44 24 10      MOV EAX, [ESP+0x10]           ; len   (arg3)
//   89 56 08         MOV [ESI+0x8], EDX            ; str->data = data
//   89 06            MOV [ESI], EAX                ; str->length = len
//   5e               POP ESI
//   c3               RET
//
// Two code paths share the same PUSH ESI prologue. In the non-null path,
// arg2 (data) and arg3 (len) are reloaded from the stack at [ESP+0x10] and
// [ESP+0x14] because an extra PUSH EAX has shifted them up by 4 relative
// to the position they occupy in the null path ([ESP+0xC] / [ESP+0x10]).
//
// The CALL at 0x46444D resolves to FUN_004632f0 (_CRYPTO_free / OPENSSL_free).
// compare.py masks the 4-byte rel32 operand so the CALL bytes are not
// byte-compared against the orig.

extern "C" void FUN_004632f0(void *);   // _CRYPTO_free / OPENSSL_free

extern "C" __declspec(naked) void FUN_00464440(void *, void *, int) {
    __asm {
        // 00064440: 56               PUSH ESI
        push    esi
        // 00064441: 8b 74 24 08      MOV ESI, [ESP+8]   ; str (arg1)
        mov     esi, dword ptr [esp + 0x8]
        // 00064445: 8b 46 08         MOV EAX, [ESI+8]   ; str->data
        mov     eax, dword ptr [esi + 0x8]
        // 00064448: 85 c0            TEST EAX, EAX
        test    eax, eax
        // 0006444a: 74 18            JZ null_data
        jz      null_data
        // 0006444c: 50               PUSH EAX
        push    eax
        // 0006444d: e8 ...           CALL FUN_004632f0 (CRYPTO_free — reloc)
        call    FUN_004632f0
        // 00064452: 8b 44 24 10      MOV EAX, [ESP+0x10]  ; data (arg2, stack shifted by PUSH EAX)
        mov     eax, dword ptr [esp + 0x10]
        // 00064456: 8b 4c 24 14      MOV ECX, [ESP+0x14]  ; len  (arg3, stack shifted by PUSH EAX)
        mov     ecx, dword ptr [esp + 0x14]
        // 0006445a: 83 c4 04         ADD ESP, 4
        add     esp, 0x4
        // 0006445d: 89 46 08         MOV [ESI+8], EAX    ; str->data = data
        mov     dword ptr [esi + 0x8], eax
        // 00064460: 89 0e            MOV [ESI], ECX      ; str->length = len
        mov     dword ptr [esi], ecx
        // 00064462: 5e               POP ESI
        pop     esi
        // 00064463: c3               RET
        ret
    null_data:
        // 00064464: 8b 54 24 0c      MOV EDX, [ESP+0xC]  ; data (arg2)
        mov     edx, dword ptr [esp + 0xc]
        // 00064468: 8b 44 24 10      MOV EAX, [ESP+0x10] ; len  (arg3)
        mov     eax, dword ptr [esp + 0x10]
        // 0006446c: 89 56 08         MOV [ESI+8], EDX    ; str->data = data
        mov     dword ptr [esi + 0x8], edx
        // 0006446f: 89 06            MOV [ESI], EAX      ; str->length = len
        mov     dword ptr [esi], eax
        // 00064471: 5e               POP ESI
        pop     esi
        // 00064472: c3               RET
        ret
    }
}
