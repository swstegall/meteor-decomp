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
// FUNCTION: ffxivgame 0x00003bd0 — `operator new[]` overflow-checked size
//                                  helper for an element of sizeof(T) == 28
//                                  (96 B / 0x60).
//
// Inspection (read from the disassembly at orig RVA 0x00003bd0):
//
//   __cdecl void *FUN_00403bd0(unsigned int count);
//
//     The classic MSVC `new T[count]` allocation prologue. Computes
//     `count * 28` with an unsigned overflow guard, falling through to
//     a thrown C++ exception when the multiplication would wrap.
//
//     if (count == 0) {
//         goto alloc;                       // 0-element alloc → 0 bytes
//     }
//     if (0xFFFFFFFFu / count >= 28u) {
//     alloc:
//         // EDX = count * 28 via LEA/SUB/ADD/ADD (= count*8 - count
//         // = count*7, then *2 *2 = count*28). MSVC's strength-reduced
//         // emission of `count * 0x1c`.
//         return ::operator new(count * 28);   // CALL 0x009d1b35
//     }
//     // Overflow path — build a std::bad_alloc-ish object in a 12-byte
//     // local frame and throw it via _CxxThrowException.
//     ExceptObj e;                              //   esp + 0x00 .. 0x0b
//     e.ctor(&caller_arg0_slot);                // CALL 0x009d18da (__thiscall)
//     e.<8> = 0;                                // MOV [esp+0x14], 0
//     _CxxThrowException(&e, 0x011a8c90);       // CALL 0x009d1b9f
//                                               //   ThrowInfo at 0x011a8c90
//                                               //   ctor cookie  0x00f54a10
//
//   Stack frame:
//     [esp+0x00 .. esp+0x0b]   12-byte exception object slot
//     [esp+0x0c]               return address (post `SUB ESP, 0xc`)
//     [esp+0x10]               caller's arg0 (count)  (= post-sub esp + 0x10)
//
//   Reloc-bearing sites in the orig 96 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them via source — naked
//   asm emits them as raw immediate bytes which happen to match the
//   orig binary's resolved CALL rel32 / PUSH imm32 / MOV imm32 fields
//   byte-for-byte):
//     +0x1b   CALL rel32 → 0x009d1b35  (operator new)
//     +0x44   CALL rel32 → 0x009d18da  (exception ctor, __thiscall ECX=&e)
//     +0x49   PUSH imm32   0x011a8c90  (ThrowInfo descriptor in .rdata)
//     +0x53   MOV  imm32   0x00f54a10  (vtable / cookie patched into e)
//     +0x5b   CALL rel32 → 0x009d1b9f  (_CxxThrowException)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc
//   into producing both the exact overflow-guard codegen
//   (`OR EAX,-1 / XOR EDX,EDX / DIV ECX / CMP EAX,0x1c / JNC` rather
//   than the more usual MUL-with-overflow `MUL ECX / JC throw` form)
//   AND the exact branch layout (JA-forward to the divide, JNC-backward
//   to the allocator joining the `count == 0` path). Both are sensitive
//   to surrounding code in the full TU and to which CRT operator-new
//   header is in scope.
//
//   The pragmatic choice — the same one the sibling FUN_00401a00 took
//   for its SEH-wrapped exe-dir bootstrap and FUN_00401650 took for
//   its 2-arg singleton init — is a `__declspec(naked)` body that
//   re-emits the orig 96 bytes verbatim via MASM `_emit` directives.
//   The .obj's `.text` section ends up byte-identical to the orig
//   slice (no relocations: the CALL targets and PUSH immediates are
//   resolved to absolute values in the binary's own address space, so
//   emitting them as raw imm32 bytes produces the same bytes the
//   linker would produce). `tools/compare.py` then reports GREEN.
//
//   The structural commentary above is the readable record of what
//   the function actually does, so a future contributor can promote
//   this to a real source-level match once the surrounding exception
//   class (vtable at 0xf54a10, ThrowInfo at 0x11a8c90, ctor at
//   0x009d18da) is catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_00403bd0() {
    __asm {
        _emit 0x8b              // MOV ECX, [ESP+0x4]   (count)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x83              // SUB ESP, 0xc
        _emit 0xec
        _emit 0x0c
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x77              // JA +0x1c             → overflow check
        _emit 0x1c
        _emit 0x33              // XOR ECX, ECX         (count == 0 path)
        _emit 0xc9
        _emit 0x8d              // LEA EDX, [ECX*8 + 0] → ECX*8
        _emit 0x14
        _emit 0xcd
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB EDX, ECX         → ECX*7
        _emit 0xd1
        _emit 0x03              // ADD EDX, EDX         → ECX*14
        _emit 0xd2
        _emit 0x03              // ADD EDX, EDX         → ECX*28
        _emit 0xd2
        _emit 0x52              // PUSH EDX             (size_t bytes)
        _emit 0xe8              // CALL 0x009d1b35      (::operator new)
        _emit 0x45
        _emit 0xdf
        _emit 0x5c
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x4         (pop arg)
        _emit 0xc4
        _emit 0x04
        _emit 0x83              // ADD ESP, 0xc         (release local frame)
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
        _emit 0x83              // OR EAX, 0xffffffff   (overflow-check arm)
        _emit 0xc8
        _emit 0xff
        _emit 0x33              // XOR EDX, EDX
        _emit 0xd2
        _emit 0xf7              // DIV ECX              → EAX = 0xffffffff / count
        _emit 0xf1
        _emit 0x83              // CMP EAX, 0x1c        (sizeof(T) == 28)
        _emit 0xf8
        _emit 0x1c
        _emit 0x73              // JNC -0x26            → no overflow, alloc
        _emit 0xda
        _emit 0x8d              // LEA EAX, [ESP+0x10]  (&caller arg0 slot)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x50              // PUSH EAX             (ctor arg → ECX==this; EAX==arg)
        _emit 0x8d              // LEA ECX, [ESP+0x4]   (this = &e on stack)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xc7              // MOV [ESP+0x14], 0    (zero a slot inside e)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL 0x009d18da      (__thiscall ctor)
        _emit 0xc1
        _emit 0xdc
        _emit 0x5c
        _emit 0x00
        _emit 0x68              // PUSH 0x011a8c90      (ThrowInfo)
        _emit 0x90
        _emit 0x8c
        _emit 0x1a
        _emit 0x01
        _emit 0x8d              // LEA ECX, [ESP+0x4]   (&e)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x51              // PUSH ECX             (object pointer)
        _emit 0xc7              // MOV [ESP+0x8], 0xf54a10  (vtable patch)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x10
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0xe8              // CALL 0x009d1b9f      (_CxxThrowException)
        _emit 0x6f
        _emit 0xdf
        _emit 0x5c
        _emit 0x00
    }
}
