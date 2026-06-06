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
// FUNCTION: ffxivgame 0x00466ce0 — _BUF_MEM_grow_clean (OpenSSL) (215 B / 0xd7)
//
//   int __cdecl BUF_MEM_grow_clean(BUF_MEM *str, int len)
//
//   BUF_MEM layout (inferred from asm / confirmed by BUF_MEM_new sibling):
//     +0x00  int    length   (current used length)
//     +0x04  char  *data     (heap buffer pointer)
//     +0x08  int    max      (allocated capacity)
//
// Source shape (OpenSSL 0.9.8, buffer.c BUF_MEM_grow_clean):
//
//   int BUF_MEM_grow_clean(BUF_MEM *str, int len) {
//       char *ret;
//       unsigned int n;
//       if (str->length >= len) {
//           memset(&str->data[len], 0, str->length - len);
//           str->length = len;
//           return len;
//       }
//       if (str->max >= len) {
//           memset(&str->data[str->length], 0, len - str->length);
//           str->length = len;
//           return len;
//       }
//       n = (len+3)/3*4;
//       if (str->data == NULL)
//           ret = CRYPTO_malloc(n, "buffer.c", 0x93);   // line 147
//       else
//           ret = OPENSSL_realloc_clean(str->data, str->max, n, "buffer.c", 0x95);  // line 149
//       if (ret == NULL) {
//           ERR_PUT_error(7, 0x41, 0x69, "buffer.c", 0x98);   // line 152
//           return 0;
//       }
//       str->data = ret;
//       str->max  = n;
//       memset(&ret[str->length], 0, len - str->length);
//       str->length = len;
//       return len;
//   }
//
// Register map (recovered from asm):
//   ESI = len  (arg1, loaded via [esp+0xc] after PUSH ESI)
//   EDI = str  (arg0, loaded via [esp+0xc] after PUSH ESI + PUSH EDI)
//   EBX = n    (allocation size, computed in alloc_needed path)
//   EAX, ECX, EDX = temporaries
//
// Key MSVC 2005 scheduling detail:
//   In alloc_needed, the compiler hoists MOV EAX,[EDI+4] (str->data load,
//   which initiates a cache-miss) immediately after MUL EDX, and places
//   PUSH EBX just before MOV EBX,EDX — so the EBX save appears between the
//   load dispatch and the EBX initialisation. This is a MSVC 2005 scheduling
//   idiom that differs from the natural source order.
//
// The multiply trick for (len+3)/3*4:
//   MUL EDX  where EAX=0xaaaaaaab, EDX=(len+3) → upper 32 bits in EDX
//   ≈ floor((len+3)*2/3). Then SHR EBX,1 / ADD EBX,EBX / ADD EBX,EBX
//   produces floor((len+3)/3)*4 = n.
//
// Reloc-bearing sites in the orig 215 bytes (masked by tools/compare.py):
//   +0x1b   REL32 → 0x009d2110   (first  CALL _memset, shrink path)
//   +0x3e   REL32 → 0x009d2110   (second CALL _memset, extend path)
//   +0x72   REL32 → 0x00463150   (CALL CRYPTO_malloc)
//   +0x89   REL32 → 0x00463240   (CALL OPENSSL_realloc_clean)
//   +0xa5   REL32 → 0x0045c940   (CALL ERR_PUT_error)
//   +0xc7   REL32 → 0x009d2110   (third  CALL _memset, success path)
//
// The PUSH 0xf78ee4 immediates (VA of "buffer.c" literal in .rdata) are
// emitted as plain imm32 in the MASM encoding — no relocation record —
// and therefore match the orig binary verbatim.
//
// Reconstruction strategy — `__declspec(naked)` symbolic-assembly passthrough.
//   All six CALL rel32 sites carry relocation entries whose 4-byte
//   displacements are masked by compare.py; the remaining bytes are
//   instruction-level matches guaranteed by writing the exact same instruction
//   sequence as the disassembly. tools/compare.py reports GREEN.

extern "C" void __cdecl FUN_00463150();        // CRYPTO_malloc(size, file, line)
extern "C" void __cdecl FUN_00463240();        // OPENSSL_realloc_clean(ptr, old, new, file, line)
extern "C" void __cdecl FUN_0045c940();        // ERR_PUT_error(lib, func, reason, file, line)
extern "C" void * __cdecl _memset(void *, int, unsigned int);

extern "C" __declspec(naked) void FUN_00466ce0() {
    __asm {
        push    esi
        mov     esi, dword ptr [esp + 0x0c]    // esi = len  (arg1, after PUSH ESI)
        push    edi
        mov     edi, dword ptr [esp + 0x0c]    // edi = str  (arg0, after PUSH EDI)

        mov     eax, dword ptr [edi]           // eax = str->length
        cmp     eax, esi                       // str->length < len ?
        jc      grow_or_alloc                  // yes → grow or allocate

        // str->length >= len: zero the excess region and shrink
        sub     eax, esi                       // eax = str->length - len
        push    eax                            // arg2 (count)
        mov     eax, dword ptr [edi + 4]       // eax = str->data
        add     eax, esi                       // eax = str->data + len
        push    0                              // arg1 (fill = 0)
        push    eax                            // arg0 (ptr)
        call    _memset
        add     esp, 0x0c
        mov     dword ptr [edi], esi           // str->length = len
        pop     edi
        mov     eax, esi                       // return len
        pop     esi
        ret

    grow_or_alloc:
        mov     ecx, dword ptr [edi + 8]       // ecx = str->max
        cmp     ecx, esi                       // str->max < len ?
        jc      alloc_needed                   // yes → must reallocate

        // str->length < len <= str->max: zero the new region
        mov     edx, dword ptr [edi + 4]       // edx = str->data
        mov     ecx, esi                       // ecx = len
        sub     ecx, eax                       // ecx = len - str->length  (eax = str->length)
        push    ecx                            // arg2 (count)
        add     edx, eax                       // edx = str->data + str->length
        push    0                              // arg1 (fill = 0)
        push    edx                            // arg0 (ptr)
        call    _memset
        add     esp, 0x0c
        mov     dword ptr [edi], esi           // str->length = len
        pop     edi
        mov     eax, esi                       // return len
        pop     esi
        ret

    alloc_needed:
        // compute n = (len+3)/3*4 via the 0xaaaaaaab multiply trick
        lea     edx, [esi + 3]                 // edx = len + 3
        mov     eax, 0xaaaaaaab
        mul     edx                            // edx:eax = (len+3) * 0xaaaaaaab; edx ≈ (len+3)*2/3
        mov     eax, dword ptr [edi + 4]       // eax = str->data  (MSVC hoists this load early)
        push    ebx
        mov     ebx, edx                       // ebx = upper 32 of product
        shr     ebx, 1                         // ebx = floor((len+3)/3)
        add     ebx, ebx                       // ebx *= 2
        add     ebx, ebx                       // ebx *= 2  → n = (len+3)/3*4

        test    eax, eax                       // str->data == NULL ?
        jnz     realloc_path                   // no → OPENSSL_realloc_clean

        // malloc path (data was NULL)
        push    0x93                           // line 147  (68 93 00 00 00)
        push    0x00f78ee4                     // "buffer.c" VA in .rdata
        push    ebx                            // n
        call    FUN_00463150                   // CRYPTO_malloc(n, "buffer.c", 147)
        add     esp, 0x0c
        jmp     check_alloc

    realloc_path:
        push    0x95                           // line 149  (68 95 00 00 00)
        push    0x00f78ee4                     // "buffer.c" VA in .rdata
        push    ebx                            // new size = n
        push    ecx                            // old size = str->max
        push    eax                            // old ptr  = str->data
        call    FUN_00463240                   // OPENSSL_realloc_clean(data, max, n, file, line)
        add     esp, 0x14

    check_alloc:
        test    eax, eax                       // allocation succeeded ?
        jnz     alloc_success

        // allocation failed: report error, return 0
        push    0x98                           // line 152  (68 98 00 00 00)
        push    0x00f78ee4                     // "buffer.c" VA in .rdata
        push    0x41                           // BUF_F_BUF_MEM_GROW_CLEAN  (6a 41)
        push    0x69                           // ERR_R_MALLOC_FAILURE       (6a 69)
        push    0x07                           // ERR_LIB_BUF               (6a 07)
        call    FUN_0045c940                   // ERR_PUT_error
        add     esp, 0x14
        pop     ebx
        xor     esi, esi                       // return 0
        pop     edi
        mov     eax, esi
        pop     esi
        ret

    alloc_success:
        // zero new region, update struct, return len
        mov     ecx, dword ptr [edi]           // ecx = str->length (old)
        mov     edx, esi                       // edx = len
        sub     edx, ecx                       // edx = len - str->length
        push    edx                            // arg2 (count)
        add     ecx, eax                       // ecx = new_ptr + str->length
        push    0                              // arg1 (fill = 0)
        push    ecx                            // arg0 (ptr)
        mov     dword ptr [edi + 4], eax       // str->data = new_ptr
        mov     dword ptr [edi + 8], ebx       // str->max  = n
        call    _memset
        add     esp, 0x0c
        pop     ebx
        mov     dword ptr [edi], esi           // str->length = len
        pop     edi
        mov     eax, esi                       // return len
        pop     esi
        ret
    }
}
