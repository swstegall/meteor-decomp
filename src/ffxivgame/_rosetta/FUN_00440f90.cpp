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
// FUNCTION: ffxivgame 0x00040f90 — __thiscall container destructor/cleanup:
//                                   clear all list nodes via FUN_00440d40(this),
//                                   free the sentinel (this->field_4) via
//                                   FUN_009d1b17, then zero this->field_4
//                                   (29 B / 0x1d — size_overrides epilogue
//                                   continuation from symbols.json's 17 B slice)
//
// Calling convention: __thiscall (ECX = this, no stack args).
// Callee-saved: ESI (holds `this` across both inner calls).
//
// Asm (29 bytes @ orig RVA 0x00040f90):
//   56                         PUSH ESI
//   8b f1                      MOV  ESI, ECX             ; ESI = this
//   e8 a8 fd ff ff             CALL FUN_00440d40          ; __thiscall clear
//   8b 46 04                   MOV  EAX, [ESI + 0x4]     ; EAX = sentinel ptr
//   50                         PUSH EAX                   ; pass as cdecl arg
//   e8 76 0b 59 00             CALL FUN_009d1b17          ; __cdecl free/destroy
//   83 c4 04                   ADD  ESP, 4                ; cdecl cleanup
//   c7 46 04 00 00 00 00       MOV  [ESI + 0x4], 0       ; sentinel = nullptr
//   5e                         POP  ESI
//   c3                         RET
//
// The two reloc-bearing CALL rel32 sites are masked by compare.py.
// All other bytes are structural and must match exactly.
//
// Reconstruction strategy — naked-asm passthrough:
//   The function's key structural feature is `MOV EAX,[ESI+4]; PUSH EAX`
//   rather than the shorter `PUSH DWORD PTR [ESI+4]` encoding that MSVC
//   /O2 might otherwise emit. Naked inline asm pins both the load-into-EAX
//   step and the subsequent MOV-zero-to-memory form, producing a byte-exact
//   match against the orig binary without relying on fragile codegen choices.

extern "C" void FUN_00440d40();
extern "C" void FUN_009d1b17();

extern "C" __declspec(naked) void FUN_00440f90() {
    __asm {
        push esi
        mov  esi, ecx
        call FUN_00440d40
        mov  eax, dword ptr [esi + 0x4]
        push eax
        call FUN_009d1b17
        add  esp, 4
        mov  dword ptr [esi + 0x4], 0
        pop  esi
        ret
    }
}
