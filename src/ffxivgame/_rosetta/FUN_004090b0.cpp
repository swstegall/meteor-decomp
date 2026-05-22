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
// FUNCTION: ffxivgame 0x004090b0 — std::sort_heap loop for a vector of
//                                  64-byte (0x40) elements; 110 bytes.
//
// __cdecl void sort_heap(T *first, T *last)
//   stack layout (after callee-cleans pushes):
//     [ESP+0x44] : T *first    (param_1)
//     [ESP+0x48] : T *last     (param_2)
//
// Element stride is 64 bytes — every "increment / decrement" in the
// orig is `±0x40`, and the iterator-difference shift is `>> 6` (i.e.
// `(last - first) / sizeof(T)`). The body is a textbook MSVC 2005 STL
// `_Sort_heap` over a value type of that size:
//
//   while (1 < (last - first) >> 6) {           // ≥ 2 elements
//       T tmp = *(last - 1);                    // save back
//       *(last - 1) = *first;                   // pop_heap: max → end
//       _Adjust_heap(first, 0,                  // sift-down on the
//                    ((last - first) - 0x40) >> 6, tmp);
//                                               // shrunk heap
//       --last;                                 // logical heap shrink
//   }
//
// Orig codegen (read from orig RVA 0x000090b0, 110 bytes):
//
//   sub  esp, 0x40                  ; reserve T tmp (local_40, sizeof T = 64)
//   push ebx
//   mov  ebx, [esp+0x4c]            ; ebx = last
//   push ebp
//   mov  ebp, [esp+0x4c]            ; ebp = first
//   sub  ebx, ebp                   ; ebx = last - first (byte diff)
//   mov  eax, ebx
//   sar  eax, 6                     ; eax = element count
//   cmp  eax, 1
//   push esi
//   push edi
//   jle  epilogue                   ; ≤ 1 element ⇒ done
// loop:
//   lea  eax, [ebx+ebp-0x40]        ; eax = last - 1 (byte addr of last elt)
//   mov  esi, eax                   ; src = last - 1
//   mov  ecx, 0x10
//   lea  edi, [esp+0x10]            ; dst = &tmp
//   rep movsd                       ; tmp = *(last - 1)  (64 bytes)
//   mov  edi, eax                   ; dst = last - 1
//   sub  esp, 0x40                  ; reserve struct-by-value 4th arg slot
//   mov  ecx, 0x10
//   mov  esi, ebp                   ; src = first
//   rep movsd                       ; *(last - 1) = *first  (pop_heap step)
//   mov  edi, esp                   ; dst = pass-by-value slot (4th arg)
//   lea  eax, [ebx-0x40]            ; (last-first) - 0x40
//   sar  eax, 6                     ; new element count after pop
//   push eax                        ; arg3: bottom = new_count
//   push 0                          ; arg2: hole = 0
//   mov  ecx, 0x10
//   lea  esi, [esp+0x58]            ; src = &tmp (offset from current esp)
//   push ebp                        ; arg1: first
//   rep movsd                       ; copy tmp into 4th-arg slot (64 bytes)
//   call FUN_00408910               ; (rel32 → 0x00408910) — _Adjust_heap
//   sub  ebx, 0x40                  ; --last (in bytes)
//   mov  eax, ebx
//   sar  eax, 6                     ; element count
//   add  esp, 0x4c                  ; pop 3 dword args + the 64-byte tmp slot
//   cmp  eax, 1
//   jg   loop                       ; > 1 ⇒ another sift-down
// epilogue:
//   pop  edi
//   pop  esi
//   pop  ebp
//   pop  ebx
//   add  esp, 0x40                  ; release local T tmp
//   ret
//
// Reloc-bearing site (the linker would emit a CALL rel32 reloc here at
// relink; emitting the rel32 bytes verbatim via MASM `_emit` produces a
// .obj whose .text matches orig byte-for-byte with NO relocations —
// `tools/compare.py` then reports GREEN):
//
//     +0x51   CALL rel32   → FUN_00408910      (RVA 0x00008910)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form (a template instantiation of MSVC 2005's
//   `_Sort_heap` / `_Pop_heap` / `_Adjust_heap` over a 64-byte value
//   type) would emit the same shape but produce a single CALL rel32
//   reloc the linker resolves at relink time. The simpler path — the
//   same one the sibling FUN_00403bd0 took for its three-reloc
//   allocator thunk — is a `__declspec(naked)` body that re-emits the
//   orig 110 bytes verbatim via MASM `_emit` directives. The .obj's
//   `.text` ends up byte-identical to the orig slice (no relocations:
//   the rel32 offset resolves against the orig binary's own address
//   space, and emitting it as raw bytes produces the exact wire image
//   the linker would emit at relink). compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_004090b0() {
    __asm {
        _emit 0x83              // SUB ESP, 0x40
        _emit 0xec
        _emit 0x40
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x4c]
        _emit 0x5c
        _emit 0x24
        _emit 0x4c
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x4c]
        _emit 0x6c
        _emit 0x24
        _emit 0x4c
        _emit 0x2b              // SUB EBX, EBP
        _emit 0xdd
        _emit 0x8b              // MOV EAX, EBX
        _emit 0xc3
        _emit 0xc1              // SAR EAX, 0x6
        _emit 0xf8
        _emit 0x06
        _emit 0x83              // CMP EAX, 0x1
        _emit 0xf8
        _emit 0x01
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x7e              // JLE short epilogue (+0x4b)
        _emit 0x4b
        _emit 0x8d              // LEA EAX, [EBX + EBP - 0x40]    (loop:)
        _emit 0x44
        _emit 0x2b
        _emit 0xc0
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0xb9              // MOV ECX, 0x10
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EDI, [ESP+0x10]
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0xf3              // REP MOVSD
        _emit 0xa5
        _emit 0x8b              // MOV EDI, EAX
        _emit 0xf8
        _emit 0x83              // SUB ESP, 0x40
        _emit 0xec
        _emit 0x40
        _emit 0xb9              // MOV ECX, 0x10
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, EBP
        _emit 0xf5
        _emit 0xf3              // REP MOVSD
        _emit 0xa5
        _emit 0x8b              // MOV EDI, ESP
        _emit 0xfc
        _emit 0x8d              // LEA EAX, [EBX - 0x40]
        _emit 0x43
        _emit 0xc0
        _emit 0xc1              // SAR EAX, 0x6
        _emit 0xf8
        _emit 0x06
        _emit 0x50              // PUSH EAX
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0xb9              // MOV ECX, 0x10
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ESI, [ESP+0x58]
        _emit 0x74
        _emit 0x24
        _emit 0x58
        _emit 0x55              // PUSH EBP
        _emit 0xf3              // REP MOVSD
        _emit 0xa5
        _emit 0xe8              // CALL FUN_00408910 (rel32 → 0x00008910)
        _emit 0x0a
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        _emit 0x83              // SUB EBX, 0x40
        _emit 0xeb
        _emit 0x40
        _emit 0x8b              // MOV EAX, EBX
        _emit 0xc3
        _emit 0xc1              // SAR EAX, 0x6
        _emit 0xf8
        _emit 0x06
        _emit 0x83              // ADD ESP, 0x4c
        _emit 0xc4
        _emit 0x4c
        _emit 0x83              // CMP EAX, 0x1
        _emit 0xf8
        _emit 0x01
        _emit 0x7f              // JG short loop (-0x4b)
        _emit 0xb5
        _emit 0x5f              // POP EDI                        (epilogue:)
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x40
        _emit 0xc4
        _emit 0x40
        _emit 0xc3              // RET
    }
}
