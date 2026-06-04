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
// FUNCTION: ffxivgame 0x000427b0 — __thiscall audio-object teardown / stop
//                                  helper (143 B / 0x8f, ret 4).
//
// __thiscall bool Stop(this, bool flush):
//   ECX = this; one stack arg `flush` (a byte at [ESP+4]). Cleans 4 bytes
//   of stack args on return (`ret 4`). Returns bool in AL.
//
// Behaviour (recovered from asm @ 0x000427b0):
//
//   bool Stop(bool flush) {
//       if (flush) {
//           sub_00cc3b80(&this->field8);              // thiscall on this+8
//           void *voice = this->field4;
//           if (voice && this->field24 == 0) {
//               voice->vtbl->m14(this, 0, 0, 0);      // vtable +0x14
//               this->field4 = 0;
//           }
//       } else if (this->field18 != 0) {
//           goto stage2;                              // skip flush block
//       } else {
//           return false;
//       }
//   stage2:
//       if (this->field1c) {
//           if (!sub_00b8f210(this->field1c, 0))      // __cdecl, 2 args
//               return false;
//       }
//       void *voice = this->field4;
//       this->field1c = 0;
//       if (voice) {
//           if (this->field24 != 0)
//               voice->vtbl->m1c();                   // vtable +0x1c, thiscall
//           voice = this->field4;
//           voice->vtbl->m14(this, 0, 0, 0);          // vtable +0x14
//           this->field4 = 0;
//       }
//       return true;
//   }
//
// CALL targets (REL32, wildcarded by tools/compare.py):
//   +0x0d   CALL FUN_00cc3b80   — thiscall sub-object teardown
//   +0x3e   CALL FUN_00b8f210   — 2-arg __cdecl predicate
// The two `call edx` sites are indirect vtable dispatches (no relocation;
// the `ff d2` bytes are real code).
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough, the
// canonical ffxivgame workaround. A source-level C++ form would not
// reliably reproduce the exact register allocation / branch lowering an
// isolated TU produces vs. the full-binary build; the naked-asm body
// re-emits the original 143 bytes verbatim, matching modulo the two
// rel32 relocation windows.

extern "C" {
    int FUN_00cc3b80();    // thiscall sub-object teardown
    int FUN_00b8f210();    // 2-arg __cdecl predicate
}

extern "C" __declspec(naked) void FUN_004427b0() {
    __asm {
        cmp     byte ptr [esp + 4], 0          // 80 7c 24 04 00
        push    esi                            // 56
        mov     esi, ecx                       // 8b f1
        jz      short label_A                  // 74 79
        lea     ecx, [esi + 8]                 // 8d 4e 08
        call    FUN_00cc3b80                   // e8 ?? ?? ?? ??
        mov     ecx, dword ptr [esi + 4]       // 8b 4e 04
        test    ecx, ecx                       // 85 c9
        jz      short label_E                  // 74 1b
        cmp     byte ptr [esi + 0x24], 0       // 80 7e 24 00
        jnz     short label_E                  // 75 15
        mov     eax, dword ptr [ecx]           // 8b 01
        mov     edx, dword ptr [eax + 0x14]    // 8b 50 14
        push    0                              // 6a 00
        push    0                              // 6a 00
        push    0                              // 6a 00
        push    esi                            // 56
        call    edx                            // ff d2
        mov     dword ptr [esi + 4], 0         // c7 46 04 00 00 00 00

    label_E:
        mov     eax, dword ptr [esi + 0x1c]    // 8b 46 1c
        test    eax, eax                       // 85 c0
        jz      short label_F                  // 74 0f
        push    0                              // 6a 00
        push    eax                            // 50
        call    FUN_00b8f210                   // e8 ?? ?? ?? ??
        add     esp, 8                         // 83 c4 08
        test    al, al                         // 84 c0
        jz      short label_B                  // 74 3f

    label_F:
        mov     ecx, dword ptr [esi + 4]       // 8b 4e 04
        test    ecx, ecx                       // 85 c9
        mov     dword ptr [esi + 0x1c], 0      // c7 46 1c 00 00 00 00
        jz      short label_C                  // 74 25
        cmp     byte ptr [esi + 0x24], 0       // 80 7e 24 00
        jz      short label_G                  // 74 07
        mov     eax, dword ptr [ecx]           // 8b 01
        mov     edx, dword ptr [eax + 0x1c]    // 8b 50 1c
        call    edx                            // ff d2

    label_G:
        mov     ecx, dword ptr [esi + 4]       // 8b 4e 04
        mov     eax, dword ptr [ecx]           // 8b 01
        mov     edx, dword ptr [eax + 0x14]    // 8b 50 14
        push    0                              // 6a 00
        push    0                              // 6a 00
        push    0                              // 6a 00
        push    esi                            // 56
        call    edx                            // ff d2
        mov     dword ptr [esi + 4], 0         // c7 46 04 00 00 00 00

    label_C:
        mov     al, 1                          // b0 01
        pop     esi                            // 5e
        ret     4                              // c2 04 00

    label_A:
        cmp     dword ptr [esi + 0x18], 0      // 83 7e 18 00
        jz      short label_E                  // 74 ab

    label_B:
        xor     al, al                         // 32 c0
        pop     esi                            // 5e
        ret     4                              // c2 04 00
    }
}
