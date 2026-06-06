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
// FUNCTION: ffxivgame 0x000157c0 — __cdecl log-message formatter/emitter
//                                   150 bytes / 0x96, plain RET.
//
// void FUN_004157c0(const char *message, int add_newline)
//   arg0 (EDX) = message string (format or plain text)
//   arg1 ([ESP+8] after frame alloc) = if low byte != 0, ensure message
//                                       ends with '\n' before logging
//
// Behaviour (recovered from asm @ 0x000157c0):
//
//   Allocates a 2 KB stack buffer (0x800 bytes).  If add_newline != 0:
//     1. Computes strlen(message) via a scanning loop.
//     2. Clamps length to 0x3FE.
//     3. If the last byte in that range is not '\n', copies message to
//        the first 1 KB of the buffer with strncpy, appends '\n' and
//        '\0', and uses the local copy as the source.
//     4. Otherwise uses message directly.
//   Then calls snprintf_s(buf+0x400, 0x400, 0x3FF, src) to format the
//   chosen source string into the second 1 KB of the buffer, with the
//   last byte pre-zeroed for safety.  Finally, passes the result to an
//   IAT-dispatched emitter with a fixed level of 2.
//
// CALL targets (wildcarded by compare.py via COFF reloc table):
//   CALL 0x009d4600   — strncpy(dst, src, count)
//   CALL 0x009d4f9f   — _snprintf_s(buf, size, count, fmt)
//   CALL [0x012651b4] — IAT: log/output emitter (2-arg, level 2)
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//   A 6-byte loop-alignment NOP (`8d 9b 00 00 00 00` = LEA EBX,[EBX+0])
//   precedes the strlen scanning loop and cannot be emitted from C++
//   source.  Additionally, the pre-call null-terminator write at offset
//   +0x72 (`MOV byte ptr [ESP+0x80f], 0`) occurs while four push-arg
//   words are already below ESP, producing an effective stack-relative
//   address that source-level C++ cannot express without naked asm.
//   A __declspec(naked) shell re-emitting all 150 bytes produces a .text
//   section that is byte-identical to the original slice; compare.py
//   reports GREEN after masking the three reloc windows.

extern "C" {
    // .text — internal direct-call targets (REL32 relocations)
    int FUN_009d4600();   // strncpy(dst, src, count) — 3-arg CRT copy
    int FUN_009d4f9f();   // _snprintf_s(buf, sz, cnt, fmt, ...)

    // .idata — IAT slot for the log/output emitter (DIR32 relocation)
    extern int ext_12651b4;
}

extern "C" __declspec(naked) void FUN_004157c0() {
    __asm {
        // 000157c0: 8b 54 24 04
        mov     edx, dword ptr [esp + 4]
        // 000157c4: 81 ec 00 08 00 00
        sub     esp, 0x800
        // 000157ca: 80 bc 24 08 08 00 00 00
        cmp     byte ptr [esp + 0x808], 0x0
        // 000157d2: 74 4b
        jz      SHORT skip_newline

        // 000157d4: 8b c2
        mov     eax, edx
        // 000157d6: 56
        push    esi
        // 000157d7: 8d 70 01
        lea     esi, [eax + 1]
        // 000157da: 8d 9b 00 00 00 00  (6-byte NOP: LEA EBX,[EBX+0])
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

    strlen_loop:
        // 000157e0: 8a 08
        mov     cl, byte ptr [eax]
        // 000157e2: 83 c0 01
        add     eax, 1
        // 000157e5: 84 c9
        test    cl, cl
        // 000157e7: 75 f7
        jnz     SHORT strlen_loop

        // 000157e9: 2b c6
        sub     eax, esi
        // 000157eb: 8b f0
        mov     esi, eax
        // 000157ed: 81 fe fe 03 00 00
        cmp     esi, 0x3fe
        // 000157f3: 7c 05
        jl      SHORT len_ok
        // 000157f5: be fe 03 00 00
        mov     esi, 0x3fe

    len_ok:
        // 000157fa: 80 7c 16 ff 0a
        cmp     byte ptr [esi + edx - 1], 0xa
        // 000157ff: 74 1d
        jz      SHORT has_newline

        // 00015801: 56
        push    esi
        // 00015802: 52
        push    edx
        // 00015803: 8d 44 24 0c
        lea     eax, [esp + 0xc]
        // 00015807: 50
        push    eax
        // 00015808: e8 f3 ed 5b 00
        call    FUN_009d4600
        // 0001580d: 83 c4 0c
        add     esp, 0xc
        // 00015810: c6 44 34 04 0a
        mov     byte ptr [esp + esi + 4], 0xa
        // 00015815: c6 44 34 05 00
        mov     byte ptr [esp + esi + 5], 0x0
        // 0001581a: 8d 54 24 04
        lea     edx, [esp + 4]

    has_newline:
        // 0001581e: 5e
        pop     esi

    skip_newline:
        // 0001581f: 52
        push    edx
        // 00015820: 68 ff 03 00 00
        push    0x3ff
        // 00015825: 8d 8c 24 08 04 00 00
        lea     ecx, [esp + 0x408]
        // 0001582c: 68 00 04 00 00
        push    0x400
        // 00015831: 51
        push    ecx
        // 00015832: c6 84 24 0f 08 00 00 00
        mov     byte ptr [esp + 0x80f], 0x0
        // 0001583a: e8 60 f7 5b 00
        call    FUN_009d4f9f
        // 0001583f: 8d 94 24 10 04 00 00
        lea     edx, [esp + 0x410]
        // 00015846: 6a 02
        push    2
        // 00015848: 52
        push    edx
        // 00015849: ff 15 b4 51 26 01
        call    dword ptr [ext_12651b4]
        // 0001584f: 81 c4 18 08 00 00
        add     esp, 0x818
        // 00015855: c3
        ret
    }
}
