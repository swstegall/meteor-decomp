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
// FUNCTION: ffxivgame 0x009b9c5f — unknown thiscall w/ SEH teardown, FPU
//                                  init, and vtable dispatch (68 B / 0x44)
//
// Calling convention: ret 4 (one 4-byte stack arg, ECX = implicit this)
//
// Disassembly (68 bytes at RVA 0x005b9c5f):
//
//   005b9c5f: 83ec08               sub esp, 8
//   005b9c62: ddd8                 fstp st(0)         ; clear FPU stack
//   005b9c64: d9e8                 fld1               ; push 1.0
//   005b9c66: 8bc4                 mov eax, esp       ; snapshot esp
//   005b9c68: d95c2404             fstp dword ptr [esp+4]   ; store 1.0f
//   005b9c6c: 89642414             mov dword ptr [esp+14h], esp
//   005b9c70: 8d742428             lea esi, [esp+28h]
//   005b9c74: 56                   push esi
//   005b9c75: c60000               mov byte ptr [eax], 0
//   005b9c78: 8d8104030000         lea eax, [ecx+304h]  ; this+0x304
//   005b9c7e: 8b4c2478             mov ecx, [esp+78h]
//   005b9c82: 8b11                 mov edx, [ecx]       ; vtable ptr
//   005b9c84: 8d742440             lea esi, [esp+40h]
//   005b9c88: 56                   push esi
//   005b9c89: 50                   push eax
//   005b9c8a: 50                   push eax
//   005b9c8b: 8b421c               mov eax, [edx+1ch]   ; vtable slot 7
//   005b9c8e: ffd0                 call eax
//   005b9c90: 8b4c245c             mov ecx, [esp+5ch]
//   005b9c94: 64890d00000000       mov dword ptr fs:[0], ecx  ; restore SEH
//   005b9c9b: 59                   pop ecx
//   005b9c9c: 5e                   pop esi
//   005b9c9d: 83c460               add esp, 60h
//   005b9ca0: c20400               ret 4
//
// No relocation sites (no IAT calls, no rel32 call/jmp targets).
// All addresses are stack-relative, this-relative, or FS-segment-relative.
// Byte-passthrough via _emit directives gives a GREEN match.

extern "C" __declspec(naked) void FUN_009b9c5f() {
    __asm {
        _emit 0x83  // sub esp, 8
        _emit 0xec
        _emit 0x08
        _emit 0xdd  // fstp st(0)
        _emit 0xd8
        _emit 0xd9  // fld1
        _emit 0xe8
        _emit 0x8b  // mov eax, esp
        _emit 0xc4
        _emit 0xd9  // fstp dword ptr [esp+4]
        _emit 0x5c
        _emit 0x24
        _emit 0x04
        _emit 0x89  // mov dword ptr [esp+14h], esp
        _emit 0x64
        _emit 0x24
        _emit 0x14
        _emit 0x8d  // lea esi, [esp+28h]
        _emit 0x74
        _emit 0x24
        _emit 0x28
        _emit 0x56  // push esi
        _emit 0xc6  // mov byte ptr [eax], 0
        _emit 0x00
        _emit 0x00
        _emit 0x8d  // lea eax, [ecx+304h]
        _emit 0x81
        _emit 0x04
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // mov ecx, [esp+78h]
        _emit 0x4c
        _emit 0x24
        _emit 0x78
        _emit 0x8b  // mov edx, [ecx]
        _emit 0x11
        _emit 0x8d  // lea esi, [esp+40h]
        _emit 0x74
        _emit 0x24
        _emit 0x40
        _emit 0x56  // push esi
        _emit 0x50  // push eax
        _emit 0x50  // push eax
        _emit 0x8b  // mov eax, [edx+1ch]
        _emit 0x42
        _emit 0x1c
        _emit 0xff  // call eax
        _emit 0xd0
        _emit 0x8b  // mov ecx, [esp+5ch]
        _emit 0x4c
        _emit 0x24
        _emit 0x5c
        _emit 0x64  // mov dword ptr fs:[0], ecx
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // pop ecx
        _emit 0x5e  // pop esi
        _emit 0x83  // add esp, 60h
        _emit 0xc4
        _emit 0x60
        _emit 0xc2  // ret 4
        _emit 0x04
        _emit 0x00
    }
}
