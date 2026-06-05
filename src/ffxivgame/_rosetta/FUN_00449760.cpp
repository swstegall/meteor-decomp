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
// FUNCTION: ffxivgame 0x00449760 — `std::basic_string<wchar_t>` capacity
//                                  growth + buffer (re)allocate prologue,
//                                  under an MSVC C++ SEH unwind frame
//                                  (__thiscall, 135 bytes / 0x87).
//
// This is the front half of a wide-string `_Grow`/reserve path. ECX is
// the string object (`this`); [ebp+8] is the requested new size. The body
// computes the grown capacity using the classic MSVC STL "geometric
// growth, clamped" idiom and then allocates `(cap + 1) * 2` bytes (the
// `+1` is the NUL terminator, `*2` is sizeof(wchar_t)) via the global
// allocator at 0x0044d500, before tail-jumping to the shared
// copy-into-new-buffer continuation at 0x00449818.
//
//   size_type __thiscall grow(basic_string *this, size_type newsize) {
//       size_type cap = newsize;
//       if ((newsize | 7) <= (size_type)-2) {            // |7: round-up bias
//           size_type half  = this->_Myres >> 1;          // [this+0x18] / 2
//           size_type third = ((newsize | 7) / 3);        // 0xaaaaaaab mulhi
//           if (third < half &&                           // geometric grow
//               this->_Myres <= (size_type)-2 - half)     //   overflow guard
//               cap = half + this->_Myres;                // = _Myres * 1.5
//       }
//       // try { ... allocate ... } SEH state machine:
//       this->_Mysize_slot = 0;                           // [ebp-4] = 0
//       void *p = operator_new((cap + 1) * 2, 0, 0xc);    // (cap*2 + 2)
//       this->_Mysize_slot = -1;                           // [ebp-4] = -1
//       return p;  /* tail → 0x00449818 copies old data in */
//   }
//
// The `0xaaaaaaab` × ESI / mulhi-then-`SHR 1` pair is the standard MSVC
// divide-by-3, and the `(size_type)-2 - half` compare is the textbook STL
// `_Grow` overflow clamp. The element width (`*2 + 2` on the allocate
// size) pins the char type to wchar_t (sizeof == 2).
//
// MSVC 2005 codegen quirks reproduced verbatim:
//   • The function does not have its own epilogue — it tail-`jmp`s
//     (`eb 31`) into the shared continuation at 0x00449818. That short
//     relative jump carries no relocation (PC-relative, self-contained),
//     so it is emitted as raw bytes; a source-level `goto`/`return f()`
//     would compile to a near `e9 rel32` with a fixup instead.
//   • The `lea ecx,[esi+esi+2]` is a base+index form (`8d 4c 36 02`), not
//     the `[esi*2+2]` disp32 scaled-index form a literal `2*x+2` would
//     pick.
//
// Reloc-bearing positions in the resulting .obj (all masked in the diff):
//   off 0x06   IMAGE_REL_I386_DIR32  → SEH handler stub (orig 0x00e57480)
//   off 0x18   IMAGE_REL_I386_DIR32  → __security_cookie (orig 0x012ea8b0)
//   off 0x74   IMAGE_REL_I386_REL32  → operator new helper (orig 0x0044d500)
//
// Calling convention: __thiscall (ECX = this). No `ret N`; control leaves
// via the tail jmp, not a local epilogue.

extern "C" {

// SEH handler stub trampoline at 0x00e57480 in orig. Provides a unique
// symbol for the DIR32 reloc; the bytes at this position are masked.
int FUN_00e57480();

// Global allocate helper (orig 0x0044d500). __cdecl, 3 stack args
// (byte_count, 0, 0xc); caller cleans 12 bytes.
int FUN_0044d500();

extern unsigned __security_cookie;

} // extern "C"

extern "C" __declspec(naked) void FUN_00449760() {
    __asm {
        // --- SEH / GS prologue --------------------------------------
        push    ebp
        mov     ebp, esp
        push    -1                            ; initial unwind state
        push    offset FUN_00e57480           ; SEH handler trampoline (DIR32)
        mov     eax, dword ptr fs:[0]
        push    eax                           ; prev fs:[0] → SEH chain link
        sub     esp, 0xc
        push    ebx
        push    esi
        push    edi
        mov     eax, __security_cookie        ; GS cookie (DIR32, a1 form)
        xor     eax, ebp
        push    eax
        lea     eax, [ebp - 0xc]
        mov     dword ptr fs:[0], eax         ; install SEH registration
        mov     dword ptr [ebp - 0x10], esp
        mov     edi, ecx                      ; edi = this
        mov     dword ptr [ebp - 0x14], edi   ; spill this

        // --- compute grown capacity ---------------------------------
        mov     eax, dword ptr [ebp + 8]      ; eax = newsize
        mov     esi, eax
        or      esi, 7                        ; bias to multiple-of-8 - 1
        cmp     esi, -2
        jbe     clamp_ok
        mov     esi, eax                      ; saturate path: cap = newsize
        jmp     do_alloc
    clamp_ok:
        mov     ebx, dword ptr [edi + 0x18]   ; ebx = this->_Myres
        mov     eax, 0xaaaaaaab
        mul     esi                           ; edx:eax = (newsize|7) * magic
        mov     ecx, ebx
        shr     ecx, 1                        ; ecx = _Myres / 2
        shr     edx, 1                        ; edx = (newsize|7) / 3
        cmp     edx, ecx
        jnc     do_alloc                      ; third >= half → keep newsize
        mov     eax, 0xfffffffe
        sub     eax, ecx                      ; eax = (size_type)-2 - half
        cmp     ebx, eax
        ja      do_alloc                      ; would overflow → keep newsize
        lea     esi, [ecx + ebx]              ; cap = _Myres + _Myres/2

        // --- allocate (cap + 1) * 2 bytes ---------------------------
    do_alloc:
        push    0xc
        lea     ecx, [esi + esi + 2]          ; (cap*2) + 2 = (cap+1)*sizeof(wchar_t)
        push    0
        push    ecx
        mov     dword ptr [ebp - 4], 0        ; SEH state = 0
        call    FUN_0044d500
        add     esp, 0xc
        mov     dword ptr [ebp + 8], eax      ; stash new buffer
        mov     dword ptr [ebp - 4], -1       ; SEH state = -1

        // --- tail jump into shared copy continuation (0x00449818) ---
        // Raw short rel8 jmp — PC-relative, no relocation.
        _emit 0xeb
        _emit 0x31
    }
}
