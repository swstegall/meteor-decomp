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
// FUNCTION: ffxivgame 0x005d56fd — `_aligned_free` — null-guarded aligned-block
//                                   deallocator (21 B / 0x15).
//
// Disassembly (RVA 0x005d56fd, 21 bytes):
//
//   8b 44 24 04    MOV  EAX, [ESP+4]       ; EAX = ptr argument
//   85 c0          TEST EAX, EAX           ; is ptr null?
//   74 0c          JZ   +0x0c              ; if null, jump to RET (offset 20)
//   83 e0 fc       AND  EAX, 0xfffffffc    ; align ptr (clear low 2 flag bits)
//   ff 70 fc       PUSH DWORD PTR [EAX-4]  ; push original (pre-aligned) pointer
//   e8 78 05 00 00 CALL _free              ; CRT free() — __cdecl, 1 arg
//   59             POP  ECX               ; cdecl stack cleanup (1 push * 4 bytes)
//   c3             RET                    ; epilogue — also target of the JZ
//
// Semantics:
//
//   The MSVC CRT `_aligned_malloc` / `_aligned_offset_malloc` family stores
//   the original (unaligned) heap pointer in the 4-byte slot immediately
//   BEFORE the aligned pointer returned to the caller.  When freeing:
//     1. No-ops on NULL (POSIX-compatible: free(NULL) is safe).
//     2. Clears the low 2 bits of the pointer (alignment-mode flags written
//        by the allocator) to recover the clean aligned base address.
//     3. Reads the original raw-heap pointer from [(aligned_base - 4)] and
//        passes it to `_free`, returning the block to the OS allocator.
//
//   Calling convention: `__cdecl` (plain `RET`; caller pops 4-byte arg).
//   No prologue (no EBP frame); /O2 /Oy; single arg cleaned with POP ECX.

extern "C" void __cdecl _free(void* p);

extern "C" void __cdecl FUN_009d56fd(void* ptr)
{
    if (ptr) {
        _free(((void**)((unsigned int)ptr & ~3U))[-1]);
    }
}
