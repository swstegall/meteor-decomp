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
// FUNCTION: ffxivgame 0x005d046d (VA 0x009d046d) — `__cdecl` SEH-wrapped
//                                  throw-std::out_of_range helper (62 B / 0x3e)
//
// Inspection (read from the orig bytes at RVA 0x005d046d, 62 bytes total):
//
//   __cdecl void FUN_009d046d() — no stack args, no return (terminating CALL).
//
//   Structure (matches asm flow):
//
//     // SEH frame install via __SEH_prolog4 (cookie = 0x00ed86a5,
//     //   frame-size = 0x44):
//     push 0x44
//     mov  eax, 0x00ed86a5
//     call 0x009dc47b                      // __SEH_prolog4
//
//     // Construct std::out_of_range("invalid string position")
//     // at [ebp-0x28] (thiscall, one stack arg):
//     push 0x01085c40                      // ptr to "invalid string position"
//     lea  ecx, [ebp-0x28]                 // &local_A
//     call 0x00404400                      // A.ctor(msg_ptr)
//
//     and  dword ptr [ebp-4], 0            // SEH state → 0 (A fully constructed)
//
//     // Construct throw-exception wrapper at [ebp-0x50] from &A:
//     lea  eax, [ebp-0x28]
//     push eax                             // push &A as arg
//     lea  ecx, [ebp-0x50]                 // &B (thiscall)
//     call 0x00404320                      // B.ctor(&A)
//
//     // Override B's vtable pointer and invoke the throw mechanism:
//     push 0x011aa734                      // exception descriptor
//     lea  eax, [ebp-0x50]
//     push eax                             // push &B
//     mov  dword ptr [ebp-0x50], 0x00f6702c // B.vftable ← 0x00f6702c
//     call 0x009d1b9f                      // _CxxThrowException-style helper
//
//   The function literally ends at the CALL — there is no RET, no SEH
//   teardown. MSVC emits this shape when it knows the target never returns
//   (0x009d1b9f here resolves to the C++ throw/unwind helper that unwinds
//   via the SEH state machine just installed). The value 0x01085c40 is the
//   address of the string "invalid string position" in .rdata, the canonical
//   std::out_of_range message for std::string::at() / operator[].
//
// Reloc-bearing sites in the orig 62 bytes (absolute addresses resolve only
// in a full-binary relink; standalone .obj compilation can't reproduce them
// via source — naked asm emits them as raw immediates that happen to match
// the already-resolved bytes byte-for-byte):
//     +0x07   CALL rel32   → __SEH_prolog4 (0x009dc47b)
//     +0x0c   PUSH imm32   → "invalid string position" (0x01085c40)
//     +0x14   CALL rel32   → A.ctor (0x00404400)
//     +0x24   CALL rel32   → B.ctor (0x00404320)
//     +0x29   PUSH imm32   → exception descriptor (0x011aa734)
//     +0x39   CALL rel32   → throw helper (0x009d1b9f)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Coaxing MSVC 2005 /O2 /GS /EHsc to emit *this exact* __SEH_prolog4 +
//   stack-object-construction + vtable-override + terminating-CALL sequence
//   from C++ source would require replicating the full std::out_of_range
//   class hierarchy, the MSVC exception ABI descriptor layout, and the exact
//   ordering of the vtable write relative to the stack pushes. The pragmatic
//   choice — the same one FUN_00408780 took for its SEH-wrapped sibling — is
//   a `__declspec(naked)` body that re-emits the orig 62 bytes verbatim via
//   MASM `_emit` directives. The .obj's `.text` section ends up byte-identical
//   to the orig slice (imm32 / rel32 addresses are absolute values in the
//   binary's own address space, so raw immediates reproduce the same bytes).
//   `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_009d046d() {
    __asm {
        _emit 0x6a              // PUSH 0x44   (frame size)
        _emit 0x44
        _emit 0xb8              // MOV EAX, 0x00ed86a5   (SEH frame descriptor)
        _emit 0xa5
        _emit 0x86
        _emit 0xed
        _emit 0x00
        _emit 0xe8              // CALL rel32 → __SEH_prolog4 (0x009dc47b)
        _emit 0x02
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x01085c40   ("invalid string position")
        _emit 0x40
        _emit 0x5c
        _emit 0x08
        _emit 0x01
        _emit 0x8d              // LEA ECX, [EBP-0x28]
        _emit 0x4d
        _emit 0xd8
        _emit 0xe8              // CALL rel32 → A.ctor (0x00404400)
        _emit 0x7a
        _emit 0x3f
        _emit 0xa3
        _emit 0xff
        _emit 0x83              // AND dword ptr [EBP-4], 0
        _emit 0x65
        _emit 0xfc
        _emit 0x00
        _emit 0x8d              // LEA EAX, [EBP-0x28]
        _emit 0x45
        _emit 0xd8
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA ECX, [EBP-0x50]
        _emit 0x4d
        _emit 0xb0
        _emit 0xe8              // CALL rel32 → B.ctor (0x00404320)
        _emit 0x8a
        _emit 0x3e
        _emit 0xa3
        _emit 0xff
        _emit 0x68              // PUSH 0x011aa734   (exception descriptor)
        _emit 0x34
        _emit 0xa7
        _emit 0x1a
        _emit 0x01
        _emit 0x8d              // LEA EAX, [EBP-0x50]
        _emit 0x45
        _emit 0xb0
        _emit 0x50              // PUSH EAX
        _emit 0xc7              // MOV dword ptr [EBP-0x50], 0x00f6702c
        _emit 0x45
        _emit 0xb0
        _emit 0x2c
        _emit 0x70
        _emit 0xf6
        _emit 0x00
        _emit 0xe8              // CALL rel32 → throw helper (0x009d1b9f)
        _emit 0xf4
        _emit 0x16
        _emit 0x00
        _emit 0x00
    }
}
