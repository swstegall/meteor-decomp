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
// FUNCTION: ffxivgame 0x00459a10 — wstring-like iterator-range helper
//                                  (195 B / 0xc3, __thiscall, RET 0x14)
//
// Layout of *this (wchar_t basic_string variant, SSO threshold 8):
//   [+0x00]  vtable or padding (4 bytes)
//   [+0x04]  union { wchar_t[8] _Buf; wchar_t* _Ptr; }  (16 bytes)
//   [+0x14]  size_t _Mysize
//   [+0x18]  size_t _Myres   (capacity; < 8 → SSO/inline mode)
//
// Arguments (5 stack args, callee-cleans via RET 0x14):
//   ECX        = this
//   [+4]  arg1 = result/output object pointer (returned in EAX)
//   [+8]  arg2 = owning-container ref for first iterator
//                (must be -2 or == this; validated if arg3 != 0)
//   [+12] arg3 = first pointer/iterator (wchar_t* into this->data, or 0)
//   [+16] arg4 = owning-container ref for second iterator
//                (must be -2 or == arg2; validated if arg5 != 0)
//   [+20] arg5 = last pointer/iterator (wchar_t* into this->data, or 0)
//
// What the function does:
//   1. Derive this->data() (EDI) with SSO/heap branch; null-check + bounds-
//      check that EDI lies within [data, data+size*2].
//   2. If arg3 != 0: validate arg2 ∈ {-2, this}; compute ESI = (arg3−data)/2.
//      If arg3 == 0: ESI = 0.
//   3. If arg5 != 0: validate arg4 ∈ {-2, arg2}; compute EDI = (arg5−arg3)/2.
//      If arg5 == 0: EDI = 0.
//   4. Call this->FUN_00440600(ESI, EDI)   [position, count].
//   5. Recompute data pointer; call arg1->FUN_00449600(data+ESI*2, this).
//   6. Return arg1.
//
// Three CALL sites to FUN_009d22b4 (assertion/error, noreturn) are
// rel32 relocations masked by compare.py — emitted verbatim below.

extern "C" {
    void FUN_009d22b4();   // noreturn assertion helper
    void FUN_00440600();   // wstring method(pos, count) — __thiscall RET 8
    void FUN_00449600();   // assign-like method         — __thiscall RET 8
}

extern "C" __declspec(naked) void FUN_00459a10()
{
    __asm {
        push    ebx
        mov     ebx, ecx
        mov     ecx, dword ptr [ebx + 0x18]
        cmp     ecx, 8
        push    esi
        push    edi
        jb      short Lsso_edi
        mov     edi, dword ptr [ebx + 0x4]
        jmp     short Ledi_done
    Lsso_edi:
        lea     edi, [ebx + 0x4]
    Ledi_done:
        test    edi, edi
        jz      short Lcall_error1
        cmp     ecx, 8
        lea     edx, [ebx + 0x4]
        jb      short Lsso_eax1
        mov     eax, dword ptr [edx]
        jmp     short Leax1_done
    Lsso_eax1:
        mov     eax, edx
    Leax1_done:
        cmp     eax, edi
        ja      short Lcall_error1
        cmp     ecx, 8
        jb      short Lsso_eax2
        mov     eax, dword ptr [edx]
        jmp     short Leax2_done
    Lsso_eax2:
        mov     eax, edx
    Leax2_done:
        mov     ecx, dword ptr [ebx + 0x14]
        lea     edx, [eax + ecx*2]
        cmp     edi, edx
        jbe     short Lbounds_ok
    Lcall_error1:
        call    FUN_009d22b4
    Lbounds_ok:
        cmp     dword ptr [esp + 0x18], 0
        push    ebp
        mov     ebp, dword ptr [esp + 0x18]
        jnz     short Lcheck_arg2
        xor     esi, esi
        jmp     short Lskip_arg2
    Lcheck_arg2:
        cmp     ebp, -2
        jz      short Larg2_ok
        test    ebp, ebp
        jz      short Lcall_error2
        cmp     ebp, ebx
        jz      short Larg2_ok
    Lcall_error2:
        call    FUN_009d22b4
    Larg2_ok:
        mov     esi, dword ptr [esp + 0x1c]
        sub     esi, edi
        sar     esi, 1
    Lskip_arg2:
        mov     edi, dword ptr [esp + 0x24]
        test    edi, edi
        jz      short Lskip_arg4
        mov     eax, dword ptr [esp + 0x20]
        cmp     eax, -2
        jz      short Larg4_ok
        test    eax, eax
        jz      short Lcall_error3
        cmp     eax, ebp
        jz      short Larg4_ok
    Lcall_error3:
        call    FUN_009d22b4
    Larg4_ok:
        sub     edi, dword ptr [esp + 0x1c]
        sar     edi, 1
    Lskip_arg4:
        push    edi
        push    esi
        mov     ecx, ebx
        call    FUN_00440600
        cmp     dword ptr [ebx + 0x18], 8
        pop     ebp
        jb      short Lsso_result
        mov     eax, dword ptr [ebx + 0x4]
        jmp     short Lresult_done
    Lsso_result:
        lea     eax, [ebx + 0x4]
    Lresult_done:
        lea     eax, [eax + esi*2]
        mov     esi, dword ptr [esp + 0x10]
        push    ebx
        push    eax
        mov     ecx, esi
        call    FUN_00449600
        pop     edi
        mov     eax, esi
        pop     esi
        pop     ebx
        ret     0x14
    }
}
