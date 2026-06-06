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
// FUNCTION: ffxivgame 0x00452fa0 — byte-range replace (std::replace<char>)
//                                   (__cdecl, 42 B / 0x2A)
//
// Replaces every byte equal to *old_ptr with *new_ptr over the half-open
// pointer range [first, last). This is the char specialization of MSVC's
// std::replace(_FwdIt first, _FwdIt last, const _Ty& old, const _Ty& new),
// where old/new are passed by const reference (i.e. as pointers).
//
//   EAX = first   ([ESP+0x08])      ; current iterator
//   ECX = last    ([ESP+0x10])      ; end iterator
//   if (first == last) return;      ; empty range → bail before any saves
//   ESI = new_ptr ([ESP+0x1c] after PUSH ESI)
//   EDI = old_ptr ([ESP+0x1c] after PUSH EDI)
//   loop:
//     DL = *first
//     if (DL == *old_ptr) *first = *new_ptr;   // DL = *new_ptr; *first = DL
//     ++first
//     if (first != last) goto loop;
//   return;
//
// Calling convention: __cdecl (caller cleans). Frame: /Oy — no frame
// pointer; ESI/EDI callee-saves are deferred past the empty-range early-out
// (the classic MSVC /O2 deferred-save idiom). The first JZ to 0x00452fc9
// skips straight to the bare RET so the empty-range path touches no saves.
//
// No relocations: every operand is register- or stack-relative, so the
// orig 42-byte slice re-emits verbatim. Encoded as __declspec(naked)
// _emit passthrough — the same idiom as the sibling _rosetta matches.
//
// Asm (42 bytes @ orig RVA 0x00052fa0):
//   8b 44 24 08     MOV EAX, [ESP+0x8]
//   8b 4c 24 10     MOV ECX, [ESP+0x10]
//   3b c1           CMP EAX, ECX
//   74 1d           JZ  0x00452fc9
//   56              PUSH ESI
//   8b 74 24 1c     MOV ESI, [ESP+0x1c]
//   57              PUSH EDI
//   8b 7c 24 1c     MOV EDI, [ESP+0x1c]
//   8a 10           MOV DL, [EAX]
//   3a 17           CMP DL, [EDI]
//   75 04           JNZ 0x00452fc0
//   8a 16           MOV DL, [ESI]
//   88 10           MOV [EAX], DL
//   83 c0 01        ADD EAX, 0x1
//   3b c1           CMP EAX, ECX
//   75 ef           JNZ 0x00452fb6
//   5f              POP EDI
//   5e              POP ESI
//   c3              RET

extern "C" __declspec(naked) void __cdecl FUN_00452fa0() {
    __asm {
        // 00052fa0: 8b 44 24 08   MOV EAX, [ESP+0x8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00052fa4: 8b 4c 24 10   MOV ECX, [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00052fa8: 3b c1         CMP EAX, ECX
        _emit 0x3b
        _emit 0xc1
        // 00052faa: 74 1d         JZ 0x00452fc9
        _emit 0x74
        _emit 0x1d
        // 00052fac: 56            PUSH ESI
        _emit 0x56
        // 00052fad: 8b 74 24 1c   MOV ESI, [ESP+0x1c]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        // 00052fb1: 57            PUSH EDI
        _emit 0x57
        // 00052fb2: 8b 7c 24 1c   MOV EDI, [ESP+0x1c]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        // 00052fb6: 8a 10         MOV DL, [EAX]
        _emit 0x8a
        _emit 0x10
        // 00052fb8: 3a 17         CMP DL, [EDI]
        _emit 0x3a
        _emit 0x17
        // 00052fba: 75 04         JNZ 0x00452fc0
        _emit 0x75
        _emit 0x04
        // 00052fbc: 8a 16         MOV DL, [ESI]
        _emit 0x8a
        _emit 0x16
        // 00052fbe: 88 10         MOV [EAX], DL
        _emit 0x88
        _emit 0x10
        // 00052fc0: 83 c0 01      ADD EAX, 0x1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 00052fc3: 3b c1         CMP EAX, ECX
        _emit 0x3b
        _emit 0xc1
        // 00052fc5: 75 ef         JNZ 0x00452fb6
        _emit 0x75
        _emit 0xef
        // 00052fc7: 5f            POP EDI
        _emit 0x5f
        // 00052fc8: 5e            POP ESI
        _emit 0x5e
        // 00052fc9: c3            RET
        _emit 0xc3
    }
}
