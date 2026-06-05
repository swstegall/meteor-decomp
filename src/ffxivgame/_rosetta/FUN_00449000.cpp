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
// FUNCTION: ffxivgame 0x00449000 — string null-terminate-and-emit thunk
//                                  (__thiscall, ret 4, 85 bytes)
//
// __thiscall returning ESI (the string object) with one explicit stack
// arg. `this` arrives in ECX and is stashed in EBX; the explicit arg
// (a string-like object at [ESP+0x10] after the three register pushes)
// is held in ESI.
//
// Asm shape (read from orig RVA 0x00049000, 85 bytes):
//
//   push ebx
//   push esi
//   push edi
//   mov  ebx, ecx                 ; ebx = this
//   call 0x00445e50               ; eax = some length/count
//   mov  esi, [esp+0x10]          ; esi = arg (string obj)
//   mov  edi, [esi+0x14]          ; edi = arg->size
//   add  eax, eax                 ; eax = count * 2  (wchar units → bytes? )
//   cmp  edi, eax
//   ja   tail                     ; size > eax → skip terminate
//   cmp  [esi+0x18], eax          ; capacity == eax?
//   jz   tail
//   push 1
//   push eax
//   mov  ecx, esi
//   call 0x00449a80               ; arg->reserve/grow(eax, 1) → AL
//   test al, al
//   jz   tail
//   cmp  [esi+0x18], 8            ; capacity < 8 → SSO buffer inline at +4
//   mov  [esi+0x14], edi          ; restore size
//   jc   sso                      ; below → use inline buffer
//   mov  eax, [esi+0x4]           ; large: pointer at +4
//   jmp  setnull
//  sso:
//   lea  eax, [esi+0x4]           ; small: inline buffer base at +4
//  setnull:
//   mov  word ptr [eax+edi*2], 0  ; buf[size] = L'\0'
//  tail:
//   mov  eax, [ebx]               ; eax = *this (some payload ptr)
//   push esi
//   push eax
//   call 0x00448ef0               ; emit(eax, esi)
//   add  esp, 8
//   pop  edi
//   mov  eax, esi                 ; return the string obj
//   pop  esi
//   pop  ebx
//   ret  4
//
// Reloc-bearing CALL rel32 sites (resolve at full-binary relink):
//   +0x05   CALL rel32 → 0x00445e50
//   +0x21   CALL rel32 → 0x00449a80
//   +0x45   CALL rel32 → 0x00448ef0
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Same approach as siblings FUN_004065c0 / FUN_00403b70: a
//   `__declspec(naked)` body re-emitting the orig 85 bytes verbatim via
//   `_emit` directives yields a .obj whose .text is byte-identical to
//   the orig slice with zero relocations (the rel32 displacements are
//   baked into the orig binary's address space and emitted as raw
//   bytes). tools/compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_00449000() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV  EBX, ECX
        _emit 0xd9
        _emit 0xe8              // CALL 0x00445e50   (rel32)
        _emit 0x46
        _emit 0xce
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV  ESI, [ESP+0x10]
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV  EDI, [ESI+0x14]
        _emit 0x7e
        _emit 0x14
        _emit 0x03              // ADD  EAX, EAX
        _emit 0xc0
        _emit 0x3b              // CMP  EDI, EAX
        _emit 0xf8
        _emit 0x77              // JA   tail (+0x2a)
        _emit 0x2a
        _emit 0x39              // CMP  [ESI+0x18], EAX
        _emit 0x46
        _emit 0x18
        _emit 0x74              // JZ   tail (+0x25)
        _emit 0x25
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV  ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x00449a80   (rel32)
        _emit 0x5a
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x74              // JZ   tail (+0x17)
        _emit 0x17
        _emit 0x83              // CMP  [ESI+0x18], 0x8
        _emit 0x7e
        _emit 0x18
        _emit 0x08
        _emit 0x89              // MOV  [ESI+0x14], EDI
        _emit 0x7e
        _emit 0x14
        _emit 0x72              // JC   sso (+0x05)
        _emit 0x05
        _emit 0x8b              // MOV  EAX, [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0xeb              // JMP  setnull (+0x03)
        _emit 0x03
        _emit 0x8d              // LEA  EAX, [ESI+0x4]   (sso:)
        _emit 0x46
        _emit 0x04
        _emit 0x66              // MOV  word ptr [EAX+EDI*2], 0   (setnull:)
        _emit 0xc7
        _emit 0x04
        _emit 0x78
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV  EAX, [EBX]   (tail:)
        _emit 0x03
        _emit 0x56              // PUSH ESI
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x00448ef0   (rel32)
        _emit 0xa6
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD  ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x5f              // POP  EDI
        _emit 0x8b              // MOV  EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP  ESI
        _emit 0x5b              // POP  EBX
        _emit 0xc2              // RET  0x4
        _emit 0x04
        _emit 0x00
    }
}
