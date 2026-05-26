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
// FUNCTION: ffxivgame 0x0040e690 — assertion-failure handler
//                                   (__cdecl, 166 B / 0xa6)
//
// void FUN_0040e690(const char *cond, const char *msg,
//                   const char *file, int line, const char *thread)
//
// Formats an assertion message into a 2048-byte stack buffer using
// _snprintf_s, then invokes a global output function pointer (at
// 0x012651b4) with (buf, 6).  Finally writes 0 to address 0x00000000
// (intentional crash / trap after the assert fires).
//
// Byte-identical to FUN_0040b0a0 — same codegen, different RVA.
//
// Calling convention: __cdecl (caller cleans, no callee-saved registers).
// Stack frame: 0x800 (2048-byte local buffer).
// No /GS cookie — must be compiled with /GS- (same as FUN_0040b0a0): the orig
// TU has no security cookie despite the 2048-byte buffer.  MSVC 2005 omits it
// when the buffer is only accessed via _snprintf_s.  Standard ROSETTA_FLAGS
// include /GS which reintroduces the cookie; use /GS- override to match.
//
// Format strings (in .rdata at the orig binary's addresses):
//   with thread: "%s(%d):[%s] <assert> (%s) %s\n"  @ 0x00f54d14
//   no thread:   "%s(%d): <assert> (%s) %s\n"       @ 0x00f54cf8
//
// Reloc-bearing sites (masked by tools/compare.py):
//     two PUSH imm32 for the format string literals
//     two CALL rel32 → _snprintf_s (RVA 0x009d4f9f)
//     one CALL ind32 → [0x012651b4] (indirect call through fn-ptr slot)
//     one MOV m32, imm32 → [0x00000000] = 0 (null-deref crash)

extern "C" {

// _snprintf_s — MSVC secure snprintf, __cdecl, caller-cleaned.
int __cdecl _snprintf_s(char *buf, unsigned int bufsz, unsigned int count,
                        const char *fmt, ...);

// Output function pointer stored at 0x012651b4.
// Called as fn(buf, 6) — likely a debug/log sink.
typedef void (__cdecl *OutputFn_e690)(const char *buf, int level);
extern OutputFn_e690 g_outputFnSlot_e690; // slot at 0x012651b4 holds the fn ptr

// Format strings in the orig binary's .rdata.
extern const char g_fmtThread_e690[];   // "%s(%d):[%s] <assert> (%s) %s\n"
extern const char g_fmtNoThread_e690[]; // "%s(%d): <assert> (%s) %s\n"

// The intentional null-deref crash target.
extern volatile int g_crashTarget_e690; // at absolute address 0x00000000

void FUN_0040e690(const char *cond, const char *msg,
                  const char *file, int line, const char *thread)
{
    char buf[0x800];

    if (thread) {
        _snprintf_s(buf, 0x800, 0x7ff,
                    g_fmtThread_e690,
                    file, line, thread, cond, msg);
    } else {
        _snprintf_s(buf, 0x800, 0x7ff,
                    g_fmtNoThread_e690,
                    file, line, cond, msg);
    }

    g_outputFnSlot_e690(buf, 6);
    g_crashTarget_e690 = 0;
}

} // extern "C"

// vim: ts=4 sts=4 sw=4 et
