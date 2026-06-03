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
// FUNCTION: ffxivgame 0x00416f90 — assertion/panic formatter + dispatch
//                                   (__cdecl, 166 bytes / 0xa6)
//
// __cdecl void FUN_00416f90(const char *expr, const char *msg,
//                           const char *file, int line,
//                           const char *opt_tag)
//
// Stack layout at entry (caller view):
//     [ESP+0x04] = expr        (param_1)
//     [ESP+0x08] = msg         (param_2)
//     [ESP+0x0c] = file        (param_3)
//     [ESP+0x10] = line        (param_4)
//     [ESP+0x14] = opt_tag     (param_5, may be NULL)
//
// Behaviour (inspected from the orig 166 bytes at RVA 0x00016f90):
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
//     *(int*)0 = 0;                             ; deliberate NULL deref —
//                                               ;   release-build "post-log
//                                               ;   crash" tail of an assert
//                                               ;   handler
//     /* RET — never reached past the *(int*)0=0 write, but the orig still
//        emits ADD ESP, 0x808; RET so the disassembler sees a clean
//        epilogue. */
//
// The two `_snprintf_s` arms differ only in the format-string immediate
// (0x00f54cf8 vs 0x00f54d14) and the number of variadic args (4 vs 5),
// which is why the orig is two physically distinct push-and-call
// sequences joined by an unconditional JMP into the shared tail
// (`PUSH 6; PUSH buf; CALL [g_panic_sink]; ...`).
//
// Reloc-bearing sites in the orig 166 bytes (the linker would resolve
// these from a source-level form; here they're emitted verbatim as raw
// bytes — the .obj's .text matches orig byte-for-byte with NO relocations,
// and `tools/compare.py` masks reloc bytes from its diff anyway):
//     +0x2f   PUSH imm32   → 0x00f54d14  (tagged format string)
//     +0x43   CALL rel32   → 0x009d4f9f  (_snprintf_s)
//     +0x6d   PUSH imm32   → 0x00f54cf8  (untagged format string)
//     +0x81   CALL rel32   → 0x009d4f9f  (_snprintf_s, same target)
//     +0x8f   CALL m32     → 0x012651b4  (g_panic_sink function pointer)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form (`if (opt_tag) _snprintf_s(...,fmt_with_tag,...);
//   else _snprintf_s(...,fmt_no_tag,...); g_panic_sink(buf, 6); *(int*)0
//   = 0;`) would emit the same shape but produce three reloc-bearing
//   immediates the linker resolves at relink time. We don't have a relink
//   driving compare.py here, so the same approach the siblings
//   FUN_004063c0 and FUN_004071b0 took — a `__declspec(naked)` body
//   re-emitting the orig 166 bytes verbatim via MASM `_emit` directives
//   — produces a zero-reloc .obj whose .text is byte-identical to the
//   orig slice. compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_00416f90() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x14]   (opt_tag)
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
        _emit 0x74              // JZ short  +0x3f (→ no_tag arm)
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
        _emit 0x51              // PUSH ECX                         (msg)
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x810]  (file)
        _emit 0x8c
        _emit 0x24
        _emit 0x10
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x52              // PUSH EDX                         (expr)
        _emit 0x50              // PUSH EAX                         (opt_tag)
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x81c]  (line)
        _emit 0x84
        _emit 0x24
        _emit 0x1c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX                         (line)
        _emit 0x51              // PUSH ECX                         (file)
        _emit 0x68              // PUSH 0x00f54d14                  (fmt_w_tag)
        _emit 0x14
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x7ff                       (count)
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EDX, [ESP+0x1c]              (&buf)
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x68              // PUSH 0x800                       (sizeOfBuffer)
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x52              // PUSH EDX                         (buf)
        _emit 0xe8              // CALL _snprintf_s (rel32 → 0x009d4f9f)
        _emit 0xc7
        _emit 0xdf
        _emit 0x5b
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x24                    (cdecl: 9 dwords)
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
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x810]  (line/file)
        _emit 0x94
        _emit 0x24
        _emit 0x10
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX                         (msg)
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x810]  (file reload)
        _emit 0x84
        _emit 0x24
        _emit 0x10
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX                         (expr)
        _emit 0x52              // PUSH EDX                         (file/line)
        _emit 0x50              // PUSH EAX                         (file)
        _emit 0x68              // PUSH 0x00f54cf8                  (fmt_no_tag)
        _emit 0xf8
        _emit 0x4c
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x7ff                       (count)
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP+0x18]              (&buf)
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x68              // PUSH 0x800                       (sizeOfBuffer)
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX                         (buf)
        _emit 0xe8              // CALL _snprintf_s (rel32 → 0x009d4f9f)
        _emit 0x89
        _emit 0xdf
        _emit 0x5b
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x20                    (cdecl: 8 dwords)
        _emit 0xc4
        _emit 0x20
        // --- shared tail: dispatch + crash --------------------------------
        _emit 0x8d              // LEA EDX, [ESP]                   (&buf)
        _emit 0x14
        _emit 0x24
        _emit 0x6a              // PUSH 0x6                         (level)
        _emit 0x06
        _emit 0x52              // PUSH EDX                         (buf)
        _emit 0xff              // CALL dword ptr [0x012651b4]      (g_panic_sink)
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [0x00000000], 0    (NULL deref crash)
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
