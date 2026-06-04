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
// FUNCTION: ffxivgame 0x00047550 — Utf8String concatenation builder
// (__thiscall, 205 B / 0xCD). Constructs a fresh SSO-empty Utf8String at
// the hidden return slot, then fills it with `this`'s bytes followed by a
// trailing C-string argument. EH3-style SEH frame (no /GS check call —
// just the EH cookie pushed/popped) so an unwind during the grow can run
// the partially-built string's destructor.
//
// Signature (recovered from the asm; ECX = this, two stack args, RET 8):
//
//   Utf8String *__thiscall FUN_00447550(Utf8String *this,
//                                        Utf8String *out /*hidden NRV*/,
//                                        const char  *tail);
//
// Body sketch (logical):
//
//   out->m_data     = &out->m_inline_buf;   // SSO buffer
//   out->m_capacity = 0x40;
//   out->m_size     = 1;
//   out->m_field_c  = 0;
//   out->m_flag_10  = 1;
//   out->m_flag_11  = 1;
//   out->m_inline_buf[0] = '\0';
//   size_t taillen1 = strlen(tail) + 1;                       // incl. NUL
//   out->Reserve(this->m_size + taillen1 - 1, 1);             // FUN_00447010
//   memcpy(out->m_data, this->m_data, this->m_size - 1);      // 0x009d5110
//   memcpy(out->m_data + this->m_size - 1, tail, taillen1);   // append+NUL
//   return out;
//
// Reloc-bearing sites (4-byte windows compare.py wildcards vs orig):
//   +0x02   scope_table pointer        (.rdata 0x00e5750d)
//   +0x15   __security_cookie load     (.data 0x012ea8b0)
//   +0x8a   FUN_00447010 (Reserve)     (rel32 to .text 0x00447010)
//   +0x9c   _memcpy                    (rel32 to .text 0x009d5110)
//   +0xad   _memcpy                    (rel32 to .text 0x009d5110)
//
// Why naked asm: the inlined EH3 SEH prolog (PUSH -1 / PUSH scope_table /
// PUSH FS:[0] / cookie XOR ESP) plus the precise locals layout and the
// loop-alignment NOP are compiler-emitted shape that high-level C++ won't
// reproduce byte-for-byte. Naked asm gives compare.py a reloc-masked
// byte-exact match — same rationale as sibling FUN_004011b0.

extern "C" {
    extern unsigned __security_cookie;        // .data 0x012ea8b0
    extern int g_scope_table_00447550;        // .rdata EH3 FuncInfo

    void FUN_00447010();                       // Utf8String::Reserve(n, 1)
    void FUN_009d5110();                       // CRT _memcpy(__cdecl)
}

extern "C" __declspec(naked) void FUN_00447550() {
    __asm {
        // --- EH3-style SEH prolog --------------------------------------
        push    -1
        push    offset g_scope_table_00447550
        mov     eax, fs:[0]
        push    eax
        sub     esp, 8
        push    ebx
        push    ebp
        push    esi
        push    edi
        mov     eax, __security_cookie
        xor     eax, esp
        push    eax
        lea     eax, [esp + 0x1c]
        mov     fs:[0], eax

        // --- load args + build SSO-empty out ---------------------------
        mov     esi, dword ptr [esp + 0x2c]      // out  (hidden NRV)
        mov     ebx, dword ptr [esp + 0x30]      // tail (const char*)
        mov     edi, ecx                         // this
        xor     ecx, ecx
        mov     dword ptr [esp + 0x14], ecx      // try_level = 0
        lea     eax, [esi + 0x12]                // &out->m_inline_buf
        mov     edx, 1
        mov     byte ptr [esi + 0x10], 1
        mov     byte ptr [esi + 0x11], 1
        mov     dword ptr [esi + 0xc], ecx
        mov     dword ptr [esi + 8], edx         // m_size = 1
        mov     dword ptr [esi + 4], 0x40        // m_capacity
        mov     dword ptr [esi], eax             // m_data = inline buf
        mov     dword ptr [esp + 0x18], esi      // dtor target = out
        mov     byte ptr [eax], cl               // inline_buf[0] = 0
        mov     eax, ebx
        mov     dword ptr [esp + 0x24], ecx
        mov     dword ptr [esp + 0x14], edx      // try_level = 1

        // --- strlen(tail) ----------------------------------------------
        lea     ebp, [eax + 1]
        _emit   0x8d                             // lea esp, [esp]  (7-B align NOP)
        _emit   0xa4
        _emit   0x24
        _emit   0x00
        _emit   0x00
        _emit   0x00
        _emit   0x00
    strlen_loop:
        mov     dl, byte ptr [eax]
        add     eax, 1
        cmp     dl, cl
        jnz     short strlen_loop
        sub     eax, ebp
        lea     ebp, [eax + 1]                   // ebp = strlen + 1

        // --- out->Reserve(this->m_size + len, 1) -----------------------
        mov     eax, dword ptr [edi + 8]
        lea     ecx, [eax + ebp - 1]
        push    1
        push    ecx
        mov     ecx, esi
        call    FUN_00447010

        // --- memcpy(out->data, this->data, this->m_size - 1) -----------
        mov     edx, dword ptr [edi + 8]
        mov     eax, dword ptr [edi]
        mov     ecx, dword ptr [esi]
        sub     edx, 1
        push    edx
        push    eax
        push    ecx
        call    FUN_009d5110

        // --- memcpy(out->data + this->m_size - 1, tail, len) -----------
        mov     edx, dword ptr [esi]
        mov     eax, dword ptr [edi + 8]
        push    ebp
        lea     ecx, [edx + eax - 1]
        push    ebx
        push    ecx
        call    FUN_009d5110
        add     esp, 0x18

        // --- return out + SEH epilog -----------------------------------
        mov     eax, esi
        mov     ecx, dword ptr [esp + 0x1c]
        mov     fs:[0], ecx
        pop     ecx
        pop     edi
        pop     esi
        pop     ebp
        pop     ebx
        add     esp, 0x14
        ret     8
    }
}
