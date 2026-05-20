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
// FUNCTION: ffxivgame 0x00003fd0 — wide-char SSO string `_Tidy/clear`
//   companion to the FUN_00404000 SSO scalar-deleting-dtor.
//
//   This function frees the heap-allocated wide buffer (if any) and
//   resets the string back to its empty SSO state. Field layout, read
//   from the sibling 0x00404000 (narrow-char variant) plus the
//   16-bit-wide write here (`mov word ptr [esi+0x4], ax`):
//
//     +0x04  wchar_t[8]  inline buffer  (or `wchar_t*` heap pointer
//                                        once capacity grows past 7)
//     +0x14  size_t       Mysize  (current length in wchar_ts)
//     +0x18  size_t       Myres   (capacity; SSO threshold = 7)
//
//   Behaviour:
//
//     __thiscall void FUN_00403fd0(this):
//       if (this->Myres >= 8)                          ; heap path
//           FUN_0044d350(this->buf,                     ; heap pointer
//                        this->Myres * 2 + 2,            ; byte count
//                        0xc);                           ; pool/tag
//       this->Myres = 7;            ; SSO capacity
//       this->Mysize = 0;
//       this->buf[0] = L'\0';       ; 16-bit zero-write — confirms wchar_t
//
//   FUN_0044d350 is the 3-arg pool free used throughout ffxivgame for
//   sized deallocation (ptr, byte_count, pool_id). The byte count is
//   `(capacity + 1) * sizeof(wchar_t)` to include the NUL terminator.
//
// Asm (48 bytes @ 0x00003fd0):
//
//   56                     PUSH ESI
//   8b f1                  MOV  ESI, ECX
//   8b 46 18               MOV  EAX, dword ptr [ESI+0x18]   ; capacity
//   83 f8 08               CMP  EAX, 0x8
//   72 13                  JB   .skip_free                  ; cap < 8 -> SSO
//   8b 4e 04               MOV  ECX, dword ptr [ESI+0x4]    ; heap ptr
//   6a 0c                  PUSH 0xc
//   8d 44 00 02            LEA  EAX, [EAX + EAX + 2]        ; bytes
//   50                     PUSH EAX
//   51                     PUSH ECX
//   e8 RR RR RR RR         CALL FUN_0044d350                ; pool free
//   83 c4 0c               ADD  ESP, 0xc                    ; cdecl cleanup
// .skip_free:
//   33 c0                  XOR  EAX, EAX
//   c7 46 18 07 00 00 00   MOV  dword ptr [ESI+0x18], 0x7
//   89 46 14               MOV  dword ptr [ESI+0x14], EAX
//   66 89 46 04             MOV  word  ptr [ESI+0x4],  AX
//   5e                     POP  ESI
//   c3                     RET
//
// Naked-asm rationale (same as the FUN_00404000 narrow-char sibling):
// MSVC 2005's instruction scheduler is what places the `xor eax, eax`
// before the imm32 store at `[esi+0x18]` so the eax = 0 value is then
// available for the two subsequent zero stores. Coaxing exactly that
// ordering out of source-level C++ under /O2 is fragile. Naked asm
// pins the layout; the single rel32 CALL is masked as a relocation by
// tools/compare.py.

extern "C" void __cdecl FUN_0044d350(void *ptr, unsigned int bytes, unsigned int pool);

extern "C" __declspec(naked) void FUN_00403fd0() {
    __asm {
        push    esi
        mov     esi, ecx
        mov     eax, dword ptr [esi + 0x18]
        cmp     eax, 8
        jb      skip_free
        mov     ecx, dword ptr [esi + 0x4]
        push    0xc
        lea     eax, [eax + eax + 2]
        push    eax
        push    ecx
        call    FUN_0044d350
        add     esp, 0xc
    skip_free:
        xor     eax, eax
        mov     dword ptr [esi + 0x18], 7
        mov     dword ptr [esi + 0x14], eax
        mov     word  ptr [esi + 0x4], ax
        pop     esi
        ret
    }
}
