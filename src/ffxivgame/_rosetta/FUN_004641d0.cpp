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
// FUNCTION: ffxivgame 0x000641d0 — `sk_find`: __cdecl wrapper that sets ESI =
//                                  arg1 (table self pointer) then forwards
//                                  (arg2=key, aux=2) to `internal_find` (22 B).
//
// Asm (22 bytes @ orig RVA 0x000641d0):
//   8b 44 24 08        MOV EAX, dword ptr [ESP+0x8]   ; EAX = arg2 (key)
//                                                      ;   — loaded before PUSH ESI
//                                                      ;     shifts the frame
//   56                 PUSH ESI                        ; save callee-saved ESI
//   8b 74 24 08        MOV ESI, dword ptr [ESP+0x8]   ; ESI = arg1 (self ptr)
//                                                      ;   — after push, ESP+0x8
//                                                      ;     is now the first arg
//   6a 02              PUSH 0x2                        ; aux param = 2
//   50                 PUSH EAX                        ; key param
//   e8 6f ff ff ff     CALL 0x00464150                 ; internal_find(key, 2)
//                                                      ;   — ESI carries self
//   83 c4 08           ADD  ESP, 0x8                   ; cdecl arg cleanup (2×4)
//   5e                 POP  ESI                        ; restore ESI
//   c3                 RET                             ; return EAX from callee
//
// Calling convention: __cdecl (bare RET — caller cleans up its own frame).
// No local frame: leaf-like; compiler uses ESP-relative addressing throughout.
//
// `internal_find` (0x00464150 / FUN_00464150) uses ESI as an implicit "self"
// register pointer (the table object base — never on the stack for that
// callee). This wrapper's role is to bridge the normal cdecl ABI (where the
// table object is passed as first stack arg) to that ESI-receiver convention,
// while forwarding a fixed auxiliary parameter of 2.
//
// Reconstruction: the byte sequence is structurally simple cdecl inline asm.
// Using a symbolic __declspec(naked) body with mnemonic MASM directives and
// an extern "C" declaration for the call target so that compare.py can mask
// the single REL32 relocation slot at +0xc.

extern "C" void FUN_00464150();

extern "C" __declspec(naked) void FUN_004641d0() {
    __asm {
        mov  eax, dword ptr [esp + 8]
        push esi
        mov  esi, dword ptr [esp + 8]
        push 2
        push eax
        call FUN_00464150
        add  esp, 8
        pop  esi
        ret
    }
}
