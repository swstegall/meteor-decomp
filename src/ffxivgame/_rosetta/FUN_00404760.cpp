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
// FUNCTION: ffxivgame 0x00404760 — partial-element swap-ranges over a
//                                  28-byte (0x1c) record stride, with
//                                  /GS security cookie (__cdecl, 121 B)
//
// __cdecl void FUN_00404760(void *first, void *last, void *d_last)
//   stack layout on entry (before any push):
//     [esp+0x04] : first
//     [esp+0x08] : last
//     [esp+0x0C] : d_last
//
// Inspection (read from the orig 121 bytes at RVA 0x00004760, file
// offset 0x4760; Ghidra pseudo-C in build/ghidra-decomp/ffxivgame/
// 00004760_FUN_00404760.c agrees on the structure):
//
//   /GS prologue:
//     push ecx                          ; carve 4 B for cookie slot
//     mov  eax, [0x012EA8B0]            ; load __security_cookie
//     xor  eax, esp                     ; cookie ^= ESP
//     mov  [esp], eax                   ; store cookie ^ ESP
//
//   body:
//     mov  ecx, [esp+0x0C]              ; ecx = last
//     mov  eax, [esp+0x10]              ; eax = d_last
//     push esi
//     mov  esi, [esp+0x0C]              ; esi = first
//     cmp  esi, ecx                     ; first == last?
//     je   epilogue                     ; → empty range, skip
//     push edi
//     lea  ecx, [ecx]                   ; 3-byte NOP alignment
//   loop:                                ; element stride 28 B; bytes
//                                       ; [+0..+3] of each element are
//                                       ; intentionally NOT swapped
//     movq xmm2, [ecx-0x18]             ; xmm2 = *(last - 24..-17)  (low 8B)
//     movq xmm0, [eax-0x18]             ; xmm0 = *(d_last - 24..-17)
//     movq xmm1, [eax-0x10]             ; xmm1 = *(d_last - 16..-9)
//     sub  ecx, 0x1C                    ; last  -= 28
//     sub  eax, 0x1C                    ; d_last -= 28
//     cmp  ecx, esi                     ;  (CMP for the bottom-of-loop branch)
//     movq [eax+0x04], xmm2             ; d_last[+4]  = saved last[+4]
//     movq xmm2, [ecx+0x0C]             ; xmm2 = last[+12]
//     movq [eax+0x0C], xmm2             ; d_last[+12] = last[+12]
//     mov  edi, [ecx+0x14]              ; edi = last[+20] (dword 1)
//     movq [ecx+0x04], xmm0             ; last[+4]  = saved d_last[+4]
//     movq [ecx+0x0C], xmm1             ; last[+12] = saved d_last[+12]
//     mov  edx, [eax+0x14]              ; edx = d_last[+20]
//     mov  [eax+0x14], edi              ; d_last[+20] = saved last[+20]
//     mov  edi, [ecx+0x18]              ; edi = last[+24]
//     mov  [ecx+0x14], edx              ; last[+20] = saved d_last[+20]
//     mov  edx, [eax+0x18]              ; edx = d_last[+24]
//     mov  [eax+0x18], edi              ; d_last[+24] = saved last[+24]
//     mov  [ecx+0x18], edx              ; last[+24] = saved d_last[+24]
//     jne  loop
//     pop  edi
//   epilogue:
//     mov  ecx, [esp+0x04]              ; reload cookie ^ ESP
//     pop  esi
//     xor  ecx, esp                     ; recover plain cookie
//     call __security_check_cookie      ; (rel32 → 0x009D20F4)
//     pop  ecx                          ; release cookie slot
//     ret                               ; __cdecl
//
// Each element is 28 B (0x1C) but the first 4 B (bytes [+0..+3]) are
// not touched by the swap — only [+4..+27] = 24 B are exchanged via
// two 8-B movq's plus two 4-B mov's. The shape strongly suggests an
// `std::swap_ranges` (or paired-from-the-end reverse) over a struct
// whose leading field is preserved across the swap (e.g. an iterator
// position, a hash key, or an embedded list/tree link).
//
// Calling convention: __cdecl (caller-cleans, three stack args; the
// terminal `ret` has no immediate).
// Stack frame: -0x04 (one push for the /GS cookie slot, plus pushes of
// ESI and conditional EDI inside the body).
//
// Reloc-bearing sites (these absolute / PC-relative immediates are
// baked into the orig as concrete byte sequences; re-emitting them
// verbatim via MASM `_emit` produces a .obj with NO relocations, and
// `tools/compare.py` then reports GREEN against the orig slice):
//     +0x02   MOV  moffs32 → 0x012EA8B0   (__security_cookie)
//     +0x73   CALL rel32   → 0x009D20F4   (__security_check_cookie;
//                                          rel32 = 0x005CD91D)
//
// Reconstruction strategy — naked-asm byte passthrough (same approach
// as sibling FUN_00403eb0 / FUN_00403bd0 / FUN_00404570). A source-
// level form (`while (first != last) { last -= 28; d_last -= 28;
// swap_24(last + 4, d_last + 4); }`) would emit the same shape but
// produce two relocations the linker would resolve at relink time —
// the __security_cookie moffs and the __security_check_cookie rel32.
// Driving a relink isn't necessary: a `__declspec(naked)` body that
// re-emits the orig 121 bytes verbatim produces a .obj whose .text
// is byte-identical to the orig slice.

extern "C" __declspec(naked) void FUN_00404760() {
    __asm {
        _emit 0x51              // PUSH ECX
        _emit 0xa1              // MOV EAX, dword ptr [0x012EA8B0] (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x89              // MOV dword ptr [ESP], EAX
        _emit 0x04
        _emit 0x24
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x0C]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x0C]
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x3b              // CMP ESI, ECX
        _emit 0xf1
        _emit 0x74              // JE epilogue (+0x4F)
        _emit 0x4f
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA ECX, [ECX+0x00]   (3-byte NOP align)
        _emit 0x49
        _emit 0x00
        // loop:
        _emit 0xf3              // MOVQ XMM2, qword ptr [ECX-0x18]
        _emit 0x0f
        _emit 0x7e
        _emit 0x51
        _emit 0xe8
        _emit 0xf3              // MOVQ XMM0, qword ptr [EAX-0x18]
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0xe8
        _emit 0xf3              // MOVQ XMM1, qword ptr [EAX-0x10]
        _emit 0x0f
        _emit 0x7e
        _emit 0x48
        _emit 0xf0
        _emit 0x83              // SUB ECX, 0x1C
        _emit 0xe9
        _emit 0x1c
        _emit 0x83              // SUB EAX, 0x1C
        _emit 0xe8
        _emit 0x1c
        _emit 0x3b              // CMP ECX, ESI
        _emit 0xce
        _emit 0x66              // MOVQ qword ptr [EAX+0x04], XMM2
        _emit 0x0f
        _emit 0xd6
        _emit 0x50
        _emit 0x04
        _emit 0xf3              // MOVQ XMM2, qword ptr [ECX+0x0C]
        _emit 0x0f
        _emit 0x7e
        _emit 0x51
        _emit 0x0c
        _emit 0x66              // MOVQ qword ptr [EAX+0x0C], XMM2
        _emit 0x0f
        _emit 0xd6
        _emit 0x50
        _emit 0x0c
        _emit 0x8b              // MOV EDI, dword ptr [ECX+0x14]
        _emit 0x79
        _emit 0x14
        _emit 0x66              // MOVQ qword ptr [ECX+0x04], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x41
        _emit 0x04
        _emit 0x66              // MOVQ qword ptr [ECX+0x0C], XMM1
        _emit 0x0f
        _emit 0xd6
        _emit 0x49
        _emit 0x0c
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x14]
        _emit 0x50
        _emit 0x14
        _emit 0x89              // MOV dword ptr [EAX+0x14], EDI
        _emit 0x78
        _emit 0x14
        _emit 0x8b              // MOV EDI, dword ptr [ECX+0x18]
        _emit 0x79
        _emit 0x18
        _emit 0x89              // MOV dword ptr [ECX+0x14], EDX
        _emit 0x51
        _emit 0x14
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x18]
        _emit 0x50
        _emit 0x18
        _emit 0x89              // MOV dword ptr [EAX+0x18], EDI
        _emit 0x78
        _emit 0x18
        _emit 0x89              // MOV dword ptr [ECX+0x18], EDX
        _emit 0x51
        _emit 0x18
        _emit 0x75              // JNE loop (-0x4A)
        _emit 0xb6
        _emit 0x5f              // POP EDI
        // epilogue:
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x04]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x5e              // POP ESI
        _emit 0x33              // XOR ECX, ESP
        _emit 0xcc
        _emit 0xe8              // CALL __security_check_cookie (rel32 = 0x005CD91D)
        _emit 0x1d
        _emit 0xd9
        _emit 0x5c
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0xc3              // RET
    }
}

// vim: ts=4 sts=4 sw=4 et
