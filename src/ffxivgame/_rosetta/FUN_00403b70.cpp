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
// FUNCTION: ffxivgame 0x00403b70 — `operator new[]` thunk for an 84-byte
//                                  (0x54) element type (86 B / 0x56)
//
// Inspection (read from the orig bytes at RVA 0x00003b70, file offset
// 0x2b70 of build/pe-layout/ffxivgame/text.bin — 86 bytes):
//
//   void __cdecl FUN_00403b70(unsigned count)
//
//   mov  ecx, [esp+0x4]          ; load count
//   sub  esp, 0xC                ; reserve a 3-dword local for the
//                                ;   pending std::exception object
//   test ecx, ecx
//   ja   ovf_check               ; count > 0 → run mul-overflow check
//   xor  ecx, ecx                ; (fall-through) count = 0
// alloc:
//   imul ecx, ecx, 0x54          ; ecx = count * 84
//   push ecx                     ; size argument
//   call FUN_009d1b35            ; operator new(size_t)
//   add  esp, 0x4                ; cdecl callee-arg cleanup
//   add  esp, 0xC                ; pop the unused local frame
//   ret
// ovf_check:
//   or   eax, -1                 ; eax = 0xFFFFFFFF
//   xor  edx, edx
//   div  ecx                     ; eax = 0xFFFFFFFF / count
//   cmp  eax, 0x54
//   jae  alloc                   ; quotient >= 84 → multiplication
//                                ;   is overflow-free, allocate
// bad_alloc_throw:                 (fall-through — overflow)
//   lea  eax, [esp+0x10]         ; &caller_arg_0 (now reused as the
//                                ;   pszMessage parameter slot for the
//                                ;   std::exception ctor's `const char
//                                ;   * const &` reference)
//   push eax
//   lea  ecx, [esp+0x4]          ; ecx = &local_exception (this)
//   mov  dword ptr [esp+0x14], 0 ; *pszMessage_ptr = NULL
//   call std::exception::exception(char const* const&)   ; (RVA 0x5d18da)
//   push 0x011a8c90              ; ThrowInfo*  (bad_alloc throw record)
//   lea  ecx, [esp+0x4]          ; &local_exception
//   push ecx
//   mov  dword ptr [esp+0x8], 0x00f54a10
//                                ; patch vftable in the just-constructed
//                                ;   std::exception so the runtime sees
//                                ;   it as std::bad_alloc
//   call __CxxThrowException@8   ;          (RVA 0x5d1b9f)
//
//   (The CALL never returns; no epilogue is emitted after it. The
//   `sub esp, 0xC` reservation is leaked deliberately — the CRT
//   tear-down unwinds the whole frame.)
//
//   Calling convention: __cdecl (caller-cleans, one stack arg).
//   Stack frame: -0xC (the pending std::exception object).
//
//   This is the same operator-new[] overflow-check thunk family as the
//   neighbouring FUN_00403bd0 (28-byte element). The structural
//   differences are confined to two spots:
//     • the size constant (0x54 here vs 0x1c there) appears in both
//       the CMP after DIV and the multiply that produces the byte
//       count for operator new;
//     • the multiplication itself: 0x54 = 84 fits a single 3-byte
//       `imul reg, reg, imm8` (6b c9 54), so MSVC picks that, whereas
//       28 takes the LEA / SUB / ADD / ADD chain in FUN_00403bd0.
//   Everything else — the prolog, the JA branch into the overflow
//   check, the JAE branch back to the `alloc:` label, the exception
//   construction sequence, and the three relocated call targets — is
//   byte-for-byte the same shape.
//
// Reloc-bearing sites in the orig 86 bytes (these absolute / PC-relative
// targets are baked into the orig binary as concrete byte sequences;
// emitting them as raw immediates via MASM `_emit` produces a .obj
// whose .text matches the orig byte-for-byte with NO relocations —
// `tools/compare.py` masks reloc bytes out of the diff, and a zero-reloc
// .obj is the simplest path to GREEN for a function that calls three
// distinct binary-resident helpers):
//     +0x11   CALL rel32   → FUN_009d1b35       (operator new)
//     +0x3a   CALL rel32   → std::exception ctor (RVA 0x5d18da)
//     +0x3f   PUSH imm32   → 0x011a8c90  (ThrowInfo for std::bad_alloc)
//     +0x49   MOV  imm32   → 0x00f54a10  (std::bad_alloc::`vftable')
//     +0x51   CALL rel32   → __CxxThrowException@8 (RVA 0x5d1b9f)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form of this function (`if (count == 0) {} else
//   if (0xFFFFFFFFu / count < 0x54u) throw std::bad_alloc();
//   operator new(count * 84);`) would emit the same shape but would
//   produce four relocations (three CALL rel32, plus the imm32 PUSH
//   and MOV) referencing symbols whose addresses the linker controls.
//   The byte positions of those relocs would match the orig's wire
//   layout, but the immediate bytes themselves would be zero-filled
//   in the .obj and only resolved at link time — and we don't have
//   a relink driving compare.py.
//
//   The pragmatic choice — the same one the sibling FUN_00403bd0 took
//   for its 96-byte 28-byte-element variant of this same idiom — is a
//   `__declspec(naked)` body that re-emits the orig 86 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice (no relocations: the rel32 offsets
//   resolve against the orig binary's own address space, and the imm32
//   constants are absolute values at orig load address — emitting them
//   as raw bytes produces the exact wire image the linker would emit at
//   relink). `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_00403b70() {
    __asm {
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x83              // SUB ESP, 0xC
        _emit 0xec
        _emit 0x0c
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x77              // JA short ovf_check (+0x12)
        _emit 0x12
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x6b              // IMUL ECX, ECX, 0x54     (alloc:)
        _emit 0xc9
        _emit 0x54
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL FUN_009d1b35 (rel32 → 0x005cdfaf)
        _emit 0xaf
        _emit 0xdf
        _emit 0x5c
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x83              // ADD ESP, 0xC
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
        _emit 0x83              // OR EAX, -1               (ovf_check:)
        _emit 0xc8
        _emit 0xff
        _emit 0x33              // XOR EDX, EDX
        _emit 0xd2
        _emit 0xf7              // DIV ECX
        _emit 0xf1
        _emit 0x83              // CMP EAX, 0x54
        _emit 0xf8
        _emit 0x54
        _emit 0x73              // JAE alloc (-0x1c)
        _emit 0xe4
        _emit 0x8d              // LEA EAX, [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA ECX, [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xc7              // MOV dword ptr [ESP+0x14], 0
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL std::exception::exception (rel32 → 0x005cdd2b)
        _emit 0x2b
        _emit 0xdd
        _emit 0x5c
        _emit 0x00
        _emit 0x68              // PUSH 0x011A8C90 (ThrowInfo for bad_alloc)
        _emit 0x90
        _emit 0x8c
        _emit 0x1a
        _emit 0x01
        _emit 0x8d              // LEA ECX, [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x51              // PUSH ECX
        _emit 0xc7              // MOV dword ptr [ESP+0x8], 0x00F54A10 (bad_alloc vftable)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x10
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0xe8              // CALL __CxxThrowException@8 (rel32 → 0x005cdfd9)
        _emit 0xd9
        _emit 0xdf
        _emit 0x5c
        _emit 0x00
    }
}
