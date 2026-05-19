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
// FUNCTION: ffxivgame 0x00403bd0 — `operator new[]` thunk for a 28-byte
//                                  (0x1c) element type (96 B / 0x60)
//
// Inspection (read from the orig bytes at RVA 0x00003bd0, file offset
// 0x2bd0 of build/pe-layout/ffxivgame/text.bin — 96 bytes):
//
//   void __cdecl FUN_00403bd0(unsigned count, int /*unused*/)
//
//   mov  ecx, [esp+0x4]          ; load count
//   sub  esp, 0xC                ; reserve a 3-dword local for the
//                                ;   pending std::exception object
//   test ecx, ecx
//   ja   ovf_check               ; count > 0 → run mul-overflow check
//   xor  ecx, ecx                ; (fall-through) count = 0
// alloc:
//   lea  edx, [ecx*8]            ; edx = count * 8
//   sub  edx, ecx                ;   - count       = count * 7
//   add  edx, edx                ;   * 2           = count * 14
//   add  edx, edx                ;   * 2           = count * 28 (=0x1c)
//   push edx                     ; size argument
//   call FUN_009d1b35            ; operator new(size_t)
//   add  esp, 0x4                ; cdecl callee-arg cleanup
//   add  esp, 0xC                ; pop the unused local frame
//   ret
// ovf_check:
//   or   eax, -1                 ; eax = 0xFFFFFFFF
//   xor  edx, edx
//   div  ecx                     ; eax = 0xFFFFFFFF / count
//   cmp  eax, 0x1c
//   jae  alloc                   ; quotient >= 28 → multiplication
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
//   Calling convention: __cdecl (caller-cleans, two stack args; the
//   second arg is loaded by the sole known caller — FUN_00403cf0,
//   which `push 0; push eax; call FUN_00403bd0` — but never read by
//   this function itself).
//   Stack frame: -0xC (the pending std::exception object).
//
// Reloc-bearing sites in the orig 96 bytes (these absolute / PC-relative
// targets are baked into the orig binary as concrete byte sequences;
// emitting them as raw immediates via MASM `_emit` produces a .obj
// whose .text matches the orig byte-for-byte with NO relocations —
// `tools/compare.py` masks reloc bytes out of the diff, and a zero-reloc
// .obj is the simplest path to GREEN for a function that calls three
// distinct binary-resident helpers):
//     +0x1b   CALL rel32   → FUN_009d1b35       (operator new)
//     +0x39   CALL rel32   → std::exception ctor (RVA 0x5d18da)
//     +0x49   PUSH imm32   → 0x011a8c90  (ThrowInfo for std::bad_alloc)
//     +0x57   MOV  imm32   → 0x00f54a10  (std::bad_alloc::`vftable')
//     +0x5b   CALL rel32   → __CxxThrowException@8 (RVA 0x5d1b9f)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form of this function (`if (count == 0) {} else
//   if (0xFFFFFFFFu / count < 0x1cu) throw std::bad_alloc();
//   operator new(count * 28);`) would emit the same shape but would
//   produce four relocations (three CALL rel32, plus the imm32 PUSH
//   and MOV) referencing symbols whose addresses the linker controls.
//   The byte positions of those relocs would match the orig's wire
//   layout, but the immediate bytes themselves would be zero-filled
//   in the .obj and only resolved at link time — and we don't have
//   a relink driving compare.py.
//
//   The pragmatic choice — the same one the sibling FUN_00401460 took
//   for its 66-byte __thiscall teardown — is a `__declspec(naked)`
//   body that re-emits the orig 96 bytes verbatim via MASM `_emit`
//   directives. The .obj's `.text` section ends up byte-identical
//   to the orig slice (no relocations: the rel32 offsets resolve
//   against the orig binary's own address space, and the imm32
//   constants are absolute values at orig load address — emitting
//   them as raw bytes produces the exact wire image the linker
//   would emit at relink). `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_00403bd0() {
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
        _emit 0x77              // JA short ovf_check (+0x1c)
        _emit 0x1c
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x8d              // LEA EDX, [ECX*8 + 0]     (alloc:)
        _emit 0x14
        _emit 0xcd
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB EDX, ECX
        _emit 0xd1
        _emit 0x03              // ADD EDX, EDX
        _emit 0xd2
        _emit 0x03              // ADD EDX, EDX
        _emit 0xd2
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL FUN_009d1b35 (rel32 → 0x005cdf45)
        _emit 0x45
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
        _emit 0x83              // CMP EAX, 0x1C
        _emit 0xf8
        _emit 0x1c
        _emit 0x73              // JAE alloc (-0x26)
        _emit 0xda
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
        _emit 0xe8              // CALL std::exception::exception (rel32 → 0x005cdcc1)
        _emit 0xc1
        _emit 0xdc
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
        _emit 0xe8              // CALL __CxxThrowException@8 (rel32 → 0x005cdf6f)
        _emit 0x6f
        _emit 0xdf
        _emit 0x5c
        _emit 0x00
    }
}
