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
// FUNCTION: ffxivgame 0x0005e680 — OpenSSL `_X509_ALGOR_get0`
//                                  (__cdecl void, 0x3b bytes / 59 bytes)
//
// Signature (derived from asm):
//
//   void X509_ALGOR_get0(
//       void       **paobj,   // [esp+0x04] — receive algor->algorithm
//       int         *pptype,  // [esp+0x08] — receive algor->parameter->type
//       void       **ppval,   // [esp+0x0c] — receive algor->parameter->value
//       X509_ALGOR  *algor    // [esp+0x10] — input struct
//   );
//
// X509_ALGOR layout (2 pointer fields, 8 bytes total):
//   +0x00  void           *algorithm      ; [edx+0]
//   +0x04  ASN1_TYPE      *parameter      ; [edx+4]
//
// ASN1_TYPE layout (int + pointer, 8 bytes):
//   +0x00  int             type           ; [eax+0] / [ecx+0]
//   +0x04  void           *value          ; [ecx+4]
//
// Function logic:
//   1. if (paobj)   *paobj  = algor->algorithm;
//   2. if (!pptype) return;
//   3. if (!algor->parameter) { *pptype = -1; return; }  // V_ASN1_UNDEF
//   4. *pptype = algor->parameter->type;
//   5. if (ppval)   *ppval  = algor->parameter->value;
//
// Register map (MSVC 2005 /O2, leaf — no callee-save push/pop):
//   EAX: arg1 (paobj), then algor->parameter, then arg3 (ppval)
//   ECX: temp for first store, then arg2 (pptype)
//   EDX: arg4 (algor), held across entire function
//
// Calling convention: __cdecl — caller cleans stack, plain `ret` (c3).
//   Two early-return `ret` would also be `c3`; the function is a void
//   leaf with no frame pointer and no callee-save registers.
//
// Asm (59 bytes, no relocations — all bytes are structural code):
//   8b 44 24 04              MOV EAX, [ESP+0x04]        ; EAX = paobj
//   85 c0                    TEST EAX, EAX              ; test paobj
//   8b 54 24 10              MOV EDX, [ESP+0x10]        ; EDX = algor
//   74 04                    JZ  skip_paobj
//   8b 0a                    MOV ECX, [EDX]             ; ECX = algor->algorithm
//   89 08                    MOV [EAX], ECX             ; *paobj = algor->algorithm
// skip_paobj:
//   8b 4c 24 08              MOV ECX, [ESP+0x08]        ; ECX = pptype
//   85 c9                    TEST ECX, ECX              ; test pptype
//   74 22                    JZ  done
//   8b 42 04                 MOV EAX, [EDX+0x04]        ; EAX = algor->parameter
//   85 c0                    TEST EAX, EAX              ; test parameter
//   75 07                    JNZ non_null
//   c7 01 ff ff ff ff        MOV dword ptr [ECX], -1    ; *pptype = V_ASN1_UNDEF
//   c3                       RET
// non_null:
//   8b 00                    MOV EAX, [EAX]             ; EAX = parameter->type
//   89 01                    MOV [ECX], EAX             ; *pptype = parameter->type
//   8b 44 24 0c              MOV EAX, [ESP+0x0c]        ; EAX = ppval
//   85 c0                    TEST EAX, EAX              ; test ppval
//   74 08                    JZ  done
//   8b 4a 04                 MOV ECX, [EDX+0x04]        ; ECX = algor->parameter
//   8b 51 04                 MOV EDX, [ECX+0x04]        ; EDX = parameter->value
//   89 10                    MOV [EAX], EDX             ; *ppval = parameter->value
// done:
//   c3                       RET

extern "C" __declspec(naked) void FUN_0045e680() {
    __asm {
        mov     eax, dword ptr [esp+4]
        test    eax, eax
        mov     edx, dword ptr [esp+10h]
        jz      skip_paobj
        mov     ecx, dword ptr [edx]
        mov     dword ptr [eax], ecx
    skip_paobj:
        mov     ecx, dword ptr [esp+8]
        test    ecx, ecx
        jz      done
        mov     eax, dword ptr [edx+4]
        test    eax, eax
        jnz     non_null
        mov     dword ptr [ecx], -1
        ret
    non_null:
        mov     eax, dword ptr [eax]
        mov     dword ptr [ecx], eax
        mov     eax, dword ptr [esp+0ch]
        test    eax, eax
        jz      done
        mov     ecx, dword ptr [edx+4]
        mov     edx, dword ptr [ecx+4]
        mov     dword ptr [eax], edx
    done:
        ret
    }
}
