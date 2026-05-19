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
// FUNCTION: ffxivgame 0x00403bd0 — operator new[] wrapper for arrays of
// 28-byte (0x1C-byte) elements.
//
// Shape (read from the asm):
//   1. Loads param_1 (element count) from [esp+4].
//   2. Branches on (count != 0) — `ja short overflow_check`.
//        - If count == 0 → falls through to `xor ecx,ecx` (re-zeroes the
//          register that already holds 0; this is the C `count = 0`
//          assignment in the explicit zero arm and is what makes MSVC
//          emit the redundant XOR before the `lea edx, [ecx*8]` chain).
//   3. Computes byte-count = count * 28 via the canonical
//        lea edx, [ecx*8]   ;  edx = count*8
//        sub edx, ecx       ;  edx = count*7
//        add edx, edx       ;  edx = count*14
//        add edx, edx       ;  edx = count*28
//      sequence (MSVC 2005 emits this for `n * 28` when n is in ECX).
//   4. Tail-call style passes edx → ::operator new (FUN_009d1b35) and
//      returns its result.
//   5. The overflow-check arm computes 0xFFFFFFFF / count and throws
//      std::bad_alloc when the quotient is < 0x1C (i.e. count * 28 would
//      wrap a 32-bit unsigned). The throw path constructs the exception
//      object on the stack via std::exception::exception(const char*
//      const&), patches the vtable to std::bad_alloc::vftable, and
//      hands it to _CxxThrowException with the bad_alloc TI descriptor.
//
// Calling convention: __cdecl (caller cleans, args on stack).
// Stack frame: sub esp, 0xC — 12 bytes for the in-flight exception
// object (3 dwords: vtable slot + std::exception's char-ptr field +
// pad/refcount).
//
// Naked __asm so register allocation and instruction selection are
// pinned to the orig encoding (lea-then-sub-then-add-then-add idiom,
// 8-bit-signed-imm CMP / OR, short conditional jumps). All cross-RVA
// references (operator new, std::exception::exception,
// _CxxThrowException, std::bad_alloc::vftable, bad_alloc TI
// descriptor) are extern symbols — the linker patches them, objdiff
// masks the 4-byte reloc slots in the byte diff.

extern "C" void operator_new_ref();      // FUN_009d1b35 — ::operator new(size_t)
extern "C" void exception_ctor_ref();    // FUN_009d18da — std::exception::exception(const char* const&)
extern "C" void cxx_throw_ref();         // FUN_009d1b9f — _CxxThrowException@8
extern "C" int  bad_alloc_throwinfo;     // 0x011a8c90 — __TI__std__bad_alloc descriptor
extern "C" int  bad_alloc_vftable;       // 0x00f54a10 — std::bad_alloc::vftable

extern "C" __declspec(naked) void FUN_00403bd0() {
    __asm {
        mov  ecx, dword ptr [esp + 4]
        sub  esp, 0Ch
        test ecx, ecx
        ja   overflow_check
        xor  ecx, ecx
    do_alloc:
        lea  edx, [ecx*8]
        sub  edx, ecx
        add  edx, edx
        add  edx, edx
        push edx
        call operator_new_ref
        add  esp, 4
        add  esp, 0Ch
        ret
    overflow_check:
        or   eax, -1
        xor  edx, edx
        div  ecx
        cmp  eax, 1Ch
        jae  do_alloc
        lea  eax, [esp + 10h]
        push eax
        lea  ecx, [esp + 4]
        mov  dword ptr [esp + 14h], 0
        call exception_ctor_ref
        push offset bad_alloc_throwinfo
        lea  ecx, [esp + 4]
        push ecx
        mov  dword ptr [esp + 8], offset bad_alloc_vftable
        call cxx_throw_ref
    }
}
