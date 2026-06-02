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
// FUNCTION: ffxivgame 0x00412b20 — __thiscall conditional virtual dispatch
//           (two JMP-EDX tail-calls, one null-return fallthrough; 37 bytes)
//
// ECX = this (pointer into a sub-object; real base is ECX - 4)
//
// Asm (37 bytes, no relocations):
//   8d 41 fc          LEA EAX, [ECX - 4]          ; base = this - 4
//   39 40 28          CMP [EAX + 0x28], EAX        ; base->field_28 == base?
//   75 0a             JNZ  skip1
//   8b 49 14          MOV ECX, [ECX + 0x14]        ; ecx = this->field_14
//   8b 01             MOV EAX, [ECX]               ; eax = vtable ptr
//   8b 50 04          MOV EDX, [EAX + 4]           ; edx = vtable[1]
//   ff e2             JMP EDX                      ; tail-dispatch slot 1
// skip1:
//   80 79 1d 00       CMP byte ptr [ECX + 0x1d], 0 ; this->field_1d != 0?
//   74 0a             JZ   skip2
//   8b 49 18          MOV ECX, [ECX + 0x18]        ; ecx = this->field_18
//   8b 01             MOV EAX, [ECX]               ; eax = vtable ptr
//   8b 50 04          MOV EDX, [EAX + 4]           ; edx = vtable[1]
//   ff e2             JMP EDX                      ; tail-dispatch slot 1
// skip2:
//   33 c0             XOR EAX, EAX
//   c3                RET
//
// Logic: if the sub-object's self-pointer field (base->field_28 == base,
// a sentinel/list-head check) is set, tail-call this->field_14's vtable[1].
// Otherwise if the byte flag at this->field_1d is non-zero, tail-call
// this->field_18's vtable[1]. Otherwise return null.
// No stack arguments (plain RET → __thiscall with no args).

extern "C" __declspec(naked) void FUN_00412b20() {
    __asm {
        lea eax, [ecx - 4]
        cmp dword ptr [eax + 0x28], eax
        jnz skip1
        mov ecx, dword ptr [ecx + 0x14]
        mov eax, dword ptr [ecx]
        mov edx, dword ptr [eax + 4]
        jmp edx
    skip1:
        cmp byte ptr [ecx + 0x1d], 0
        jz  skip2
        mov ecx, dword ptr [ecx + 0x18]
        mov eax, dword ptr [ecx]
        mov edx, dword ptr [eax + 4]
        jmp edx
    skip2:
        xor eax, eax
        ret
    }
}
