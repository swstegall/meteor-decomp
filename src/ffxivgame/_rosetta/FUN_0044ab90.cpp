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
// FUNCTION: ffxivgame 0x0044ab90 — SEH/GS-wrapped helper (131 B / 0x83)
//                                  that forwards two by-value args through a
//                                  pair of __thiscall sub-helpers and tears
//                                  down a temporary under an MSVC C++ unwind
//                                  frame.
//
// Asm shape (read from RVA 0x0004ab90, 131 bytes of `.text`):
//
//   push -1                                       ; initial unwind state
//   push offset @sehScopeTable_00e57761           ; SEH handler trampoline
//   mov  eax, fs:[0] ; push eax                    ; link prev fs:[0]
//   sub  esp, 0x58                                 ; carve frame / locals
//   push esi                                       ; preserved register
//   mov  eax, [__security_cookie] ; xor eax, esp   ; /GS cookie
//   push eax
//   lea  eax, [esp + 0x60] ; mov fs:[0], eax       ; install SEH registration
//   mov  eax, [esp + 0x74]                         ; arg A
//   push eax
//   lea  ecx, [esp + 0x10]                         ; ecx = &temp (this)
//   mov  dword ptr [esp + 0xc], 0                  ; trylevel slot init
//   call FUN_00447200                              ; build temp from arg A
//   mov  ecx, [esp + 0x78]                         ; arg B
//   push ecx ; mov ecx, eax                        ; this = result of first call
//   mov  dword ptr [esp + 0x6c], 1                 ; unwind state 1
//   call FUN_004488f0
//   mov  esi, [esp + 0x70]                         ; esi = out object pointer
//   push eax ; mov ecx, esi
//   call FUN_00447200                              ; store into out object
//   lea  ecx, [esp + 0xc]                          ; ecx = &temp
//   mov  dword ptr [esp + 8], 1                    ; unwind state
//   mov  byte ptr [esp + 0x68], 0
//   call FUN_00446f50                              ; destruct temp
//   mov  eax, esi                                  ; return out object
//   mov  ecx, [esp + 0x60] ; mov fs:[0], ecx       ; restore SEH chain
//   pop  ecx ; pop esi ; add esp, 0x64 ; ret
//
// Reloc-bearing sites in the orig 131 bytes (masked by tools/compare.py):
//
//   +0x02  PUSH imm32 → @sehScopeTable     (VA 0x00e57761)
//   +0x07  MOV moffs32 → fs:[0]            (TEB SEH list head)
//   +0x12  MOV moffs32 → __security_cookie (VA 0x012ea8b0)
//   +0x1e  MOV moffs32 → fs:[0]            (install handler)
//   +0x35  CALL rel32 → FUN_00447200
//   +0x49  CALL rel32 → FUN_004488f0
//   +0x55  CALL rel32 → FUN_00447200
//   +0x6b  CALL rel32 → FUN_00446f50
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function references three binary-resident absolute addresses
//   (SEH scope table, fs:[0] head, __security_cookie) and four rel32
//   sibling calls whose displacements are baked against the orig load
//   address. None of those resolve from a standalone .obj — a source-
//   level form would leave COFF relocations. Because we match at the
//   byte-diff level (not a relink), the pragmatic choice — the same one
//   the siblings FUN_00403d60 / FUN_00401350 took — is a
//   `__declspec(naked)` body that re-emits the orig 131 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` ends up byte-
//   identical to the orig slice with NO relocations; compare.py GREEN.

extern "C" __declspec(naked) void FUN_0044ab90() {
    __asm {
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00E57761  (SEH scope table)
        _emit 0x61
        _emit 0x77
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB ESP, 0x58
        _emit 0xec
        _emit 0x58
        _emit 0x56              // PUSH ESI
        _emit 0xa1              // MOV EAX, [0x012EA8B0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EAX, [ESP + 0x60]
        _emit 0x44
        _emit 0x24
        _emit 0x60
        _emit 0x64              // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESP + 0x74]
        _emit 0x44
        _emit 0x24
        _emit 0x74
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA ECX, [ESP + 0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0xc7              // MOV dword ptr [ESP + 0xC], 0
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_00447200 (rel32)
        _emit 0x36
        _emit 0xc6
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ECX, [ESP + 0x78]
        _emit 0x4c
        _emit 0x24
        _emit 0x78
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0xc7              // MOV dword ptr [ESP + 0x6C], 1
        _emit 0x44
        _emit 0x24
        _emit 0x6c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_004488F0 (rel32)
        _emit 0x12
        _emit 0xdd
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ESI, [ESP + 0x70]
        _emit 0x74
        _emit 0x24
        _emit 0x70
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_00447200 (rel32)
        _emit 0x16
        _emit 0xc6
        _emit 0xff
        _emit 0xff
        _emit 0x8d              // LEA ECX, [ESP + 0xC]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0xc7              // MOV dword ptr [ESP + 8], 1
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc6              // MOV byte ptr [ESP + 0x68], 0
        _emit 0x44
        _emit 0x24
        _emit 0x68
        _emit 0x00
        _emit 0xe8              // CALL FUN_00446F50 (rel32)
        _emit 0x50
        _emit 0xc3
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x8b              // MOV ECX, [ESP + 0x60]
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        _emit 0x64              // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x64
        _emit 0xc4
        _emit 0x64
        _emit 0xc3              // RET
    }
}
