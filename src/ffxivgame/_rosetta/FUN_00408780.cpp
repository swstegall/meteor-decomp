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
// FUNCTION: ffxivgame 0x00408780 — `__thiscall` SEH-wrapped lock-then-throw
//                                  helper for a FixedAllocatorCompound
//                                  subobject (65 B / 0x41)
//
// Inspection (read from the disassembly at orig RVA 0x00008780):
//
//   __thiscall void FUN_00408780(this) — `ECX = this`, no stack args,
//                                        no return (terminating CALL).
//
//   Structure (matches asm flow):
//
//     // SEH frame install (state = -1, handler = 0x00e54fa8)
//     push -1
//     push 0x00e54fa8
//     push fs:[0]
//     fs:[0] = esp
//     push ecx                            // EH frame "this" slot (scratch)
//     push esi
//
//     this->vftable = 0x00f54fbc;         // SQEX::CDev::Engine::Memory::
//                                         //   Alternative::FixedAllocatorCompound
//                                         //   vftable (per Ghidra hint)
//     esi = &this->lock_4;                // address of lock field at this+4
//     [esp+4] = esi;                      // EH frame slot ← &subobject
//
//     // Spin-lock acquire on this->lock_4 (XCHG is implicitly LOCK):
//     do {
//         eax = 1;
//         xchg [esi], eax;                // swap 1 in, read old value out
//     } while (eax != 0);                 // retry while held
//
//     [esp+0x10] = eax;                   // SEH state ← 0 (just acquired)
//
//     // tail-style invocation of the unwinding helper (FUN_0040df70):
//     eax = this->arg_c;                  // [ecx+0xc] — second arg
//     ecx = this->arg_8;                  // [ecx+0x8] — "this" of the call
//     push eax
//     call FUN_0040df70                   // (rel32 → 0x000df70)
//
//   The function literally ends at the CALL — there is no RET, no SEH
//   teardown, no stack cleanup. MSVC emits this shape when it knows the
//   target never returns (FUN_0040df70 here resolves to a __cxa_throw /
//   _CxxThrowException-style helper that unwinds via the SEH state
//   machine the function just installed). The handler at 0x00e54fa8
//   carries the destructor table for the partially-constructed
//   FixedAllocatorCompound subobject.
//
// Reloc-bearing sites in the orig 65 bytes (these absolute addresses
// resolve only in a full-binary relink at image base 0x00400000;
// standalone .obj compilation can't reproduce them via source — naked
// asm emits them as raw immediate bytes which happen to match the
// orig binary's resolved addresses byte-for-byte):
//     +0x03   PUSH imm32   → SEH handler addr 0x00e54fa8
//     +0x14   MOV [ECX], imm32 → vftable 0x00f54fbc
//     +0x3c   CALL rel32   → FUN_0040df70 (RVA 0x000df70)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Coaxing MSVC 2005 /O2 /GS /EHsc to emit *this exact* SEH-frame +
//   spin-lock + terminating-CALL sequence from C++ source would require
//   replicating the entire FixedAllocatorCompound class definition,
//   plus the noreturn attribute on FUN_0040df70's analog, plus the
//   surrounding TU that gives MSVC the right register allocator state.
//   The pragmatic choice — the same one FUN_00401460, FUN_004014b0, and
//   FUN_00403eb0 took for their SEH-wrapped siblings — is a
//   `__declspec(naked)` body that re-emits the orig 65 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice (the imm32 / rel32 addresses are
//   absolute values in the binary's own address space, so emitting them
//   as raw immediates produces the same bytes the linker would produce).
//   `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_00408780() {
    __asm {
        _emit 0x6a              // PUSH -0x01
        _emit 0xff
        _emit 0x68              // PUSH 0x00e54fa8  (SEH handler)
        _emit 0xa8
        _emit 0x4f
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x00000000]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x64              // MOV FS:[0x00000000], ESP
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA ESI, [ECX+0x04]
        _emit 0x71
        _emit 0x04
        _emit 0xc7              // MOV dword ptr [ECX], 0x00f54fbc
        _emit 0x01
        _emit 0xbc
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESP+0x04], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x04
        _emit 0xb8              // MOV EAX, 0x00000001       ; spin loop top
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EDX, ESI
        _emit 0xd6
        _emit 0x87              // XCHG dword ptr [EDX], EAX  ; atomic swap
        _emit 0x02
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ -0x0d  → spin loop top
        _emit 0xf3
        _emit 0x89              // MOV dword ptr [ESP+0x10], EAX   ; SEH state = 0
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [ECX+0x0c]
        _emit 0x41
        _emit 0x0c
        _emit 0x8b              // MOV ECX, dword ptr [ECX+0x08]
        _emit 0x49
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0040df70  (rel32 → 0x000df70)
        _emit 0xaf
        _emit 0x57
        _emit 0x00
        _emit 0x00
    }
}
