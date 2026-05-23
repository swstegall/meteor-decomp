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
// FUNCTION: ffxivgame 0x00011bc0 — allocator-gated factory for a
//                                   RemovableHeapSpace-like object (81 B / 0x51)
//
// __cdecl void* FUN_00411bc0(param_1, alloc, param_3, param_4, param_5)
//   [ESP+0x04] : param_1
//   [ESP+0x08] : alloc   — pointer to an e110_Obj allocator (thiscall target)
//   [ESP+0x0c] : param_3
//   [ESP+0x10] : param_4
//   [ESP+0x14] : param_5
//
// Sequence:
//   1. Build an 8-byte MemorySpace descriptor on the stack:
//      space.field0 = 0x10;  space.field4 = "CDev.Engine.Memory.Alternative"
//      via FUN_0040e2d0(__thiscall init, ECX=&space, args 0x10 and the string).
//   2. Call alloc->FUN_0040e110(0x84, &space) — allocate 0x84 bytes.
//   3. If allocation failed (null): return 0.
//   4. If allocation succeeded: call result->FUN_004119b0(param_1, alloc, param_3,
//      param_4, param_5) and return its result.
//
// Calling convention: __cdecl (5 stack args, plain RET — caller cleans).
// Callee-saves: ESI only (holds `alloc` throughout).
// Stack frame: SUB ESP, 0x8 for the MemorySpace local + PUSH ESI.
//
// Reconstruction strategy:
//   All three CALL rel32 bytes are masked by tools/compare.py (relocation
//   sites), so a source-level C++ form can match byte-for-byte provided
//   MSVC 2005 /O2 /Oy picks the same register allocation:
//   - ESI = alloc (callee-saved, loaded after the first CALL)
//   - EAX = alloc result (from FUN_0040e110)
//   - ECX = `this` argument for each thiscall
//
// String reference:
//   0xf56ca8 → "CDev.Engine.Memory.Alternative"

// Forward declarations matching the callee signatures.
// FUN_0040e2d0: __thiscall void init_space(this, int a, const char *name)
// FUN_0040e110: __thiscall void* alloc(this, int size, void *space)
// FUN_004119b0: __thiscall void* init_obj(this, int, void*, int, int, int)

struct MemSpace { int field0; const char *field4; };

class AllocObj {
public:
    void *FUN_0040e110(int size, MemSpace *space);
};

class ResultObj {
public:
    void *FUN_004119b0(int p1, AllocObj *p2, int p3, int p4, int p5);
};

class MemSpaceObj {
public:
    MemSpaceObj *FUN_0040e2d0(int a, const char *name);
};

extern "C" const char g_mem_name_alternative[];   // 0xf56ca8

extern "C" void * __cdecl FUN_00411bc0(int param_1, AllocObj *alloc, int param_3, int param_4, int param_5)
{
    MemSpace space;
    MemSpaceObj *sp = ((MemSpaceObj *)&space)->FUN_0040e2d0(0x10, g_mem_name_alternative);
    ResultObj *result = (ResultObj *)alloc->FUN_0040e110(0x84, (MemSpace *)sp);
    if (result != 0) {
        return result->FUN_004119b0(param_1, alloc, param_3, param_4, param_5);
    }
    return 0;
}
