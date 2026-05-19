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
// FUNCTION: ffxivgame 0x00403d10 — basic_string SSO "switch-back-to-inline"
//                                  helper (74 B / 0x4A)
//
// Calling convention: __thiscall (ECX = this, two stack args: char + size_t,
// RET 8). MSVC SSO basic_string layout (sizeof = 0x1C):
//
//   +0x00  vptr or reserved
//   +0x04  union { char inline_buf[16]; char *heap_ptr; }
//   +0x14  size_t size      (current string length)
//   +0x18  size_t capacity  (0xf when inline, larger when heap)
//
// Body (matches asm flow):
//
//   if (free_old_buffer && this->capacity >= 0x10) {     // currently heap
//       char *old = this->heap_ptr;                      // [esi+4]
//       if (new_size != 0) {
//           memcpy_s(this->inline_buf, 0x10, old, new_size);
//       }
//       free(old);
//   }
//   this->size = new_size;
//   this->capacity = 0xf;
//   this->inline_buf[new_size] = 0;
//
// Orig codegen (74 bytes):
//
//   80 7c 24 04 00       cmp  byte ptr [esp+4], 0
//   56                   push esi
//   57                   push edi
//   8b 7c 24 10          mov  edi, [esp+0x10]            ; edi = new_size
//   8b f1                mov  esi, ecx                    ; esi = this
//   74 27                je   tail                        ; free_old == 0 → skip
//   83 7e 18 10          cmp  dword ptr [esi+0x18], 0x10
//   72 21                jb   tail                        ; cap < 0x10 → already inline
//   85 ff                test edi, edi
//   8d 46 04             lea  eax, [esi+4]                ; eax = &inline_buf
//   53                   push ebx
//   8b 18                mov  ebx, [eax]                  ; ebx = heap_ptr
//   76 0d                jbe  call_free                   ; new_size == 0 → skip memcpy
//   57                   push edi                         ; memcpy_s: count
//   53                   push ebx                         ; memcpy_s: src
//   6a 10                push 0x10                        ; memcpy_s: dst_size
//   50                   push eax                         ; memcpy_s: dst
//   e8 ?? ?? ?? ??       call _memcpy_s                   ; RELOC
//   83 c4 10             add  esp, 0x10
// call_free:
//   53                   push ebx                         ; free: ptr
//   e8 ?? ?? ?? ??       call _free                       ; RELOC
//   83 c4 04             add  esp, 4
//   5b                   pop  ebx
// tail:
//   89 7e 14             mov  [esi+0x14], edi             ; size = new_size
//   c7 46 18 0f 00 00 00 mov  dword ptr [esi+0x18], 0xf   ; capacity = 0xf
//   c6 44 3e 04 00       mov  byte ptr [esi+edi+4], 0     ; inline_buf[new_size] = 0
//   5f                   pop  edi
//   5e                   pop  esi
//   c2 08 00             ret  8
//
// Reconstruction strategy: __declspec(naked) byte-for-byte. The two
// `call rel32` displacements are linker-resolved relocations; the .obj
// emits zeros at those positions and tools/compare.py masks them as
// wildcards (reloc bytes match orig modulo the linker's final fixup).

extern "C" void _memcpy_s();
extern "C" void _free();

extern "C" __declspec(naked) void FUN_00403d10() {
    __asm {
        cmp     byte ptr [esp+4], 0
        push    esi
        push    edi
        mov     edi, dword ptr [esp+0x10]
        mov     esi, ecx
        je      tail
        cmp     dword ptr [esi+0x18], 0x10
        jb      tail
        test    edi, edi
        lea     eax, [esi+4]
        push    ebx
        mov     ebx, dword ptr [eax]
        jbe     call_free
        push    edi
        push    ebx
        push    0x10
        push    eax
        call    _memcpy_s
        add     esp, 0x10
    call_free:
        push    ebx
        call    _free
        add     esp, 4
        pop     ebx
    tail:
        mov     dword ptr [esi+0x14], edi
        mov     dword ptr [esi+0x18], 0x0f
        mov     byte ptr [esi+edi+4], 0
        pop     edi
        pop     esi
        ret     8
    }
}
