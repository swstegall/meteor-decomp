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
// FUNCTION: ffxivgame 0x000011b0 — ANSI → UTF-8 string initializer
// (__cdecl, 209 B). Constructs a Utf8String *out from a const char *
// ANSI/Shift-JIS source, going through a stack-local wchar_t buffer
// for the MBCS → UTF-16 conversion via MultiByteToWideChar.
//
// Signature (recovered from the lone caller at +0x38e5 in FUN_00403640):
//
//   Utf8String *FUN_004011b0(Utf8String *out, const char *ansi_str);
//
// The caller stack-allocates a Utf8String at [esp+0x54], pushes the
// pointer + a `.rdata` string literal (0xf53eee) onto the stack, calls
// us, then `ADD ESP, 0x8` cleans the 2 cdecl args. We return `out`
// (echoed through EAX) so the caller can chain the result into a
// subsequent push (`PUSH EAX`).
//
// Body sketch (logical):
//
//   Utf8String *FUN_004011b0(Utf8String *out, const char *ansi) {
//       new (out) Utf8String();            // FUN_00445cf0
//       try {
//           wchar_t wbuf[1024];
//           wbuf[0] = L'\0';               // explicit MSVC-emitted init
//           memset(wbuf + 1, 0, sizeof(wbuf) - sizeof(wchar_t));
//           MultiByteToWideChar(CP_ACP, 0, ansi, -1, wbuf, 1024);
//           AssignFromUtf16(out, wbuf);    // FUN_004476e0 (this = out)
//       } catch (...) {
//           out->~Utf8String();
//           throw;
//       }
//       return out;
//   }
//
// EH3-style C++ SEH frame protects the Utf8String construction: the
// state-index local at [esp+0x10] goes 0 → 1 around the placement-new
// so that on unwind the ~Utf8String at offset 0 of *out gets called.
// The destructor target pointer is stashed at [esp+0x14] (set to ESI
// = out before the ctor). The scope_table sits in .rdata at the
// MSVC-conventional `__ehfuncinfo$<sym>` location — its absolute VA
// in this build is 0x00e542b2 (PUSHed as the second SEH-frame field).
//
// /GS layout:
//   [esp+0x808] — primary cookie (cookie XOR esp, guards the buffer)
//   [esp+0x000] — secondary EH cookie (PUSH'd as the topmost saved
//                  register; used by __security_check_cookie at exit)
//
// Why naked asm: this mirrors FUN_00401650's rationale verbatim. The
// inlined EH3 SEH prolog (PUSH -1 / PUSH scope_table / PUSH FS:[0] /
// double-cookie XOR ESP) plus the mid-body `MOV [esp+0x10], 1`
// state-index transition is compiler-emitted shape that depends on
// (a) precise locals layout, (b) the linker-laid scope table, (c)
// MSVC's `a1 / a3` short EAX-to-moffs32 encoding choices. Coaxing
// exactly this 209-byte sequence out of C++ source under
// /O2 /GS /EHsc is impractical — each high-level rewrite shifts at
// least one encoding (modrm vs moffs32, SEH state numbering, branch
// short-vs-near, or the wbuf init pattern). Naked asm gives the
// reloc-masking diff (compare.py) a byte-exact match modulo the
// 7 relocations.
//
// Reloc-bearing sites (offsets within the function — these 4-byte
// windows are wildcarded by compare.py against orig):
//   +0x03   scope_table pointer       (.rdata 0x00e542b2)
//   +0x15   __security_cookie load    (.data 0x012ea8b0)
//   +0x26   __security_cookie load    (.data 0x012ea8b0)
//   +0x5b   Utf8String::Utf8String()  (rel32 to .text 0x00445cf0)
//   +0x80   _memset                   (rel32 to .text 0x009d2110)
//   +0x97   MultiByteToWideChar IAT   (.idata 0x00f3e1f4)
//   +0xa4   AssignFromUtf16           (rel32 to .text 0x004476e0)
//   +0xc6   __security_check_cookie   (rel32 to .text 0x009d20f4)

extern "C" {
    // .data — single security cookie shared across the whole TU.
    extern unsigned __security_cookie;

    // .rdata — MSVC-emitted EH3 scope table (FuncInfo) for this fn.
    extern int g_scope_table_4011b0;

    // .idata — IAT slot for kernel32!MultiByteToWideChar. Declared as
    // a plain `int` so MASM accepts `call dword ptr [ext_f3e1f4]` —
    // the assembler emits `ff 15 ?? ?? ?? ??` with a DIR32 reloc on
    // the imm32 slot address.
    extern int ext_f3e1f4;

    // .text — Utf8String::Utf8String() default ctor at RVA 0x00045cf0.
    // Initialises *this to SSO-empty state (m_flag_10/_11/_size = 1,
    // m_capacity = 0x40, m_data = &m_inline_buf, inline_buf[0] = 0).
    // __thiscall (this in ECX), no args, no return value (echoes ECX
    // through EAX out of the asm-coincidental MOV EAX, ECX pattern).
    int Utf8String_ctor();

    // .text — Utf8String::AssignFromUtf16(const wchar_t *) at RVA
    // 0x000476e0. __thiscall, RET 4 (one stack arg). Two-pass: first
    // measures the UTF-8 byte count of the UTF-16 input (via the
    // converter at 0x00445ae0 with dst=NULL), Reserves the right
    // amount, then re-runs the converter to copy bytes in.
    int Utf8String_AssignFromUtf16();

    // .text — CRT _memset at RVA 0x009d2110 (matched).
    void * __cdecl _memset(void *dst, int val, unsigned n);

    // .text — CRT __security_check_cookie at RVA 0x009d20f4.
    void __cdecl _security_check_cookie(unsigned cookie);
}

extern "C" __declspec(naked) void FUN_004011b0() {
    __asm {
        // --- /GS + EH3-style SEH prolog --------------------------------
        push    -1                                  // 6a ff                          (2 B)
        push    offset g_scope_table_4011b0         // 68 ?? ?? ?? ??                 (5 B, reloc +1)
        mov     eax, fs:[0]                         // 64 a1 00 00 00 00              (6 B)
        push    eax                                 // 50                             (1 B)
        sub     esp, 0x80c                          // 81 ec 0c 08 00 00              (6 B)
        mov     eax, __security_cookie              // a1 ?? ?? ?? ??                 (5 B, reloc +1)
        xor     eax, esp                            // 33 c4                          (2 B)
        mov     dword ptr [esp + 0x808], eax        // 89 84 24 08 08 00 00           (7 B)
        push    ebx                                 // 53                             (1 B)
        push    esi                                 // 56                             (1 B)
        push    edi                                 // 57                             (1 B)
        mov     eax, __security_cookie              // a1 ?? ?? ?? ??                 (5 B, reloc +1)
        xor     eax, esp                            // 33 c4                          (2 B)
        push    eax                                 // 50                             (1 B)
        lea     eax, [esp + 0x81c]                  // 8d 84 24 1c 08 00 00           (7 B)
        mov     fs:[0], eax                         // 64 a3 00 00 00 00              (6 B)

        // --- load args + pre-ctor state ---------------------------------
        mov     esi, dword ptr [esp + 0x82c]        // 8b b4 24 2c 08 00 00           (7 B) — arg1 (out)
        mov     edi, dword ptr [esp + 0x830]        // 8b bc 24 30 08 00 00           (7 B) — arg2 (ansi)
        xor     ebx, ebx                            // 33 db                          (2 B)
        mov     ecx, esi                            // 8b ce                          (2 B) — this for ctor
        mov     dword ptr [esp + 0x824], ebx        // 89 9c 24 24 08 00 00           (7 B) — try_level = 0
        mov     dword ptr [esp + 0x14], esi         // 89 74 24 14                    (4 B) — dtor target = out
        mov     dword ptr [esp + 0x10], ebx         // 89 5c 24 10                    (4 B) — state = 0
        call    Utf8String_ctor                     // e8 ?? ?? ?? ??                 (5 B, reloc +1)

        // --- zero wbuf[1..1024) via memset; wbuf[0] cleared inline -----
        push    0x7fe                               // 68 fe 07 00 00                 (5 B)
        lea     eax, [esp + 0x1e]                   // 8d 44 24 1e                    (4 B) — &wbuf[1]
        push    ebx                                 // 53                             (1 B) — val = 0
        push    eax                                 // 50                             (1 B) — dst
        mov     dword ptr [esp + 0x830], ebx        // 89 9c 24 30 08 00 00           (7 B) — (try_level again, no-op)
        mov     dword ptr [esp + 0x1c], 1           // c7 44 24 1c 01 00 00 00        (8 B) — state = 1
        mov     word ptr [esp + 0x24], bx           // 66 89 5c 24 24                 (5 B) — wbuf[0] = 0
        call    _memset                             // e8 ?? ?? ?? ??                 (5 B, reloc +1)
        add     esp, 0xc                            // 83 c4 0c                       (3 B)

        // --- MultiByteToWideChar(CP_ACP, 0, ansi, -1, wbuf, 1024) ------
        push    0x400                               // 68 00 04 00 00                 (5 B) — cchWideChar
        lea     ecx, [esp + 0x1c]                   // 8d 4c 24 1c                    (4 B) — &wbuf[0]
        push    ecx                                 // 51                             (1 B) — lpWideCharStr
        push    -1                                  // 6a ff                          (2 B) — cbMultiByte
        push    edi                                 // 57                             (1 B) — lpMultiByteStr
        push    ebx                                 // 53                             (1 B) — dwFlags = 0
        push    ebx                                 // 53                             (1 B) — CodePage = CP_ACP
        call    dword ptr [ext_f3e1f4]              // ff 15 ?? ?? ?? ??              (6 B, reloc +2)

        // --- out->AssignFromUtf16(wbuf) --------------------------------
        lea     edx, [esp + 0x18]                   // 8d 54 24 18                    (4 B) — &wbuf[0]
        push    edx                                 // 52                             (1 B)
        mov     ecx, esi                            // 8b ce                          (2 B) — this = out
        call    Utf8String_AssignFromUtf16          // e8 ?? ?? ?? ??                 (5 B, reloc +1)

        // --- normal return (no destructor on success) ------------------
        mov     eax, esi                            // 8b c6                          (2 B) — return out
        mov     ecx, dword ptr [esp + 0x81c]        // 8b 8c 24 1c 08 00 00           (7 B) — restore old FS:[0]
        mov     fs:[0], ecx                         // 64 89 0d 00 00 00 00           (7 B)
        pop     ecx                                 // 59                             (1 B) — pop EH-cookie
        pop     edi                                 // 5f                             (1 B)
        pop     esi                                 // 5e                             (1 B)
        pop     ebx                                 // 5b                             (1 B)
        mov     ecx, dword ptr [esp + 0x808]        // 8b 8c 24 08 08 00 00           (7 B) — load /GS cookie
        xor     ecx, esp                            // 33 cc                          (2 B)
        call    _security_check_cookie              // e8 ?? ?? ?? ??                 (5 B, reloc +1)
        add     esp, 0x818                          // 81 c4 18 08 00 00              (6 B) — drop locals + SEH frame
        ret                                         // c3                             (1 B)
    }
}
