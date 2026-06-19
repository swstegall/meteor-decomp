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
// FUNCTION: ffxivgame 0x00044e40 — `__thiscall` range-validated splice-all
//                                  wrapper (57 B / 0x39, plain RET).
//
// Layout (inferred from context — same container class as FUN_00444dc0):
//   this (ECX → ESI):
//     +0x04  void*  _Myfirst  (begin pointer)
//     +0x08  void*  _Mylast   (end pointer)
//
// Source shape:
//
//   void Container::eraseAll() {
//       // Check 1: range validity (unsigned: first must not exceed last)
//       if (this->_Myfirst > this->_Mylast)
//           _invalid_parameter_noinfo();
//       // Load begin into EDI; Check 2: same condition via local
//       void* first = this->_Myfirst;
//       if (first > this->_Mylast)
//           _invalid_parameter_noinfo();
//       // Erase [first, last) of self, discarding the returned iterator pair
//       IterPair _Tmp;
//       this->splice(&_Tmp, this, first, this, this->_Mylast);
//   }
//
// Calling convention: __thiscall (ECX = this).  No stack args; plain RET.
//
// Frame:
//   SUB ESP, 8          ; 8-byte local _Tmp (IterPair out-param slot)
//   PUSH EBX            ; callee-save
//   PUSH ESI            ; callee-save (= this)
//   (code begins)
//   PUSH EDI            ; callee-save — hoisted between CMP and JBE by the
//                       ; MSVC 2005 scheduler (PUSH doesn't touch EFLAGS)
//
// Stack when FUN_00444dc0 is called (after 5 pushes + CALL):
//   [ESP+0x0c]  EAX = &_Tmp         (hidden return-struct pointer)
//   [ESP+0x10]  ESI = this          (arg a)
//   [ESP+0x14]  EDI = _Myfirst      (arg b)
//   [ESP+0x18]  ESI = this          (arg c)
//   [ESP+0x1c]  EBX = _Mylast       (arg d)
//   ECX         ESI = this          (__thiscall implicit)
// FUN_00444dc0 cleans 5 dwords with RET 0x14.
//
// LEA EAX, [ESP+0x1c]: after the 4 pushes ESP is 16 bytes below the
// callee-save frame; the 8-byte _Tmp starts at ESP_on_entry−8, so
// ESP_current + 0x1c = (ESP_entry − 4*4) + 0x1c = ESP_entry − 8 ✓.
//
// Reloc-bearing sites (all REL32, masked by tools/compare.py):
//   +0x10  CALL rel32 → _invalid_parameter_noinfo (0x009d22b4)
//   +0x1d  CALL rel32 → _invalid_parameter_noinfo (0x009d22b4)
//   +0x2d  CALL rel32 → FUN_00444dc0              (0x00444dc0)
//
// Naked __asm pins the deferred PUSH EDI scheduling and the exact
// JBE encoding (short `76 05`) around the two validation calls.

extern "C" void _invalid_parameter_noinfo(void);
extern "C" void FUN_00444dc0(void);

extern "C" __declspec(naked) void FUN_00444e40() {
    __asm {
        sub     esp, 8
        push    ebx
        push    esi
        mov     esi, ecx
        mov     ebx, dword ptr [esi + 0x8]
        cmp     dword ptr [esi + 0x4], ebx
        push    edi
        jbe     check1_ok
        call    _invalid_parameter_noinfo
    check1_ok:
        mov     edi, dword ptr [esi + 0x4]
        cmp     edi, dword ptr [esi + 0x8]
        jbe     check2_ok
        call    _invalid_parameter_noinfo
    check2_ok:
        push    ebx
        push    esi
        push    edi
        push    esi
        lea     eax, [esp + 0x1c]
        push    eax
        mov     ecx, esi
        call    FUN_00444dc0
        pop     edi
        pop     esi
        pop     ebx
        add     esp, 8
        ret
    }
}
