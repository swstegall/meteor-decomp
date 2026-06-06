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
// FUNCTION: ffxivgame 0x009e0216 — FUN_009e0216 (__cdecl, 26 B)
//
// Small flag-check stub: loads [arg+0xc], tests bits 0x83 (flags 0/1/7)
// and bit 0x08 separately.  Either zero → jump to the epilogue at
// 0x9e0240 (pop esi / ret, shared with the enclosing cluster).
// Otherwise: push [arg+0x8], call FUN_009d5c88, then fall through to
// the AND+clear block at 0x9e022e (declared outside this 26-byte slice).
//
// The declared size (26 B / 0x1a) in symbols.json cuts through the
// 7-byte AND [ESI+0xc], 0xFFFFFBF7 instruction at 0x9e022e; only the
// first two bytes (81 66) fall inside this function's range.  The
// diff tool masks the 4-byte CALL displacement; the remaining 24
// structural bytes must be verbatim, so the implementation uses
// __declspec(naked) with _emit for the JE displacement bytes and the
// trailing AND prefix.
//
// Asm (26 bytes @ RVA 0x5e0216, VA 0x9e0216):
//   56                    PUSH ESI
//   8B 74 24 08           MOV  ESI, [ESP+8]          ; first arg
//   8B 46 0C              MOV  EAX, [ESI+0Ch]        ; flags field
//   A8 83                 TEST AL, 83h               ; bits 0, 1, 7
//   74 1E                 JE   → 0x9e0240 (epilogue)
//   A8 08                 TEST AL, 08h               ; bit 3
//   74 1A                 JE   → 0x9e0240 (epilogue)
//   FF 76 08              PUSH dword ptr [ESI+8]
//   E8 RR RR RR RR        CALL FUN_009d5c88           ; REL32 reloc
//   81 66                 (first 2 bytes of AND [ESI+0Ch], 0xFFFFFBF7)

extern "C" void FUN_009d5c88();

extern "C" __declspec(naked) void FUN_009e0216() {
    __asm {
        push    esi
        mov     esi, dword ptr [esp + 8]
        mov     eax, dword ptr [esi + 0x0c]
        test    al, 0x83
        _emit   0x74
        _emit   0x1e
        test    al, 0x08
        _emit   0x74
        _emit   0x1a
        push    dword ptr [esi + 8]
        call    FUN_009d5c88
        _emit   0x81
        _emit   0x66
    }
}
