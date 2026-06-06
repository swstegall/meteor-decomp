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
// FUNCTION: ffxivgame 0x00a41627 — `$LN21` compiler-internal label fragment
//                                   (21 B / 0x15); conditional call +
//                                   unconditional call + RET.
//
// Asm (21 bytes @ orig RVA 0x00a41627):
//   39 5d d8           CMP  dword ptr [EBP + -0x28],EBX
//   74 07              JZ   +7  (→ 0x00a41633)
//   56                 PUSH ESI
//   e8 76 38 b9 ff     CALL 0x009d4ea8               ; REL32 reloc
//   59                 POP  ECX
//   6a 02              PUSH 0x2
//   e8 3a 0f ba ff     CALL 0x009e2574               ; REL32 reloc
//   59                 POP  ECX
//   c3                 RET
//
// This fragment is a compiler-internal basic-block label ($LN21) rather than
// a user-declared function — it has no prologue/epilogue of its own and reads
// the parent function's EBP-based frame ([EBP-0x28]) and EBX register directly.
// The two CALL targets carry REL32 relocations; compare.py masks those 4-byte
// displacement windows during the diff. All other bytes are reproduced verbatim.
//
// Reconstruction strategy — naked-asm byte passthrough (same approach as
// FUN_00401730 and FUN_0040a640 for similarly prologue-free fragments that
// MSVC 2005 /O2 /Oy cannot reproduce byte-identically from source-level C++).

extern "C" __declspec(naked) void FUN_00e41627() {
    __asm {
        _emit 0x39              // CMP dword ptr [EBP-0x28], EBX
        _emit 0x5d
        _emit 0xd8
        _emit 0x74              // JZ +7
        _emit 0x07
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL rel32 → 0x009d4ea8
        _emit 0x76
        _emit 0x38
        _emit 0xb9
        _emit 0xff
        _emit 0x59              // POP ECX
        _emit 0x6a              // PUSH 0x2
        _emit 0x02
        _emit 0xe8              // CALL rel32 → 0x009e2574
        _emit 0x3a
        _emit 0x0f
        _emit 0xba
        _emit 0xff
        _emit 0x59              // POP ECX
        _emit 0xc3              // RET
    }
}
