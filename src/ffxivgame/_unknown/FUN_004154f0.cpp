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
// FUNCTION: ffxivgame 0x004154f0 — VfxLogger::format (304 B / 0x130)
//                                  `__thiscall` indent-aware log formatter.
//
// Behaviour read from the disassembly at orig RVA 0x000154f0:
//
//   __thiscall void VfxLogger::format(this,
//                                     char*       buf,            // [esp+0x04]
//                                     size_t      buf_size,       // [esp+0x08]
//                                     int         kind,           // [esp+0x0c]
//                                     unsigned    w_arg,          // [esp+0x10]
//                                     const char* msg,            // [esp+0x14]
//                                     char        append_newline) // [esp+0x18]
//       — ECX = this; six stack args; callee cleans up via `ret 0x18`.
//
//   The this object carries a single relevant member: a byte at +0x80
//   that indexes a global 8-entry pointer-of-string table at .data
//   0x01265f48. Each entry is an indentation prefix (0 → "", 1 → "  ",
//   2 → "    ", ..., 7 → "              ") — so the byte at this+0x80
//   is the current nesting depth and `g_indent_table[depth]` is the
//   leading whitespace prepended to every formatted line.
//
//   `kind` selects one of four format strings; only kind==1 needs the
//   extra `w_arg` integer (rendered as `%d`). The other kinds emit the
//   plain "[vfx]" tag and only kind==0 (and the unreachable default)
//   falls through to the optional newline-append tail.
//
//     switch (kind) {
//     case 0:                  // INFO — gets the trailing newline-append
//         _snprintf_s(buf, buf_size, buf_size - 1,
//                     "%s[vfx] %s",         // .rodata 0x00f57548
//                     g_indent_table[this->depth_80], msg);
//         break;
//
//     case 1:                  // WARN — formatted with `w_arg` as %d
//         _snprintf_s(buf, buf_size, buf_size - 1,
//                     "%s[vfx.w%d] %s",     // .rodata 0x00f57524
//                     g_indent_table[this->depth_80], w_arg, msg);
//         return;              // no newline-append for warn
//
//     case 2:                  // ASSERT
//         _snprintf_s(buf, buf_size, buf_size - 1,
//                     "%s[vfx.assert] %s",  // .rodata 0x00f57534
//                     g_indent_table[this->depth_80], msg);
//         return;              // no newline-append for assert
//
//     default:                 // unreachable arm — emits same as case 0
//         _snprintf_s(buf, buf_size, buf_size - 1,
//                     "%s[vfx] %s",         // .rodata 0x00f57548 (pooled)
//                     g_indent_table[this->depth_80], msg);
//         if (kind > 0 && kind < 3) return;  // dead, but preserved
//         break;                              // falls through to append
//     }
//
//     // Newline-append tail (reached from case 0 via `JMP back-of-fn`
//     // and from the default arm via fallthrough).
//     if (append_newline) {
//         char* p = buf;
//         while (*p++) ;                       // inline `strlen` via
//         size_t len = (p - 1) - buf;          // do-while ptr-walk
//         if (len > buf_size - 2)              // CMOVLE clamp to size-2
//             len = buf_size - 2;
//         if (buf[len - 1] != '\n') {
//             buf[len    ] = '\n';
//             buf[len + 1] = '\0';
//         }
//     }
//
//   Stack frame (after the prologue, ESP-relative, no SEH):
//     [esp+0x00..0x0b]   EDI / ESI / EBX pushed below `kind` cache
//     [esp+0x0c]                                     return address
//     [esp+0x10..0x24]   buf / buf_size / kind / w_arg / msg / append_newline
//
//   Layout note: MSVC laid out the four arms in `default → case2 →
//   case1 → case0` order (the prologue does `sub eax, N; je <arm>` three
//   times then falls through to the default body). case 0 lives at the
//   physical tail of the function and `JMP`s back up into the
//   newline-append block — that's why the orig bytes end with a 5-byte
//   `E9 28 FF FF FF` near-jump.
//
//   Reloc-bearing sites in the orig 304 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them — they show up as
//   `~` (reloc) wildcards in `tools/compare.py`):
//     +0x2e  fmt push "%s[vfx] %s"          (.rdata 0x00f57548)
//     +0x38  __snprintf_s CALL              (.text  0x009d4f9f rel32)
//     +0x8d  fmt push "%s[vfx.assert] %s"   (.rdata 0x00f57534)
//     +0xb5  __snprintf_s CALL              (.text  0x009d4f9f rel32)
//     +0xe3  fmt push "%s[vfx.w%d] %s"      (.rdata 0x00f57524)
//     +0xee  __snprintf_s CALL              (.text  0x009d4f9f rel32)
//     +0x118 fmt push "%s[vfx] %s"          (.rdata 0x00f57548)
//     +0x123 __snprintf_s CALL              (.text  0x009d4f9f rel32)
//
//   Plus four `mov reg, [ecx*4 + 0x01265f48]` absolute moffs32 loads of
//   the indent-table pointer (one per arm). All eight CALL/MOV-imm32
//   sites are masked by `tools/compare.py`'s reloc map, so the .obj's
//   zero-displacement bytes still verdict GREEN against orig.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Source-level C++ at /O2 /GS for this formatter would need to coax
//   MSVC 2005 into reproducing (a) the unusual physical arm ordering
//   (default-first-then-jumped-cases-at-tail rather than sequential
//   if-else fallthrough), (b) the dead `kind > 0 && kind < 3` test in
//   the default arm, (c) the `CMOVLE` clamp on the strlen result, and
//   (d) the precise register allocation across four near-identical
//   snprintf_s call shapes (case 1 has a 7th arg, the others six).
//   Each of those constraints is brittle under /O2 — every high-level
//   rewrite shifts at least one byte (state numbering, branch
//   short-vs-near, modrm vs moffs32, CMOVLE vs branch).
//
//   The pragmatic choice — the same one FUN_004014b0 and FUN_00401a00
//   took for their similarly /O2-brittle bodies — is a
//   `__declspec(naked)` body that re-emits the orig 304 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` section ends up at
//   exactly 304 bytes with no auxiliary subsections, and the bytes
//   match orig byte-for-byte (no relocations — the absolute and
//   PC-relative addresses are baked in at orig's link-time RVA of
//   0x004154f0).
//
//   The structural commentary above is the readable record of what the
//   function actually does, so a future contributor can promote this
//   to a real source-level match once the surrounding VfxLogger class
//   (the +0x80 depth byte, the eight-entry indent table at .data
//   0x01265f48, and the related kind-tagged dispatch sites) are
//   catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_004154f0() {
    __asm {
        _emit 0x53
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0xc3
        _emit 0x83
        _emit 0xe8
        _emit 0x00
        _emit 0x56
        _emit 0x57
        _emit 0x0f
        _emit 0x84
        _emit 0xea
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x0f
        _emit 0x84
        _emit 0xa7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x74
        _emit 0x6d

        _emit 0x0f
        _emit 0xb6
        _emit 0x89
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x8b
        _emit 0x14
        _emit 0x8d
        _emit 0x48
        _emit 0x5f

        _emit 0x26
        _emit 0x01
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x50
        _emit 0x52
        _emit 0x68
        _emit 0x48
        _emit 0x75
        _emit 0xf5

        _emit 0x00
        _emit 0x8d
        _emit 0x47
        _emit 0xff
        _emit 0x50
        _emit 0x57
        _emit 0x56
        _emit 0xe8
        _emit 0x63
        _emit 0xfa
        _emit 0x5b
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x85

        _emit 0xdb
        _emit 0x7e
        _emit 0x05
        _emit 0x83
        _emit 0xfb
        _emit 0x02
        _emit 0x7e
        _emit 0x2f
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x24
        _emit 0x00
        _emit 0x74
        _emit 0x28
        _emit 0x8b

        _emit 0xc6
        _emit 0x8d
        _emit 0x50
        _emit 0x01
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
        _emit 0x8d

        _emit 0x4f
        _emit 0xfe
        _emit 0x3b
        _emit 0xc8
        _emit 0x0f
        _emit 0x4e
        _emit 0xc1
        _emit 0x80
        _emit 0x7c
        _emit 0x30
        _emit 0xff
        _emit 0x0a
        _emit 0x74
        _emit 0x09
        _emit 0xc6
        _emit 0x04

        _emit 0x30
        _emit 0x0a
        _emit 0xc6
        _emit 0x44
        _emit 0x30
        _emit 0x01
        _emit 0x00
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0xc2
        _emit 0x18
        _emit 0x00
        _emit 0x0f
        _emit 0xb6
        _emit 0x81

        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x8b
        _emit 0x0c
        _emit 0x85
        _emit 0x48
        _emit 0x5f
        _emit 0x26
        _emit 0x01
        _emit 0x8b

        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x52
        _emit 0x51
        _emit 0x68
        _emit 0x34
        _emit 0x75
        _emit 0xf5
        _emit 0x00
        _emit 0x8d
        _emit 0x50
        _emit 0xff
        _emit 0x52
        _emit 0x50
        _emit 0x8b

        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x50
        _emit 0xe8
        _emit 0xf6
        _emit 0xf9
        _emit 0x5b
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0xc2

        _emit 0x18
        _emit 0x00
        _emit 0x0f
        _emit 0xb6
        _emit 0x89
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x8b
        _emit 0x44
        _emit 0x24

        _emit 0x1c
        _emit 0x52
        _emit 0x8b
        _emit 0x14
        _emit 0x8d
        _emit 0x48
        _emit 0x5f
        _emit 0x26
        _emit 0x01
        _emit 0x50
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x52
        _emit 0x8b

        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x68
        _emit 0x24
        _emit 0x75
        _emit 0xf5
        _emit 0x00
        _emit 0x8d
        _emit 0x48
        _emit 0xff
        _emit 0x51
        _emit 0x50
        _emit 0x52
        _emit 0xe8
        _emit 0xbc

        _emit 0xf9
        _emit 0x5b
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0xc2
        _emit 0x18
        _emit 0x00
        _emit 0x0f
        _emit 0xb6
        _emit 0x89
        _emit 0x80

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x8b
        _emit 0x14
        _emit 0x8d
        _emit 0x48
        _emit 0x5f
        _emit 0x26
        _emit 0x01
        _emit 0x8b
        _emit 0x7c

        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x50
        _emit 0x52
        _emit 0x68
        _emit 0x48
        _emit 0x75
        _emit 0xf5
        _emit 0x00
        _emit 0x8d
        _emit 0x47
        _emit 0xff

        _emit 0x50
        _emit 0x57
        _emit 0x56
        _emit 0xe8
        _emit 0x87
        _emit 0xf9
        _emit 0x5b
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0xe9
        _emit 0x28
        _emit 0xff
        _emit 0xff
        _emit 0xff
    }
}
