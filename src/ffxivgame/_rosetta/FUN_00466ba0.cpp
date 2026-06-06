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
// FUNCTION: ffxivgame 0x00466ba0 — _BUF_MEM_new (OpenSSL) (56 B / 0x38)
//
//   BUF_MEM * __cdecl _BUF_MEM_new(void)
//
//   BUF_MEM layout (inferred from the asm):
//     +0x00  int    length
//     +0x04  char  *data
//     +0x08  int    max
//
// Source shape (OpenSSL buffer.c BUF_MEM_new):
//
//   BUF_MEM *BUF_MEM_new(void) {
//       BUF_MEM *ret;
//       ret = CRYPTO_malloc(sizeof(BUF_MEM), "buffer.c", 67);
//       if (ret == NULL) {
//           ERR_PUT_error(ERR_LIB_BUF, BUF_F_BUF_MEM_NEW,
//                         ERR_R_MALLOC_FAILURE, "buffer.c", 70);
//           return NULL;
//       }
//       ret->length = 0;
//       ret->max    = 0;
//       ret->data   = NULL;
//       return ret;
//   }
//
// Inspection (orig bytes at RVA 0x00066ba0, 56 bytes):
//
//   push  0x43               ; line 67 (CRYPTO_malloc line arg)
//   push  0x00f78ee4         ; "buffer.c" string (VA in .rdata)
//   push  0x0c               ; sizeof(BUF_MEM) = 12
//   call  0x00463150         ; rel32 → CRYPTO_malloc
//   xor   ecx, ecx           ; ecx = 0  (reused for CMP and struct init)
//   add   esp, 0x0c
//   cmp   eax, ecx           ; ret == NULL ?
//   jnz   success            ; non-NULL → init and return
//   push  0x46               ; line 70 (ERR_PUT_error line arg)
//   push  0x00f78ee4         ; "buffer.c" string
//   push  0x41               ; BUF_F_BUF_MEM_NEW (func code)
//   push  0x65               ; ERR_R_MALLOC_FAILURE (reason)
//   push  0x07               ; ERR_LIB_BUF (lib code)
//   call  0x0045c940         ; rel32 → ERR_PUT_error
//   add   esp, 0x14
//   xor   eax, eax           ; return NULL
//   ret
// success:
//   mov   dword ptr [eax],       ecx   ; ret->length = 0
//   mov   dword ptr [eax + 0x8], ecx   ; ret->max    = 0
//   mov   dword ptr [eax + 0x4], ecx   ; ret->data   = NULL
//   ret
//
// Calling convention: __cdecl (no args; caller cleans nothing — zero args).
// No stack frame; no callee-saved registers.
//
// Key MSVC 2005 scheduling idiom:
//   XOR ECX, ECX is hoisted before the ADD ESP / CMP so that ECX=0 is live
//   as both the comparand for CMP EAX, ECX and as the source for the three
//   MOV stores that zero the returned struct.  A simpler compiler would
//   emit TEST EAX, EAX / JZ and three MOV EAX, 0 stores instead.
//
// The 0x00f78ee4 immediate is the virtual address of the "buffer.c" string
// literal in the game binary's .rdata section.  In inline MASM it is emitted
// as a plain imm32 immediate (no relocation record) so the four bytes match
// the orig binary verbatim.  The two CALL rel32 immediates are masked by
// tools/compare.py.

extern "C" void __cdecl FUN_00463150();   // CRYPTO_malloc
extern "C" void __cdecl FUN_0045c940();   // ERR_PUT_error

extern "C" __declspec(naked) void FUN_00466ba0() {
    __asm {
        push    0x43
        push    0x00f78ee4
        push    0x0c
        call    FUN_00463150
        xor     ecx, ecx
        add     esp, 0x0c
        cmp     eax, ecx
        jnz     success
        push    0x46
        push    0x00f78ee4
        push    0x41
        push    0x65
        push    0x07
        call    FUN_0045c940
        add     esp, 0x14
        xor     eax, eax
        ret
    success:
        mov     dword ptr [eax], ecx
        mov     dword ptr [eax + 0x8], ecx
        mov     dword ptr [eax + 0x4], ecx
        ret
    }
}
