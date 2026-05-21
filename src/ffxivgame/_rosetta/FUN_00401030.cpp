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
// FUNCTION: ffxivgame 0x00401030 — std::bad_alloc::bad_alloc(const char *
//                                  const &) (__thiscall, 25 B)
//
// One of MSVC 2005's canonical std::exception-derived ctors: takes its
// `const char *const &msg` argument, forwards it to the base
// std::exception ctor at VA 0x009d18da, then stamps the std::bad_alloc
// vftable VA (0x00f54a10) into [this] and returns `this` in EAX.
//
// Structurally identical to its three immediate siblings in the CRT
// EH support cluster (all four are in src/ffxivgame/_rosetta/ once
// landed):
//   bad_cast           @ 0x009d19bb  — same 25-byte shape
//   bad_typeid         @ 0x009d19f7  — same 25-byte shape
//   __non_rtti_object  @ 0x009d1a33  — 24 B; uses `push [esp+8]` in
//                                       place of `lea+push` (no &msg
//                                       materialisation — exception ctor
//                                       gets the original-arg slot
//                                       directly)
//
// Asm (25 bytes):
//   56                   PUSH ESI
//   8D 44 24 08          LEA  EAX, [ESP+8]        ; &msg
//   50                   PUSH EAX
//   8B F1                MOV  ESI, ECX            ; ESI = this
//   E8 RR RR RR RR       CALL exception_ctor      ; std::exception::
//                                                 ;   exception(const
//                                                 ;   char *const&)
//                                                 ;   @ VA 0x009d18da
//   C7 06 RR RR RR RR    MOV  dword ptr [ESI],
//                              offset bad_alloc_vftable
//                                                 ; std::bad_alloc vftable
//                                                 ;   @ VA 0x00f54a10
//   8B C6                MOV  EAX, ESI            ; return this
//   5E                   POP  ESI
//   C2 04 00             RET  4                   ; __thiscall cleanup
//
// Translated as a `__declspec(naked)` body so the 25 bytes come out
// exactly verbatim. The two reloc-bearing operands (CALL rel32 to
// std::exception::exception, DIR32 immediate for the bad_alloc vftable)
// are referenced via `extern "C"` symbols; tools/compare.py masks
// those 4-byte windows in its diff.

extern "C" {

// std::exception::exception(const char *const&) ctor at VA 0x009d18da.
// 78 bytes upstream; copies the message pointer into the exception
// object's `_Mywhat` slot and stamps the std::exception vftable.
void exception_ctor();

// std::bad_alloc vftable at VA 0x00f54a10. Slot 0 is the destructor
// (FUN_00401060), slot 1 is the standard MSVC vector-deleting-dtor
// helper (FUN_009d19ae). Referenced via DIR32 reloc on the immediate
// of `mov dword ptr [esi], offset bad_alloc_vftable`.
int bad_alloc_vftable;

__declspec(naked) void FUN_00401030() {
    __asm {
        push    esi
        lea     eax, [esp+8]
        push    eax
        mov     esi, ecx
        call    exception_ctor
        mov     dword ptr [esi], offset bad_alloc_vftable
        mov     eax, esi
        pop     esi
        ret     4
    }
}

}  // extern "C"
