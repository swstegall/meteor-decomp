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
// FUNCTION: ffxivgame 0x00037560 — FUN_00437560 (68 B / 0x44)
//
// Calling convention: __thiscall (ECX = this, no stack args, void return)
//
// Analysis (read from asm/ffxivgame/00037560_FUN_00437560.s):
//
//   Pool-entry allocation + vtable-set + method-dispatch helper.
//
//   A byte field at *g_PoolMgr is used as a pool-array index; each pool
//   entry is 28 bytes wide (stride = index * 7 * 4). The function:
//     1. Looks up the correct allocator pool entry:
//          pool_idx  = (unsigned char)(*g_PoolMgr)
//          pool_ptr  = *(g_PoolMgr + 4) + pool_idx * 28
//     2. Calls pool_ptr->FUN_00417ab0(4) to allocate 4 bytes.
//     3a. On success: writes vtable pointer 0xf64998 into the new object,
//         then calls this->field_8->FUN_0043c2d0(new_obj).
//     3b. On failure: calls this->field_8->FUN_0043c2d0(NULL).
//
// Disassembly (verbatim, 68 bytes):
//
//   00037560:  56                    push    esi
//   00037561:  8b f1                 mov     esi, ecx              ; ESI = this
//   00037563:  8b 0d 90 8d 32 01     mov     ecx, [0x01328d90]     ; ECX = *g_PoolMgr
//   00037569:  0f b6 01              movzx   eax, byte ptr [ecx]   ; EAX = pool_idx (u8)
//   0003756c:  8d 14 c5 00 00 00 00  lea     edx, [eax*8+0]        ; EDX = idx*8
//   00037573:  2b d0                 sub     edx, eax              ; EDX = idx*7
//   00037575:  8b 41 04              mov     eax, [ecx+4]          ; EAX = array_base
//   00037578:  8d 0c 90              lea     ecx, [eax+edx*4]      ; ECX = base+idx*28
//   0003757b:  6a 04                 push    4                     ; alloc size
//   0003757d:  e8 2e 05 fe ff        call    0x00417ab0            ; pool->alloc(4)
//   00037582:  85 c0                 test    eax, eax
//   00037584:  74 11                 jz      0x00437597            ; NULL → alloc_failed
//   00037586:  c7 00 98 49 f6 00     mov     dword ptr [eax], 0xf64998  ; set vftable
//   0003758c:  8b 4e 08              mov     ecx, [esi+8]          ; ECX = this->field_8
//   0003758f:  50                    push    eax                   ; arg = new_obj
//   00037590:  e8 3b 4d 00 00        call    0x0043c2d0            ; field_8->method(obj)
//   00037595:  5e                    pop     esi
//   00037596:  c3                    ret
//   00037597:  8b 4e 08              mov     ecx, [esi+8]          ; ECX = this->field_8
//   0003759a:  33 c0                 xor     eax, eax              ; EAX = NULL
//   0003759c:  50                    push    eax                   ; arg = NULL
//   0003759d:  e8 2e 4d 00 00        call    0x0043c2d0            ; field_8->method(NULL)
//   000375a2:  5e                    pop     esi
//   000375a3:  c3                    ret
//
// Byte layout confirmed (68 bytes = 0x44):
//   offsets 0x05..0x08   DIR32 reloc → g_PoolMgr           (compare.py wildcards)
//   offsets 0x1e..0x21   REL32 reloc → FUN_00417ab0        (compare.py wildcards)
//   offsets 0x31..0x34   REL32 reloc → FUN_0043c2d0 (1st)  (compare.py wildcards)
//   offsets 0x3e..0x41   REL32 reloc → FUN_0043c2d0 (2nd)  (compare.py wildcards)
//
// The vtable immediate 0x00f64998 at offset 0x27..0x2a is NOT a relocation
// in the standalone .obj — the binary is fixed-base so those 4 bytes match
// verbatim.
//
// Reconstruction strategy: __declspec(naked) with inline MASM.
//   Named externs for the global access and both calls produce proper COFF
//   DIR32/REL32 entries so compare.py wildcards them. The vtable imm32 and
//   all arithmetic/control-flow bytes must match byte-for-byte.

// Global pool manager pointer (value at this address loaded into ECX)
extern "C" void *g_PoolMgr;

// __thiscall pool allocator: ECX = pool entry, stack arg = size → returns ptr
extern "C" void FUN_00417ab0();

// __thiscall method dispatch: ECX = this->field_8, stack arg = obj_or_null
extern "C" void FUN_0043c2d0();

extern "C" __declspec(naked) void FUN_00437560() {
    __asm {
        push    esi
        mov     esi, ecx                          // ESI = this

        // Compute the pool entry pointer: base + pool_idx * 28
        mov     ecx, dword ptr [g_PoolMgr]        // ECX = *g_PoolMgr (DIR32 reloc)
        movzx   eax, byte ptr [ecx]              // EAX = pool_idx (unsigned byte)
        lea     edx, [eax*8]                      // EDX = pool_idx * 8
        sub     edx, eax                          // EDX = pool_idx * 7
        mov     eax, dword ptr [ecx + 4]          // EAX = array base pointer
        lea     ecx, [eax + edx*4]                // ECX = base + pool_idx*7*4 (stride=28)

        // Allocate 4 bytes from the pool entry
        push    4
        call    FUN_00417ab0                      // EAX = pool->alloc(4)  (REL32 reloc)
        test    eax, eax
        jz      alloc_failed                      // NULL → skip vtable write

        // Allocation succeeded: write vftable and call method with new object
        mov     dword ptr [eax], 0x00f64998       // *(DWORD*)new_obj = vftable_ptr
        mov     ecx, dword ptr [esi + 8]          // ECX = this->field_8
        push    eax                               // arg = new_obj
        call    FUN_0043c2d0                      // field_8->method(new_obj) (REL32 reloc)
        pop     esi
        ret

    alloc_failed:
        mov     ecx, dword ptr [esi + 8]          // ECX = this->field_8
        xor     eax, eax                          // EAX = NULL
        push    eax                               // arg = NULL
        call    FUN_0043c2d0                      // field_8->method(NULL)  (REL32 reloc)
        pop     esi
        ret
    }
}
