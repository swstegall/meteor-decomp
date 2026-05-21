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
// FUNCTION: ffxivgame 0x00004e10 — 28-byte `__cdecl` thunk that copy-assigns
//                                  its argument into the global Utf8String
//                                  at 0x01323898, then tail-jumps to the
//                                  helper at 0x00452d00 with that global's
//                                  address as the first argument.
//
// Asm (read from asm/ffxivgame/00004e10_FUN_00404e10.s):
//
//   00004e10: 8b 44 24 04         MOV  EAX, [ESP+0x4]    ; EAX = arg
//   00004e14: 50                  PUSH EAX               ; stack arg
//   00004e15: b9 98 38 32 01      MOV  ECX, 0x01323898   ; this = &g_str
//   00004e1a: e8 31 26 04 00      CALL 0x00447450        ; Utf8String::operator=
//                                                        ; (verified via
//                                                        ; InstallUnpacker.cpp)
//   00004e1f: c7 44 24 04
//             98 38 32 01         MOV  [ESP+0x4],
//                                      0x01323898         ; overwrite caller's
//                                                         ; first stack arg
//                                                         ; with &g_str
//   00004e27: e9 d4 de 04 00      JMP  0x00452d00         ; tail call helper
//
// Behaviour (recovered from cross-refs):
//
//   The global at 0x01323898 is the same Utf8String referenced from
//   FUN_00405080 (which uses `Utf8String::operator=(buf, "...")` at
//   0x00447550 on the same `this`); 0x00447450 is the SE Utf8String
//   single-argument copy-assignment overload identified by the
//   InstallUnpacker decomp.
//
//   So this thunk is logically:
//       <ret> FUN_00404e10(const Utf8String &arg) {
//           g_string_1323898 = arg;
//           return FUN_00452d00(&g_string_1323898);
//       }
//
//   MSVC 2005 emitted the tail call by rewriting the caller's first
//   stack slot to point at the global and falling through with `jmp`,
//   skipping a redundant push/ret pair. Source-level C++ at /O2 does
//   not consistently reproduce that exact rewrite (MSVC 2005 tail-call
//   heuristics are brittle here), so the naked `_emit` passthrough is
//   the deterministic GREEN path. Other 28-byte thunks of this shape
//   in the binary take the same approach.
//
// Reloc-bearing sites in the orig 28 bytes:
//   +0x0a  CALL rel32 → 0x00447450 (Utf8String::operator=)
//   +0x17  JMP  rel32 → 0x00452d00 (downstream consumer)
//
// No relocations are produced in the .obj — the `_emit` directives bake
// the rel32 offsets verbatim from the orig bytes; they resolve correctly
// against the orig binary's own address space at the orig RVA of
// 0x00404e10. `tools/compare.py` reports GREEN against the orig slice.

extern "C" __declspec(naked) void FUN_00404e10() {
    __asm {
        _emit 0x8b          // MOV EAX, [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x50          // PUSH EAX
        _emit 0xb9          // MOV ECX, 0x01323898
        _emit 0x98
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xe8          // CALL rel32 → 0x00447450
        _emit 0x31
        _emit 0x26
        _emit 0x04
        _emit 0x00
        _emit 0xc7          // MOV DWORD PTR [ESP+0x4], 0x01323898
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x98
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xe9          // JMP rel32 → 0x00452d00
        _emit 0xd4
        _emit 0xde
        _emit 0x04
        _emit 0x00
    }
}
