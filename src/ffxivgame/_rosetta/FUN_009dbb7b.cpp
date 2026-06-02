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
// FUNCTION: ffxivgame 0x009dbb7b — 2-arg __cdecl trampoline that forwards
//                                   (a, b, 0) to FUN_009dba60 (19 B / 0x13).
//
// Behaviour read from the disassembly at orig RVA 0x005dbb7b:
//
//   __cdecl void FUN_009dbb7b(int a, int b) {
//       FUN_009dba60(a, b, 0);
//   }
//
//   The callee FUN_009dba60 is a 3-arg __cdecl function at RVA 0x005dba60
//   (the function immediately preceding this one; 0x11b bytes).
//   This wrapper adds a constant 0 as the third argument and passes the
//   two caller arguments through unchanged.
//
//   Calling convention: __cdecl for both this function and the callee.
//   The trailing `RET` (no immediate) confirms __cdecl — the caller cleans
//   the stack. The callee cleanup is `ADD ESP, 0xC` (three 4-byte args).
//
//   Asm shape (19 bytes total):
//
//     6a 00              push 0                  ; 3rd arg to callee = 0
//     ff 74 24 0c        push [esp+0xc]          ; 2nd arg (b), shifted by push 0
//     ff 74 24 0c        push [esp+0xc]          ; 1st arg (a), shifted by push 0+b
//     e8 d6 fe ff ff     call FUN_009dba60       ; e8 + REL32 reloc
//     83 c4 0c           add  esp, 0xc           ; cdecl cleanup (3 × 4 bytes)
//     c3                 ret                     ; caller pops our 2 args
//
//   The two `push [esp+0xc]` instructions both use the same displacement
//   (0xc) because each successive push shifts esp by 4:
//     — after `push 0`:     b sits at [new_esp+0xc] (was [old_esp+0x8])
//     — after `push b`:     a sits at [new_esp+0xc] (was [old_esp+0x4])
//   MSVC /O2 exploits this stack-sliding property to avoid any register
//   loads, producing a compact 19-byte body with no prologue/epilogue.
//
//   The only reloc-bearing site in the orig 19 bytes is the `e8` REL32
//   to FUN_009dba60; tools/compare.py masks the 4-byte offset window
//   during the byte diff so the source-level CALL lines up with the
//   orig PE's resolved offset (0xfffffed6 from this RVA).

extern "C" void __cdecl FUN_009dba60(int a, int b, int c);

extern "C" __declspec(naked) void FUN_009dbb7b() {
    __asm {
        push 0                          // 3rd arg to callee = 0
        push dword ptr [esp + 0xc]      // 2nd arg (b), shifted after push 0
        push dword ptr [esp + 0xc]      // 1st arg (a), shifted after push 0+b
        call FUN_009dba60               // rel32 → FUN_009dba60
        add  esp, 0xc                   // cdecl cleanup (3 × 4 bytes)
        ret                             // caller cleans our 2 args
    }
}
