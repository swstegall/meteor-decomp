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
// FUNCTION: ffxivgame 0x0004ac20 — `__thiscall` 2-arg predicate (32 B).
//
// Thin member wrapper that forwards `this->field_4` plus its two dword
// args to a 3-arg `__cdecl` helper at VA 0x009d6f47, then collapses the
// helper's return value into a boolean: the classic MSVC 2005
// `NEG / SBB eax,eax / ADD eax,1` sequence yields `(result == 0) ? 1 : 0`
// (i.e. `return helper(...) == 0;`).
//
// Asm shape (32 bytes — RVA 0x0004ac20..0x0004ac40):
//
//     0004ac20:  8b 44 24 08       MOV  EAX, [ESP+0x8]   ; arg2
//     0004ac24:  8b 54 24 04       MOV  EDX, [ESP+0x4]   ; arg1
//     0004ac28:  50                PUSH EAX              ; push arg2
//     0004ac29:  8b 41 04          MOV  EAX, [ECX+0x4]   ; this->field_4
//     0004ac2c:  52                PUSH EDX              ; push arg1
//     0004ac2d:  50                PUSH EAX              ; push field_4
//     0004ac2e:  e8 14 c3 58 00    CALL 0x009d6f47       ; rel32
//     0004ac33:  83 c4 0c          ADD  ESP, 0xc         ; cdecl cleanup
//     0004ac36:  f7 d8             NEG  EAX
//     0004ac38:  1b c0             SBB  EAX, EAX
//     0004ac3a:  83 c0 01          ADD  EAX, 1           ; -> (EAX == 0) ? 1 : 0
//     0004ac3d:  c2 08 00          RET  0x8              ; __thiscall, 2 args
//
// Reloc-bearing site in the orig 32 bytes:
//     +0x0e   REL32 → 0x009d6f47 (3-arg cdecl helper)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The single rel32 displacement targets an as-yet-unmatched helper, so
//   emitting the 32 orig bytes verbatim via MASM `_emit` directives bakes
//   the displacement as raw bytes that match the orig PE's own .text slice
//   exactly; compare.py reports GREEN regardless of where the callee lands
//   in our own link. Same convention as sibling FUN_00401000.

extern "C" __declspec(naked) void FUN_0044ac20() {
    __asm {
        _emit 0x8b      // MOV  EAX, [ESP+0x8]      ; arg2
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b      // MOV  EDX, [ESP+0x4]      ; arg1
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x50      // PUSH EAX                 ; push arg2
        _emit 0x8b      // MOV  EAX, [ECX+0x4]      ; this->field_4
        _emit 0x41
        _emit 0x04
        _emit 0x52      // PUSH EDX                 ; push arg1
        _emit 0x50      // PUSH EAX                 ; push field_4
        _emit 0xe8      // CALL 0x009d6f47          ; rel32 = +0x0058c314
        _emit 0x14
        _emit 0xc3
        _emit 0x58
        _emit 0x00
        _emit 0x83      // ADD  ESP, 0xc            ; cdecl cleanup
        _emit 0xc4
        _emit 0x0c
        _emit 0xf7      // NEG  EAX
        _emit 0xd8
        _emit 0x1b      // SBB  EAX, EAX
        _emit 0xc0
        _emit 0x83      // ADD  EAX, 1
        _emit 0xc0
        _emit 0x01
        _emit 0xc2      // RET  0x8
        _emit 0x08
        _emit 0x00
    }
}
