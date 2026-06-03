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
// FUNCTION: ffxivgame 0x008b17df — FUN_00cb17df
//           Accumulate sizes over an 8-byte-stride interval array up to a
//           given key value (142 B / 0x8e). Non-standard calling convention:
//           EAX = iterator pointer (first element), EDI = container struct
//           pointer, one stack argument (key) at [ESP+0x18] after prologue.
//           Epilogue pops EDI and ECX (pushed by caller as part of the call
//           frame), then cleans 4 more bytes via RET 4.
//
// Container layout inferred from offsets (EDI):
//   [EDI+0x4]  = begin/some lower bound ptr
//   [EDI+0x8]  = end ptr
//
// Element layout (8 bytes, iterated by ESI):
//   [ESI+0x0]  = element key  (unsigned, compared with stack arg)
//   [ESI+0x4]  = element size (accumulated into EBP → returned in EAX)
//
// Algorithm: for each element in [EAX, [EDI+8]):
//   if key <= element_key+element_size: add (key - element_start) and stop
//   else: add element_size and continue
//
// All CALL FUN_009d22b4 sites are bounds/assert checks (_invalid_parameter);
// they are never reached at runtime (the matching JC/JBE always wins first).
// compare.py wildcards the 4-byte REL32 displacement after each CALL e8.
//
// Reconstruction strategy — __declspec(naked) inline asm:
//   The non-standard CC (EAX input, POP EDI + POP ECX in epilogue, RET 4)
//   cannot be expressed in MSVC 2005 C++ source without naked asm.

extern "C" int FUN_009d22b4();   // _invalid_parameter bounds-check helper

extern "C" __declspec(naked) void FUN_00cb17df() {
    __asm {
        push    ebx
        push    ebp
        push    esi
        mov     esi, eax
        xor     ebp, ebp
        cmp     esi, dword ptr [edi + 8]
        jbe     short loop_top
        call    FUN_009d22b4
    loop_top:
        mov     ebx, dword ptr [edi + 8]
        cmp     dword ptr [edi + 4], ebx
        jbe     short chk_edi
        call    FUN_009d22b4
    chk_edi:
        cmp     edi, edi
        jz      short chk_eq
        call    FUN_009d22b4
    chk_eq:
        cmp     esi, ebx
        jz      short done
        cmp     esi, dword ptr [edi + 8]
        jc      short load_arg
        call    FUN_009d22b4
    load_arg:
        mov     ebx, dword ptr [esp + 0x18]
        cmp     dword ptr [esi], ebx
        ja      short accum_full
        cmp     esi, dword ptr [edi + 8]
        jc      short compute_end
        call    FUN_009d22b4
        cmp     esi, dword ptr [edi + 8]
        jc      short compute_end
        call    FUN_009d22b4
    compute_end:
        mov     edx, dword ptr [esi + 4]
        add     edx, dword ptr [esi]
        cmp     ebx, edx
        jbe     short partial_path
    accum_full:
        cmp     esi, dword ptr [edi + 8]
        jc      short do_accum
        call    FUN_009d22b4
    do_accum:
        add     ebp, dword ptr [esi + 4]
        cmp     esi, dword ptr [edi + 8]
        jc      short advance
        call    FUN_009d22b4
    advance:
        add     esi, 8
        jmp     short loop_top
    partial_path:
        cmp     esi, dword ptr [edi + 8]
        jc      short do_partial
        call    FUN_009d22b4
    do_partial:
        sub     ebx, dword ptr [esi]
        add     ebp, ebx
    done:
        pop     esi
        mov     eax, ebp
        pop     ebp
        pop     ebx
        pop     edi
        pop     ecx
        ret     4
    }
}
