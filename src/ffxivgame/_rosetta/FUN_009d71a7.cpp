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
// FUNCTION: ffxivgame 0x009d71a7 — CRT flsall helper ($LN24): flush one FILE
//                                   slot by index when the loop reaches a
//                                   stream that needs special handling
//                                   (17 B / 0x11).
//
// Context (read from the containing flsall body at orig RVA 0x005d7105):
//
//   flsall() loops over all open CRT file slots (ESI = 0..max_slots) and
//   flushes each one.  After processing a slot, the loop body issues:
//
//     call 0x009d71a7           ; <-- our function (this label is $LN24)
//     inc  esi                  ; advance to next slot
//     jmp  <loop-top>
//
//   Our function is the continuation helper called with ESI still live as
//   the current slot index.  It re-reads the global FILE* array base
//   (__piob, at .data 0x0137b900), pushes __piob[esi] and esi, then
//   calls FUN_009d4ede with those two arguments (stream ptr + index).
//   The caller pops both args via two POP ECX instructions (cdecl cleanup).
//
//   Pseudo-C (ESI = current slot index, passed in a non-standard register):
//
//     void helper() {
//         FILE** base = *(__piob_ptr);          // MOV EAX, [0x0137b900]
//         FUN_009d4ede(base[esi], (int)esi);    // PUSH + PUSH + CALL + 2×POP
//     }
//
//   Calling convention: non-standard (no prologue, no ESI save/restore —
//   ESI is a live register supplied by the enclosing flsall loop body).
//   The only safe encoding strategy is a naked-asm byte passthrough:
//   any source-level spelling that goes through /O2 would either materialise
//   a frame or choose the MOV EAX,r/m32 (8b 05) encoding over the compact
//   MOV EAX,moffset (a1) form, shifting the total to 18 bytes and causing a
//   MISMATCH against the 17-byte orig.
//
// Asm (17 bytes @ orig RVA 0x005d71a7 / abs 0x009d71a7):
//
//   a1 00 b9 37 01   MOV  EAX,  DWORD PTR DS:[0x0137b900]   ; base = *__piob_ptr
//   ff 34 b0         PUSH DWORD PTR DS:[EAX + ESI*4]        ; push base[esi]
//   56               PUSH ESI                                ; push esi (slot index)
//   e8 29 dd ff ff   CALL 0x009d4ede  (FUN_009d4ede)        ; cdecl, 2 args
//   59               POP  ECX                                ; clean arg1
//   59               POP  ECX                                ; clean arg2
//   c3               RET

extern "C" __declspec(naked) void FUN_009d71a7() {
    __asm {
        _emit 0xa1   // MOV EAX, DWORD PTR [0x0137b900]
        _emit 0x00
        _emit 0xb9
        _emit 0x37
        _emit 0x01

        _emit 0xff   // PUSH DWORD PTR [EAX + ESI*4]
        _emit 0x34
        _emit 0xb0

        _emit 0x56   // PUSH ESI

        _emit 0xe8   // CALL FUN_009d4ede (rel32 = 0xFFFFDD29)
        _emit 0x29
        _emit 0xdd
        _emit 0xff
        _emit 0xff

        _emit 0x59   // POP ECX
        _emit 0x59   // POP ECX
        _emit 0xc3   // RET
    }
}
