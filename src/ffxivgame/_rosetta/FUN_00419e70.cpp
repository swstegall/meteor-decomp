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
// FUNCTION: ffxivgame 0x00019e70 — `__thiscall` scalar-deleting destructor
//                                  for an object with a singleton-tracked
//                                  member pointer (59 B / 0x3b)
//
// Calling convention: __thiscall (ECX = this), one DWORD stack arg (flags),
//                     callee-cleans 4 bytes via `ret 4`.
//
// Disassembly (59 bytes):
//
//   56                               PUSH ESI
//   8b f1                            MOV ESI, ECX                ; this
//   c7 06 38 7e f5 00                MOV dword ptr [ESI], 0xf57e38   ; reset vftable
//   8b 46 04                         MOV EAX, dword ptr [ESI+4]  ; member ptr
//   85 c0                            TEST EAX, EAX
//   74 15                            JZ   +0x15  (skip_release)
//   8b 0d 20 99 32 01                MOV ECX, dword ptr [0x01329920]  ; global singleton
//   8b 11                            MOV EDX, dword ptr [ECX]    ; vtable of singleton
//   50                               PUSH EAX                    ; push member ptr as arg
//   8b 42 2c                         MOV EAX, dword ptr [EDX+0x2c] ; vtable slot 11
//   ff d0                            CALL EAX                    ; release member
//   c7 46 04 00 00 00 00             MOV dword ptr [ESI+4], 0    ; null out member
// skip_release:
//   f6 44 24 08 01                   TEST byte ptr [ESP+8], 1    ; flags & 1?
//   74 09                            JZ   +0x09  (skip_delete)
//   8b 4e fc                         MOV ECX, dword ptr [ESI-4]  ; allocator ctx
//   56                               PUSH ESI                    ; push this
//   e8 cb 40 ff ff                   CALL FUN_0040df70           ; free object
// skip_delete:
//   8b c6                            MOV EAX, ESI                ; return this
//   5e                               POP ESI
//   c2 04 00                         RET 4
//
// Shape:
//   The function is a textbook MSVC 2005 scalar deleting destructor.
//   - [ESI] (offset 0) is reset to the compiled-in vftable VA 0xf57e38 so
//     the virtual dispatch is correct even if a subclass overrode it.
//   - [ESI+4] holds a member pointer that was registered with a global
//     manager object (at .data 0x01329920, loaded double-indirect).  If
//     non-null it is released via that manager's vtable slot 11 and then
//     zeroed.
//   - The `flags & 1` guard is the standard MSVC scalar-delete convention:
//     when the caller wants to free the heap allocation it passes 1; the
//     allocator context lives at [this-4] (stored by the custom allocator
//     just before the user pointer).
//   - FUN_0040df70 is a custom allocator free helper (ECX = allocator ctx,
//     stack arg = user pointer).
//
// Reloc-bearing sites masked by compare.py:
//   +0x08  MOV [glob]  → DIR32 for g_01329920 (0x01329920)
//   +0x20  CALL rel32  → REL32 for FUN_0040df70

extern "C" void* g_01329920;    // global singleton pointer @ .data 0x01329920
extern "C" void  FUN_0040df70();// custom allocator free helper @ 0x0040df70

extern "C" __declspec(naked) void FUN_00419e70() {
    __asm {
        push    esi
        mov     esi, ecx
        mov     dword ptr [esi], 0x00f57e38
        mov     eax, dword ptr [esi + 4]
        test    eax, eax
        jz      skip_release
        mov     ecx, dword ptr [g_01329920]
        mov     edx, dword ptr [ecx]
        push    eax
        mov     eax, dword ptr [edx + 0x2c]
        call    eax
        mov     dword ptr [esi + 4], 0
    skip_release:
        test    byte ptr [esp + 8], 1
        jz      skip_delete
        mov     ecx, dword ptr [esi - 4]
        push    esi
        call    FUN_0040df70
    skip_delete:
        mov     eax, esi
        pop     esi
        ret     4
    }
}
