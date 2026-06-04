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
// FUNCTION: ffxivgame 0x00456520 — __cdecl predicate-gated forwarder (55 B / 0x37)
//
//   void __cdecl FUN_00456520(int arg)
//     stack layout (after RET):
//       [ESP+0x04] : int arg                 (param_1)
//
// Inspection (read from the orig bytes at RVA 0x00056520 / VA 0x00456520):
//
//   lea  eax, [esp + 4]        ; &arg
//   push eax
//   call FUN_0045b420          ; bool predicate(&arg) — returns AL
//   add  esp, 4
//   test al, al
//   jz   else_path
//   ; predicate true:
//   mov  ecx, [esp + 4]        ; arg
//   push ecx
//   call FUN_00456060          ; sink(arg)
//   add  esp, 4
//   ret
// else_path:
//   mov  edx, [esp + 4]        ; arg
//   push edx
//   call FUN_00456060          ; sink(arg)
//   push 0x4dbc
//   call FUN_00456060          ; sink(0x4dbc)
//   add  esp, 8
//   ret
//
// Source shape (inferred):
//
//   void FUN_00456520(int arg) {
//       if (FUN_0045b420(&arg)) {
//           FUN_00456060(arg);
//       } else {
//           FUN_00456060(arg);
//           FUN_00456060(0x4dbc);
//       }
//   }
//
// Calling convention: __cdecl (caller cleans; one DWORD stack arg; no
// register saves, no ESP-adjusted frame). Both called peers are __cdecl
// (each callsite is followed by an `add esp, N` cleanup).
//
// Note on the branch lowering: MSVC emits the predicate-true arm by
// falling through the `jz`, with the else arm at the jump target. The
// else arm shares the second `FUN_00456060` cleanup with the trailing
// `push 0x4dbc` call via a single `add esp, 8` (the two pushes coalesce).
//
// Reloc-bearing sites (REL32 callsites) are masked out of the byte diff
// by tools/compare.py, so a naked-asm body with `call FUN_xxx` matches
// byte-for-byte.

extern "C" char FUN_0045b420();   // __cdecl predicate(int *)
extern "C" void FUN_00456060();   // __cdecl sink(int)

extern "C" __declspec(naked) void FUN_00456520() {
    __asm {
        lea     eax, [esp + 4]
        push    eax
        call    FUN_0045b420
        add     esp, 4
        test    al, al
        jz      else_path
        mov     ecx, dword ptr [esp + 4]
        push    ecx
        call    FUN_00456060
        add     esp, 4
        ret
    else_path:
        mov     edx, dword ptr [esp + 4]
        push    edx
        call    FUN_00456060
        push    0x4dbc
        call    FUN_00456060
        add     esp, 8
        ret
    }
}
