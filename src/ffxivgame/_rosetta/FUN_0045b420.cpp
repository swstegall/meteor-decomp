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
// FUNCTION: ffxivgame 0x0045b420 — sparse-value translation / remap
//                                  (__cdecl, 73 B / 0x49, zero relocs)
//
// bool __cdecl FUN_0045b420(unsigned int *p)
//
// Loads *p, checks it against four sparse values, and if it matches one
// writes back a remapped value and returns true (1). Returns false (0)
// for any other input. The four mappings are:
//
//   0x2ee2 → 0x2723
//   0x0070 → 0x29d7
//   0x2ee5 → 0x2722
//   0x2ee7 → 0x2724
//
// Orig codegen (73 bytes):
//
//   8b 4c 24 04           mov  ecx, [esp+4]         ; ecx = p
//   8b 01                 mov  eax, [ecx]            ; eax = *p
//   3d e5 2e 00 00        cmp  eax, 0x2ee5
//   77 29                 ja   above                 ; eax > 0x2ee5
//   74 1e                 jz   eq_2ee5               ; eax == 0x2ee5
//   83 f8 70              cmp  eax, 0x70
//   74 10                 jz   eq_70                 ; eax == 0x70
//   3d e2 2e 00 00        cmp  eax, 0x2ee2
//   75 22                 jnz  ret_false             ; eax != 0x2ee2 → false
//   c7 01 23 27 00 00     mov  dword ptr [ecx], 0x2723
//   b0 01                 mov  al, 1
//   c3                    ret
// eq_70:
//   c7 01 d7 29 00 00     mov  dword ptr [ecx], 0x29d7
//   b0 01                 mov  al, 1
//   c3                    ret
// eq_2ee5:
//   c7 01 22 27 00 00     mov  dword ptr [ecx], 0x2722
//   b0 01                 mov  al, 1
//   c3                    ret
// above:
//   3d e7 2e 00 00        cmp  eax, 0x2ee7
//   74 03                 jz   eq_2ee7               ; eax == 0x2ee7
// ret_false:
//   32 c0                 xor  al, al
//   c3                    ret
// eq_2ee7:
//   c7 01 24 27 00 00     mov  dword ptr [ecx], 0x2724
//   b0 01                 mov  al, 1
//   c3                    ret
//
// Calling convention: __cdecl (one stack arg, no callee-saves, plain ret).
// Reloc-bearing sites: NONE. All immediates are self-contained constants.
//
// Reconstruction strategy: __declspec(naked) byte-for-byte passthrough.
// The sparse values and branch targets encode no external references, so
// the .obj's .text matches the orig slice byte-for-byte with zero fixups.

extern "C" __declspec(naked) void FUN_0045b420() {
    __asm {
        // 0005b420: mov ecx, dword ptr [esp+0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0005b424: mov eax, dword ptr [ecx]
        _emit 0x8b
        _emit 0x01
        // 0005b426: cmp eax, 0x2ee5
        _emit 0x3d
        _emit 0xe5
        _emit 0x2e
        _emit 0x00
        _emit 0x00
        // 0005b42b: ja +0x29 (-> 0x0045b456)
        _emit 0x77
        _emit 0x29
        // 0005b42d: jz +0x1e (-> 0x0045b44d)
        _emit 0x74
        _emit 0x1e
        // 0005b42f: cmp eax, 0x70
        _emit 0x83
        _emit 0xf8
        _emit 0x70
        // 0005b432: jz +0x10 (-> 0x0045b444)
        _emit 0x74
        _emit 0x10
        // 0005b434: cmp eax, 0x2ee2
        _emit 0x3d
        _emit 0xe2
        _emit 0x2e
        _emit 0x00
        _emit 0x00
        // 0005b439: jnz +0x22 (-> 0x0045b45d)
        _emit 0x75
        _emit 0x22
        // 0005b43b: mov dword ptr [ecx], 0x2723
        _emit 0xc7
        _emit 0x01
        _emit 0x23
        _emit 0x27
        _emit 0x00
        _emit 0x00
        // 0005b441: mov al, 0x1
        _emit 0xb0
        _emit 0x01
        // 0005b443: ret
        _emit 0xc3
        // 0005b444: mov dword ptr [ecx], 0x29d7
        _emit 0xc7
        _emit 0x01
        _emit 0xd7
        _emit 0x29
        _emit 0x00
        _emit 0x00
        // 0005b44a: mov al, 0x1
        _emit 0xb0
        _emit 0x01
        // 0005b44c: ret
        _emit 0xc3
        // 0005b44d: mov dword ptr [ecx], 0x2722
        _emit 0xc7
        _emit 0x01
        _emit 0x22
        _emit 0x27
        _emit 0x00
        _emit 0x00
        // 0005b453: mov al, 0x1
        _emit 0xb0
        _emit 0x01
        // 0005b455: ret
        _emit 0xc3
        // 0005b456: cmp eax, 0x2ee7
        _emit 0x3d
        _emit 0xe7
        _emit 0x2e
        _emit 0x00
        _emit 0x00
        // 0005b45b: jz +0x03 (-> 0x0045b460)
        _emit 0x74
        _emit 0x03
        // 0005b45d: xor al, al
        _emit 0x32
        _emit 0xc0
        // 0005b45f: ret
        _emit 0xc3
        // 0005b460: mov dword ptr [ecx], 0x2724
        _emit 0xc7
        _emit 0x01
        _emit 0x24
        _emit 0x27
        _emit 0x00
        _emit 0x00
        // 0005b466: mov al, 0x1
        _emit 0xb0
        _emit 0x01
        // 0005b468: ret
        _emit 0xc3
    }
}
