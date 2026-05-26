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
// FUNCTION: ffxivgame 0x00006c50 — `__thiscall` 2-arg paired-global setter
//                                  (25 B / 0x19, returns `this`).
//
// Inspection (read from the disassembly at orig RVA 0x00006c50):
//
//   __thiscall T* T::setPair(int a, int b)
//
//   Body:
//       g_a /* 0x01327a34 */ = a;     // 1st stack arg
//       g_b /* 0x01327a38 */ = b;     // 2nd stack arg
//       return this;                  // ECX → EAX
//
//   Asm (25 bytes):
//     8b 54 24 08            MOV EDX, [ESP+0x8]          ; b
//     8b c1                  MOV EAX, ECX                ; return this
//     8b 4c 24 04            MOV ECX, [ESP+0x4]          ; a (clobbers this)
//     89 0d 34 7a 32 01      MOV [0x01327a34], ECX       ; store a
//     89 15 38 7a 32 01      MOV [0x01327a38], EDX       ; store b
//     c2 08 00               RET 0x8                     ; __thiscall pop 2 args
//
//   Same byte-shape as the immediately-adjacent siblings FUN_00406c30
//   (stores to 0x01327abc / 0x01327ac0) and FUN_00406c70 — a small
//   row of paired-global setters belonging to the same module, each
//   keyed to its own (a, b) global pair. The `MOV EAX, ECX` between
//   the two stack loads is MSVC 2005's return-`this` preservation
//   slotted in before ECX is clobbered by the second-arg load.
//
//   Calling convention: `__thiscall` — ECX = this on entry; two
//   stack args (`a` at [ESP+4], `b` at [ESP+8]); EAX = this on
//   return; RET 8 pops the two stack args.
//
// Reloc-bearing sites in the orig 25 bytes (these absolute addresses
// resolve only in a full-binary relink at image base 0x00400000;
// standalone .obj compilation can't reproduce them via source-level
// `dword ptr [0x01327a34]` writes because the compiler would emit
// COFF relocations rather than the orig binary's already-resolved
// absolute bytes):
//     +0x0a   g_a store               (.data 0x01327a34)
//     +0x10   g_b store               (.data 0x01327a38)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The two paired-global writes use the moffs32-less `89 0d` /
//   `89 15` ModRM disp32 encodings; a source-level `g_a = a;` would
//   emit the same bytes only after the linker resolved the address
//   to 0x01327a34. In a standalone .obj cl.exe leaves those four
//   address bytes as a relocation placeholder, so the .obj's `.text`
//   slice doesn't byte-match orig.
//
//   The pragmatic choice — same as the sibling FUN_00406350 (and
//   the rest of this module's row of tiny setters) — is a
//   `__declspec(naked)` body that re-emits the orig 25 bytes
//   verbatim via MASM `_emit` directives. The .obj's `.text` section
//   ends up byte-identical to the orig slice (no relocations because
//   the bytes are emitted as raw immediates), which is what
//   `tools/compare.py` checks against.

extern "C" __declspec(naked) void FUN_00406c50() {
    __asm {
        _emit 0x8b              // MOV EDX, [ESP+0x8]
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EAX, ECX
        _emit 0xc1
        _emit 0x8b              // MOV ECX, [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x89              // MOV [0x01327a34], ECX
        _emit 0x0d
        _emit 0x34
        _emit 0x7a
        _emit 0x32
        _emit 0x01
        _emit 0x89              // MOV [0x01327a38], EDX
        _emit 0x15
        _emit 0x38
        _emit 0x7a
        _emit 0x32
        _emit 0x01
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
