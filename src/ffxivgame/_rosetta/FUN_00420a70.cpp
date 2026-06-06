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
// FUNCTION: ffxivgame 0x00020a70 — FUN_00420a70 (__cdecl, 55 bytes)
//
// Factory / placement-new wrapper. Allocates 0x28 (40) bytes via the game's
// internal allocator (FUN_009d1b35, the same `operator new[]`-like thunk used
// by FUN_009d085b and FUN_00403bd0), then zero-initialises three DWORD fields
// at offsets +0x00, +0x04, +0x08, and sets two flag bytes at +0x20 and +0x21
// before returning the pointer (in EAX).
//
// Struct layout (inferred, 0x28 bytes allocated):
//   +0x00  int/ptr field0     (zeroed)
//   +0x04  int/ptr field1     (zeroed)
//   +0x08  int/ptr field2     (zeroed)
//   ...    (padding / other fields not touched here)
//   +0x20  char  initialized  (set to 1)
//   +0x21  char  flags        (set to 0)
//
// The three TEST/JZ null-guards after each LEA are a MSVC 2005 checked-pointer
// idiom: the compiler emits a null check on each derived interior pointer even
// though mathematically they cannot be null when the base allocation succeeded.
// Because the first guard jumps to the second LEA on EAX==0, the code is
// correct only when the allocator never returns null (i.e. it throws or aborts
// on OOM).
//
// Calling convention: __cdecl, no parameters, no frame (no PUSH EBP).
// Return: EAX = pointer to the allocated block (or 0 if allocation fails).
//
// Asm (55 bytes):
//   6a 28                     PUSH 0x28
//   e8 RR RR RR RR            CALL FUN_009d1b35  (rel32, masked by compare.py)
//   83 c4 04                  ADD  ESP, 0x4
//   85 c0                     TEST EAX, EAX
//   74 06                     JZ   +6  (to lea ecx, [eax+4])
//   c7 00 00 00 00 00         MOV  dword ptr [EAX], 0
//   8d 48 04                  LEA  ECX, [EAX + 0x4]
//   85 c9                     TEST ECX, ECX
//   74 06                     JZ   +6  (to lea ecx, [eax+8])
//   c7 01 00 00 00 00         MOV  dword ptr [ECX], 0
//   8d 48 08                  LEA  ECX, [EAX + 0x8]
//   85 c9                     TEST ECX, ECX
//   74 06                     JZ   +6  (to mov byte ptr [eax+0x20], 1)
//   c7 01 00 00 00 00         MOV  dword ptr [ECX], 0
//   c6 40 20 01               MOV  byte ptr [EAX + 0x20], 1
//   c6 40 21 00               MOV  byte ptr [EAX + 0x21], 0
//   c3                        RET

extern "C" void FUN_009d1b35();    // allocator (operator new[]-like thunk @ 0x009d1b35)

extern "C" __declspec(naked) void FUN_00420a70() {
    __asm {
        push    0x28
        call    FUN_009d1b35
        add     esp, 4
        test    eax, eax
        jz      skip1
        mov     dword ptr [eax], 0
    skip1:
        lea     ecx, [eax + 4]
        test    ecx, ecx
        jz      skip2
        mov     dword ptr [ecx], 0
    skip2:
        lea     ecx, [eax + 8]
        test    ecx, ecx
        jz      skip3
        mov     dword ptr [ecx], 0
    skip3:
        mov     byte ptr [eax + 0x20], 1
        mov     byte ptr [eax + 0x21], 0
        ret
    }
}
