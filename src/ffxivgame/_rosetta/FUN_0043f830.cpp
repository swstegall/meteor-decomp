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
// FUNCTION: ffxivgame 0x0003f830 — `__cdecl` range-clear loop that resets
//                                   an array of 0x1c-byte string-like
//                                   elements to SSO-empty state (63 bytes).
//
// __cdecl void FUN_0043f830(Elem* begin, Elem* end)
//   stack layout (at entry):
//     [ESP+0x04] : Elem*  begin  (first element in range)
//     [ESP+0x08] : Elem*  end    (one-past-end of range)
//
// Element layout (stride 0x1c = 28 bytes), inferred from accesses:
//   +0x00 : int           (not accessed — tag or vtable link)
//   +0x04 : union { char buf[16]; char* ptr; }   (SSO buf / heap ptr)
//   +0x14 : int           _Mysize   (string length)
//   +0x18 : int           _Myres   (capacity; <0x10 → SSO, ≥0x10 → heap)
//   Total = 4 + 16 + 4 + 4 = 28 = 0x1c ✓
//
// Body:
//   If begin == end, return immediately.
//   Otherwise do–while over [begin, end) with stride 0x1c:
//     if (_Myres >= 0x10 && ptr != NULL):
//         ECX = *(int*)(ptr - 4)    // allocation header (owner object)
//         FUN_0040df70(ptr)         // release heap buffer (__thiscall, ECX=owner)
//     _Myres = 0xf    // re-enter SSO mode (capacity = 15)
//     _Mysize = 0     // empty string
//     buf[0] = '\0'   // null-terminate SSO buf (byte write via BL=0)
//
// Calling convention: __cdecl (plain RET; caller cleans args).
//
// The ECX = [ptr − 4] before PUSH ptr + CALL FUN_0040df70 is the same
// pattern as FUN_0043bc60 — FUN_0040df70 is a __thiscall string-buffer
// release helper where the owner object lives 4 bytes before the buffer.
//
// Reloc-bearing sites (masked by tools/compare.py):
//   +0x22  CALL  rel32   → FUN_0040df70  (REL32)

extern "C" void FUN_0040df70();

extern "C" __declspec(naked) void FUN_0043f830()
{
    __asm {
        // 0003f830: 56
        push    esi
        // 0003f831: 8b 74 24 08
        mov     esi, dword ptr [esp + 0x8]
        // 0003f835: 57
        push    edi
        // 0003f836: 8b 7c 24 10
        mov     edi, dword ptr [esp + 0x10]
        // 0003f83a: 3b f7
        cmp     esi, edi
        // 0003f83c: 74 2e
        jz      done
        // 0003f83e: 53
        push    ebx
        // 0003f83f: 33 db
        xor     ebx, ebx
    loop_start:
        // 0003f841: 83 7e 18 10
        cmp     dword ptr [esi + 0x18], 0x10
        // 0003f845: 72 10
        jc      cleanup
        // 0003f847: 8b 46 04
        mov     eax, dword ptr [esi + 0x4]
        // 0003f84a: 3b c3
        cmp     eax, ebx
        // 0003f84c: 74 09
        jz      cleanup
        // 0003f84e: 8b 48 fc
        mov     ecx, dword ptr [eax - 0x4]
        // 0003f851: 50
        push    eax
        // 0003f852: e8 19 e7 fc ff
        call    FUN_0040df70
    cleanup:
        // 0003f857: c7 46 18 0f 00 00 00
        mov     dword ptr [esi + 0x18], 0x0f
        // 0003f85e: 89 5e 14
        mov     dword ptr [esi + 0x14], ebx
        // 0003f861: 88 5e 04
        mov     byte ptr [esi + 0x4], bl
        // 0003f864: 83 c6 1c
        add     esi, 0x1c
        // 0003f867: 3b f7
        cmp     esi, edi
        // 0003f869: 75 d6
        jnz     loop_start
        // 0003f86b: 5b
        pop     ebx
    done:
        // 0003f86c: 5f
        pop     edi
        // 0003f86d: 5e
        pop     esi
        // 0003f86e: c3
        ret
    }
}
