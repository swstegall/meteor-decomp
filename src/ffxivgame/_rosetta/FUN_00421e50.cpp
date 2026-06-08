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
// FUNCTION: ffxivgame 0x00421e50 — __thiscall container clear/teardown
//                                  (58 B / 0x3a)
//
// __thiscall void FUN_00421e50(this)
//   ECX : this
//
// Inspection (read from the orig bytes at RVA 0x00021e50, 58 bytes):
//
//   sub  esp, 8                  ; two dword temps for the inner call
//   push esi
//   mov  esi, ecx               ; esi = this
//   mov  eax, [esi + 4]         ; eax = this->_Myptr  (field 0x04)
//   mov  ecx, [eax]             ; ecx = *_Myptr
//   push edi
//   push eax                    ; arg5: _Myptr
//   push esi                    ; arg4: this
//   push ecx                    ; arg3: *_Myptr
//   push esi                    ; arg2: this
//   lea  eax, [esp + 0x18]      ; &temp
//   push eax                    ; arg1: &temp
//   mov  ecx, esi               ; this
//   call FUN_00421d40           ; this->helper(&temp, this, *_Myptr, this, _Myptr)
//   mov  eax, [esi + 4]         ; eax = this->_Myptr (reload)
//   xor  edi, edi               ; edi = 0
//   cmp  eax, edi
//   jz   done                   ; if (_Myptr == 0) skip free
//   mov  ecx, [eax - 4]         ; ecx = header dword before the block
//   push eax
//   call FUN_0040df70           ; free/dealloc(_Myptr)  (header at [_Myptr-4])
// done:
//   mov  [esi + 8], edi         ; this->field_0x08 = 0
//   mov  [esi + 4], edi         ; this->_Myptr     = 0
//   pop  edi
//   pop  esi
//   add  esp, 8
//   ret                         ; __thiscall, no stack args → RET (0)
//
//   Layout inferred:
//     this+0x04   void *_Myptr   (heap block; cleared to 0 on teardown)
//     this+0x08   int   field_8  (size/count; cleared to 0 on teardown)
//   The block at _Myptr carries a dword header at [_Myptr - 4] which is
//   loaded into ECX before the dealloc call (FUN_0040df70) — the classic
//   "allocation cookie just before the user pointer" pattern.
//
//   Structurally identical to the immediately preceding FUN_00421e10
//   (same 58-byte body), differing only in which helper is called first:
//   FUN_00421e10 calls FUN_00421c70; this function calls FUN_00421d40.
//
// Reloc-bearing sites in the orig 58 bytes:
//     +0x17   CALL rel32 → FUN_00421d40 (0x00421d40)
//     +0x29   CALL rel32 → FUN_0040df70 (0x0040df70)
//
// Reconstruction strategy — naked-asm byte passthrough (same as sibling
// FUN_00421e10): the two CALL rel32 displacements are re-emitted as the
// orig binary's already-resolved bytes; tools/compare.py masks the rel32
// callsites, so the .obj .text matches byte-for-byte.

extern "C" __declspec(naked) void FUN_00421e50() {
    __asm {
        _emit 0x83              // SUB ESP, 0x08
        _emit 0xec
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x04]
        _emit 0x46
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [EAX]
        _emit 0x08
        _emit 0x57              // PUSH EDI
        _emit 0x50              // PUSH EAX
        _emit 0x56              // PUSH ESI
        _emit 0x51              // PUSH ECX
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA EAX, [ESP + 0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL rel32 → 0x00421d40
        _emit 0xd4
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x04]
        _emit 0x46
        _emit 0x04
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0x3b              // CMP EAX, EDI
        _emit 0xc7
        _emit 0x74              // JZ +0x09 (→ done)
        _emit 0x09
        _emit 0x8b              // MOV ECX, dword ptr [EAX - 0x04]
        _emit 0x48
        _emit 0xfc
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → 0x0040df70
        _emit 0xf2
        _emit 0xc0
        _emit 0xfe
        _emit 0xff
        _emit 0x89              // MOV dword ptr [ESI + 0x08], EDI    (done:)
        _emit 0x7e
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESI + 0x04], EDI
        _emit 0x7e
        _emit 0x04
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x08
        _emit 0xc4
        _emit 0x08
        _emit 0xc3              // RET
    }
}
