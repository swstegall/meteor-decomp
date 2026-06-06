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
// FUNCTION: ffxivgame 0x0045a920 — basic_string buffer-unwrap shim for
//                                  FUN_0045a590 (69 B / 0x45)
//
// Calling convention: __cdecl, 2 stack args:
//   arg1 — destination object pointer (returned)
//   arg2 — pointer to MSVC basic_string (sizeof=0x1C SSO layout):
//             +0x04  union { char inline_buf[16]; char *heap_ptr; }
//             +0x14  size_t size
//             +0x18  size_t capacity  (0xf when SSO, >=0x10 when heap)
//
// Unwraps arg2's buffer based on capacity:
//   capacity >= 0x10  →  heap path: buf = [arg2+4] (heap ptr)
//   capacity <  0x10  →  SSO  path: buf =  arg2+4  (inline buffer)
// Then calls FUN_0045a590(arg1, buf, size) and returns arg1.
//
// The PUSH ECX / MOV dword ptr [ESP],0 sequence allocates a dead
// zero-initialized 4-byte local; MSVC 2005 generates this artifact for
// certain optimized SSO-dispatch patterns.
//
// Orig codegen (69 bytes):
//
//   51                    push    ecx
//   8b 44 24 0c           mov     eax, [esp+0ch]         ; arg2
//   83 78 18 10           cmp     dword ptr [eax+18h], 10h
//   8b 48 14              mov     ecx, [eax+14h]         ; size
//   c7 04 24 00 00 00 00  mov     dword ptr [esp], 0     ; dead local
//   72 18                 jc      sso_path               ; +0x18
//   8b 40 04              mov     eax, [eax+4]           ; heap_ptr
//   56                    push    esi
//   8b 74 24 0c           mov     esi, [esp+0ch]         ; arg1
//   51                    push    ecx
//   50                    push    eax
//   56                    push    esi
//   e8 ?? ?? ?? ??        call    FUN_0045a590            ; RELOC
//   83 c4 0c              add     esp, 0ch
//   8b c6                 mov     eax, esi
//   5e                    pop     esi
//   59                    pop     ecx
//   c3                    ret
// sso_path:
//   56                    push    esi
//   8b 74 24 0c           mov     esi, [esp+0ch]         ; arg1
//   51                    push    ecx
//   83 c0 04              add     eax, 4                 ; inline buf
//   50                    push    eax
//   56                    push    esi
//   e8 ?? ?? ?? ??        call    FUN_0045a590            ; RELOC
//   83 c4 0c              add     esp, 0ch
//   8b c6                 mov     eax, esi
//   5e                    pop     esi
//   59                    pop     ecx
//   c3                    ret

extern "C" void FUN_0045a590();

extern "C" __declspec(naked) void FUN_0045a920() {
    __asm {
        push    ecx
        mov     eax, dword ptr [esp+0xc]
        cmp     dword ptr [eax+0x18], 0x10
        mov     ecx, dword ptr [eax+0x14]
        mov     dword ptr [esp], 0x0
        jc      sso_path

        mov     eax, dword ptr [eax+0x4]
        push    esi
        mov     esi, dword ptr [esp+0xc]
        push    ecx
        push    eax
        push    esi
        call    FUN_0045a590
        add     esp, 0xc
        mov     eax, esi
        pop     esi
        pop     ecx
        ret

    sso_path:
        push    esi
        mov     esi, dword ptr [esp+0xc]
        push    ecx
        add     eax, 0x4
        push    eax
        push    esi
        call    FUN_0045a590
        add     esp, 0xc
        mov     eax, esi
        pop     esi
        pop     ecx
        ret
    }
}
