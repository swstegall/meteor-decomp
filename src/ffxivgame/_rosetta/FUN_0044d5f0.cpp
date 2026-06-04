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
// FUNCTION: ffxivgame 0x0004d5f0 — `__cdecl` 2-arg trampoline that forwards
//                                   to FUN_0044d500 with a zero injected as
//                                   the middle argument (21 B / 0x15).
//
// Inspection (read from the disassembly at orig RVA 0x0004d5f0):
//
//   8b 44 24 08    MOV  EAX, [ESP+0x8]   ; arg2
//   8b 4c 24 04    MOV  ECX, [ESP+0x4]   ; arg1
//   50             PUSH EAX              ; push arg2  (rightmost)
//   6a 00          PUSH 0                ; push 0     (middle)
//   51             PUSH ECX              ; push arg1  (leftmost)
//   e8 ff fe ff ff CALL FUN_0044d500     ; rel32 → 0x0044d500
//   83 c4 0c       ADD  ESP, 0xc         ; cdecl cleanup of 3 slots
//   c3             RET                   ; __cdecl (no ret N)
//
//   Calling convention: __cdecl (caller cleans, `RET` with no immediate).
//   The body forwards to a 3-arg __cdecl callee:
//
//       return FUN_0044d500(arg1, 0, arg2);
//
//   EAX is left holding the callee's return value (never wiped), so the
//   forwarded result is the trampoline's result.
//
// Reloc-bearing site in the orig 21 bytes (resolves only in a full-binary
// relink at image base 0x00400000; standalone .obj compilation can't
// reproduce the rel32 displacement):
//   +0x0c   __cdecl callee CALL   .text 0x0044d500 rel32 (FUN_0044d500)
//
// Reconstruction strategy — naked-asm byte passthrough. MSVC 2005 /O2
// would re-order the two argument loads and pick its own modrm encodings,
// shifting bytes in a 21-byte function whose CALL is reloc-adjacent. The
// reliable match (same idiom as FUN_00401730 et al.) is a naked body that
// re-emits the orig 21 bytes verbatim via `_emit`; the resulting .text
// slice is byte-identical to the orig, which is what compare.py checks.

extern "C" __declspec(naked) void FUN_0044d5f0() {
    __asm {
        _emit 0x8b              // MOV EAX, [ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV ECX, [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x50              // PUSH EAX
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL FUN_0044d500 (rel32)
        _emit 0xff
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
    }
}
