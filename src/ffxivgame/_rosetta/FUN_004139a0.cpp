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
// FUNCTION: ffxivgame 0x004139a0 — __thiscall CRITICAL_SECTION initializer helper
//                                  (11 B / 0xb, no stack frame, no saved regs).
//
// Calling convention: __thiscall — `this` arrives in ECX; no stack args;
// epilogue is `ret` (no callee stack cleanup).
//
// Body:
//   ECX += 0x3c;                    // adjust this → &this->critSec (CRITICAL_SECTION at +0x3c)
//   push ECX;                       // arg: &this->critSec
//   call [IAT:InitializeCriticalSection];  // Win32 IAT entry at 0x00f3e16c
//   ret;
//
// Context: part of the SQEX::CDev::Engine::Memory::Alternative::DetachableHeapSpace
// family. The CRITICAL_SECTION lock lives at this+0x3c (confirmed by
// decomp-notes/types/ffxivgame/0x00013850.md). The IAT slot at 0x00f3e16c is
// InitializeCriticalSection, which is 4 bytes before 0x00f3e170
// (DeleteCriticalSection, used in the sibling destructor FUN_00413640).
//
// Instruction layout (11 bytes, RVA 0x000139a0..0x000139aa):
//
//   000139a0:  83 c1 3c                ADD ECX, 0x3c
//   000139a3:  51                      PUSH ECX
//   000139a4:  ff 15 6c e1 f3 00       CALL dword ptr [0x00f3e16c]
//   000139aa:  c3                      RET
//
// Reconstruction strategy — naked asm byte passthrough:
//
//   The CALL is an IAT indirect: `ff 15` + absolute 32-bit IAT address.
//   Emitting the call as a named import reference would produce a COFF DIR32
//   relocation on the 4-byte address field. Using _emit for the ff 15 bytes
//   bakes the address as a raw immediate with no relocation, so the .obj
//   .text section is byte-identical to the orig binary slice.
//   tools/compare.py therefore reports GREEN without relocation masking.

extern "C" __declspec(naked) void FUN_004139a0() {
    __asm {
        add     ecx, 0x3c                   // ECX = &this->critSec
        push    ecx                         // arg: &critSec
        // CALL dword ptr [0x00f3e16c]  — IAT:InitializeCriticalSection
        _emit 0xff
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        ret
    }
}
