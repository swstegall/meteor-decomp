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
// FUNCTION: ffxivgame 0x00046ef0 — buffer/string less-than operator
//                                  (45 bytes / 0x2d)
//
// Calling convention: __thiscall (ECX = this; one stack arg = other ptr;
//   callee cleans via `ret 4`).
// Returns: bool (byte in AL), true if this < other lexicographically.
//
// Layout (inferred from the asm):
//   struct Buf (this, ECX):
//     +0x00  char *data
//     +0x08  int   count
//   arg:
//     +0x00  char *data
//     +0x08  int   count
//
// Source shape:
//   bool operator<(const Buf *other) const {
//       int n = this->count < other->count ? this->count : other->count;
//       int cmp = FUN_00445b70(this->data, other->data, n);
//       return cmp < 0;
//   }
//
// Asm (45 bytes @ orig RVA 0x00046ef0):
//   8b 41 08          MOV EAX, [ECX+0x8]        ; this->count
//   56                PUSH ESI                   ; save ESI
//   8b 74 24 08       MOV ESI, [ESP+0x8]         ; ESI = other arg
//   8b 56 08          MOV EDX, [ESI+0x8]         ; EDX = other->count
//   3b c2             CMP EAX, EDX               ; this->count vs other->count
//   72 02             JC  skip                   ; skip if this->count < other->count
//   8b c2             MOV EAX, EDX               ; clamp to min
//   skip:
//   8b 09             MOV ECX, [ECX]             ; ECX = this->data
//   50                PUSH EAX                   ; arg3 = min count
//   8b 06             MOV EAX, [ESI]             ; EAX = other->data
//   50                PUSH EAX                   ; arg2 = other->data
//   51                PUSH ECX                   ; arg1 = this->data
//   e8 63 ec ff ff    CALL FUN_00445b70          ; strncmp-like compare
//   83 c4 0c          ADD ESP, 0xc               ; caller cleans (cdecl)
//   33 d2             XOR EDX, EDX
//   85 c0             TEST EAX, EAX
//   0f 9c c2          SETL DL                    ; DL = (cmp < 0)
//   8a c2             MOV AL, DL
//   5e                POP ESI
//   c2 04 00          RET 0x4
//
// Reconstruction: __declspec(naked) inline asm — pins the register
// schedule (EAX load before PUSH ESI, ESI as arg holder, ECX clobbered
// for this->data before CALL) and the SETL/MOV AL,DL epilogue.

extern "C" int __cdecl FUN_00445b70(const char *s1, const char *s2, int n);

extern "C" __declspec(naked) void FUN_00446ef0() {
    __asm {
        mov  eax, dword ptr [ecx + 8]
        push esi
        mov  esi, dword ptr [esp + 8]
        mov  edx, dword ptr [esi + 8]
        cmp  eax, edx
        jc   skip
        mov  eax, edx
    skip:
        mov  ecx, dword ptr [ecx]
        push eax
        mov  eax, dword ptr [esi]
        push eax
        push ecx
        call FUN_00445b70
        add  esp, 12
        xor  edx, edx
        test eax, eax
        setl dl
        mov  al, dl
        pop  esi
        ret  4
    }
}
