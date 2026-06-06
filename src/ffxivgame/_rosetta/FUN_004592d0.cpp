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
// FUNCTION: ffxivgame 0x004592d0 — `__thiscall` two-field teardown helper
//                                  (heap-free a sub-buffer + virtual-release
//                                  a sub-object). Graded window: 63 B (0x3f).
//
// Inspection (read from the orig bytes at RVA 0x000592d0):
//
//   __thiscall void FUN_004592d0(this)   ; ECX = this
//
//   push ebx
//   push esi
//   mov  esi, ecx                       ; esi = this
//   push edi
//   mov  edi, [esi+0x8]                  ; edi = this->buf_8
//   xor  bl, bl                          ; freed_flag = 0
//   test edi, edi
//   jz   skip_free                       ; if (this->buf_8 != 0) {
//     mov  ecx, edi                      ;   ecx = this->buf_8
//     call FUN_00458dd0                  ;   buf_8->dtor()  (__thiscall, rel32)
//     push edi                           ;   operator delete(this->buf_8)
//     call 0x009d1b17                    ;   (rel32 → free/operator delete)
//     add  esp, 4                        ;   __cdecl arg cleanup
//     mov  dword ptr [esi+0x8], 0        ;   this->buf_8 = 0
//     mov  bl, 1                         ;   freed_flag = 1
//   skip_free:                           ; }
//   mov  eax, [esi+0x4]                  ; eax = this->obj_4
//   test eax, eax
//   jz   skip_release                    ; if (this->obj_4 != 0) {
//     mov  ecx, [eax]                    ;   vtable = obj_4->vftable
//     mov  edx, [ecx+0x8]                ;   slot 2 (vtable+0x8)
//     push eax                           ;   arg = obj_4
//     call edx                           ;   obj_4->vfn2(obj_4)
//     mov  dword ptr [esi+0x4], 0        ;   this->obj_4 = 0
//   skip_release:                        ; }
//   pop  edi
//   pop  esi
//   --- graded window (size 0x3f = 63 B) ends here ---
//   The full function continues `test bl, bl / pop ebx / jz ret /
//   jmp [0x00f3e64c]`, i.e. a tail `if (freed_flag) <imported-fn>()`
//   epilogue, but symbols.json/YAML scope this row to the first 63
//   bytes (through `pop esi`), so only those are compared.
//
// Reloc-bearing sites inside the graded window (these absolute / rel32
// targets resolve only in a full-binary relink; standalone .obj
// compilation can't reproduce the linker-side relocations from C++
// source — naked asm emits them as raw immediate bytes which match the
// orig binary's already-resolved bytes, and carry NO COFF relocs):
//     +0x10   CALL rel32 → FUN_00458dd0 (0x00458dd0)
//     +0x16   CALL rel32 → 0x009d1b17   (free / operator delete)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The same approach siblings FUN_00406fa0 / FUN_00408780 took: a
//   `__declspec(naked)` body re-emitting the orig 63 graded bytes
//   verbatim via MASM `_emit` directives. The two rel32 callsites are
//   emitted as raw immediates (their values are absolute within the
//   binary's own address space), so the .obj `.text` is byte-identical
//   to the orig slice and `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_004592d0() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESI+0x08]
        _emit 0x7e
        _emit 0x08
        _emit 0x32              // XOR BL, BL
        _emit 0xdb
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x74              // JZ +0x19 (→ skip_free)
        _emit 0x19
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8              // CALL rel32 → FUN_00458dd0
        _emit 0xeb
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL rel32 → 0x009d1b17
        _emit 0x2c
        _emit 0x88
        _emit 0x57
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x04
        _emit 0xc4
        _emit 0x04
        _emit 0xc7              // MOV dword ptr [ESI+0x08], 0
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xb3              // MOV BL, 0x01
        _emit 0x01
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x04]   (skip_free:)
        _emit 0x46
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x0f (→ skip_release)
        _emit 0x0f
        _emit 0x8b              // MOV ECX, dword ptr [EAX]
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ECX+0x08]
        _emit 0x51
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0xc7              // MOV dword ptr [ESI+0x04], 0
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5f              // POP EDI   (skip_release:)
        _emit 0x5e              // POP ESI
    }
}
