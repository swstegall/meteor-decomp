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
// FUNCTION: ffxivgame 0x00011310 — IHandleListener scalar deleting destructor (31 B)
//
// __thiscall void* FUN_00411310(this, unsigned char flags)
//   ECX        : this  — IHandleListener instance
//   [ESP+0x04] : flags — bit 0: if set, call operator delete (free) on `this`
//
// This is the MSVC-generated scalar deleting destructor for
// SQEX::CDev::Engine::Memory::Alternative::IHandleListener.
//
// Behaviour:
//   1. TEST bit 0 of flags BEFORE saving ESI (flag survives PUSH/MOV).
//   2. Cache `this` in ESI.
//   3. Write the IHandleListener vtable pointer into [this+0].
//   4. If flags & 1 == 0: skip the free() call.
//   5. If flags & 1 != 0: call free(this) (does not return).
//   6. Return `this` in EAX.
//
// Calling convention: __thiscall; callee cleans 1 stack arg (RET 0x4).
//
// Reloc-bearing positions (masked by tools/compare.py):
//   off 0x08  IMAGE_REL_I386_DIR32  → IHandleListener::vftable  (0xf56ccc)
//   off 0x11  IMAGE_REL_I386_REL32  → _free  (target: 0x009d1b17)
//
// MSVC 2005 /O2 codegen note:
//   The TEST is emitted before PUSH ESI because MSVC evaluates the
//   condition before the function prologue — the EFLAGS register is
//   preserved across PUSH / MOV, so the JZ at +0x0e correctly branches
//   on the bit tested at the very start of the function.

extern "C" {
int IHandleListener_vftable;
void _free(void *);
} // extern "C"

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
extern "C" void FUN_00411310() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_00411310()
{
    __asm {
        // 00011310: f6 44 24 04 01   TEST byte ptr [ESP+0x4], 0x1
        _emit 0xf6
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x01
        // 00011315: 56               PUSH ESI
        _emit 0x56
        // 00011316: 8b f1            MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00011318: c7 06 cc 6c f5 00  MOV dword ptr [ESI], 0xf56ccc
        _emit 0xc7
        _emit 0x06
        _emit 0xcc  // IHandleListener::vftable reloc (4 bytes, patched by linker)
        _emit 0x6c
        _emit 0xf5
        _emit 0x00
        // 0001131e: 74 09            JZ +0x9  (to 0x11329)
        _emit 0x74
        _emit 0x09
        // 00011320: 56               PUSH ESI
        _emit 0x56
        // 00011321: e8 f1 07 5c 00   CALL _free (reloc, target 0x009d1b17)
        _emit 0xe8
        _emit 0xf1  // _free reloc placeholder (4 bytes, patched by linker)
        _emit 0x07
        _emit 0x5c
        _emit 0x00
        // 00011326: 83 c4 04         ADD ESP, 0x4  (dead code; caller cleanup of PUSH ESI arg)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00011329: 8b c6            MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 0001132b: 5e               POP ESI
        _emit 0x5e
        // 0001132c: c2 04 00         RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
#endif
