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
// FUNCTION: ffxivgame 0x004044e0 — `std::_Destroy_range` over a contiguous
// array of `std::basic_string<char>` (56 B). Walks `[first, last)` of
// `basic_string` records (sizeof == 0x1c) and, for each one, inlines the
// `_Tidy(true)` reset: if the string is heap-allocated (`_Myres >= 0x10`)
// free the buffer, then reset to an empty SSO string (`_Mysize = 0`,
// `_Myres = 0xF`, `_Bx._Buf[0] = '\0'`).
//
// Layout of each `basic_string<char>` record (0x1c bytes total):
//   +0x00 : (4-byte header — opaque to this routine)
//   +0x04 : SSO inline buffer (16 bytes) OR heap pointer when capacity >= 0x10
//   +0x14 : `_Mysize` (current length, not counting the null terminator)
//   +0x18 : `_Myres`  (capacity)
//
// Shape (reconstructed from the asm):
//
//   void _Destroy_range(basic_string *first, basic_string *last) {
//       if (first == last) return;
//       do {
//           if (first->_Myres >= 0x10) {
//               free(first->_Bx._Ptr);
//           }
//           first->_Myres  = 0xF;            // reset capacity to SSO max
//           first->_Mysize = 0;              // reset length
//           first->_Bx._Buf[0] = '\0';       // null-terminate the SSO buf
//           ++first;                         // advance by sizeof(basic_string)
//       } while (first != last);
//   }
//
// Calling convention: `__cdecl` (two stack args; caller cleans). The
// function shares its three-register epilogue (`pop edi; pop esi; ret`)
// with the empty-range early-exit path at orig 0x00404518 — that epilogue
// is 3 bytes of code + the `ret` byte (0xC3) which sit immediately after
// the 56-byte body and are NOT part of this function's `size` per the
// work-pool YAML. Both the `je` early-exit and the post-loop fall-through
// converge on those bytes; the naked __asm therefore ends with `pop ebx`
// and a bare `done:` label so MSVC emits exactly the orig 56-byte body
// with the `je` displacement computed against a one-past-end label.
//
// MSVC 2005 idioms pinned by the naked __asm:
//   - `xor ebx, ebx` once, then reused for both the dword-store at +0x14
//     (`mov [esi+0x14], ebx`) and the byte-store at +4 (`mov [esi+4], bl`) —
//     saves an immediate and lets the compiler eliminate the constant-0
//     materialisation per iteration.
//   - The `je 0x00404518` early-exit on empty range jumps PAST the body
//     into the shared 4-byte epilogue (`pop edi; pop esi; ret`) rather than
//     emitting a second epilogue inline. The short-jump displacement is
//     exactly 0x2A — one byte past the function body.
//   - The post-loop fall-through after `jne loop_top` exits via the same
//     shared epilogue by falling off the end of the function's 56-byte
//     code region into the same bytes.
//
// Reloc-bearing positions in the resulting .obj (masked in the diff):
//   off 0x1c   IMAGE_REL_I386_REL32  → _free (orig 0x009d1b17)

extern "C" {

// MSVCRT `free` thunk at orig 0x009d1b17. Declared as a function so the
// inline-asm `call _free` produces a REL32 reloc.
int _free();

} // extern "C"

extern "C" __declspec(naked) void FUN_004044e0() {
    __asm {
        push    esi
        mov     esi, dword ptr [esp + 8]       ; esi = first
        push    edi
        mov     edi, dword ptr [esp + 0x10]    ; edi = last
        cmp     esi, edi
        je      done                           ; empty range → fall through to shared epilogue
        push    ebx
        xor     ebx, ebx                       ; ebx = 0 (reused as zero source)
    loop_top:
        cmp     dword ptr [esi + 0x18], 0x10   ; _Myres >= 0x10 ?
        jb      skip_free
        mov     eax, dword ptr [esi + 4]       ; heap buffer pointer
        push    eax
        call    _free
        add     esp, 4
    skip_free:
        mov     dword ptr [esi + 0x18], 0xf    ; _Myres = 0xF (SSO max)
        mov     dword ptr [esi + 0x14], ebx    ; _Mysize = 0
        mov     byte ptr [esi + 4], bl         ; _Bx._Buf[0] = '\0'
        add     esi, 0x1c                      ; ++first
        cmp     esi, edi
        jne     loop_top
        pop     ebx
    done:
        // Shared epilogue at orig 0x00404518 (3 bytes + ret). Both the
        // `je done` early-exit on empty range and the post-loop
        // fall-through after `jne loop_top` converge here. The size
        // override at config/ffxivgame.size_overrides.json extends the
        // function's compared range to cover these bytes.
        pop     edi
        pop     esi
        ret
    }
}

// vim: ts=4 sts=4 sw=4 et
