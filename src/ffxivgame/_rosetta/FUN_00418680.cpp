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
// FUNCTION: ffxivgame 0x00018680 — __cdecl 1-arg forwarding wrapper that
//                                   obtains a singleton via FUN_0041b950
//                                   and calls FUN_00430dc0 on it (18 B / 0x12).
//
// Behaviour read from the disassembly at orig RVA 0x00018680:
//
//   __cdecl void FUN_00418680(void* arg) {
//       CObj* obj = FUN_0041b950();   // no-arg singleton getter
//       obj->FUN_00430dc0(arg);       // __thiscall method with one stack arg
//   }
//
//   Calling convention: __cdecl (plain `RET` — caller cleans the single
//   inbound arg).  No stack frame; no callee-saved registers touched; no
//   security cookie.
//
//   Asm shape (18 bytes total):
//
//     e8 RR RR RR RR     call  FUN_0041b950          ; no args → EAX = obj
//     8b 4c 24 04        mov   ecx, [esp+4]           ; load arg1 into ecx
//     51                 push  ecx                    ; push arg for method
//     8b c8              mov   ecx, eax               ; ecx = this (obj)
//     e8 RR RR RR RR     call  FUN_00430dc0           ; __thiscall(this, arg)
//     c3                 ret                          ; plain __cdecl ret
//
//   MSVC 2005 /O2 register allocation: after CALL FUN_0041b950, EAX holds
//   the object pointer that must survive until `MOV ECX,EAX`. Loading arg1
//   into ECX (rather than EAX) preserves EAX intact, then PUSH ECX saves
//   the arg on the stack before ECX is overwritten with the this pointer.

// Opaque forward declaration for the type returned by FUN_0041b950.
class CObj_0041b950 {
public:
    void FUN_00430dc0(void* arg);
};

extern "C" CObj_0041b950* FUN_0041b950();

extern "C" void FUN_00418680(void* arg) {
    CObj_0041b950* obj = FUN_0041b950();
    obj->FUN_00430dc0(arg);
}
