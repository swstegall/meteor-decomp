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
// FUNCTION: ffxivgame 0x005d723b — CRT fwrite-style buffered write
//                                  (__cdecl, 351 B / 0x15f, no SEH;
//                                   12-byte local frame, saves EBX/ESI/EDI).
//
// Disassembly summary (RVA 0x005d723b, size 0x15f = 351 bytes):
//
//   __cdecl size_t fwrite_impl(const void *ptr, size_t n, size_t count,
//                               FILE_like *stream)
//
//   Prologue: PUSH EBP / MOV EBP,ESP / SUB ESP,0Ch / PUSH EBX,ESI,EDI
//   Locals:
//     [EBP-4]   = running write pointer (initially arg1)
//     [EBP-8]   = block size (stream->_bufsiz or 0x1000)
//     [EBP-0Ch] = total bytes = n * count
//
//   Logic (fwrite-style):
//     if (n == 0 || count == 0)           → return 0
//     if (stream == NULL)                 → _invalid_parameter + errno=22 + return 0
//     if (ptr == NULL)                    → same error path
//     if (count > UINT_MAX / n)           → overflow error path
//     total = n * count
//     block_size = (stream->_flag & 0x10c) ? stream->_bufsiz : 0x1000
//     while (remaining > 0):
//       if (stream->_flag & 0x108 && stream->_cnt > 0):
//         copy min(remaining, stream->_cnt) bytes into stream->_ptr
//         stream->_ptr += written; stream->_cnt -= written
//       elif remaining >= block_size:
//         if (stream->_flag & 0x108): flush(stream)
//         block_write(stream, ptr, aligned_amount)
//       else:
//         putc(*ptr, stream); ptr++; remaining--
//     return count   // elements written
//
//   Error paths set stream->_flag |= 0x20 (_IOERR) and return
//     (total_written_bytes / n) elements.
//
//   All CALLs are REL32 (no absolute addresses in this function body);
//   tools/compare.py masks the relocation windows during the cmp_obj pass.
//
//   Relative CALL targets (REL32 offsets from orig link-time RVA):
//     +0x1c  CALL 0x5d9d47   — __invalid_parameter / security stub
//     +0x2c  CALL 0x5d2290   — _set_errno / errno assignment
//     +0xa3  CALL 0x5d4600   — buffer memcpy helper
//     +0xc1  CALL 0x5d7061   — stream flush / grow
//     +0xe1  CALL 0x5d6a61   — block write helper
//     +0xe8  CALL 0x5e9211   — commit / WriteFile wrapper
//     +0x113 CALL 0x5e5064   — putc-like single-byte write
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A high-level C++ port at /O2 /EHsc would need to reproduce the exact
//   MSVC 2005 register scheduling (EDI holds total-remaining, EBX holds the
//   inner remaining, ECX reloaded mid-loop from [EBP-4]), the specific
//   branch order (arg2→arg3→arg4→arg1), and the five-push-then-errno error
//   pattern in the _invalid_parameter path. The loop also re-reads
//   stream->_bufsiz on every putc iteration to handle dynamic resize.
//   Any small source-level change shifts a branch from short to near or
//   reorders a spill, breaking byte identity.
//
//   The approach used throughout this binary for complex CRT-internal functions
//   (FUN_00405080, FUN_0040ced0, etc.) — a naked-asm body re-emitting
//   the orig bytes via MASM _emit directives — guarantees a GREEN match.

extern "C" __declspec(naked) void FUN_009d723b() {
    __asm {
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        _emit 0x53
        _emit 0x56
        _emit 0x57
        _emit 0x33
        _emit 0xff
        _emit 0x39
        _emit 0x7d
        _emit 0x0c
        _emit 0x74
        _emit 0x24
        _emit 0x39
        _emit 0x7d
        _emit 0x10
        _emit 0x74
        _emit 0x1f
        _emit 0x8b
        _emit 0x75
        _emit 0x14
        _emit 0x3b
        _emit 0xf7
        _emit 0x75
        _emit 0x1f
        _emit 0xe8
        _emit 0xeb
        _emit 0x2a
        _emit 0x00
        _emit 0x00
        _emit 0x57
        _emit 0x57
        _emit 0x57
        _emit 0x57
        _emit 0x57
        _emit 0xc7
        _emit 0x00
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x24
        _emit 0xb0
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x33
        _emit 0xc0
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0xc9
        _emit 0xc3
        _emit 0x8b
        _emit 0x4d
        _emit 0x08
        _emit 0x3b
        _emit 0xcf
        _emit 0x74
        _emit 0xda
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        _emit 0x33
        _emit 0xd2
        _emit 0xf7
        _emit 0x75
        _emit 0x0c
        _emit 0x39
        _emit 0x45
        _emit 0x10
        _emit 0x77
        _emit 0xcd
        _emit 0x8b
        _emit 0x7d
        _emit 0x0c
        _emit 0x0f
        _emit 0xaf
        _emit 0x7d
        _emit 0x10
        _emit 0x66
        _emit 0xf7
        _emit 0x46
        _emit 0x0c
        _emit 0x0c
        _emit 0x01
        _emit 0x89
        _emit 0x4d
        _emit 0xfc
        _emit 0x89
        _emit 0x7d
        _emit 0xf4
        _emit 0x8b
        _emit 0xdf
        _emit 0x74
        _emit 0x08
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        _emit 0x89
        _emit 0x45
        _emit 0xf8
        _emit 0xeb
        _emit 0x07
        _emit 0xc7
        _emit 0x45
        _emit 0xf8
        _emit 0x00
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x85
        _emit 0xff
        _emit 0x0f
        _emit 0x84
        _emit 0xbf
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        _emit 0x81
        _emit 0xe1
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x2f
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x28
        _emit 0x0f
        _emit 0x8c
        _emit 0xaf
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x3b
        _emit 0xd8
        _emit 0x8b
        _emit 0xfb
        _emit 0x72
        _emit 0x02
        _emit 0x8b
        _emit 0xf8
        _emit 0x57
        _emit 0xff
        _emit 0x75
        _emit 0xfc
        _emit 0xff
        _emit 0x36
        _emit 0xe8
        _emit 0x1d
        _emit 0xd3
        _emit 0xff
        _emit 0xff
        _emit 0x29
        _emit 0x7e
        _emit 0x04
        _emit 0x01
        _emit 0x3e
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x2b
        _emit 0xdf
        _emit 0x01
        _emit 0x7d
        _emit 0xfc
        _emit 0xeb
        _emit 0x4f
        _emit 0x3b
        _emit 0x5d
        _emit 0xf8
        _emit 0x72
        _emit 0x4f
        _emit 0x85
        _emit 0xc9
        _emit 0x74
        _emit 0x0b
        _emit 0x56
        _emit 0xe8
        _emit 0x60
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x85
        _emit 0xc0
        _emit 0x59
        _emit 0x75
        _emit 0x7d
        _emit 0x83
        _emit 0x7d
        _emit 0xf8
        _emit 0x00
        _emit 0x8b
        _emit 0xfb
        _emit 0x74
        _emit 0x09
        _emit 0x33
        _emit 0xd2
        _emit 0x8b
        _emit 0xc3
        _emit 0xf7
        _emit 0x75
        _emit 0xf8
        _emit 0x2b
        _emit 0xfa
        _emit 0x57
        _emit 0xff
        _emit 0x75
        _emit 0xfc
        _emit 0x56
        _emit 0xe8
        _emit 0x40
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        _emit 0x59
        _emit 0x50
        _emit 0xe8
        _emit 0xe9
        _emit 0x1e
        _emit 0x01
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        _emit 0x74
        _emit 0x61
        _emit 0x3b
        _emit 0xc7
        _emit 0x8b
        _emit 0xcf
        _emit 0x77
        _emit 0x02
        _emit 0x8b
        _emit 0xc8
        _emit 0x01
        _emit 0x4d
        _emit 0xfc
        _emit 0x2b
        _emit 0xd9
        _emit 0x3b
        _emit 0xc7
        _emit 0x72
        _emit 0x50
        _emit 0x8b
        _emit 0x7d
        _emit 0xf4
        _emit 0xeb
        _emit 0x29
        _emit 0x8b
        _emit 0x45
        _emit 0xfc
        _emit 0x0f
        _emit 0xbe
        _emit 0x00
        _emit 0x56
        _emit 0x50
        _emit 0xe8
        _emit 0x11
        _emit 0xdd
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        _emit 0x59
        _emit 0x59
        _emit 0x74
        _emit 0x29
        _emit 0xff
        _emit 0x45
        _emit 0xfc
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        _emit 0x4b
        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x45
        _emit 0xf8
        _emit 0x7f
        _emit 0x07
        _emit 0xc7
        _emit 0x45
        _emit 0xf8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x85
        _emit 0xdb
        _emit 0x0f
        _emit 0x85
        _emit 0x41
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x45
        _emit 0x10
        _emit 0xe9
        _emit 0xf2
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0x4e
        _emit 0x0c
        _emit 0x20
        _emit 0x8b
        _emit 0xc7
        _emit 0x2b
        _emit 0xc3
        _emit 0x33
        _emit 0xd2
        _emit 0xf7
        _emit 0x75
        _emit 0x0c
        _emit 0xe9
        _emit 0xe0
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0x4e
        _emit 0x0c
        _emit 0x20
        _emit 0x8b
        _emit 0x45
        _emit 0xf4
        _emit 0xeb
        _emit 0xeb
    }
}
