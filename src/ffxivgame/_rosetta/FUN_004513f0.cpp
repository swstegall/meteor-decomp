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
// FUNCTION: ffxivgame 0x004513f0 — __cdecl object factory: allocate 0x48-byte
//                                  object via operator new, conditionally zero
//                                  the first three DWORDs (each with its own
//                                  null guard), set byte flags at +0x44 / +0x45,
//                                  and return the pointer (55 bytes).
//
// Calling convention: __cdecl (no arguments; returns pointer in EAX).
// Frame: none (no prologue/epilogue push/pop; no local variables).
//
// Layout of the allocated object (inferred):
//   +0x00  int / ptr  field0
//   +0x04  int / ptr  field1
//   +0x08  int / ptr  field2
//   ...0x38 bytes of other fields...
//   +0x44  bool       flag_active  (initialised to 1)
//   +0x45  bool       flag_done    (initialised to 0)
//   total size: 0x48 bytes
//
// Source shape (inferred — the triple null-guard pattern for offset-derived
// addresses is a MSVC 2005 defensive idiom emitted for pointer-offset
// initialisers when the compiler cannot prove the allocation is non-null
// at each derived site):
//
//   SomeObject *FUN_004513f0() {
//       SomeObject *p = (SomeObject *)::operator new(0x48);
//       if (p)
//           p->field0 = 0;
//       int *p1 = &p->field1;   // = (int *)p + 1
//       if (p1)
//           *p1 = 0;
//       int *p2 = &p->field2;   // = (int *)p + 2
//       if (p2)
//           *p2 = 0;
//       p->flag_active = true;
//       p->flag_done   = false;
//       return p;
//   }
//
// Reloc-bearing site in the orig 55 bytes (compare.py masks):
//   +0x03   REL32 → 0x009d1b35  (operator new; same thunk as FUN_00403bd0
//                                  / FUN_0044a900 / FUN_00420a70)

extern "C" void FUN_009d1b35(void);   // operator new @ 0x009d1b35

extern "C" __declspec(naked) void FUN_004513f0() {
    __asm {
        push    048h
        call    FUN_009d1b35
        add     esp, 4
        test    eax, eax
        jz      skip1
        mov     dword ptr [eax], 0
    skip1:
        lea     ecx, [eax + 4]
        test    ecx, ecx
        jz      skip2
        mov     dword ptr [ecx], 0
    skip2:
        lea     ecx, [eax + 8]
        test    ecx, ecx
        jz      skip3
        mov     dword ptr [ecx], 0
    skip3:
        mov     byte ptr [eax + 044h], 1
        mov     byte ptr [eax + 045h], 0
        ret
    }
}
