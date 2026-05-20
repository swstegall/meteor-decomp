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
// FUNCTION: ffxivgame 0x00404000 — scalar deleting destructor for the
//   SSO string class whose `_Eos(n)` is the sibling at 0x00403c40
//   (same `this+0x4` SSO buffer / heap-pointer, `this+0x14` Mysize,
//   `this+0x18` Myres layout).
//
//   __thiscall void *FUN_00404000(int delete_flag):
//     this (= ecx) and a single __stdcall-popped int arg.
//     If capacity (this+0x18) >= 16, frees the heap buffer at this+0x4.
//     Then resets the string to empty SSO state:
//        capacity = 15, size = 0, inline_buf[0] = 0.
//     If (delete_flag & 1), frees `this` (scalar-deleting-dtor convention).
//     Returns `this` in eax.
//
// Asm (58 bytes — Ghidra under-counted to 52; size_overrides.json
// extends to 58 to cover the `mov eax, esi; pop esi; ret 4` epilogue):
//
//   56                     push   esi
//   8b f1                  mov    esi, ecx
//   83 7e 18 10            cmp    dword ptr [esi+0x18], 0x10
//   72 0c                  jb     .skip_free                     ; cap < 16 -> SSO
//   8b 46 04               mov    eax, dword ptr [esi+0x4]
//   50                     push   eax
//   e8 RR RR RR RR         call   _free
//   83 c4 04               add    esp, 4
// .skip_free:
//   33 c0                  xor    eax, eax
//   f6 44 24 08 01         test   byte ptr [esp+8], 1            ; flags hold past 3 movs
//   c7 46 18 0f 00 00 00   mov    dword ptr [esi+0x18], 0x0f
//   89 46 14               mov    dword ptr [esi+0x14], eax
//   88 46 04               mov    byte ptr  [esi+0x4],  al
//   74 09                  jz     .no_delete
//   56                     push   esi
//   e8 RR RR RR RR         call   _free
//   83 c4 04               add    esp, 4
// .no_delete:
//   8b c6                  mov    eax, esi                       ; return this
//   5e                     pop    esi
//   c2 04 00               ret    4
//
// MSVC 2005's instruction scheduler is what slots the three non-flag-
// touching mov's between `test [esp+8],1` and the `jz`, so coaxing this
// out of source-level C++ is fragile. Naked asm pins the layout; the
// two `call _free` rel32 fields are masked out as relocations by
// tools/compare.py.

extern "C" void __cdecl _free(void *);

extern "C" __declspec(naked) void FUN_00404000() {
    __asm {
        push    esi
        mov     esi, ecx
        cmp     dword ptr [esi + 0x18], 0x10
        jb      skip_free
        mov     eax, dword ptr [esi + 0x4]
        push    eax
        call    _free
        add     esp, 4
    skip_free:
        xor     eax, eax
        test    byte ptr [esp + 8], 1
        mov     dword ptr [esi + 0x18], 0x0f
        mov     dword ptr [esi + 0x14], eax
        mov     byte ptr  [esi + 0x4],  al
        jz      no_delete
        push    esi
        call    _free
        add     esp, 4
    no_delete:
        mov     eax, esi
        pop     esi
        ret     4
    }
}
