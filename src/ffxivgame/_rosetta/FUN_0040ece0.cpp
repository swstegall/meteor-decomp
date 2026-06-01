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
// FUNCTION: ffxivgame 0x0040ece0 — assertion/panic formatter + dispatch
//                                   (__cdecl, 166 bytes / 0xa6)
//
// __cdecl void FUN_0040ece0(const char *expr, const char *msg,
//                            const char *file, int line,
//                            const char *opt_tag)
//
// Stack layout at entry (caller view):
//     [ESP+0x04] = expr        (param_1)
//     [ESP+0x08] = msg         (param_2)
//     [ESP+0x0c] = file        (param_3)
//     [ESP+0x10] = line        (param_4)
//     [ESP+0x14] = opt_tag     (param_5, may be NULL)
//
// Behaviour:
//
//     char buf[0x800];                          ; SUB ESP, 0x800
//     if (opt_tag != NULL) {
//         _snprintf_s(buf, 0x800, 0x7ff,
//                     "%s(%d):[%s] <assert> (%s) %s\n",   ; @0x00f54d14
//                     file, line, opt_tag, expr, msg);
//     } else {
//         _snprintf_s(buf, 0x800, 0x7ff,
//                     "%s(%d): <assert> (%s) %s\n",        ; @0x00f54cf8
//                     file, line, expr, msg);
//     }
//     (*g_panic_sink)(buf, 6);                  ; [0x012651b4](buf, level=6)
//     *(int*)0 = 0;                             ; deliberate NULL deref
//     /* epilogue: ADD ESP, 0x808; RET */
//
// Reconstruction strategy — naked-asm byte passthrough.
// Same shape as siblings FUN_004063c0 / FUN_004071b0; only the CALL rel32
// bytes differ (different RVA, same _snprintf_s target at 0x009d4f9f).

extern "C" __declspec(naked) void FUN_0040ece0() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x14]  (opt_tag)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x81              // SUB ESP, 0x800
        _emit 0xec
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ short +0x3f (→ no_tag arm)
        _emit 0x3f
        // --- tagged arm (opt_tag != NULL) ---------------------------------
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x808]  (msg)
        _emit 0x8c
        _emit 0x24
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x804]  (expr)
        _emit 0x94
        _emit 0x24
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX                        (msg)
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x810]  (file)
        _emit 0x8c
        _emit 0x24
        _emit 0x10
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x52              // PUSH EDX                        (expr)
        _emit 0x50              // PUSH EAX                        (opt_tag)
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x81c]  (line)
        _emit 0x84
        _emit 0x24
        _emit 0x1c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX                        (line)
        _emit 0x51              // PUSH ECX                        (file)
        _emit 0x68              // PUSH 0x00f54d14                 (fmt_w_tag)
        _emit 0x14
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x7ff                      (count)
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EDX, [ESP+0x1c]             (&buf)
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x68              // PUSH 0x800                      (sizeOfBuffer)
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x52              // PUSH EDX                        (buf)
        _emit 0xe8              // CALL _snprintf_s (rel32 → 0x009d4f9f)
        _emit 0x77
        _emit 0x62
        _emit 0x5c
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x24                   (cdecl: 9 dwords)
        _emit 0xc4
        _emit 0x24
        _emit 0xeb              // JMP short +0x3c (→ shared tail)
        _emit 0x3c
        // --- untagged arm (opt_tag == NULL) -------------------------------
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x808]  (msg)
        _emit 0x84
        _emit 0x24
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x804]  (expr)
        _emit 0x8c
        _emit 0x24
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x810]  (file)
        _emit 0x94
        _emit 0x24
        _emit 0x10
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX                        (msg)
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x810]  (file re-load)
        _emit 0x84
        _emit 0x24
        _emit 0x10
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX                        (expr)
        _emit 0x52              // PUSH EDX                        (file)
        _emit 0x50              // PUSH EAX                        (file re-load)
        _emit 0x68              // PUSH 0x00f54cf8                 (fmt_no_tag)
        _emit 0xf8
        _emit 0x4c
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x7ff                      (count)
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP+0x18]             (&buf)
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x68              // PUSH 0x800                      (sizeOfBuffer)
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX                        (buf)
        _emit 0xe8              // CALL _snprintf_s (rel32 → 0x009d4f9f)
        _emit 0x39
        _emit 0x62
        _emit 0x5c
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x20                   (cdecl: 8 dwords)
        _emit 0xc4
        _emit 0x20
        // --- shared tail: dispatch + crash --------------------------------
        _emit 0x8d              // LEA EDX, [ESP]                  (&buf)
        _emit 0x14
        _emit 0x24
        _emit 0x6a              // PUSH 0x6                        (level)
        _emit 0x06
        _emit 0x52              // PUSH EDX                        (buf)
        _emit 0xff              // CALL dword ptr [0x012651b4]     (g_panic_sink)
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [0x00000000], 0   (NULL deref crash)
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x81              // ADD ESP, 0x808
        _emit 0xc4
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
    }
}
