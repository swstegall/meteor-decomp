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
// FUNCTION: ffxivgame 0x00414c10 — refcounted "init once" guard
//                                  (__cdecl, 40 B)
//
// Tracks a global initialisation refcount.  On the *first* call, the
// caller-supplied param's byte at +0x8 is captured into a static byte,
// and a thiscall initializer (5-byte JMP-thunk FUN_00414d60, which
// forwards to FUN_00414df0 — a `_time64`/`srand` pair) is invoked on a
// trailing singleton storage slot.  Every call — first or not — bumps
// the refcount by one.
//
//   DAT_01328040  int   refcount (0 ⇒ uninitialised)
//   DAT_01328044  void  singleton storage; passed as ECX to the
//                       __thiscall initializer
//   DAT_01328058  char  captured init flag (param->byte_at_0x8)
//   FUN_00414d60        5-byte JMP-thunk → FUN_00414df0 (init body)
//
// param        = pointer to a small param struct constructed at the
//                callsite by FUN_00414bf0 (fields 0/4 zeroed, byte at
//                +0x8 set to 1).  Only param->byte_at_0x8 is read.
//
// Calling convention: __cdecl (caller cleans), one stack arg, plain RET.
// No locals, no prolog/epilog.
//
// Why `__declspec(naked)`:
//
//   MSVC /O2 picks CL specifically for the byte transfer because it
//   knows ECX is about to be overwritten by `mov ecx, offset
//   DAT_01328044` for the thiscall.  A natural source-level lowering
//   can't reliably pin the byte register choice across a forward call,
//   and the use of `add [mem], 1` (vs `inc [mem]`) for the refcount
//   bump is also brittle to MSVC's flag-dependency heuristic.  Naked
//   asm fixes both encodings at the exact 40-byte shape orig shipped.
//
// Reloc-bearing positions in the resulting .obj (all masked in the
// byte-level diff by compare.py via COFF DIR32 / REL32 mask):
//
//   off 0x02 : DIR32 → DAT_01328040 (cmp)
//   off 0x16 : DIR32 → DAT_01328058 (mov byte ptr [...], cl)
//   off 0x1c : DIR32 → DAT_01328044 (mov ecx, offset ...)
//   off 0x21 : REL32 → FUN_00414d60 (call)
//   off 0x26 : DIR32 → DAT_01328040 (add)

extern "C" {
// External symbols supplying the four DIR32 + one REL32 fixups.
// Declared as int / char so `offset SYM` and direct `[SYM]` references
// in inline asm produce the right-sized fixup.
extern int  DAT_01328040;   // refcount
extern int  DAT_01328044;   // singleton storage (this for the call)
extern char DAT_01328058;   // captured init flag
void FUN_00414d60();        // JMP-thunk → FUN_00414df0
} // extern "C"

extern "C" __declspec(naked) void FUN_00414c10()
{
    __asm {
        cmp     DAT_01328040, 0
        jne     short skip
        mov     eax, dword ptr [esp + 4]
        mov     cl,  byte ptr [eax + 8]
        mov     DAT_01328058, cl
        mov     ecx, offset DAT_01328044
        call    FUN_00414d60
    skip:
        add     DAT_01328040, 1
        ret
    }
}

// vim: ts=4 sts=4 sw=4 et
