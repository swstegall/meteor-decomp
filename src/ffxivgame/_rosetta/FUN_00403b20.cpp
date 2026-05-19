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
// FUNCTION: ffxivgame 0x403b20 — case-insensitive key/value lookup
//
// __cdecl bool(const Entry *arr, unsigned count, C *obj, unsigned *out)
//
// Walks `count` entries of an 8-byte struct array. For each entry, calls
// `obj->get_field()` (sub_445210 — `mov eax, [ecx]; ret`, the canonical
// int-field-at-offset-0 getter) and `_stricmp`s its result against
// `arr[i].key`. On a match, copies `arr[i].value` to `*out` and returns 1;
// if the whole array is exhausted without a match, returns 0.
//
// Orig codegen (read from orig RVA 0x00003b20, 77 bytes):
//
//   push ebx                       ; save callee-saved
//   mov  ebx, [esp+0xC]            ; ebx = count   (interleaved load)
//   push ebp
//   push esi
//   xor  esi, esi                  ; i = 0
//   test ebx, ebx
//   push edi
//   jbe  not_found                 ; count == 0 — bail with al = 0
//   mov  ebp, [esp+0x1C]           ; ebp = obj
//   mov  edi, [esp+0x14]           ; edi = arr
// loop:
//   mov  eax, [edi+8*esi]          ; arr[i].key
//   push eax                       ; stricmp arg2
//   mov  ecx, ebp
//   call sub_445210                ; obj->get_field()
//   push eax                       ; stricmp arg1
//   call __stricmp
//   add  esp, 8
//   test eax, eax
//   je   found                     ; == 0 means equal
//   add  esi, 1
//   cmp  esi, ebx
//   jb   loop                      ; unsigned compare → JB / JBE
// not_found:
//   pop edi; pop esi; pop ebp
//   xor al, al
//   pop ebx
//   ret
// found:
//   mov ecx, [edi+8*esi+4]         ; arr[i].value
//   mov edx, [esp+0x20]            ; out
//   pop edi; pop esi; pop ebp
//   mov [edx], ecx
//   mov al, 1
//   pop ebx
//   ret

#include <string.h>

// sub_445210 — `int field; int get_field() { return field; }` (Asm: 8b 01 c3).
// Declared as a __thiscall member function so MSVC emits the same
// `mov ecx, this; call get_field` sequence as the orig. The class layout
// stores a pointer in the first slot; we read it as `const char *` so the
// stricmp arg is well-typed at the source level.
class C { const char *key_; public: const char *get_field(); };

struct Entry {
    const char *key;
    unsigned int value;
};

extern "C" char __cdecl FUN_00403b20(
    const Entry *arr, unsigned int count, C *obj, unsigned int *out)
{
    for (unsigned int i = 0; i < count; ++i) {
        if (_stricmp(obj->get_field(), arr[i].key) == 0) {
            *out = arr[i].value;
            return 1;
        }
    }
    return 0;
}
