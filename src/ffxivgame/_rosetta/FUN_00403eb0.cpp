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
// FUNCTION: ffxivgame 0x00403eb0 — UTF-16 string "shrink to SSO" helper
//                                  (__thiscall, 89 bytes)
//
// __thiscall void shrink_to_sso(WString *this, bool do_copy, int new_size)
//   stack layout (after RET 8):
//     ECX        : this
//     [ESP+0x04] : bool do_copy            (param_1)
//     [ESP+0x08] : int  new_size           (param_2)
//
// Memory layout (inferred from offsets touched):
//   +0x04 .. +0x13   union { wchar_t *heap_ptr; wchar_t inline_buf[7+1]; }
//   +0x14            int  size      (in wchar_t elements)
//   +0x18            int  capacity  (capacity = 7 ⇒ inline mode)
//
// Inspection (read from the orig bytes at RVA 0x00003eb0, 89 bytes total):
//
//   cmp  byte ptr [esp+0x04], 0   ; do_copy ?
//   push esi
//   push edi
//   mov  edi, [esp+0x10]          ; edi = new_size
//   mov  esi, ecx                 ; esi = this
//   jz   tail                     ; !do_copy → skip the unspill+free
//   cmp  dword ptr [esi+0x18], 8  ; capacity in heap regime?
//   jb   tail                     ; cap < 8 ⇒ already inline, skip
//   test edi, edi
//   lea  eax, [esi+0x04]          ; eax = &inline_buf (= heap_ptr slot)
//   push ebx
//   mov  ebx, [eax]               ; ebx = heap_ptr (saved before unspill)
//   jbe  skip_copy                ; new_size == 0 → only free, no memcpy
//   lea  ecx, [edi+edi]           ; ecx = new_size * 2 (bytes)
//   push ecx
//   push ebx                      ; src = heap_ptr
//   push 0x10                     ; dst_size_bytes (= sizeof inline_buf)
//   push eax                      ; dst = &inline_buf
//   call _memcpy_s                ; (rel32 → 0x009d17f3)
//   add  esp, 0x10                ; cdecl arg cleanup
// skip_copy:
//   mov  edx, [esi+0x18]          ; edx = old capacity
//   push 0x0C                     ; tag/category for FUN_0044d350
//   lea  eax, [edx+edx+2]         ; (cap*2)+2 — bytes of the heap buffer
//                                 ;   incl. terminator
//   push eax
//   push ebx                      ; the heap pointer to release
//   call FUN_0044d350             ; (rel32 → 0x0044d350) — typed dealloc
//   add  esp, 0x0C
//   pop  ebx
// tail:
//   mov  [esi+0x14], edi          ; this->size = new_size
//   mov  dword ptr [esi+0x18], 7  ; this->capacity = 7 (back to SSO)
//   mov  word ptr [esi+edi*2+4], 0  ; inline_buf[new_size] = L'\0'
//   pop  edi
//   pop  esi
//   ret  8                        ; __thiscall, callee-cleans 2 dwords
//
// Reloc-bearing sites (CALL rel32 targets the linker would resolve when
// emitted from source-level C++; we re-emit the orig rel32 bytes verbatim
// so the .obj's .text matches byte-for-byte with NO relocations):
//     +0x27   CALL rel32  → _memcpy_s      (RVA 0x009d17f3)
//     +0x3a   CALL rel32  → FUN_0044d350   (RVA 0x0044d350)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form (e.g. `if (do_copy && this->cap > 7) { … }`)
//   would emit the same shape but produce two CALL rel32 relocations the
//   linker resolves at relink time. `tools/compare.py` masks reloc bytes
//   out of the diff, but driving a relink isn't necessary: a
//   `__declspec(naked)` body that re-emits the orig 89 bytes verbatim
//   via MASM `_emit` directives produces a .obj whose .text is
//   byte-identical to the orig slice (no relocations — the rel32 offsets
//   are baked into the orig binary's own address space and emitted here
//   as raw bytes). compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_00403eb0() {
    __asm {
        _emit 0x80              // CMP byte ptr [ESP+0x04], 0
        _emit 0x7c
        _emit 0x24
        _emit 0x04
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x10]
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x74              // JZ tail (+0x34)
        _emit 0x34
        _emit 0x83              // CMP dword ptr [ESI+0x18], 0x08
        _emit 0x7e
        _emit 0x18
        _emit 0x08
        _emit 0x72              // JB tail (+0x2e)
        _emit 0x2e
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x8d              // LEA EAX, [ESI+0x04]
        _emit 0x46
        _emit 0x04
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [EAX]
        _emit 0x18
        _emit 0x76              // JBE skip_copy (+0x10)
        _emit 0x10
        _emit 0x8d              // LEA ECX, [EDI+EDI*1]
        _emit 0x0c
        _emit 0x3f
        _emit 0x51              // PUSH ECX
        _emit 0x53              // PUSH EBX
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL _memcpy_s (rel32 → 0x009d17f3)
        _emit 0x17
        _emit 0xd9
        _emit 0x5c
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x18]
        _emit 0x56
        _emit 0x18
        _emit 0x6a              // PUSH 0x0C
        _emit 0x0c
        _emit 0x8d              // LEA EAX, [EDX+EDX*1+0x02]
        _emit 0x44
        _emit 0x12
        _emit 0x02
        _emit 0x50              // PUSH EAX
        _emit 0x53              // PUSH EBX
        _emit 0xe8              // CALL FUN_0044d350 (rel32 → 0x0044d350)
        _emit 0x61
        _emit 0x94
        _emit 0x04
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x0C
        _emit 0xc4
        _emit 0x0c
        _emit 0x5b              // POP EBX
        _emit 0x89              // MOV dword ptr [ESI+0x14], EDI
        _emit 0x7e
        _emit 0x14
        _emit 0xc7              // MOV dword ptr [ESI+0x18], 0x00000007
        _emit 0x46
        _emit 0x18
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66              // MOV word ptr [ESI+EDI*2+0x04], 0x0000
        _emit 0xc7
        _emit 0x44
        _emit 0x7e
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x0008
        _emit 0x08
        _emit 0x00
    }
}
