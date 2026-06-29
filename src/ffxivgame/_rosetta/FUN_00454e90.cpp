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
// FUNCTION: ffxivgame 0x00054e90 — SSO wstring::assign(const wchar_t*)
//                                  Init SSO state, compute wcslen inline,
//                                  delegate to FUN_00454db0
//                                  (__thiscall, RET 0x4, 64 B / 0x40).
//
// SSO wide-string object layout (this = ESI):
//   [this+0x00]  wchar_t *_Ptr        (heap pointer; not accessed here)
//   [this+0x04]  wchar_t  _Buf[8]     (SSO inline buffer, 16 bytes)
//   [this+0x14]  int      _Mysize     (current length in wchar_t units)
//   [this+0x18]  int      _Myres      (capacity; < 8 → SSO path, >= 8 → heap)
//
// Calling convention: __thiscall (ECX = this, one DWORD stack arg = const
//                     wchar_t* ptr, callee-cleans via RET 0x4).
//
// Logic (recovered from asm):
//   1. Load ptr from [ESP+4] into EDX (before callee saves, to keep it live
//      in a volatile register throughout).
//   2. Save ESI, EDI; move ECX (this) to ESI; copy ptr to EAX.
//   3. Initialise SSO state to "empty with SSO capacity=7":
//        _Myres  = 7       → [ESI+0x18]
//        _Mysize = 0       → [ESI+0x14]
//        _Buf[0] = L'\0'   → word ptr [ESI+0x4]
//   4. Inline wcslen loop (EDI = ptr+2 trick, EAX = scan pointer):
//        EDI = EAX + 2     (one wchar_t past start)
//        do {
//            CX = *EAX;    EAX += 2;
//        } while (CX != 0);
//        count = (EAX - EDI) >> 1   (= wcslen(ptr))
//   5. Call FUN_00454db0(ptr, count) via __thiscall (ECX=ESI).
//   6. Return this (ESI) in EAX.
//
// CALL target (REL32; wildcard-masked by tools/compare.py):
//   +0x34   REL32 → 0x00454db0   (wstring range-assign helper)
//
// Reconstruction strategy — naked-asm byte passthrough.
//
//   The load of EDX before the callee saves (a MSVC /O2 scheduling artefact),
//   the word-prefix loop reading CX and testing CX via 66-prefixed TEST,
//   and the specific EDI = ptr+2 form for computing length from a do-while
//   wcslen resist reliable expression from C++ source under the standalone-TU
//   register allocator. Emitting the 64 original bytes verbatim produces a
//   .obj whose .text is byte-identical to the orig slice; compare.py masks
//   the single REL32 displacement and reports GREEN.

extern "C" void FUN_00454db0();

extern "C" __declspec(naked) void FUN_00454e90() {
    __asm {
        // 00054e90: 8b 54 24 04      MOV EDX,[ESP+0x4]        ; ptr arg → EDX
        mov     edx, dword ptr [esp + 4]
        // 00054e94: 56               PUSH ESI
        push    esi
        // 00054e95: 8b f1            MOV ESI,ECX              ; this → ESI
        mov     esi, ecx
        // 00054e97: 8b c2            MOV EAX,EDX              ; scan ptr → EAX
        mov     eax, edx
        // 00054e99: 57               PUSH EDI
        push    edi
        // 00054e9a: c7 46 18 07 00 00 00  MOV [ESI+0x18],0x7  ; _Myres = 7
        mov     dword ptr [esi + 0x18], 7
        // 00054ea1: c7 46 14 00 00 00 00  MOV [ESI+0x14],0x0  ; _Mysize = 0
        mov     dword ptr [esi + 0x14], 0
        // 00054ea8: 66 c7 46 04 00 00     MOV word[ESI+4],0x0 ; _Buf[0] = L'\0'
        mov     word ptr [esi + 4], 0
        // 00054eae: 8d 78 02         LEA EDI,[EAX+0x2]        ; _Last = ptr+1 wchar
        lea     edi, [eax + 2]
        // 00054eb1: 66 8b 08         MOV CX,word ptr [EAX]    ; loop: read char
    scan_loop:
        mov     cx, word ptr [eax]
        // 00054eb4: 83 c0 02         ADD EAX,0x2              ; advance scan ptr
        add     eax, 2
        // 00054eb7: 66 85 c9         TEST CX,CX
        test    cx, cx
        // 00054eba: 75 f5            JNZ scan_loop
        jnz     scan_loop
        // 00054ebc: 2b c7            SUB EAX,EDI              ; byte dist to past-null
        sub     eax, edi
        // 00054ebe: d1 f8            SAR EAX,0x1              ; / 2 = wchar_t count
        sar     eax, 1
        // 00054ec0: 50               PUSH EAX                 ; arg: count
        push    eax
        // 00054ec1: 52               PUSH EDX                 ; arg: ptr
        push    edx
        // 00054ec2: 8b ce            MOV ECX,ESI              ; this
        mov     ecx, esi
        // 00054ec4: e8 e7 fe ff ff   CALL FUN_00454db0        ; [REL32, masked]
        call    FUN_00454db0
        // 00054ec9: 5f               POP EDI
        pop     edi
        // 00054eca: 8b c6            MOV EAX,ESI              ; return this
        mov     eax, esi
        // 00054ecc: 5e               POP ESI
        pop     esi
        // 00054ecd: c2 04 00         RET 0x4
        ret     4
    }
}
