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
// FUNCTION: ffxivgame 0x00019cc0 — __cdecl null-guarded allocator-free wrapper.
//
// __cdecl twin of engine_memory_global_free (FUN_0040a510, the __stdcall
// tail-call variant). The custom allocator stores a back-pointer to its
// owning instance in the 4 bytes immediately before the user-visible block.
// This wrapper null-guards the input pointer, loads the Allocator* from
// [p - 4] into ECX, and calls the __thiscall Free method (FUN_0040df70).
//
// Calling convention: __cdecl (plain RET — caller cleans the one inbound arg).
//
// Asm (18 bytes @ RVA 0x00019cc0):
//   8b 44 24 04        MOV  EAX, [ESP+4]    ; eax = p
//   85 c0              TEST EAX, EAX
//   74 09              JZ   done            ; skip if null
//   8b 48 fc           MOV  ECX, [EAX-4]   ; ecx = owning Allocator*
//   50                 PUSH EAX            ; push p as Free() argument
//   e8 RR RR RR RR     CALL FUN_0040df70   ; __thiscall Free(p)
// done:
//   c3                 RET                 ; __cdecl, caller cleans arg
//
// The CALL target (FUN_0040df70) is a __thiscall method with one stack
// argument; the callee pops the argument (RET 4 in its epilogue), leaving
// no ADD ESP cleanup needed here before the final RET.

struct Allocator {
    void Free(void* p);
};

extern "C" void __cdecl FUN_00419cc0(void* p) {
    if (p) {
        Allocator* alloc = *(Allocator**)((char*)p - 4);
        alloc->Free(p);
    }
}
