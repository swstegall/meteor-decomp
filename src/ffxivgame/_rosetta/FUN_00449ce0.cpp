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
// FUNCTION: ffxivgame 0x00049ce0 — __thiscall SSO-container erase helper
//                                   (147 B / 0x93, ret 8)
//
// __thiscall void* Erase(this, unsigned int pos, unsigned int count):
//   ECX = this (SSO string/vector object);  two stack args; ret 8.
//
// Object layout (inferred from asm offsets):
//   [this + 0x04]  void* _Ptr / char _Buf[]  — heap ptr (large) or inline buf (small)
//   [this + 0x14]  unsigned int _Mysize       — element count
//   [this + 0x18]  unsigned int _Myres        — capacity; _Myres < 4 → SSO inline mode
//
// Element size: 4 bytes (DWORD).  SSO threshold: _Myres < 4.
//
// Behaviour:
//   1. Assert pos <= _Mysize  (else call FUN_009d046d — error handler).
//   2. count = min(count, _Mysize - pos)  via CMOVB (carry-set conditional move).
//   3. If count > 0:
//        a. Resolve data pointer twice (heap ptr or &_Buf) into param slots.
//        b. Call FUN_004494d0(src, cap-pos, &data[pos+count], remaining) to
//           shift the tail left by `count` elements.
//        c. _Mysize -= count;  write 0 at new end (null-terminate).
//   4. Return this in EAX.
//
// CALL targets (REL32 — compare.py masks these 4-byte displacement windows):
//   +0x0f  CALL FUN_009d046d  — out-of-range error handler (noreturn)
//   +0x6a  CALL FUN_004494d0  — element-shift/copy helper (__cdecl, 4 args)
//
// Reconstruction strategy — __declspec(naked) inline asm:
//   Source-level C++ cannot reproduce the CMOVB for the min(), the
//   PUSH EBP deferred until inside the count>0 branch, the double SSO
//   pointer-resolve with [ESP+0x14]/[ESP+0x18] parameter-slot reuse,
//   or the c7 44 85 00 ... null-terminate via [EBP+EAX*4+0] SIB form.
//   The naked asm body is the canonical byte-passthrough approach.

extern "C" {
    int FUN_009d046d();  // out-of-range error handler (noreturn)
    int FUN_004494d0();  // element-shift helper (__cdecl, 4 args)
}

extern "C" __declspec(naked) void* FUN_00449ce0() {
    __asm {
        push    ebx
        mov     ebx, dword ptr [esp + 0x8]        ; EBX = pos (param1)
        push    esi
        mov     esi, ecx                           ; ESI = this
        cmp     dword ptr [esi + 0x14], ebx        ; _Mysize >= pos?
        push    edi
        jnc     short pos_ok
        call    FUN_009d046d                       ; error: pos > _Mysize

    pos_ok:
        mov     eax, dword ptr [esi + 0x14]       ; EAX = _Mysize
        mov     edi, dword ptr [esp + 0x14]       ; EDI = count (param2)
        sub     eax, ebx                           ; EAX = _Mysize - pos (available)
        cmp     eax, edi                           ; available vs count
        cmovb   edi, eax                           ; EDI = min(count, available)
        test    edi, edi
        jbe     short early_out                    ; count == 0 → skip entire body

        mov     ecx, dword ptr [esi + 0x18]       ; ECX = _Myres (capacity)
        cmp     ecx, 0x4
        push    ebp
        lea     ebp, [esi + 0x4]                  ; EBP = &_Bx (ptr field or inline buf)

        jc      short small_1                      ; _Myres < 4 → inline mode
        mov     edx, dword ptr [ebp]               ; EDX = heap data pointer
        mov     dword ptr [esp + 0x14], edx        ; save into param slot
        jmp     short end_1
    small_1:
        mov     dword ptr [esp + 0x14], ebp        ; save inline buf address

    end_1:
        cmp     ecx, 0x4
        jc      short small_2
        mov     edx, dword ptr [ebp]               ; EDX = heap data pointer (second copy)
        mov     dword ptr [esp + 0x18], edx
        jmp     short end_2
    small_2:
        mov     dword ptr [esp + 0x18], ebp

    end_2:
        mov     edx, dword ptr [esp + 0x14]       ; EDX = first data ptr
        sub     eax, edi                           ; EAX = remaining = available - count
        push    eax                                ; arg4: remaining elements
        lea     eax, [ebx + edi]                   ; EAX = pos + count (end of erased range)
        lea     eax, [edx + eax*4]                 ; EAX = &data[pos+count] (move source)
        push    eax                                ; arg3: move source ptr
        sub     ecx, ebx                           ; ECX = _Myres - pos
        push    ecx                                ; arg2: cap - pos
        mov     ecx, dword ptr [esp + 0x24]       ; ECX = second data ptr (from param2 slot)
        lea     edx, [ecx + ebx*4]                ; EDX = &data[pos] (move destination)
        push    edx                                ; arg1: move destination ptr
        call    FUN_004494d0

        mov     eax, dword ptr [esi + 0x14]       ; EAX = _Mysize
        sub     eax, edi                           ; EAX = new size = _Mysize - count
        add     esp, 0x10                          ; clean 4 cdecl args
        cmp     dword ptr [esi + 0x18], 0x4
        mov     dword ptr [esi + 0x14], eax        ; write new _Mysize
        jc      short already_inline
        mov     ebp, dword ptr [ebp]               ; EBP = heap ptr (large mode)

    already_inline:
        mov     dword ptr [ebp + eax*4], 0         ; null-terminate at new end
        pop     ebp

    early_out:
        pop     edi
        mov     eax, esi                           ; return this
        pop     esi
        pop     ebx
        ret     8
    }
}
