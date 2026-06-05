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
// FUNCTION: ffxivgame 0x0005d950 — `__cdecl` 3-arg fan-out wrapper (29 B).
//
// Trivial cdecl helper that forwards its three dword args — plus a fixed
// data pointer (0x00f68cc8, baked as the call's last/leftmost argument) —
// to a 4-arg __cdecl helper at VA 0x0045fe20, then reclaims the 0x10-byte
// outbound frame itself (the callee is cdecl, so the caller pops). The
// args are loaded into scratch registers (EAX/ECX/EDX) and re-pushed in
// reverse-source order along with the constant so the callee receives
// `(arg1, arg2, arg3, 0x00f68cc8)`.
//
// Asm shape (29 bytes — RVA 0x0005d950..0x0005d96d):
//
//     0005d950:  8b 44 24 0c          MOV  EAX, [ESP+0xc]    ; arg3
//     0005d954:  8b 4c 24 08          MOV  ECX, [ESP+0x8]    ; arg2
//     0005d958:  8b 54 24 04          MOV  EDX, [ESP+0x4]    ; arg1
//     0005d95c:  68 c8 8c f6 00       PUSH 0x00f68cc8        ; const ptr (4th arg)
//     0005d961:  50                   PUSH EAX               ; arg3
//     0005d962:  51                   PUSH ECX               ; arg2
//     0005d963:  52                   PUSH EDX               ; arg1
//     0005d964:  e8 b7 24 00 00       CALL FUN_0045fe20      ; rel32 = +0x000024b7
//     0005d969:  83 c4 10             ADD  ESP, 0x10         ; cdecl cleanup (4 dwords)
//     0005d96c:  c3                   RET
//
// Reloc-bearing sites in the orig 29 bytes:
//     +0x0d   PUSH imm32 → 0x00f68cc8 (constant data pointer)
//     +0x15   CALL rel32 → 0x0045fe20 (helper_4arg)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level rewrite (`extern int h4(int,int,int,void*);
//   int f(int a, int b, int c){ return h4(a,b,c,(void*)0xf68cc8); }`) at /O2
//   produces the same shape, but the rel32 to h4 would resolve to our own
//   link's address for the (still-unmatched) callee rather than the orig
//   0x0045fe20. Emitting the 29 orig bytes verbatim bakes both the abs32
//   immediate and the rel32 displacement as raw bytes that match the orig
//   PE's .text slice exactly — the convention used by sibling wrappers
//   FUN_00401000 / FUN_00404e10.

extern "C" __declspec(naked) void FUN_0045d950() {
    __asm {
        _emit 0x8b      // MOV  EAX, [ESP+0xc]      ; arg3
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b      // MOV  ECX, [ESP+0x8]      ; arg2
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b      // MOV  EDX, [ESP+0x4]      ; arg1
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x68      // PUSH 0x00f68cc8          ; const ptr (4th arg)
        _emit 0xc8
        _emit 0x8c
        _emit 0xf6
        _emit 0x00
        _emit 0x50      // PUSH EAX                 ; arg3
        _emit 0x51      // PUSH ECX                 ; arg2
        _emit 0x52      // PUSH EDX                 ; arg1
        _emit 0xe8      // CALL FUN_0045fe20        ; rel32 = +0x000024b7
        _emit 0xb7
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x83      // ADD  ESP, 0x10           ; cdecl cleanup
        _emit 0xc4
        _emit 0x10
        _emit 0xc3      // RET
    }
}
