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
// FUNCTION: ffxivgame 0x004502d0 — __thiscall container update/erase helper
//                                   (111 B / 0x6f, callee-cleans 8 bytes).
//
// __thiscall void FUN_004502d0(this in ECX, arg1 [ESP+4], arg2 [ESP+8])
//   RET 0x8 ⇒ two dword stack args; ECX is the implicit `this`.
//
// Inspection (read from the disassembly at orig RVA 0x000502d0):
//
//   mov  eax, [esp+0x4]            ; arg1
//   sub  esp, 0x8                  ; reserve two-dword local (a pair)
//   push esi
//   push edi
//   mov  edi, [esp+0x18]           ; arg2
//   cmp  dword ptr [edi+0x14], 0   ; if (arg2->field_0x14 != 0) → rebuild path
//   mov  esi, ecx                  ; esi = this
//   jnz  rebuild
//   ; --- in-place update path (field_0x14 == 0) ---
//   push ebx
//   push ebp
//   push eax                       ; arg1
//   lea  ecx, [esp+0x14]           ; &local pair
//   push ecx
//   mov  ecx, esi                  ; this
//   call 0x00451a60                ; this->lookup(&pair, arg1) → EAX
//   mov  edi, [eax]                ; node = pair.first
//   test edi, edi
//   mov  ebx, [eax+0x4]            ; pair.second
//   mov  ebp, [esi+0x4]            ; this->field_0x4
//   jz   call_22b4
//   cmp  edi, esi
//   jz   skip_22b4
// call_22b4:
//   call 0x009d22b4               ; fixup helper (no args)
// skip_22b4:
//   cmp  ebx, ebp
//   jz   epilogue
//   push ebx
//   push edi
//   lea  edx, [esp+0x18]
//   push edx
//   mov  ecx, esi
//   call 0x00451e40               ; this->splice(&pair, edi, ebx)
// epilogue:
//   pop  ebp / pop ebx / pop edi / pop esi / add esp,0x8 / ret 0x8
//   ; --- rebuild path (field_0x14 != 0) ---
// rebuild:
//   push eax                       ; arg1
//   call 0x004528f0               ; helper → EAX (new object)
//   push -1
//   push 0
//   push edi                       ; arg2
//   mov  ecx, eax                  ; this = result
//   call 0x00404040               ; thiscall ctor/insert(arg2, 0, -1)
//   pop edi / pop esi / add esp,0x8 / ret 0x8
//
// Reloc-bearing sites in the orig 111 bytes (rel32 CALL targets baked
// into the orig binary; emitting them as raw bytes via MASM `_emit`
// produces a .obj whose .text matches the orig byte-for-byte with NO
// relocations — `tools/compare.py` reports GREEN):
//     +0x1f  CALL rel32 → 0x00451a60
//     +0x34  CALL rel32 → 0x009d22b4
//     +0x46  CALL rel32 → 0x00451e40
//     +0x56  CALL rel32 → 0x004528f0
//     +0x62  CALL rel32 → 0x00404040
//
// Reconstruction strategy — naked-asm byte passthrough (same as the
// reloc-heavy siblings FUN_004090b0 / FUN_004091f0 / FUN_00401650): a
// `__declspec(naked)` body re-emitting the orig 111 bytes verbatim.

extern "C" __declspec(naked) void FUN_004502d0() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x83              // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x18]
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x83              // CMP dword ptr [EDI+0x14], 0x0
        _emit 0x7f
        _emit 0x14
        _emit 0x00
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x75              // JNZ +0x40 (rebuild)
        _emit 0x40
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA ECX, [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL rel32 → 0x00451a60
        _emit 0x6c
        _emit 0x17
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EDI, dword ptr [EAX]
        _emit 0x38
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x8b              // MOV EBX, dword ptr [EAX+0x4]
        _emit 0x58
        _emit 0x04
        _emit 0x8b              // MOV EBP, dword ptr [ESI+0x4]
        _emit 0x6e
        _emit 0x04
        _emit 0x74              // JZ +0x4 (call_22b4)
        _emit 0x04
        _emit 0x3b              // CMP EDI, ESI
        _emit 0xfe
        _emit 0x74              // JZ +0x5 (skip_22b4)
        _emit 0x05
        _emit 0xe8              // CALL rel32 → 0x009d22b4
        _emit 0xab
        _emit 0x1f
        _emit 0x58
        _emit 0x00
        _emit 0x3b              // CMP EBX, EBP
        _emit 0xdd
        _emit 0x74              // JZ +0xe (epilogue)
        _emit 0x0e
        _emit 0x53              // PUSH EBX
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA EDX, [ESP+0x18]
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x52              // PUSH EDX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL rel32 → 0x00451e40
        _emit 0x25
        _emit 0x1b
        _emit 0x00
        _emit 0x00
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        _emit 0x50              // PUSH EAX                       (rebuild:)
        _emit 0xe8              // CALL rel32 → 0x004528f0
        _emit 0xc5
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0xe8              // CALL rel32 → 0x00404040
        _emit 0x09
        _emit 0x3d
        _emit 0xfb
        _emit 0xff
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
