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
// FUNCTION: ffxivgame 0x00403c60 — `__stdcall` thin allocate wrapper around
//                                   the `_Allocate<char>` overflow-checking
//                                   helper at 0x00401090 (18 B / 0x12).
//
// Behaviour read from the disassembly at orig RVA 0x00003c60:
//
//   __stdcall void* FUN_00403c60(unsigned int count) {
//       return _Allocate(count, /*type-deduction ptr*/ 0);
//   }
//
//   The callee at 0x00401090 is MSVC's `std::_Allocate<_Ty>` template
//   instantiation for `_Ty = char` (see src/ffxivgame/_rosetta/
//   FUN_00401090.cpp). Its declared signature is the two-argument form
//   `__cdecl _Ty* _Allocate(size_t count, _Ty*)` — the second argument
//   exists only for template type deduction and is discarded by the
//   callee, which loads `[ESP+4]` (count) and ignores `[ESP+8]`. This
//   wrapper passes `(count, 0)` accordingly: an `std::allocator<char>`-
//   style adapter that drops the `(_Ty*)0` for the template machinery
//   below it.
//
//   Calling convention: `__stdcall` (one 4-byte stack arg, callee pops
//   via `RET 4`). Zero callee-saved registers touched, no stack frame.
//
//   Asm shape (18 bytes total):
//
//     8b 44 24 04        mov  eax, [esp+4]      ; load count
//     6a 00              push 0                  ; type-ptr (discarded)
//     50                 push eax                ; count
//     e8 RR RR RR RR     call _Allocate          ; e8 + REL32 reloc
//     83 c4 08           add  esp, 8             ; cdecl cleanup (2 args)
//     c2 04 00           ret  4                  ; stdcall epilogue
//
//   The only reloc-bearing site in the orig 18 bytes is the `e8` REL32
//   to FUN_00401090; `tools/compare.py` masks the 4-byte offset window
//   during the byte diff so the source-level CALL (which resolves to
//   the differently-placed sibling symbol in the .obj) lines up with
//   the orig PE's resolved offset (0xffffd424 from this RVA).

extern "C" void* __cdecl FUN_00401090(unsigned int count, void* type_ptr);

extern "C" void* __stdcall FUN_00403c60(unsigned int count) {
    return FUN_00401090(count, 0);
}
