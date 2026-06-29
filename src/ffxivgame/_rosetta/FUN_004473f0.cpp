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
// FUNCTION: ffxivgame 0x004473f0 — growable-buffer constructor-from-string
//                                  (__thiscall, 1 stack arg, ret 4, 94 bytes).
//
// __thiscall Buffer* FUN_004473f0(this, SrcString *src):
//   ECX = this (destination buffer); one stack arg = src string pointer.
//   Callee cleans 4 bytes (RET 0x4).
//
// Behaviour:
//   Initialises *this as an empty growable-buffer object, then resizes it to
//   hold src->size + 1 bytes, then copies the src string data (including the
//   null terminator) into the freshly-allocated storage. Returns this in EAX.
//
//   Object layout set up on *this:
//     [this + 0x00]  void  *data;       initially &this->inlineBuf (this+0x12)
//     [this + 0x04]  uint32 capacity;   initialised to 0x40
//     [this + 0x08]  uint32 size;       initialised to 1 (set to src->size+1 by Resize)
//     [this + 0x0c]  uint32 reserved;   initialised to 0
//     [this + 0x10]  char   flagA;      initialised to 1
//     [this + 0x11]  char   flagB;      initialised to 1 (ownership flag)
//     [this + 0x12]  char   inlineBuf[]; starts with NUL
//
//   Source string layout (arg1):
//     [src + 0x04]  union { char inlineBuf[]; char *ptr; }  (SSO union)
//     [src + 0x14]  uint32 size
//     [src + 0x18]  uint32 capacity    (< 0x10 → use inline buf at &src+0x4,
//                                        >= 0x10 → use heap ptr at src->ptr)
//
// Step-by-step (from asm @ RVA 0x000473f0):
//   1. Save EBX/ESI/EDI; load src → EBX; save this → ESI.
//   2. EDI = src->size + 1         (newSize for the copy including NUL)
//   3. Set up *this fields; data = &inlineBuf; inlineBuf[0] = '\0'.
//   4. call FUN_00447010(newSize=EDI, clearFlag=1)   (Resize)
//   5. Resolve src data pointer: heap or inline (SSO check on src->capacity).
//   6. memcpy(this->data, srcPtr, newSize)
//   7. Return this in EAX.
//
// CALL targets (REL32 wildcarded by tools/compare.py):
//   +0x36   CALL FUN_00447010  — __thiscall Resize(newSize, clearFlag)
//   +0x4e   CALL memcpy        — CRT memcpy (VA 0x009d4600)

extern "C" {
    void FUN_00447010();
    void crt_memcpy();   // alias for CRT memcpy at VA 0x009d4600 (rel32 wildcarded)
}

extern "C" __declspec(naked) void FUN_004473f0() {
    __asm {
        push    ebx
        mov     ebx, dword ptr [esp + 0x8]
        push    esi
        mov     esi, ecx
        push    edi
        mov     edi, dword ptr [ebx + 0x14]
        mov     ecx, 1
        add     edi, ecx
        lea     eax, [esi + 0x12]
        push    ecx
        mov     byte ptr [esi + 0x10], cl
        mov     byte ptr [esi + 0x11], cl
        mov     dword ptr [esi + 0x8], ecx
        mov     dword ptr [esi + 0xc], 0
        mov     dword ptr [esi + 0x4], 0x40
        mov     dword ptr [esi], eax
        push    edi
        mov     ecx, esi
        mov     byte ptr [eax], 0
        call    FUN_00447010
        cmp     dword ptr [ebx + 0x18], 0x10
        jc      L_short_buf
        mov     ebx, dword ptr [ebx + 0x4]
        jmp     L_do_copy
    L_short_buf:
        add     ebx, 0x4
    L_do_copy:
        mov     eax, dword ptr [esi]
        push    edi
        push    ebx
        push    eax
        call    crt_memcpy
        add     esp, 0xc
        pop     edi
        mov     eax, esi
        pop     esi
        pop     ebx
        ret     0x4
    }
}
