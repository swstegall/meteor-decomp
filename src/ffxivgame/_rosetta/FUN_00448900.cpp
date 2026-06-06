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
// FUNCTION: ffxivgame 0x00448900 — __thiscall string-append helper (72 B / 0x48)
//
// __thiscall <this> FUN_00448900(this, char *str)   // RET 4, returns this
//
// Layout of the receiver (a growable string/buffer object):
//   +0x00  char  *m_data    // backing buffer
//   +0x08  size_t m_len     // current logical length (includes terminator)
//
// Body (matches asm flow exactly):
//
//   size_t old = this->m_len;                       // EBX = [esi+8]
//   size_t n   = strlen(str) + 1;                   // EDI = strlen+1
//   this->reserve(old + n - 1, 1);                  // FUN_00447010(old+n-1, 1)
//   memcpy(this->m_data + old - 1, str, n);         // FUN_009d5110
//   return this;
//
// Inspection (read from the orig 72-byte slice at RVA 0x00048900):
//
//   53                 push ebx
//   55                 push ebp
//   8b 6c 24 0c        mov  ebp, [esp+0xc]          ; ebp = str
//   56                 push esi
//   8b f1              mov  esi, ecx                ; esi = this
//   8b 5e 08           mov  ebx, [esi+8]            ; ebx = m_len
//   8b c5              mov  eax, ebp
//   57                 push edi
//   8d 50 01           lea  edx, [eax+1]            ; edx = str+1 (loop base)
//  loop:                                            ; inline strlen
//   8a 08              mov  cl, [eax]
//   83 c0 01           add  eax, 1
//   84 c9              test cl, cl
//   75 f7              jnz  loop
//   2b c2              sub  eax, edx                ; eax = strlen
//   8d 78 01           lea  edi, [eax+1]            ; edi = strlen+1
//   6a 01              push 1
//   8d 44 3b ff        lea  eax, [ebx+edi-1]        ; eax = m_len + strlen
//   50                 push eax
//   8b ce              mov  ecx, esi
//   e8 e2 e6 ff ff     call FUN_00447010            ; reserve (rel32 reloc)
//   8b 0e              mov  ecx, [esi]              ; ecx = m_data
//   57                 push edi                     ; memcpy count = strlen+1
//   8d 54 19 ff        lea  edx, [ecx+ebx-1]        ; dst = m_data + m_len - 1
//   55                 push ebp                     ; memcpy src = str
//   52                 push edx                     ; memcpy dst
//   e8 d4 c7 58 00     call FUN_009d5110            ; memcpy (rel32 reloc)
//   83 c4 0c           add  esp, 0xc
//   5f                 pop  edi
//   8b c6              mov  eax, esi                ; return this
//   5e                 pop  esi
//   5d                 pop  ebp
//   5b                 pop  ebx
//   c2 04 00           ret  4
//
// Reloc-bearing sites (masked by tools/compare.py):
//   off 0x29  REL32 → FUN_00447010 (reserve/grow, .text)
//   off 0x37  REL32 → FUN_009d5110 (_memcpy, statically-linked CRT)
//
// Reconstruction strategy: __declspec(naked) so the inline-strlen loop
// shape, the EBX-cached m_len, and the two rel32 call fixups are pinned to
// the orig encoding.

extern "C" {

// FUN_00447010: __thiscall reserve/grow on the receiver. ECX = this,
// two stack args (new_len, flag); epilogue cleans both.
void FUN_00447010();

// FUN_009d5110: statically-linked CRT _memcpy (__cdecl).
void FUN_009d5110();

__declspec(naked) void FUN_00448900() {
    __asm {
        push    ebx
        push    ebp
        mov     ebp, dword ptr [esp + 0xc]
        push    esi
        mov     esi, ecx
        mov     ebx, dword ptr [esi + 8]
        mov     eax, ebp
        push    edi
        lea     edx, [eax + 1]
    strlen_loop:
        mov     cl, byte ptr [eax]
        add     eax, 1
        test    cl, cl
        jnz     strlen_loop
        sub     eax, edx
        lea     edi, [eax + 1]
        push    1
        lea     eax, [ebx + edi - 1]
        push    eax
        mov     ecx, esi
        call    FUN_00447010
        mov     ecx, dword ptr [esi]
        push    edi
        lea     edx, [ecx + ebx - 1]
        push    ebp
        push    edx
        call    FUN_009d5110
        add     esp, 0xc
        pop     edi
        mov     eax, esi
        pop     esi
        pop     ebp
        pop     ebx
        ret     4
    }
}

}  // extern "C"
