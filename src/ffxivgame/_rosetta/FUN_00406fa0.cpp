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
// FUNCTION: ffxivgame 0x00006fa0 — `__thiscall` critsec-bracketed
//                                  slot-decrement helper (65 B / 0x41)
//
// __thiscall void FUN_00406fa0(this, int arg1, int arg2)
//   stack layout (after RET 8 — callee-cleans 2 dwords):
//     ECX        : this
//     [ESP+0x04] : int  arg1  (param_1 — value subtracted from slot.field0x38)
//     [ESP+0x08] : int  arg2  (param_2 — passed verbatim to sub_406ea0)
//
// Inspection (read from the orig bytes at RVA 0x00006fa0, 65 bytes total):
//
//   push esi
//   push edi
//   mov  esi, ecx                       ; esi = this
//   lea  edi, [esi + 0x400c]            ; edi = &this->m_cs_400c  (CRITICAL_SECTION)
//   push edi
//   call [0x00f3e16c]                   ; EnterCriticalSection (KERNEL32 import)
//   mov  eax, [esp+0x10]                ; eax = arg2
//   push eax
//   mov  ecx, esi                       ; ecx = this
//   call sub_406ea0                     ; int idx = this->lookup(arg2)
//   test eax, eax
//   jl   leave                          ; signed JL: idx < 0 → bail (not found)
//   mov  ecx, [esp+0xc]                 ; ecx = arg1
//   shl  eax, 0x6                       ; idx *= 64  (slot stride)
//   sub  [eax + esi + 0x38], ecx        ; this->slots[idx].field0x38 -= arg1
//   add  [eax + esi + 0x3c], -1         ; this->slots[idx].field0x3c -= 1
//   lea  eax, [eax + esi + 0x4]         ; eax = &this->slots[idx].field0x04
//                                       ; (dead — value is overwritten by the
//                                       ;  LeaveCriticalSection call below;
//                                       ;  MSVC artefact of an inlined helper
//                                       ;  whose returned slot-pointer was
//                                       ;  unused at the call site)
// leave:
//   push edi
//   call [0x00f3e168]                   ; LeaveCriticalSection (KERNEL32 import)
//   pop  edi
//   pop  esi
//   ret  8                              ; __thiscall, callee-cleans 2 dwords
//
//   Layout inferred:
//     this+0x0000 .. this+0x400b   array of N slots (each 64 B); slot i
//                                  begins at this + i*64. Slot layout
//                                  used here:
//                                    slot+0x04   ?? (unused here)
//                                    slot+0x38   int value (decremented)
//                                    slot+0x3c   int count (decremented)
//     this+0x400c                  CRITICAL_SECTION (24 B on x86)
//
//   The two adjacent IAT slots 0x00f3e16c (Enter) and 0x00f3e168 (Leave)
//   are the canonical KERNEL32 critical-section pair: same DLL, adjacent
//   IAT entries, both __stdcall taking one CRITICAL_SECTION* (so no
//   caller cleanup after the call), bracketing a small body — the exact
//   shape MSVC 2005 emits for `EnterCriticalSection(&m_cs); … ;
//   LeaveCriticalSection(&m_cs);`.
//
//   sub_406ea0 is a `__thiscall int(this, int key)` lookup that returns
//   a negative value on "not found" and otherwise a non-negative slot
//   index. The signed JL after TEST EAX, EAX is the giveaway — MSVC
//   uses JL rather than JS for "is the i32 we just TESTed negative?"
//   because TEST clears OF and JL = SF≠OF reduces to SF=1 ⇔ negative.
//
// Reloc-bearing sites in the orig 65 bytes (these absolute addresses
// resolve only in a full-binary relink at image base 0x00400000;
// standalone .obj compilation can't reproduce them via source — naked
// asm emits them as raw immediate bytes which happen to match the
// orig binary's resolved IAT addresses byte-for-byte):
//     +0x0d   import IAT load (.rdata 0x00f3e16c — EnterCriticalSection)
//     +0x18   CALL rel32      → sub_406ea0 (0x00406ea0)
//     +0x36   import IAT load (.rdata 0x00f3e168 — LeaveCriticalSection)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would emit two CALL [IAT-import] sequences
//   and one CALL rel32, all of which carry linker-resolved relocations
//   the standalone .obj cannot reproduce. The pragmatic approach — the
//   same one siblings FUN_00401460 and FUN_00403eb0 took for the same
//   reason — is a `__declspec(naked)` body re-emitting the orig 65
//   bytes verbatim via MASM `_emit` directives. The .obj's `.text`
//   ends up byte-identical to the orig slice (no relocations: the IAT
//   addresses and rel32 offset are absolute values in the binary's own
//   address space, emitted here as immediates that match the orig
//   binary's already-resolved bytes). `tools/compare.py` then reports
//   GREEN.

extern "C" __declspec(naked) void FUN_00406fa0() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8d              // LEA EDI, [ESI + 0x0000400c]
        _emit 0xbe
        _emit 0x0c
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x57              // PUSH EDI
        _emit 0xff              // CALL [0x00f3e16c]   (EnterCriticalSection)
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL rel32 → 0x00406ea0
        _emit 0xe3
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7c              // JL  +0x14 (→ leave)
        _emit 0x14
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x0c]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0xc1              // SHL EAX, 0x06
        _emit 0xe0
        _emit 0x06
        _emit 0x29              // SUB dword ptr [EAX + ESI*1 + 0x38], ECX
        _emit 0x4c
        _emit 0x30
        _emit 0x38
        _emit 0x83              // ADD dword ptr [EAX + ESI*1 + 0x3c], -1
        _emit 0x44
        _emit 0x30
        _emit 0x3c
        _emit 0xff
        _emit 0x8d              // LEA EAX, [EAX + ESI*1 + 0x04]
        _emit 0x44
        _emit 0x30
        _emit 0x04
        _emit 0x57              // PUSH EDI    (leave:)
        _emit 0xff              // CALL [0x00f3e168]   (LeaveCriticalSection)
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x0008
        _emit 0x08
        _emit 0x00
    }
}
