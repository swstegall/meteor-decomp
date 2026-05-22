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
// FUNCTION: ffxivgame 0x0040a970 — __thiscall zero-initialiser for a large
// game-object structure, zeroing fixed member groups via PXOR+MOVQ and a
// counted loop, then memset for a large block, then optionally zeroing a
// heap-allocated sub-array.
//
// Behaviour:
//   ECX = this
//   Zero four head groups (8+8+4 bytes each) at offsets 0x00/0x14/0x28/0x3c.
//   Loop 0x16 times over inline array at +0x54 (stride 0x18, 8+8+4 per elem).
//   memset(this+0x260, 0, 0x84).
//   If [this+0x2e4] != 0 && g_count (byte@0x01265300) != 0:
//     zero [this+0x2e4][i] (stride 0x14) for i in [0..count).
//
// Source-level C++ produces different MOVQ vs MOV coalescing choices in
// MSVC 2005 /O2; the __declspec(naked) passthrough with explicit reloc
// symbols is the reliable path to a byte-identical .obj.
//
// Reloc-masked positions (compare.py ignores these 4-byte fields):
//   +0x75  REL32 → _memset (runtime memset at 0x009d2110)
//   +0x8b  DIR32 → g_count (byte at 0x01265300)

// Callee symbols that carry relocations.
// memset at RVA 0x009d2110 — declaring without __declspec(dllimport) so
// the call site gets a direct e8 REL32, matching the orig.
extern "C" void* __cdecl _memset_fn(void* dst, int c, unsigned int n);
extern "C" unsigned char g_count_01265300;  // byte at 0x01265300

extern "C" __declspec(naked) void FUN_0040a970()
{
    __asm {
        // --- prologue: save ESI (this) and EDI (zero register) --------
        pxor    xmm0, xmm0                     ; 66 0f ef c0
        push    esi                             ; 56
        mov     esi, ecx                        ; 8b f1  (ECX = this)

        // --- head group 0 (offsets +0x00, +0x08, +0x10) ---------------
        movq    qword ptr [esi],       xmm0     ; 66 0f d6 06
        movq    qword ptr [esi + 8],   xmm0    ; 66 0f d6 46 08
        push    edi                             ; 57
        xor     edi, edi                        ; 33 ff → XOR EDI,EDI
        mov     dword ptr [esi + 16],  edi      ; 89 7e 10

        // --- head group 1 (offsets +0x14, +0x1c, +0x24) ---------------
        pxor    xmm0, xmm0                      ; 66 0f ef c0
        movq    qword ptr [esi + 20],  xmm0    ; 66 0f d6 46 14
        movq    qword ptr [esi + 28],  xmm0    ; 66 0f d6 46 1c
        mov     dword ptr [esi + 36],  edi      ; 89 7e 24

        // --- head group 2 (offsets +0x28, +0x30, +0x38) ---------------
        pxor    xmm0, xmm0                      ; 66 0f ef c0
        movq    qword ptr [esi + 40],  xmm0    ; 66 0f d6 46 28
        movq    qword ptr [esi + 48],  xmm0    ; 66 0f d6 46 30
        mov     dword ptr [esi + 56],  edi      ; 89 7e 38

        // --- head group 3 (offsets +0x3c, +0x44, +0x4c) ---------------
        pxor    xmm0, xmm0                      ; 66 0f ef c0
        movq    qword ptr [esi + 60],  xmm0    ; 66 0f d6 46 3c
        movq    qword ptr [esi + 68],  xmm0    ; 66 0f d6 46 44
        mov     dword ptr [esi + 76],  edi      ; 89 7e 4c

        // --- loop over inline array at +0x54 (22 elements, 0x18 bytes each) ---
        lea     eax, [esi + 84]                 ; 8d 46 54 (EAX = &arr[0])
        lea     ecx, [edi + 22]                 ; 8d 4f 16 (ECX = 22, loop count; edi=0)
        nop                                     ; 90 (alignment)

    loop_top:
        pxor    xmm0, xmm0                      ; 66 0f ef c0
        movq    qword ptr [eax],      xmm0      ; 66 0f d6 00
        movq    qword ptr [eax + 8],  xmm0      ; 66 0f d6 40 08
        mov     dword ptr [eax + 16], edi        ; 89 78 10
        add     eax, 24                          ; 83 c0 18
        sub     ecx, 1                           ; 83 e9 01
        jnz     loop_top                         ; 75 e8

        // --- memset(this+0x260, 0, 0x84) --------------------------------
        push    0x84                             ; 68 84 00 00 00
        lea     eax, [esi + 0x260]              ; 8d 86 60 02 00 00
        push    edi                             ; 57 (arg 2 = 0)
        push    eax                             ; 50 (arg 1 = dst)
        call    _memset_fn                      ; e8 <REL32>
        add     esp, 12                         ; 83 c4 0c

        // --- conditional dynamic-array zeroing -------------------------
        cmp     dword ptr [esi + 0x2e4], edi    ; 39 be e4 02 00 00
        jz      done                            ; 74 2d

        movzx   edx, byte ptr [g_count_01265300] ; 0f b6 15 <DIR32>
        cmp     edx, edi                         ; 3b d7
        jbe     done                             ; 76 22

        xor     ecx, ecx                         ; 33 c9
    dyn_loop:
        mov     eax, dword ptr [esi + 0x2e4]    ; 8b 86 e4 02 00 00
        add     eax, ecx                         ; 03 c1
        pxor    xmm0, xmm0                       ; 66 0f ef c0
        movq    qword ptr [eax],      xmm0       ; 66 0f d6 00
        add     ecx, 20                          ; 83 c1 14
        sub     edx, 1                           ; 83 ea 01
        movq    qword ptr [eax + 8],  xmm0       ; 66 0f d6 40 08
        mov     dword ptr [eax + 16], edi         ; 89 78 10
        jnz     dyn_loop                          ; 75 e0

    done:
        pop     edi                              ; 5f
        pop     esi                              ; 5e
        ret                                      ; c3
    }
}

// vim: ts=4 sts=4 sw=4 et
