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
// FUNCTION: ffxivgame 0x004169c0 — conditional buffer-release / reset (48 B / 0x30)
//
// __thiscall void FUN_004169c0(this)
//
// Checks a "disabled / skip" flag bit at [this+0x12] (bit 0). If the bit is
// set the function returns immediately without touching anything. Otherwise
// it unconditionally zeroes [this+0xc] and [this+0x8], then checks whether
// [this+0x4] is non-null; if so it pushes that pointer and calls
// FUN_004162c0 (the engine free routine also used by the Printer siblings at
// 0x00415860 / 0x00415c40), then zeroes [this+0x4].
//
// Structurally a narrower sibling of Printer::Reset (FUN_00415860): same
// __thiscall void / ESI-only save / FUN_004162c0 call pattern but smaller
// object layout (+0x04/+0x08/+0x0c instead of +0x08/+0x0c/+0x7c) and the
// extra early-exit on the flag byte.
//
// Inspection (read from orig bytes at RVA 0x000169c0, 48 bytes total):
//
//   56                          push esi
//   8b f1                       mov  esi, ecx              ; esi = this
//   f6 46 12 01                 test byte ptr [esi+0x12], 0x1 ; flag byte & 1
//   75 25                       jnz  epilogue (+0x25)      ; flag set → skip all
//   8b 46 04                    mov  eax, [esi+0x04]       ; eax = ptr
//   85 c0                       test eax, eax
//   c7 46 0c 00 00 00 00        mov  dword ptr [esi+0x0c], 0
//   c7 46 08 00 00 00 00        mov  dword ptr [esi+0x08], 0
//   74 10                       jz   epilogue (+0x10)      ; ptr==NULL → skip free
//   50                          push eax
//   e8 dc f8 ff ff              call FUN_004162c0          ; rel32 → 0x004162c0
//   83 c4 04                    add  esp, 4                ; __cdecl pop x1
//   c7 46 04 00 00 00 00        mov  dword ptr [esi+0x04], 0
// epilogue:
//   5e                          pop  esi
//   c3                          ret
//
// Calling convention: __thiscall (ECX = this, no stack args, plain RET).
// Stack frame: -4 (PUSH ESI only).
//
// MSVC 2005 scheduling note: the two zero-stores to +0x0c and +0x08 are
// emitted BEFORE the TEST EAX,EAX / JZ check — a standard /O2 latency-hiding
// move that hoists independent stores between the load and its dependent
// branch.
//
// Reloc-bearing sites (4-byte rel32, wildcarded by compare.py):
//   +0x13  CALL rel32 → FUN_004162c0  (engine free)
//
// Reconstruction: naked-asm _emit byte passthrough (same strategy as
// FUN_00415860 / FUN_00415c40 / FUN_00416320 in this module).

extern "C" __declspec(naked) void FUN_004169c0() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xf6              // TEST byte ptr [ESI+0x12], 0x1
        _emit 0x46
        _emit 0x12
        _emit 0x01
        _emit 0x75              // JNZ epilogue (+0x25)
        _emit 0x25
        _emit 0x8b              // MOV EAX, [ESI+0x04]
        _emit 0x46
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0xc7              // MOV dword ptr [ESI+0x0c], 0
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI+0x08], 0
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ epilogue (+0x10)
        _emit 0x10
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_004162c0 (rel32 → 0x004162c0)
        _emit 0xdc
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x04
        _emit 0xc4
        _emit 0x04
        _emit 0xc7              // MOV dword ptr [ESI+0x04], 0
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
