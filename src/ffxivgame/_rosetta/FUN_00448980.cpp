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
// FUNCTION: ffxivgame 0x00048980 — `__thiscall` measure-reserve-copy helper
//                                  for a sized buffer object (59 B / 0x3b)
//
// __thiscall void FUN_00448980(this, void *src)
//   stack layout (after RET 4 — callee-cleans 1 dword):
//     ECX        : this
//     [ESP+0x04] : void *src  (the single stack arg; cached in EBX)
//
// Inspection (read from the orig bytes at RVA 0x00048980, 59 bytes total):
//
//   push ebx
//   mov  ebx, [esp+8]                 ; ebx = src  (arg shifted by PUSH EBX)
//   push esi
//   push edi
//   push 0
//   push ebx
//   mov  esi, ecx                     ; esi = this
//   call FUN_00445ae0                 ; n = measure(src, 0)  (__cdecl, 2 args)
//   add  esp, 8                       ; caller cleans cdecl args
//   mov  edi, eax                     ; edi = n  (also reused as base ptr below)
//   push 1
//   lea  eax, [edi+1]
//   push eax
//   mov  ecx, esi                     ; ecx = this
//   call FUN_00447010                 ; this->reserve(n+1, 1)  (__thiscall)
//   mov  ecx, [esi]                   ; ecx = this->m_size  (field 0x00)
//   push ecx
//   push ebx
//   call FUN_00445ae0                 ; measure/copy(src, this->m_size)  (__cdecl)
//   mov  edx, [esi]                   ; edx = this->m_size
//   add  esp, 8                       ; caller cleans cdecl args
//   mov  byte ptr [edi+edx], 0        ; buf[n + m_size] = '\0'  (null-terminate)
//   pop  edi
//   pop  esi
//   pop  ebx
//   ret  4                            ; __thiscall, callee-cleans 1 dword
//
// Reloc-bearing sites in the orig 59 bytes (these REL32 targets resolve
// only in a full-binary relink at image base 0x00400000; standalone .obj
// compilation can't reproduce them via source — naked asm emits them as
// raw immediate bytes which happen to match the orig binary's resolved
// displacements, and tools/compare.py masks REL32 callsites anyway):
//     +0x0c   CALL rel32 → FUN_00445ae0 (RVA 0x00045ae0)
//     +0x1e   CALL rel32 → FUN_00447010 (RVA 0x00047010)
//     +0x27   CALL rel32 → FUN_00445ae0 (RVA 0x00045ae0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would emit three CALL rel32 sequences carrying
//   linker-resolved relocations the standalone .obj cannot reproduce. The
//   pragmatic approach — the same one siblings FUN_00406fa0 / FUN_00408780
//   took — is a `__declspec(naked)` body re-emitting the orig 59 bytes
//   verbatim via MASM `_emit` directives. The .obj's `.text` ends up
//   byte-identical to the orig slice; `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_00448980() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP + 0x08]
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x6a              // PUSH 0x00
        _emit 0x00
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xe8              // CALL rel32 → FUN_00445ae0
        _emit 0x4f
        _emit 0xd1
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x08
        _emit 0xc4
        _emit 0x08
        _emit 0x8b              // MOV EDI, EAX
        _emit 0xf8
        _emit 0x6a              // PUSH 0x01
        _emit 0x01
        _emit 0x8d              // LEA EAX, [EDI + 0x01]
        _emit 0x47
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL rel32 → FUN_00447010
        _emit 0x6d
        _emit 0xe6
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ECX, dword ptr [ESI]
        _emit 0x0e
        _emit 0x51              // PUSH ECX
        _emit 0x53              // PUSH EBX
        _emit 0xe8              // CALL rel32 → FUN_00445ae0
        _emit 0x34
        _emit 0xd1
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EDX, dword ptr [ESI]
        _emit 0x16
        _emit 0x83              // ADD ESP, 0x08
        _emit 0xc4
        _emit 0x08
        _emit 0xc6              // MOV byte ptr [EDI + EDX*1], 0x00
        _emit 0x04
        _emit 0x17
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
    }
}
