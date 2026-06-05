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
// FUNCTION: ffxivgame 0x00047770 — null-terminated DWORD array accumulate
//                                  then resize buffer  (75 B / 0x4B, ret 4)
//
// __thiscall void(int *arr):
//   Walks a null-terminated DWORD array, calling FUN_00445670(elem, 0) on
//   each non-zero element and summing the return values into `total`.
//   After the walk, calls FUN_00447010(this, total+1, 1) (the growable-buffer
//   Resize helper — __thiscall, RET 8) and then FUN_004457f0(arr, this->data).
//
// Object layout (only [this+0x00] is touched):
//   [this + 0x00]  void *data;   — backing storage pointer passed to FUN_004457f0
//
// Calling convention: __thiscall — ECX = this, one stack arg `arr`;
//   callee cleans 4 bytes (RET 4).
//
// Branch shape:
//   +0x12  JZ  +0x16  →  after_loop  (arr[0] == 0: skip loop entirely)
//   +0x28  JNZ −0x16  →  loop_top    (next element non-null: continue loop)
//
// CALL targets (all REL32, wildcarded by tools/compare.py):
//   +0x17  CALL FUN_00445670   __cdecl(int elem, int 0) → int
//   +0x32  CALL FUN_00447010   __thiscall Resize(unsigned newSize, char flag); RET 8
//   +0x3c  CALL FUN_004457f0   __cdecl(int *arr, void *data)

extern "C" {
    int FUN_00445670(int, int);
    void FUN_00447010();
    void FUN_004457f0(int *, void *);
}

extern "C" __declspec(naked) void FUN_00447770() {
    __asm {
        push    ebx
        mov     ebx, dword ptr [esp + 0x8]
        mov     eax, dword ptr [ebx]
        push    ebp
        push    esi
        push    edi
        xor     edi, edi
        test    eax, eax
        mov     ebp, ecx
        mov     esi, ebx
        jz      after_loop
    loop_top:
        push    0
        push    eax
        call    FUN_00445670
        add     esi, 4
        add     edi, eax
        mov     eax, dword ptr [esi]
        add     esp, 8
        test    eax, eax
        jnz     loop_top
    after_loop:
        push    1
        add     edi, 1
        push    edi
        mov     ecx, ebp
        call    FUN_00447010
        mov     eax, dword ptr [ebp]
        push    eax
        push    ebx
        call    FUN_004457f0
        add     esp, 8
        pop     edi
        pop     esi
        pop     ebp
        pop     ebx
        ret     4
    }
}
