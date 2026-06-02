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
// FUNCTION: ffxivgame 0x005d085b — FUN_009d085b (__thiscall, 24 B)
//
// A minimal __thiscall constructor-like function. It:
//   1. Calls FUN_009d1b35(0x18) — likely an allocator — stores result in
//      [this] (first field / vtable slot).
//   2. Calls FUN_009d1782(result) — likely an initializer for the
//      allocated sub-object.
//   3. Returns `this` in EAX via `mov eax, esi`.
//
// Calling convention: __thiscall (ECX = this, no explicit params).
// RET is bare (c3) — no extra stack args to clean up beyond the two
// `pop ecx` pairs used for caller-side cleanup of the two cdecl calls.
//
// Asm (24 bytes):
//   56                   PUSH ESI
//   6a 18                PUSH 0x18          ; arg: size 24
//   8b f1                MOV  ESI, ECX      ; ESI = this
//   e8 RR RR RR RR       CALL FUN_009d1b35  ; allocate/sub-ctor (rel32)
//   50                   PUSH EAX           ; pass result as arg
//   89 06                MOV  [ESI], EAX    ; this->field0 = result
//   e8 RR RR RR RR       CALL FUN_009d1782  ; initialise result (rel32)
//   59                   POP  ECX           ; clean call2 arg
//   59                   POP  ECX           ; clean call1 arg (0x18)
//   8b c6                MOV  EAX, ESI      ; return this
//   5e                   POP  ESI
//   c3                   RET

extern "C" {

// FUN_009d1b35: allocator / sub-object constructor called with size 0x18.
void FUN_009d1b35();

// FUN_009d1782: initializer called with the pointer returned by FUN_009d1b35.
void FUN_009d1782();

__declspec(naked) void FUN_009d085b() {
    __asm {
        push    esi
        push    0x18
        mov     esi, ecx
        call    FUN_009d1b35
        push    eax
        mov     dword ptr [esi], eax
        call    FUN_009d1782
        pop     ecx
        pop     ecx
        mov     eax, esi
        pop     esi
        ret
    }
}

}  // extern "C"
