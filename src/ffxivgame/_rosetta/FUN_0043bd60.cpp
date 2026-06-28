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
// FUNCTION: ffxivgame 0x0043bd60 — __thiscall vector assign-from-other
//                                   (293 B / 0x125).
//
// Calling convention: __thiscall (ECX = this), one stack arg (Vec* other),
// callee-cleans via `RET 4`. Returns this in EAX.
//
// Object layout (MSVC 2005 _Vector_val style, element = DWORD/4 bytes):
//   this+0x00  vtptr or other field (never accessed in this fn)
//   this+0x04  _Myfirst  (begin pointer)
//   this+0x08  _Mylast   (end of used data)
//   this+0x0c  _Myend    (end of allocated capacity)
//
// High-level shape (inferred from asm):
//
//   Vec* Vec::assign(Vec* other):
//     if (this == other) return this;
//
//     int* ob = other->_Myfirst;
//     if (!ob) throw_range_error();          // FUN_00a2c790 (never returns)
//
//     int* oe = other->_Mylast;
//     int  param_cnt = (oe - ob) >> 2;      // element count
//     if (!param_cnt) throw_range_error();   // same throw
//
//     int* tb  = this->_Myfirst;
//     int  this_cnt = tb ? (this->_Mylast - tb) >> 2 : 0;
//
//     if (param_cnt <= this_cnt):
//       // overwrite first param_cnt used slots
//       FUN_007c46a0(ob, oe, tb);            // copy [ob,oe) → tb  (cdecl)
//       int* ob2 = other->_Myfirst;          // re-read (alias guard)
//       if (!ob2)
//         this->_Mylast = this->_Myfirst;
//       else
//         this->_Mylast = this->_Myfirst + ((other->_Mylast - ob2) >> 2);
//       return this;
//
//     // param_cnt > this_cnt
//     int cap = tb ? (this->_Myend - tb) >> 2 : 0;
//
//     if (param_cnt <= cap):
//       // fits in allocated capacity; copy existing count in-place,
//       // then append the remainder via insert helper
//       int old_cnt = tb ? (this->_Mylast - tb) >> 2 : 0;
//       int* mid = ob + old_cnt;
//       FUN_007c46a0(ob, mid, tb);           // overwrite existing elements (cdecl)
//       // re-use EBX=mid, ECX=this via __thiscall:
//       FUN_00da8ea0(mid, oe, this->_Mylast);  // append [mid,oe) (callee-cleans 0xC)
//       this->_Mylast = result;
//       return this;
//
//     // param_cnt > cap: must reallocate
//     if (tb):
//       FUN_0040df70(*(int**)(tb - 4), tb);  // free old buffer (__thiscall, RET 4)
//     int param_cnt2 = other->_Myfirst ? (other->_Mylast - other->_Myfirst)>>2 : 0;
//     if (!FUN_00436580(this, param_cnt2))    // reserve (thiscall, RET 4) → bool
//       return this;                          // allocation failed
//     FUN_00da8ea0(other->_Myfirst, other->_Mylast, this->_Myfirst);  // fill
//     this->_Mylast = result;
//     return this;
//
// Called helpers:
//   FUN_00a2c790 (@0x00a2c790) — throw/range-error, never returns (cdecl)
//   FUN_007c46a0 (@0x007c46a0) — copy [src,end) to dest; returns new dest end (cdecl)
//   FUN_00da8ea0 (@0x00da8ea0) — same shape but callee-cleans 0xC (stdcall/thiscall)
//   FUN_0040df70 (@0x0040df70) — buffer free (__thiscall ECX=hdr, RET 4)
//   FUN_00436580 (@0x00436580) — vector reserve-init (__thiscall, bool AL, RET 4)
//
// Reconstruction strategy — __declspec(naked) MASM:
//   The function has complex register re-use (EDI overloaded as both the
//   "other" pointer AND a temporary byte-count register before POP EDI),
//   four separate epilogue sequences (three early RET 4s, one shared
//   tail at 0x0043be7c), and mixed push-depths at the same label
//   (0x0043be7e is reached both from the initial self-test JZ with only
//   ESI/EDI on the stack and as fall-through from the common epilogue
//   after POP EBP/EBX). This makes a source-level C++ reconstruction
//   structurally fragile; a naked MASM body reproduces the exact encoding
//   including the 0F 84 rel32 near-JZ on the self-comparison and all
//   the rel8 short branches for the inner tests. All five CALL rel32
//   displacements are linker-resolved relocations that compare.py masks.

extern "C" {

void FUN_00a2c790();   // throw range-error (no return)
void FUN_007c46a0();   // copy range, cdecl
void FUN_00da8ea0();   // copy range, callee-cleans 0xC
void FUN_0040df70();   // buffer free, thiscall+RET4
void FUN_00436580();   // vector reserve, thiscall+RET4

__declspec(naked) void FUN_0043bd60()
{
    __asm {
        // 0003bd60
        push    esi
        // 0003bd61
        push    edi
        // 0003bd62
        mov     edi, dword ptr [esp + 0x0c]
        // 0003bd66
        mov     esi, ecx
        // 0003bd68
        cmp     esi, edi
        // 0003bd6a  JZ near (0F 84 rel32 — distance > 127)
        je      early_exit
        // 0003bd70
        push    ebx
        // 0003bd71
        mov     ebx, dword ptr [edi + 0x4]
        // 0003bd74
        test    ebx, ebx
        // 0003bd76
        push    ebp
        // 0003bd77
        je      do_throw
        // 0003bd79
        mov     ebp, dword ptr [edi + 0x8]
        // 0003bd7c
        mov     edx, ebp
        // 0003bd7e
        sub     edx, ebx
        // 0003bd80
        sar     edx, 0x2
        // 0003bd83
        jne     have_count

        // 0003bd85
    do_throw:
        call    FUN_00a2c790
        // 0003bd8a
        pop     ebp
        // 0003bd8b
        pop     ebx
        // 0003bd8c
        pop     edi
        // 0003bd8d
        mov     eax, esi
        // 0003bd8f
        pop     esi
        // 0003bd90
        ret     4

        // 0003bd93
    have_count:
        mov     eax, dword ptr [esi + 0x4]
        // 0003bd96
        test    eax, eax
        // 0003bd98
        jne     this_not_null
        // 0003bd9a
        xor     ecx, ecx
        // 0003bd9c
        jmp     got_this_count

        // 0003bd9e
    this_not_null:
        mov     ecx, dword ptr [esi + 0x8]
        // 0003bda1
        sub     ecx, eax
        // 0003bda3
        sar     ecx, 0x2

        // 0003bda6
    got_this_count:
        cmp     edx, ecx
        // 0003bda8
        ja      param_larger

        // 0003bdaa  — param_cnt <= this_cnt: overwrite in-place
        push    eax
        // 0003bdab
        push    ebp
        // 0003bdac
        push    ebx
        // 0003bdad
        call    FUN_007c46a0
        // 0003bdb2
        mov     eax, dword ptr [edi + 0x4]
        // 0003bdb5
        add     esp, 0x0c
        // 0003bdb8
        test    eax, eax
        // 0003bdba
        jne     param_not_null_after

        // 0003bdbc  — param->begin was null (alias guard): set this->end = this->begin
        mov     eax, dword ptr [esi + 0x4]
        // 0003bdbf
        pop     ebp
        // 0003bdc0
        xor     edi, edi
        // 0003bdc2
        lea     ecx, dword ptr [eax + edi*4]
        // 0003bdc5
        pop     ebx
        // 0003bdc6
        pop     edi
        // 0003bdc7
        mov     dword ptr [esi + 0x8], ecx
        // 0003bdca
        mov     eax, esi
        // 0003bdcc
        pop     esi
        // 0003bdcd
        ret     4

        // 0003bdd0  — normal path: recompute and store new end
    param_not_null_after:
        mov     edi, dword ptr [edi + 0x8]
        // 0003bdd3
        sub     edi, eax
        // 0003bdd5
        mov     eax, dword ptr [esi + 0x4]
        // 0003bdd8
        pop     ebp
        // 0003bdd9
        sar     edi, 0x2
        // 0003bddc
        lea     ecx, dword ptr [eax + edi*4]
        // 0003bddf
        pop     ebx
        // 0003bde0
        pop     edi
        // 0003bde1
        mov     dword ptr [esi + 0x8], ecx
        // 0003bde4
        mov     eax, esi
        // 0003bde6
        pop     esi
        // 0003bde7
        ret     4

        // 0003bdea  — param_cnt > this_cnt: check capacity
    param_larger:
        test    eax, eax
        // 0003bdec
        jne     has_capacity
        // 0003bdee
        xor     ecx, ecx
        // 0003bdf0
        jmp     got_capacity

        // 0003bdf2
    has_capacity:
        mov     ecx, dword ptr [esi + 0x0c]
        // 0003bdf5
        sub     ecx, eax
        // 0003bdf7
        sar     ecx, 0x2

        // 0003bdfa
    got_capacity:
        cmp     edx, ecx
        // 0003bdfc
        ja      need_realloc

        // 0003bdfe  — fits in capacity: compute old size for split copy
        test    eax, eax
        // 0003be00
        jne     has_size2
        // 0003be02
        xor     ecx, ecx
        // 0003be04
        jmp     got_size2

        // 0003be06
    has_size2:
        mov     ecx, dword ptr [esi + 0x8]
        // 0003be09
        sub     ecx, eax
        // 0003be0b
        sar     ecx, 0x2

        // 0003be0e
    got_size2:
        mov     edx, ebx
        // 0003be10
        push    eax
        // 0003be11
        lea     ebx, dword ptr [edx + ecx*4]
        // 0003be14
        push    ebx
        // 0003be15
        push    edx
        // 0003be16
        call    FUN_007c46a0
        // 0003be1b
        mov     edx, dword ptr [esi + 0x8]
        // 0003be1e
        mov     eax, dword ptr [edi + 0x8]
        // 0003be21
        add     esp, 0x0c
        // 0003be24
        push    edx
        // 0003be25
        push    eax
        // 0003be26
        push    ebx
        // 0003be27
        mov     ecx, esi
        // 0003be29
        call    FUN_00da8ea0
        // 0003be2e
        pop     ebp
        // 0003be2f
        pop     ebx
        // 0003be30
        mov     dword ptr [esi + 0x8], eax
        // 0003be33
        pop     edi
        // 0003be34
        mov     eax, esi
        // 0003be36
        pop     esi
        // 0003be37
        ret     4

        // 0003be3a  — need full reallocation
    need_realloc:
        test    eax, eax
        // 0003be3c
        je      no_free
        // 0003be3e
        mov     ecx, dword ptr [eax - 0x4]
        // 0003be41
        push    eax
        // 0003be42
        call    FUN_0040df70

        // 0003be47
    no_free:
        mov     ecx, dword ptr [edi + 0x4]
        // 0003be4a
        test    ecx, ecx
        // 0003be4c
        jne     has_param2
        // 0003be4e
        xor     eax, eax
        // 0003be50
        jmp     got_count2

        // 0003be52
    has_param2:
        mov     eax, dword ptr [edi + 0x8]
        // 0003be55
        sub     eax, ecx
        // 0003be57
        sar     eax, 0x2

        // 0003be5a
    got_count2:
        push    eax
        // 0003be5b
        mov     ecx, esi
        // 0003be5d
        call    FUN_00436580
        // 0003be62
        test    al, al
        // 0003be64
        je      common_end

        // 0003be66
        mov     ecx, dword ptr [esi + 0x4]
        // 0003be69
        mov     edx, dword ptr [edi + 0x8]
        // 0003be6c
        mov     eax, dword ptr [edi + 0x4]
        // 0003be6f
        push    ecx
        // 0003be70
        push    edx
        // 0003be71
        push    eax
        // 0003be72
        mov     ecx, esi
        // 0003be74
        call    FUN_00da8ea0
        // 0003be79
        mov     dword ptr [esi + 0x8], eax

        // 0003be7c
    common_end:
        pop     ebp
        // 0003be7d
        pop     ebx
        // 0003be7e  — shared tail for self-assign early exit AND common epilogue
    early_exit:
        pop     edi
        // 0003be7f
        mov     eax, esi
        // 0003be81
        pop     esi
        // 0003be82
        ret     4
    }
}

} // extern "C"
