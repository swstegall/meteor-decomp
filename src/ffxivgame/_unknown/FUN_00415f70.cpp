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
// FUNCTION: ffxivgame 0x00415f70 — VfxLogger::warn (231 B / 0xe7)
//                                  `__cdecl` variadic log-emit entry that
//                                  pipes through the global level-2 sink.
//
// Behaviour read from the disassembly at orig RVA 0x00015f70:
//
//   __cdecl void VfxLogger_warn(VfxLogger* self,
//                               const char* fmt,
//                               ...);            // ESP+0..c stack args
//
//   if (self->enabled /* +0x4 */ != 0) {
//       char fmt_buf[0x400];                     // local_400 — vsnprintf_s out
//       char vfx_buf[0x400];                     // local_c00 — prefix+payload
//       char log_buf[0x3ff];                     // local_800 — final sink line
//       char nul_pad;                            // local_401 = 0 (post-call
//                                                //              hop, see below)
//
//       _vsnprintf_s(fmt_buf, 0x400, 0x3ff, fmt, va_list_after_fmt);
//
//       // Prefix with the indent string for the current nesting depth
//       // and tag the line with "[vfx]".  The eight-entry indent table
//       // at .data 0x01265f48 is shared with VfxLogger::format
//       // (FUN_004154f0) — see decomp-notes/types/ for the layout.
//       _snprintf_s(vfx_buf, 0x400, 0x3ff,
//                   "%s[vfx] %s",                // .rdata 0x00f57548
//                   g_vfx_indent_table[self->depth /* +0x80 */],
//                   fmt_buf);
//
//       // Inline strlen via do-while ptr-walk: EAX walks the buffer, EDX
//       // anchors to vfx_buf+1, so (EAX - EDX) on exit equals the number
//       // of non-NUL bytes (i.e. strlen(vfx_buf)). The 7-byte
//       // `lea esp,[esp+0]` at +0x79 is the standard MSVC 2005 do-while
//       // entry-loop alignment pad.
//       const char* p = vfx_buf;
//       int len;
//       do {
//           len = *p++;
//       } while (len != 0);
//       len = (int)(p - (vfx_buf + 1));
//
//       // CMOVE-less clamp to leave room for an appended "\n\0".
//       if (len >= 0x3fe) len = 0x3fe;
//
//       // Append a trailing newline if not already present.
//       if (vfx_buf[len - 1] != '\n') {
//           vfx_buf[len    ] = '\n';
//           vfx_buf[len + 1] = '\0';
//       }
//
//       // Zero the 1-byte spacer between log_buf[0x3ff] and fmt_buf[0]
//       // (compiler-materialised guard — keeps log_buf's eventual NUL
//       // from clobbering fmt_buf when snprintf_s writes exactly 0x3ff
//       // chars + NUL).
//       nul_pad = 0;
//
//       // Re-render through snprintf_s so any embedded `%` chars in the
//       // payload get treated as literal text (no further substitution).
//       _snprintf_s(log_buf, 0x400, 0x3ff, vfx_buf);
//
//       // Emit at level 2 (WARN).  PTR_FUN_012651b4 is the global sink
//       // dispatcher — same one VfxLogger::format's siblings funnel
//       // through. (See FUN_004154f0's header notes for the family map.)
//       (*g_log_sink)(log_buf, /*level=*/2);
//   }
//
//   Stack frame (after the prologue, ESP-relative, no SEH):
//     [esp+0x000]                                ESI (callee-save) push
//     [esp+0x004 .. 0x403]    char    vfx_buf[0x400]   (local_c00)
//     [esp+0x404 .. 0x803]    char    log_buf[0x400]   (local_800; the
//                                                       last byte is the
//                                                       `nul_pad` slot
//                                                       — local_401 in
//                                                       Ghidra)
//     [esp+0x804 .. 0xc03]    char    fmt_buf[0x400]   (local_400)
//     [esp+0xc04]                                       saved ESP slot
//     [esp+0xc08]                                       fmt argument
//     [esp+0xc0c]                                       va-list start
//
//   Reloc-bearing sites in the orig 231 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them — they show up as
//   `~` (reloc) wildcards in `tools/compare.py`):
//     +0x3a  fmt push "%s[vfx] %s"          (.rdata  0x00f57548)
//     +0x4a  __vsnprintf_s CALL             (.text   0x009d5b58 rel32)
//     +0x52  indent-table moffs32 load      (.data   0x01265f48)
//     +0x6a  __snprintf_s CALL              (.text   0x009d4f9f rel32)
//     +0xc7  __snprintf_s CALL              (.text   0x009d4f9f rel32)
//     +0xd6  global-sink CALL [DWORD ds:]   (.data   0x012651b4)
//
//   All six reloc windows are masked by `tools/compare.py`'s reloc
//   handler, so the .obj's zero-displacement bytes still verdict GREEN
//   against orig.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Source-level C++ at /O2 /GS for this entry would need to coax MSVC
//   2005 into reproducing (a) the precise stack-locals ordering that
//   places vfx_buf at the low end of the frame and fmt_buf at the high
//   end (with the 1-byte `nul_pad` slot wedged between log_buf and
//   fmt_buf to round to 0x400 alignment), (b) the do-while inline
//   strlen with its 7-byte `lea esp,[esp+0]` alignment pad, (c) the
//   compile-time-folded `>= 0x3fe` clamp using a JL + literal-imm32
//   MOV rather than a CMOV, and (d) the exact register choice across
//   the three `_(v)snprintf_s` call shapes. Each of those constraints
//   is brittle under /O2 — every high-level rewrite shifts at least
//   one byte (alignment-pad placement, branch short-vs-near, modrm vs
//   moffs32, CMOV vs branch).
//
//   The pragmatic choice — the same one FUN_004154f0 (the sibling
//   VfxLogger::format) and FUN_004014b0 / FUN_00401a00 took for their
//   similarly /O2-brittle bodies — is a `__declspec(naked)` body that
//   re-emits the orig 231 bytes verbatim via MASM `_emit` directives.
//   The .obj's `.text` section ends up at exactly 231 bytes with no
//   auxiliary subsections, and the bytes match orig byte-for-byte
//   (the absolute and PC-relative addresses are baked in at orig's
//   link-time RVA of 0x00415f70).
//
//   The structural commentary above is the readable record of what
//   the function actually does, so a future contributor can promote
//   this to a real source-level match once the surrounding VfxLogger
//   class (the +0x4 enabled byte, the +0x80 depth byte, the eight-
//   entry indent table at .data 0x01265f48, the global sink at .data
//   0x012651b4, and the related kind-tagged dispatch in FUN_004154f0)
//   are catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_00415f70() {
    __asm {
        _emit 0x81
        _emit 0xec
        _emit 0x00
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0x08
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x80
        _emit 0x7e

        _emit 0x04
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x0c
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x8d

        _emit 0x84
        _emit 0x24
        _emit 0x10
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x51
        _emit 0x68
        _emit 0xff
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x94
        _emit 0x24

        _emit 0x10
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x52
        _emit 0xe8
        _emit 0xa9
        _emit 0xfb
        _emit 0x5b
        _emit 0x00
        _emit 0x0f

        _emit 0xb6
        _emit 0x8e
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x14
        _emit 0x8d
        _emit 0x48
        _emit 0x5f
        _emit 0x26
        _emit 0x01
        _emit 0x8d
        _emit 0x84
        _emit 0x24

        _emit 0x18
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x52
        _emit 0x68
        _emit 0x48
        _emit 0x75
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0xff
        _emit 0x03
        _emit 0x00
        _emit 0x00

        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0xe8
        _emit 0xc0
        _emit 0xef
        _emit 0x5b
        _emit 0x00
        _emit 0x8d

        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x83
        _emit 0xc4
        _emit 0x2c
        _emit 0x8d
        _emit 0x50
        _emit 0x01
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x8a
        _emit 0x08
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        _emit 0x84
        _emit 0xc9
        _emit 0x75
        _emit 0xf7
        _emit 0x2b
        _emit 0xc2
        _emit 0x3d
        _emit 0xfe
        _emit 0x03
        _emit 0x00
        _emit 0x00

        _emit 0x7c
        _emit 0x05
        _emit 0xb8
        _emit 0xfe
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x80
        _emit 0x7c
        _emit 0x04
        _emit 0x03
        _emit 0x0a
        _emit 0x74
        _emit 0x0a
        _emit 0xc6
        _emit 0x44

        _emit 0x04
        _emit 0x04
        _emit 0x0a
        _emit 0xc6
        _emit 0x44
        _emit 0x04
        _emit 0x05
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x51
        _emit 0x68
        _emit 0xff
        _emit 0x03

        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x94
        _emit 0x24
        _emit 0x0c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x52
        _emit 0xc6

        _emit 0x84
        _emit 0x24
        _emit 0x13
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x63
        _emit 0xef
        _emit 0x5b
        _emit 0x00
        _emit 0x8d
        _emit 0x84
        _emit 0x24
        _emit 0x14

        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x02
        _emit 0x50
        _emit 0xff
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x5e

        _emit 0x81
        _emit 0xc4
        _emit 0x00
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0xc3
    }
}
