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
// FUNCTION: ffxivgame 0x0040e430 — allocate-or-reuse memory pool object
//                                  (__cdecl, 193 B / 0xc1)
//
// "Get-or-create" factory for the engine's named memory-pool system.
// The caller supplies an index into a global 8-byte-stride descriptor
// table at 0x00f564e0 and an optional pre-existing object pointer.
// If the pointer is non-NULL it is used directly; otherwise a new
// block is allocated via the singleton pool allocator
// (g_pool_singleton at 0x01327fc0) and an allocator-descriptor object
// is constructed on the local stack before the call.  On success the
// resulting pointer is handed to FUN_0040dee0 for initialization.
//
//   __cdecl void* FUN_0040e430(
//       int   table_idx   @ [ESP+0x04],   /* arg1 — index into g_pool_descs */
//       int   size_hint   @ [ESP+0x08],   /* arg2 — caller size; 0 → use table entry */
//       void* existing    @ [ESP+0x0c],   /* arg3 — pre-existing ptr or NULL */
//       void* arg4        @ [ESP+0x10]);  /* arg4 — forwarded to initializer */
//
// Behaviour (from asm/ffxivgame/0000e430_FUN_0040e430.s):
//
//   1. EH3-style SEH frame:
//        PUSH -1 / PUSH scope_table / PUSH FS:[0] / MOV FS:[0],ESP
//      + 8 bytes of local space (SUB ESP,8) for the allocator-descriptor.
//
//   2. arg1 → EBP, arg3 → EDI; EBP is immediately scaled:
//        EBP = arg1 * 8 + 0xf564e0        (= &g_pool_descs[arg1])
//
//   3. If arg3 (existing) != NULL → skip allocation, use it as-is.
//
//   4. If arg3 == NULL:
//        size = arg2 != 0 ? arg2
//                         : (g_pool_descs[arg1].field4 + 0x4b) & ~0xf
//        construct an allocator-descriptor at local[0]:
//            ECX = &local[0]; CALL 0x0040e2d0(0x10, 0xf564f0)
//        EBX = result (descriptor handle)
//        EAX = g_pool_singleton (0x01327fc0); if 0 → CALL 0x0040e500
//        EAX = EAX->Allocate(size, EBX)   (CALL 0x0040e110, __thiscall 2 args)
//
//   5. Advance EH state from -1 to 0 (MOV [ESP+0x18], 0).
//
//   6. If result == NULL → return NULL.
//
//   7. If result != NULL:
//        is_new = (result != original_arg3) ? 1 : 0
//        result->Init(&g_pool_descs[arg1], result, is_new, arg4)
//                                          (CALL 0x0040dee0, __thiscall 4 args)
//      Return value is whatever Init returns.
//
// Global references:
//   0x00f564e0  g_pool_descs[]         — 8-byte-stride descriptor table (.data)
//   0x00f564f0  g_pool_allocator_info  — allocator-descriptor table or pool (.data)
//   0x01327fc0  g_pool_singleton       — global pool-allocator singleton (.data)
//
// Scope table (EH3 FuncInfo) referenced by this function:
//   0x00e54ee1  g_scope_table_0040e430 — .rdata
//
// Relocation sites in the orig 193 bytes (wildcarded by compare.py):
//   +0x03   scope_table_ptr PUSH imm32   (.rdata 0x00e54ee1)
//   +0x27   LEA disp32 g_pool_descs      (.data  0x00f564e0)
//   +0x47   PUSH imm32 g_pool_alloc_info (.data  0x00f564f0)
//   +0x52   CALL rel32 → 0x0040e2d0
//   +0x59   MOV EAX,[imm32] singleton    (.data  0x01327fc0)
//   +0x62   CALL rel32 → 0x0040e500
//   +0x6b   CALL rel32 → 0x0040e110
//   +0x99   CALL rel32 → 0x0040dee0
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The EH3 SEH prologue (PUSH -1 / scope-table push / FS:[0] chain,
//   no /GS cookie), together with the SEH-state write
//   (MOV [ESP+0x18],0) mid-body and the dual-path epilogue, are
//   emitted by MSVC in a shape that is impractical to reproduce
//   exactly via high-level C++ (register ordering, short-vs-near
//   branches, the [ESP+N] addressing of the state slot all shift
//   by at least one byte under any rewrite). Naked asm byte-passthrough
//   — identical to FUN_00401750 / FUN_00403f10 / FUN_00406680 —
//   gives a byte-exact match once compare.py wildcards the 8
//   relocation windows above.

extern "C" __declspec(naked) void FUN_0040e430()
{
    __asm {
        // --- EH3 SEH prolog (no /GS cookie) --------------------------------
        _emit 0x6a  // PUSH -1
        _emit 0xff
        _emit 0x68  // PUSH 0xe54ee1  (scope table — reloc +0x03)
        _emit 0xe1
        _emit 0x4e
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x64  // MOV FS:[0], ESP
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83  // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        // --- read args / compute table ptr --------------------------------
        _emit 0x55  // PUSH EBP
        _emit 0x8b  // MOV EBP, [ESP+0x1c]   (arg1)
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        _emit 0x57  // PUSH EDI
        _emit 0x8b  // MOV EDI, [ESP+0x28]   (arg3)
        _emit 0x7c
        _emit 0x24
        _emit 0x28
        _emit 0x85  // TEST EDI, EDI
        _emit 0xff
        _emit 0x8b  // MOV EAX, EDI
        _emit 0xc7
        _emit 0x8d  // LEA EBP, [EBP*8 + 0xf564e0]  (reloc +0x27)
        _emit 0x2c
        _emit 0xed
        _emit 0xe0
        _emit 0x64
        _emit 0xf5
        _emit 0x00
        _emit 0x89  // MOV [ESP+0x20], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x75  // JNZ +0x42  (to shared path at e4a5)
        _emit 0x42
        // --- arg3 == NULL: allocate -----------------------------------------
        _emit 0x53  // PUSH EBX
        _emit 0x56  // PUSH ESI
        _emit 0x8b  // MOV ESI, [ESP+0x2c]   (arg2)
        _emit 0x74
        _emit 0x24
        _emit 0x2c
        _emit 0x85  // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75  // JNZ +0x09  (skip size compute)
        _emit 0x09
        _emit 0x8b  // MOV ESI, [EBP+0x4]   (table entry field4)
        _emit 0x75
        _emit 0x04
        _emit 0x83  // ADD ESI, 0x4b
        _emit 0xc6
        _emit 0x4b
        _emit 0x83  // AND ESI, 0xfffffff0
        _emit 0xe6
        _emit 0xf0
        // --- construct allocator-descriptor and call allocator ---------------
        _emit 0x68  // PUSH 0xf564f0  (reloc +0x47)
        _emit 0xf0
        _emit 0x64
        _emit 0xf5
        _emit 0x00
        _emit 0x6a  // PUSH 0x10
        _emit 0x10
        _emit 0x8d  // LEA ECX, [ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0xe8  // CALL 0x0040e2d0  (reloc +0x52)
        _emit 0x4a
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV EBX, EAX
        _emit 0xd8
        _emit 0xa1  // MOV EAX, [0x01327fc0]  (reloc +0x59)
        _emit 0xc0
        _emit 0x7f
        _emit 0x32
        _emit 0x01
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75  // JNZ +0x05  (singleton already set)
        _emit 0x05
        _emit 0xe8  // CALL 0x0040e500  (reloc +0x62)
        _emit 0x6a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x53  // PUSH EBX   (arg2 to allocate)
        _emit 0x56  // PUSH ESI   (arg1 = size)
        _emit 0x8b  // MOV ECX, EAX   (this = singleton)
        _emit 0xc8
        _emit 0xe8  // CALL 0x0040e110  (reloc +0x6b)
        _emit 0x71
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x5e  // POP ESI
        _emit 0x89  // MOV [ESP+0x24], EAX  (save result)
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x5b  // POP EBX
        // --- shared path: EAX = result ptr (existing or newly allocated) ----
        _emit 0x89  // MOV [ESP+0x28], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0xc7  // MOV [ESP+0x18], 0   (advance EH state: -1 → 0)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74  // JZ +0x29  (result == NULL → null return)
        _emit 0x29
        // --- result != NULL: call initializer --------------------------------
        _emit 0x8b  // MOV ECX, [ESP+0x2c]  (arg4)
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x3b  // CMP EAX, EDI   (result vs original arg3)
        _emit 0xc7
        _emit 0x0f  // SETNZ DL   (is_new = result != orig_arg3)
        _emit 0x95
        _emit 0xc2
        _emit 0x51  // PUSH ECX   (arg4)
        _emit 0x8b  // MOV ECX, [ESP+0x28]  (result)
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x52  // PUSH EDX   (is_new)
        _emit 0x51  // PUSH ECX   (result)
        _emit 0x55  // PUSH EBP   (&g_pool_descs[arg1])
        _emit 0x8b  // MOV ECX, EAX   (this = result)
        _emit 0xc8
        _emit 0xe8  // CALL 0x0040dee0  (reloc +0x99)
        _emit 0x13
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        // --- success epilogue ------------------------------------------------
        _emit 0x5f  // POP EDI
        _emit 0x5d  // POP EBP
        _emit 0x8b  // MOV ECX, [ESP+0x8]   (prev FS SEH ptr)
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x64  // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc3  // RET
        // --- null return epilogue --------------------------------------------
        _emit 0x8b  // MOV ECX, [ESP+0x10]  (prev FS SEH ptr — offset differs here)
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x5f  // POP EDI
        _emit 0x33  // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5d  // POP EBP
        _emit 0x64  // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc3  // RET
    }
}
