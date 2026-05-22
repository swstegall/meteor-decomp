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
// FUNCTION: ffxivgame 0x0040afc0 — __thiscall linked-list drain for a
// pool-tracked node list.
//
// ECX  = this (the container object)
// arg0 = index into the container's list-head array
//
// For each node in the singly-linked list at this[index]:
//   - If the node's value field ([node+0]) is non-zero:
//       Load the sub-object at this+0x60.
//       If the sub-object's allocation count (short at [sub+8+8]) is zero,
//       or FUN_0040ddd0 says the value is NOT valid → call FUN_0040df70
//       (pool abort/free); otherwise call FUN_0040de80 (mark freed slot),
//       increment [sub+0x30], and subtract 0x1000 from [sub+0x38].
//   - Try FUN_0040d600 on the node pointer; if it returns non-zero,
//     call FUN_0040de80 on that result.
//   - Advance to next node ([node+0x1c]).
// After the loop: zero the list-head entry this[index].
//
// Calling convention: __thiscall (ECX = this), one stack arg,
// callee cleans via RET 4.
//
// Frame: sub esp,8 + push ebp (saved ECX/this) + push edi (loop ptr) +
//        push ebx/esi (inside loop body). Total 28 bytes below entry ESP.
//   [esp+0x10] = copy of `this` (used to restore ebp after it's clobbered
//                by `lea ebp, [esi+4]` in the ddd0-path)
//   [esp+0x14] = param_1 (index) before first loop iteration; becomes
//                next-node pointer during the loop

extern "C" int FUN_0040ddd0();   // __thiscall — pool membership check
extern "C" int FUN_0040de80();   // __thiscall — mark freed slot in pool
extern "C" int FUN_0040df70();   // __thiscall — pool free / abort
extern "C" int FUN_0040d600();   // __thiscall — node-validity query

extern "C" __declspec(naked) void FUN_0040afc0()
{
    __asm {
        sub     esp, 8
        push    ebp
        mov     ebp, ecx
        test    ebp, ebp
        mov     dword ptr [esp + 4], ebp
        je      done_all

        mov     eax, dword ptr [esp + 0x10]
        push    edi
        mov     edi, dword ptr [ebp + eax*4]
        test    edi, edi
        je      exit_loop

        push    ebx
        push    esi

    loop_top:
        mov     ebx, dword ptr [edi]
        test    ebx, ebx
        mov     ecx, dword ptr [edi + 0x1c]
        mov     esi, dword ptr [ebp + 0x60]
        mov     dword ptr [esp + 0x14], ecx
        je      node_done

        mov     edx, dword ptr [esi + 8]
        cmp     word ptr [edx + 8], 0
        je      call_df70

        lea     ebp, [esi + 4]
        push    ebx
        mov     ecx, ebp
        call    FUN_0040ddd0
        test    al, al
        je      restore_and_call_df70

        push    ebx
        mov     ecx, ebp
        call    FUN_0040de80
        mov     ebp, dword ptr [esp + 0x10]
        jmp     after_df70_block

    restore_and_call_df70:
        mov     ebp, dword ptr [esp + 0x10]

    call_df70:
        mov     ecx, dword ptr [esi]
        push    ebx
        call    FUN_0040df70

    after_df70_block:
        add     dword ptr [esi + 0x30], 1
        add     dword ptr [esi + 0x38], 0xFFFFF000

    node_done:
        mov     ecx, dword ptr [ebp + 0x5c]
        push    edi
        call    FUN_0040d600
        test    eax, eax
        je      skip_node_free

        push    edi
        mov     ecx, eax
        call    FUN_0040de80

    skip_node_free:
        mov     edi, dword ptr [esp + 0x14]
        test    edi, edi
        jne     loop_top

        pop     esi
        pop     ebx

    exit_loop:
        mov     eax, dword ptr [esp + 0x14]
        mov     dword ptr [ebp + eax*4], 0
        pop     edi

    done_all:
        pop     ebp
        add     esp, 8
        ret     4
    }
}

// vim: ts=4 sts=4 sw=4 et
