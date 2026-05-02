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
// MSVC 2005 CRT helpers — `__alloca_probe`, `__alloca_probe_8`,
// `__alloca_probe_16`. These are the runtime stack-probing routines
// MSVC inserts when a function allocates >= 0x1000 bytes of stack
// (one PAGE boundary). Each probe touches every page on the way down
// to ensure the OS commits stack pages.
//
// The 8/16 variants round the requested size up to 8/16-byte alignment
// before forwarding to the main __alloca_probe. They're emitted
// directly in MSVC's compiler-rt-style assembly (chkstk.asm in CRT).
//
// Recovered 2026-05-02 via byte-pattern lookup against the canonical
// CRT chkstk.asm + alloca16.asm sources.

extern "C" {

// FUNCTION: ffxivgame 0x005d29d0 — __alloca_probe (43 B)
//
//   51              push    ecx
//   8d 4c 24 04     lea     ecx, [esp+4]
//   2b c8           sub     ecx, eax
//   1b c0           sbb     eax, eax
//   f7 d0           not     eax
//   23 c8           and     ecx, eax
//   8b c4           mov     eax, esp
//   25 00 f0 ff ff  and     eax, 0xfffff000
// cs10:
//   3b c8           cmp     ecx, eax
//   72 0a           jb      cs20
//   8b c1           mov     eax, ecx
//   59              pop     ecx
//   94              xchg    eax, esp
//   8b 00           mov     eax, [eax]
//   89 04 24        mov     [esp], eax
//   c3              ret
// cs20:
//   2d 00 10 00 00  sub     eax, 0x1000
//   85 00           test    [eax], eax
//   eb e9           jmp     cs10

__declspec(naked) void __alloca_probe(void) {
    __asm {
        push    ecx
        lea     ecx, [esp+4]
        sub     ecx, eax
        sbb     eax, eax
        not     eax
        and     ecx, eax
        mov     eax, esp
        and     eax, 0xfffff000
    cs10:
        cmp     ecx, eax
        jb      cs20
        mov     eax, ecx
        pop     ecx
        xchg    eax, esp
        mov     eax, [eax]
        mov     [esp], eax
        ret
    cs20:
        sub     eax, 0x1000
        test    [eax], eax
        jmp     cs10
    }
}

// FUNCTION: ffxivgame 0x005d8be0 — __alloca_probe_16 (22 B)
//
//   51              push    ecx
//   8d 4c 24 08     lea     ecx, [esp+8]
//   2b c8           sub     ecx, eax
//   83 e1 0f        and     ecx, 0xf
//   03 c1           add     eax, ecx
//   1b c9           sbb     ecx, ecx
//   0b c1           or      eax, ecx
//   59              pop     ecx
//   e9 ?? ?? ?? ??  jmp     __alloca_probe
//
// Rounds the request `eax` up to 16-byte alignment, then tail-calls
// __alloca_probe.

__declspec(naked) void __alloca_probe_16(void) {
    __asm {
        push    ecx
        lea     ecx, [esp+8]
        sub     ecx, eax
        and     ecx, 0xf
        add     eax, ecx
        sbb     ecx, ecx
        or      eax, ecx
        pop     ecx
        jmp     __alloca_probe
    }
}

// FUNCTION: ffxivgame 0x005d8bf6 — __alloca_probe_8 (22 B)
//
// Same as __alloca_probe_16 but rounds to 8-byte alignment
// (`AND ECX, 7` instead of `AND ECX, 0xf`).

__declspec(naked) void __alloca_probe_8(void) {
    __asm {
        push    ecx
        lea     ecx, [esp+8]
        sub     ecx, eax
        and     ecx, 7
        add     eax, ecx
        sbb     ecx, ecx
        or      eax, ecx
        pop     ecx
        jmp     __alloca_probe
    }
}

}  // extern "C"
