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
// FUNCTION: ffxivgame 0x004089f0 — `std::_Median` Tukey-ninther pivot
//                                  selection (142 B / 0x8e), iterator
//                                  over a 64-byte element type
//
// MSVC 2005 STL `<algorithm>` helper (instantiated for an iterator
// whose pointee is 64 bytes):
//
//     template<class _RanIt>
//     void _Median(_RanIt _First, _RanIt _Mid, _RanIt _Last)
//     {
//         if (40 < _Last - _First) {
//             auto _Step = (_Last - _First + 1) / 8;
//             _Med3(_First,            _First + _Step,   _First + 2*_Step);
//             _Med3(_Mid - _Step,      _Mid,             _Mid + _Step);
//             _Med3(_Last - 2*_Step,   _Last - _Step,    _Last);
//             _Med3(_First + _Step,    _Mid,             _Last - _Step);
//         } else
//             _Med3(_First, _Mid, _Last);
//     }
//
// The 64-byte element size shows up in two places in the codegen:
//   • `SAR EAX, 6`   — pointer-difference → element count (sizeof=64).
//   • `SHL EAX, 6`   — element count × 64 → byte offset, used to scale
//     the LEAs and SUBs that build the four trio addresses.
//
// Sibling `_Med3` (FUN_004087f0, 284 B) is the 3-element median-of-3
// swap over the same 64-byte record; it runs three string-compare-then-
// `MOVSD.REP ×16`-swap rounds (16 dwords = 64 bytes per record).
//
// MSVC 2005 codegen quirk reproduced verbatim — the small (else) path
// only needs ESI live, so MSVC pushes only ESI in the prologue and
// delays the EBX/EBP/EDI saves until *inside* the `if` branch after
// the JLE. A source-level translation would push all four callee-saves
// in a single prologue and pop them in a single epilogue (the layout
// FUN_004087f0 itself uses), so reproducing the delayed-save shape
// requires `__declspec(naked)` with explicit inline asm.
//
// Reloc-bearing sites: the four `CALL FUN_004087f0` instructions emit
// four rel32 fixups (at +0x3c, +0x4f, +0x61, +0x71). The CALL opcode
// byte (`e8`) is real code and must match; the 4-byte displacement
// after each is what the linker fills in. `tools/compare.py` masks
// those four 4-byte windows out of the byte-level diff.

extern "C" int FUN_004087f0();

extern "C" __declspec(naked) void FUN_004089f0() {
    __asm {
        mov     ecx, [esp + 0x4]            ; ecx = _First
        push    esi
        mov     esi, [esp + 0x10]           ; esi = _Last
        mov     eax, esi
        sub     eax, ecx
        sar     eax, 6                      ; eax = (_Last - _First) [elt count]
        cmp     eax, 0x28
        jle     small_path
        add     eax, 1
        cdq
        push    ebx                         ; delayed callee-save: big path only
        and     edx, 7
        add     eax, edx
        push    ebp
        push    edi
        sar     eax, 3                      ; eax = _Step = (n+1) / 8
        mov     edi, eax
        shl     eax, 6                      ; eax = _Step * 64 (bytes)
        shl     edi, 7                      ; edi = _Step * 128 (bytes)
        mov     ebx, eax                    ; ebx = _Step*64 cached for reuse
        lea     edx, [edi + ecx]            ; edx = _First + 2*_Step
        lea     eax, [ebx + ecx]            ; eax = _First + _Step
        push    edx
        push    eax
        push    ecx
        mov     [esp + 0x20], eax           ; stash _First+_Step (overwrites caller's _First slot)
        call    FUN_004087f0                ; _Med3(_First, _First+_Step, _First+2*_Step)
        mov     ebp, [esp + 0x24]           ; ebp = _Mid
        lea     eax, [ebx + ebp]            ; eax = _Mid + _Step
        push    eax
        mov     ecx, ebp
        sub     ecx, ebx                    ; ecx = _Mid - _Step
        push    ebp
        push    ecx
        call    FUN_004087f0                ; _Med3(_Mid-_Step, _Mid, _Mid+_Step)
        mov     eax, esi
        sub     eax, ebx                    ; eax = _Last - _Step
        push    esi
        push    eax
        sub     esi, edi                    ; esi = _Last - 2*_Step
        push    esi
        mov     [esp + 0x40], eax           ; stash _Last-_Step (overwrites caller's _Last slot)
        call    FUN_004087f0                ; _Med3(_Last-2*_Step, _Last-_Step, _Last)
        mov     edx, [esp + 0x40]           ; edx = _Last - _Step
        mov     eax, [esp + 0x38]           ; eax = _First + _Step
        push    edx
        push    ebp
        push    eax
        call    FUN_004087f0                ; _Med3(_First+_Step, _Mid, _Last-_Step)
        add     esp, 0x30                   ; pop 4 calls × 3 args × 4 bytes
        pop     edi
        pop     ebp
        pop     ebx
        pop     esi
        ret
    small_path:
        mov     edx, [esp + 0xc]            ; edx = _Mid (param_2)
        push    esi
        push    edx
        push    ecx
        call    FUN_004087f0                ; _Med3(_First, _Mid, _Last)
        add     esp, 0xc
        pop     esi
        ret
    }
}
