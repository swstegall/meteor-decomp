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
// FUNCTION: ffxivgame 0x0002f550 — __cdecl 3-arg forwarder to the
//           __thiscall matrix×vector transform at 0x0042f210 (24 B).
//
// Signature (reconstructed from the asm):
//
//   __cdecl void* FUN_0042f550(void* out, Matrix* mat, void* in) {
//       mat->transform(out, in);   // __thiscall, this=mat in ECX
//       return out;
//   }
//
//   FUN_0042f210 is __thiscall (RET 0x8): ECX = a 4x4 matrix (0x40
//   bytes), [EBP+8] = output vector, [EBP+0xc] = input vector — it
//   applies the SSE matrix transform and writes the result through the
//   output pointer. This wrapper merely shuffles the three __cdecl stack
//   args into that calling convention (mat -> ECX, out/in -> stack) and
//   returns the output pointer (out, held in ESI) in EAX.
//
// Asm (24 bytes @ 0x0002f550):
//   8b 44 24 0c      MOV EAX, [ESP+0xc]   ; in
//   8b 4c 24 08      MOV ECX, [ESP+0x8]   ; mat -> this
//   56               PUSH ESI
//   8b 74 24 08      MOV ESI, [ESP+0x8]   ; out (after push: orig [ESP+4])
//   50               PUSH EAX             ; in
//   56               PUSH ESI             ; out
//   e8 ?? ?? ?? ??   CALL FUN_0042f210    ; rel32 reloc, masked by compare.py
//   8b c6            MOV EAX, ESI         ; return out
//   5e               POP ESI
//   c3               RET                  ; __cdecl: caller cleans args
//
// Written as a __declspec(naked) body so the 24 bytes come out verbatim;
// the lone CALL rel32 reloc window is masked by tools/compare.py.

extern "C" void FUN_0042f210();

extern "C" __declspec(naked) void FUN_0042f550() {
    __asm {
        mov     eax, [esp + 0xc]
        mov     ecx, [esp + 8]
        push    esi
        mov     esi, [esp + 8]
        push    eax
        push    esi
        call    FUN_0042f210
        mov     eax, esi
        pop     esi
        ret
    }
}
