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
// FUNCTION: ffxivgame 0x000472e0 — __thiscall growable-buffer single-char
//                                  "assign" initialiser (69 B / 0x45, ret 4).
//
// __thiscall this* FUN_004472e0(this, char ch):
//   ECX = this; one stack arg (char ch); callee cleans 4 bytes via `ret 4`.
//   Returns `this` (ESI preserved in EAX on exit).
//
// Object layout (same buffer descriptor as FUN_00447010 / 0x447010):
//   [this + 0x00]  void  *data;        // pointer to backing storage
//   [this + 0x04]  unsigned capacity;  // allocated capacity (set to 0x40)
//   [this + 0x08]  unsigned size;      // element count (set to 1 pre-call)
//   [this + 0x0c]  unsigned field0c;   // reserved / zero
//   [this + 0x10]  char    flagA;      // set to 1, then cleared by Resize(2,1)
//   [this + 0x11]  char    flagB;      // ownership flag (set to 1)
//   [this + 0x12]  char    inlineBuf[]; // inline storage; data ptr is aimed here
//
// Behaviour (recovered from asm @ 0x000472e0):
//
//   1. ESI = this (thiscall).
//   2. ECX = 1; compute &inlineBuf (ESI+0x12) into EAX.
//   3. Push 1 (later Resize arg2 = clearFlag).
//   4. Set this->flagA = 1, this->flagB = 1, this->size = 1.
//   5. Set this->field0c = 0, this->capacity = 0x40, this->data = &inlineBuf.
//   6. Push 2 (Resize arg1 = newSize); MOV ECX = this.
//   7. Zero inlineBuf[0].
//   8. Call FUN_00447010(this, newSize=2, clearFlag=1)  — Resize;
//      callee cleans 8 bytes (RET 8), so stack restores to [ESI, retaddr, ch].
//   9. Reload this->data; read ch from [ESP+0x8]; write to data[0].
//  10. Reload this->data into EDX; set EAX = this; zero data[1] (null-term).
//  11. Pop ESI; RET 4.
//
// Reloc-bearing position (masked by tools/compare.py):
//   +0x2c   CALL FUN_00447010  (REL32, IMAGE_REL_I386_REL32)
//
// Reconstruction strategy — `__declspec(naked)` inline asm:
//   All instructions encode unambiguously from their mnemonics in MASM
//   (all displacements fit in 8-bit signed form; no encoding ambiguity).
//   The single REL32 call is emitted as a named extern; compare.py masks
//   the 4-byte payload.

extern "C" void FUN_00447010();  // __thiscall Resize(newSize, clearFlag)

extern "C" __declspec(naked) void FUN_004472e0() {
    __asm {
        push    esi                             ; 56
        mov     esi, ecx                        ; 8b f1   (ESI = this)
        mov     ecx, 1                          ; b9 01 00 00 00
        lea     eax, [esi + 0x12]               ; 8d 46 12  (&inlineBuf)
        push    ecx                             ; 51  (clearFlag=1 for Resize)
        mov     byte ptr [esi + 0x10], cl       ; 88 4e 10  flagA = 1
        mov     byte ptr [esi + 0x11], cl       ; 88 4e 11  flagB = 1
        mov     dword ptr [esi + 0x8], ecx      ; 89 4e 08  size = 1
        mov     dword ptr [esi + 0xc], 0        ; c7 46 0c 00000000  field0c = 0
        mov     dword ptr [esi + 0x4], 0x40     ; c7 46 04 40000000  capacity = 64
        mov     dword ptr [esi], eax            ; 89 06  data = &inlineBuf
        push    0x2                             ; 6a 02  (newSize=2 for Resize)
        mov     ecx, esi                        ; 8b ce  (this for Resize call)
        mov     byte ptr [eax], 0               ; c6 00 00  inlineBuf[0] = 0
        call    FUN_00447010                    ; e8 rel32  Resize(this,2,1)
        mov     eax, dword ptr [esi]            ; 8b 06  EAX = this->data
        mov     cl, byte ptr [esp + 0x8]        ; 8a 4c 24 08  CL = ch arg
        mov     byte ptr [eax], cl              ; 88 08  data[0] = ch
        mov     edx, dword ptr [esi]            ; 8b 16
        mov     eax, esi                        ; 8b c6  return this
        mov     byte ptr [edx + 0x1], 0         ; c6 42 01 00  data[1] = 0
        pop     esi                             ; 5e
        ret     0x4                             ; c2 04 00
    }
}
