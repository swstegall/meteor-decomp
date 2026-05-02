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
// MSVC 2005 CRT — `__EH_prolog3`, `__EH_prolog3_catch`,
// `__EH_prolog3_GS`. Compiler-emitted SEH prolog helpers used in
// functions with `__try`/`__except` or stack-destructible C++ objects.
// Recovered 2026-05-02 from byte signatures vs ehprolog3.asm in the
// MSVC 2005 SP1 CRT source.

extern "C" {

// Declared by the CRT (defined in cookie.c). Lives at .data
// 0x012ea8b0 in ffxivgame.exe — referenced via `MOV EAX, [moffs32]`
// (`a1`) when accessed by name, but `MOV EAX, imm32` (`b8`) when
// the address is written as a numeric literal in inline asm.
extern unsigned __security_cookie;

// FUNCTION: ffxivgame 0x005dc47b — __EH_prolog3 (51 B)
//
// Sets up the per-function SEH frame on entry to a function with
// `__try { ... } __except (filter) { ... }` blocks. Pushes a new
// FS:[0] handler, anchors the cookie, and reserves the EH state slot.
//
// On entry:  EAX = handler-table pointer (from caller)
//            [ESP] = retaddr
//            [ESP+4] = local-frame size (in bytes)
//
// On return: EBP set up as the EH frame anchor; FS:[0] points to the
//            new handler; security cookie installed.

__declspec(naked) void __EH_prolog3(void) {
    __asm {
        push    eax
        push    fs:[0]
        lea     eax, [esp+0xc]
        sub     esp, [esp+0xc]
        push    ebx
        push    esi
        push    edi
        mov     [eax], ebp
        mov     ebp, eax
        mov     eax, __security_cookie   ; security_cookie
        xor     eax, ebp
        push    eax
        push    [ebp-4]
        mov     dword ptr [ebp-4], 0xffffffff
        lea     eax, [ebp-0xc]
        mov     fs:[0], eax
        ret
    }
}

// FUNCTION: ffxivgame 0x005dc4ae — __EH_prolog3_catch (54 B)
//
// Variant of __EH_prolog3 used by functions with C++ try/catch
// blocks (vs SEH-only). 3 extra bytes for `MOV [EBP-0x10], ESP`
// which saves the catch-time stack pointer.

__declspec(naked) void __EH_prolog3_catch(void) {
    __asm {
        push    eax
        push    fs:[0]
        lea     eax, [esp+0xc]
        sub     esp, [esp+0xc]
        push    ebx
        push    esi
        push    edi
        mov     [eax], ebp
        mov     ebp, eax
        mov     eax, __security_cookie
        xor     eax, ebp
        push    eax
        mov     [ebp-0x10], esp
        push    [ebp-4]
        mov     dword ptr [ebp-4], 0xffffffff
        lea     eax, [ebp-0xc]
        mov     fs:[0], eax
        ret
    }
}

// FUNCTION: ffxivgame 0x005dc4e4 — __EH_prolog3_GS (54 B)
//
// Variant used by functions compiled with `/GS` that ALSO have
// `__try`/`__except`. Anchors the GS cookie to a known frame slot
// before the SEH chain links in.

__declspec(naked) void __EH_prolog3_GS(void) {
    __asm {
        push    eax
        push    fs:[0]
        lea     eax, [esp+0xc]
        sub     esp, [esp+0xc]
        push    ebx
        push    esi
        push    edi
        mov     [eax], ebp
        mov     ebp, eax
        mov     eax, __security_cookie
        xor     eax, ebp
        push    eax
        mov     [ebp-0x10], eax
        push    [ebp-4]
        mov     dword ptr [ebp-4], 0xffffffff
        lea     eax, [ebp-0xc]
        mov     fs:[0], eax
        ret
    }
}

// FUNCTION: ffxivlogin/updater 0x?????? — __EH_prolog3_catch_GS (57 B)
//
// Combined _catch + _GS variant. Sets BOTH the catch-frame ESP marker
// and the GS cookie copy in EBP-relative slots. Only present in
// binaries that use both `__try`/`__except` AND `/GS` extensively
// (login.exe, updater.exe in our set).

__declspec(naked) void __EH_prolog3_catch_GS(void) {
    __asm {
        push    eax
        push    fs:[0]
        lea     eax, [esp+0xc]
        sub     esp, [esp+0xc]
        push    ebx
        push    esi
        push    edi
        mov     [eax], ebp
        mov     ebp, eax
        mov     eax, __security_cookie
        xor     eax, ebp
        push    eax
        mov     [ebp-0x14], eax
        mov     [ebp-0x10], esp
        push    [ebp-4]
        mov     dword ptr [ebp-4], 0xffffffff
        lea     eax, [ebp-0xc]
        mov     fs:[0], eax
        ret
    }
}

}  // extern "C"
