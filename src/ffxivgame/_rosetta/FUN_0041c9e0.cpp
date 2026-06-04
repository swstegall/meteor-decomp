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
// FUNCTION: ffxivgame 0x0001c9e0 — conditional-channel flag builder + color-swizzle render call
// (__cdecl void FUN_0041c9e0(unsigned char flags, int color, float fval, int arg4), 179 B / 0xb3)
//
// The function reads two global renderer-context pointers (data_01328d98,
// data_01328da8) and conditionally loads their field at +0x20 into two
// channel values (channelA = EDX, channelB = EAX). Based on a flags byte
// passed as arg1, it builds a 3-bit enable mask in ESI:
//
//   bit 0 set → if channelA != 0: ESI |= 1
//   bit 1 set → if channelB != 0: ESI |= 2
//   bit 2 set → if channelB == 0x0f || channelB == 0x10: ESI |= 4
//
// It then byte-swizzles a 32-bit color argument (arg2 = ARGB 0xAABBCCDD)
// into output order [byte3][byte0][byte2][byte1] = 0xAADDCCBB and forwards
// the results together with a float (arg3) and integer (arg4) to a
// __thiscall rendering method at 0x00423280, using ECX = data_0132987c.
//
// Frame analysis:
//   PUSH ECX    → 1 local temp slot (used as scratch for channelB load)
//   PUSH ESI    → callee-save for flag accumulator
//   Callee FUN_00423280 uses RET 16 (4 dword args × 4 bytes) to restore
//   the stack so POP ESI + POP ECX + RET correctly unwind the frame.
//
// Calling convention: __cdecl (RET with no immediate, no ADD ESP after CALL).
//
// Reloc-bearing sites (wildcarded by compare.py):
//   +0x01  DIR32  data_01328d98   (MOV EAX, moffs32)
//   +0x1f  DIR32  data_01328da8   (MOV EAX, moffs32)
//   +0x9f  DIR32  data_0132987c   (MOV ECX, [mem32])
//   +0xac  REL32  FUN_00423280    (CALL rel32)

extern "C" {
    // .data — global renderer-context pointer A
    // (read via A1 moffs32 encoding: MOV EAX, [addr])
    extern int data_01328d98;

    // .data — global renderer-context pointer B
    // (read via A1 moffs32 encoding: MOV EAX, [addr])
    extern int data_01328da8;

    // .data — pointer to the render command object used as ECX for thiscall
    // (read via 8B 0D ModRM encoding: MOV ECX, [addr])
    extern int data_0132987c;

    // .text — thiscall rendering method (ECX = object, 4 dword args, RET 16)
    int FUN_00423280();
}

extern "C" __declspec(naked) void FUN_0041c9e0() {
    __asm {
        // --- load channelA: if global context A non-null, read field +0x20 ---
        push    ecx                             // 51
        mov     eax, [data_01328d98]            // a1 ?? ?? ?? ??
        test    eax, eax                        // 85 c0
        jz      short loc_null_a                // 74 08
        mov     eax, [eax + 0x20]              // 8b 40 20
        mov     [esp], eax                     // 89 04 24
        jmp     short loc_load_b               // eb 07
    loc_null_a:
        mov     dword ptr [esp], 0             // c7 04 24 00 00 00 00
    loc_load_b:
        // EDX = channelA value
        lea     eax, [esp]                     // 8d 04 24
        mov     edx, [eax]                     // 8b 10
        // --- load channelB: if global context B non-null, read field +0x20 ---
        mov     eax, [data_01328da8]            // a1 ?? ?? ?? ??
        test    eax, eax                        // 85 c0
        jz      short loc_null_b               // 74 08
        mov     ecx, [eax + 0x20]              // 8b 48 20
        mov     [esp], ecx                     // 89 0c 24
        jmp     short loc_flags                // eb 07
    loc_null_b:
        mov     dword ptr [esp], 0             // c7 04 24 00 00 00 00
    loc_flags:
        // CL = flags byte (arg1); EAX = channelB (loaded from temp slot below)
        mov     cl, byte ptr [esp + 0x8]       // 8a 4c 24 08
        push    esi                            // 56
        lea     eax, [esp + 0x4]               // 8d 44 24 04
        mov     eax, [eax]                     // 8b 00
        xor     esi, esi                       // 33 f6
        // bit 0: (flags & 1) && channelA != 0 → ESI |= 1
        test    cl, 0x1                        // f6 c1 01
        jz      short loc_bit1                 // 74 09
        test    edx, edx                       // 85 d2
        jz      short loc_bit1                 // 74 05
        mov     esi, 1                         // be 01 00 00 00
    loc_bit1:
        // bit 1: (flags & 2) && channelB != 0 → ESI |= 2
        test    cl, 0x2                        // f6 c1 02
        jz      short loc_bit2                 // 74 07
        test    eax, eax                       // 85 c0
        jz      short loc_bit2                 // 74 03
        or      esi, 2                         // 83 ce 02
    loc_bit2:
        // bit 2: (flags & 4) && (channelB == 0xf || channelB == 0x10) → ESI |= 4
        test    cl, 0x4                        // f6 c1 04
        jz      short loc_done_flags           // 74 0d
        cmp     eax, 0xf                       // 83 f8 0f
        jz      short loc_set4                 // 74 05
        cmp     eax, 0x10                      // 83 f8 10
        jnz     short loc_done_flags           // 75 03
    loc_set4:
        or      esi, 4                         // 83 ce 04
    loc_done_flags:
        // --- load remaining function arguments ---
        mov     edx, [esp + 0x18]              // 8b 54 24 18  (arg4)
        fld     dword ptr [esp + 0x14]         // d9 44 24 14  (arg3 float → FPU)
        mov     eax, [esp + 0x10]              // 8b 44 24 10  (arg2 color)
        // push arg4 and a slot for the float (FSTP overwrites the ECX slot)
        push    edx                            // 52
        push    ecx                            // 51  (will be overwritten by FSTP)
        xor     edx, edx                       // 33 d2
        fstp    dword ptr [esp]                // d9 1c 24  (store float, overwrite ECX slot)
        // byte-swizzle EAX (0xAABBCCDD) → EDX (0xAADDCCBB)
        mov     ecx, eax                       // 8b c8
        shr     ecx, 0x18                      // c1 e9 18  ECX = byte3 (AA)
        mov     dh, cl                         // 8a f1     DH  = byte3
        mov     ecx, eax                       // 8b c8
        shr     ecx, 0x8                       // c1 e9 08
        movzx   ecx, cl                        // 0f b6 c9  ECX = byte1 (CC)
        mov     dl, al                         // 8a d0     DL  = byte0 (DD)
        shr     eax, 0x10                      // c1 e8 10
        movzx   eax, al                        // 0f b6 c0  EAX = byte2 (BB)
        shl     edx, 0x8                       // c1 e2 08  EDX = (AA<<16)|(DD<<8)
        or      edx, ecx                       // 0b d1     EDX |= CC
        mov     ecx, [data_0132987c]            // 8b 0d ?? ?? ?? ??  (thiscall object)
        shl     edx, 0x8                       // c1 e2 08  EDX = (AA<<24)|(DD<<16)|(CC<<8)
        or      edx, eax                       // 0b d0     EDX |= BB → 0xAADDCCBB
        // call thiscall render method with (ESI=flags, EDX=color, float, arg4)
        push    edx                            // 52
        push    esi                            // 56
        call    FUN_00423280                   // e8 ?? ?? ?? ??
        pop     esi                            // 5e  restore original ESI (callee did RET 16)
        pop     ecx                            // 59  clean local temp slot
        ret                                    // c3
    }
}
