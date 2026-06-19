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
// FUNCTION: ffxivgame 0x00047d40 — `__thiscall` buffer-append helper
//                                  appends `count` bytes from `src` into
//                                  a growable byte-buffer object (62 B).
//
// __thiscall BufClass* FUN_00447d40(this, const char* src, int count)
//   ECX        : this
//   [ESP+0x04] : const char*  src    (source data)
//   [ESP+0x08] : int          count  (byte count to append)
//   Returns: this in EAX.
//   Callee cleans 8 bytes via `ret 0x8`.
//
// Object layout (offsets touched, matching FUN_00447010's comments):
//   this+0x00  char*        data      — pointer to character buffer
//   this+0x04  unsigned     capacity  — allocated bytes (32-aligned)
//   this+0x08  unsigned     size      — current element/byte count (incl. null)
//
// Source shape (inferred):
//
//   BufClass* Append(const char* src, int count) {
//       int oldSize = this->size;
//       this->Resize(oldSize + count, 1);        // FUN_00447010 (__thiscall, ret 8)
//       memcpy(this->data + oldSize - 1, src, count);  // FUN_009d5110 (__cdecl)
//       this->data[this->size - 1] = '\0';       // null-terminate at new end
//       return this;
//   }
//
// FUN_00447010 is __thiscall (ECX=this, `ret 8` cleans 2 DWORD stack args).
// ECX still holds `this` at the call site since no instruction clobbers it
// between `mov esi,ecx` and the call. Callee-saves EBX (count), EDI (oldSize),
// ESI (this) to preserve them across both calls.
//
// After FUN_00447010 returns, this->data may have been reallocated: the
// destination for the memcpy uses the OLD size (EDI) with the NEW data
// pointer (re-read from [ESI]). The null terminator uses both fields
// re-read from the object after the memcpy.
//
// Calling convention: __thiscall (ECX = this; 2 DWORD stack args;
// callee cleans 0x8 via `ret 0x8`). Returns `this` in EAX.
//
// Callee-saved: EBX (count/param_2), ESI (this), EDI (old this->size).
//
// CALL targets (REL32, wildcarded by tools/compare.py):
//   +0x12  CALL FUN_00447010  — __thiscall Resize(newSize, flag)
//   +0x24  CALL FUN_009d5110  — __cdecl memcpy(dst, src, n)

extern "C" {
    int FUN_00447010();     // __thiscall Resize helper  (ECX=this, ret 8)
    int FUN_009d5110();     // __cdecl memcpy            (3 args, caller cleans)
}

extern "C" __declspec(naked) void FUN_00447d40() {
    __asm {
        push    ebx
        mov     ebx, dword ptr [esp + 0xc]
        push    esi
        push    edi
        mov     esi, ecx
        mov     edi, dword ptr [esi + 0x8]
        push    1
        lea     eax, [edi + ebx]
        push    eax
        call    FUN_00447010
        mov     ecx, dword ptr [esp + 0x10]
        mov     edx, dword ptr [esi]
        push    ebx
        push    ecx
        lea     eax, [edx + edi - 1]
        push    eax
        call    FUN_009d5110
        mov     ecx, dword ptr [esi]
        mov     edx, dword ptr [esi + 0x8]
        add     esp, 0xc
        pop     edi
        mov     eax, esi
        pop     esi
        mov     byte ptr [ecx + edx - 1], 0x0
        pop     ebx
        ret     0x8
    }
}
