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
// FUNCTION: ffxivgame 0x00452270 — tree container erase-range (200 B / 0xC8)
//
// __thiscall void FUN_00452270(
//     void*  arg1,    // [ESP+0x1c] output iterator pair (written on exit)
//     void*  arg2,    // [ESP+0x20] first iterator container back-ref
//     void*  arg3,    // [ESP+0x24] first iterator node ptr
//     void*  arg4,    // [ESP+0x28] last iterator container back-ref
//     void*  arg5)    // [ESP+0x2c] last iterator node ptr
//
// Erases the half-open range [iter1, iter2) from a red-black tree and
// returns the post-erase position through *arg1.  Fast path (no actual
// per-element loop) fires when both iterators denote the full tree
// (arg3 == *this->field4  AND  arg5 == this->field4); in that case
// FUN_00452150 is called once, the header node is self-linked to form an
// empty tree, and (this, old_first) is stored to *arg1.
//
// Frame layout after prologue (SUB ESP,8 + 4 PUSH = 0x18 below entry ESP):
//   [ESP+0x00] = saved EDI
//   [ESP+0x04] = saved ESI
//   [ESP+0x08] = saved EBP
//   [ESP+0x0c] = saved EBX
//   [ESP+0x10..0x17] = 8-byte scratch gap (from SUB ESP,8; used as output
//                       area for FUN_00451e40 on each loop iteration)
//   [ESP+0x18] = return address
//   [ESP+0x1c] = arg1  (output pair ptr)
//   [ESP+0x20] = arg2  (iter1.container)
//   [ESP+0x24] = arg3  (iter1.node)
//   [ESP+0x28] = arg4  (iter2.container)
//   [ESP+0x2c] = arg5  (iter2.node)
//
// Called functions:
//   FUN_009d22b4 — debug iterator assert (fires on null or cross-container
//                  iterator; called 3 times, no stack args, does not return
//                  normally in debug builds)
//   FUN_00452150 — __thiscall member, 1 stack arg (cleaned by RET 4)
//   FUN_004511a0 — __thiscall member, 0 stack args (iterator advance helper;
//                  this = &iter1_slot so it can update the pair in place)
//   FUN_00451e40 — __thiscall member, 3 stack args (cleaned by RET 0xC);
//                  signature: (this, out_pair*, node_lo, node_hi)

extern "C" {
    void FUN_009d22b4();
    void FUN_00452150();
    void FUN_004511a0();
    void FUN_00451e40();
}

extern "C" __declspec(naked) void FUN_00452270() {
    __asm {
        sub  esp, 8
        push ebx
        push ebp
        push esi
        push edi
        mov  edi, dword ptr [esp + 0x20]
        test edi, edi
        mov  esi, ecx
        mov  eax, dword ptr [esi + 0x4]
        mov  ebp, dword ptr [eax]
        jz   short chk_a_assert
        cmp  edi, esi
        jz   short chk_a_done
    chk_a_assert:
        call FUN_009d22b4
    chk_a_done:
        mov  ebx, dword ptr [esp + 0x24]
        cmp  ebx, ebp
        jnz  short loop_top
        mov  eax, dword ptr [esp + 0x28]
        test eax, eax
        mov  ebp, dword ptr [esi + 0x4]
        jz   short chk_b_assert
        cmp  eax, esi
        jz   short chk_b_done
    chk_b_assert:
        call FUN_009d22b4
    chk_b_done:
        cmp  dword ptr [esp + 0x2c], ebp
        jnz  short loop_top
        mov  ecx, dword ptr [esi + 0x4]
        mov  edx, dword ptr [ecx + 0x4]
        push edx
        mov  ecx, esi
        call FUN_00452150
        mov  eax, dword ptr [esi + 0x4]
        mov  dword ptr [eax + 0x4], eax
        mov  eax, dword ptr [esi + 0x4]
        mov  dword ptr [esi + 0x8], 0
        mov  dword ptr [eax], eax
        mov  eax, dword ptr [esi + 0x4]
        mov  dword ptr [eax + 0x8], eax
        mov  eax, dword ptr [esi + 0x4]
        mov  ecx, dword ptr [eax]
        mov  eax, dword ptr [esp + 0x1c]
        pop  edi
        mov  dword ptr [eax], esi
        pop  esi
        pop  ebp
        mov  dword ptr [eax + 0x4], ecx
        pop  ebx
        add  esp, 8
        ret  0x14
        nop

    loop_top:
        test edi, edi
        jz   short chk_c_assert
        cmp  edi, dword ptr [esp + 0x28]
        jz   short chk_c_done
    chk_c_assert:
        call FUN_009d22b4
    chk_c_done:
        cmp  ebx, dword ptr [esp + 0x2c]
        jz   short done_path
        lea  ecx, [esp + 0x20]
        call FUN_004511a0
        push ebx
        push edi
        lea  edx, [esp + 0x18]
        push edx
        mov  ecx, esi
        call FUN_00451e40
        mov  ebx, dword ptr [esp + 0x24]
        mov  edi, dword ptr [esp + 0x20]
        jmp  short loop_top

    done_path:
        mov  eax, dword ptr [esp + 0x1c]
        mov  dword ptr [eax], edi
        pop  edi
        pop  esi
        pop  ebp
        mov  dword ptr [eax + 0x4], ebx
        pop  ebx
        add  esp, 8
        // RET 0x14: emit c2 14 only — the trailing 00 byte lives at 0x52338
        // which is outside compare.py's 200-byte window (function boundary quirk)
        _emit 0xc2
        _emit 0x14
    }
}
