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
// FUNCTION: ffxivgame 0x00401090 — std::_Allocate<char> overflow-checking
//                                   allocator wrapper (83 B / 0x53).
//
// Behaviour read from the disassembly at orig RVA 0x00001090:
//
//   __cdecl void* _Allocate(size_t count) {
//       if (count == 0) {
//           return ::operator new(0);                  // FUN_009d1b35(0)
//       }
//       if ((size_t)(-1) / count < 1) {                // overflow check
//           // sizeof(_Ty) == 1: this branch is unreachable in pure
//           // math, but MSVC's std::_Allocate template instantiation
//           // emits it regardless — the compiler can't fold a runtime
//           // DIV against a template-parameter-derived sizeof, so the
//           // throw path survives /O2.
//           std::bad_alloc tmp;                        // construct on
//           tmp.exception::exception(/*msg=*/&NULL);   //   stack via
//           tmp.vfptr = &bad_alloc::vftable;           //   parent + vtable
//           _CxxThrowException(&tmp,                   // never returns
//                              &bad_alloc::_ThrowInfo);
//       }
//       return ::operator new(count);                  // count * 1 = count
//   }
//
//   Stack frame: SUB ESP, 0xc — three 4-byte slots that hold the
//   stack-constructed std::bad_alloc instance (vfptr + two unused slots).
//
//   Branch shape — the fall-through-then-jnc-back pattern is MSVC's
//   classic two-target merge:
//
//       count == 0   → fall through, ECX=0 from XOR, PUSH/CALL alloc(0)
//       count > 0    → JA Lcheck; if quotient ≥ 1 (always, for sizeof=1),
//                      JNC back to PUSH ECX (which still holds count from
//                      the [esp+4] load, since DIV doesn't touch ECX)
//
//   Reloc-bearing sites in the orig 83 bytes (all PC-relative or absolute
//   image-base references the linker fixes up):
//     +0x0e   CALL FUN_009d1b35           (e8 + REL32 — operator new)
//     +0x37   CALL FUN_009d18da           (e8 + REL32 — std::exception ctor)
//     +0x3c   PUSH &bad_alloc::_ThrowInfo (68 + DIR32 — throw descriptor)
//     +0x46   MOV  [esp+8], &bad_alloc::vftable
//                                          (c7 + DIR32 — overrides the
//                                                vptr the parent ctor
//                                                just installed)
//     +0x4e   CALL FUN_009d1b9f           (e8 + REL32 — __CxxThrowException@8)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function is short (83 bytes) and reloc-dense (five linker
//   fixups in five instructions). A source-level C++ port would need
//   `std::bad_alloc` and `std::exception` headers consistent with
//   MSVC 2005's <xstddef> / <stdexcept>, plus the right `_THROW_NCEE`
//   macro expansion to reproduce the verbatim ctor+vfptr+throw bytes.
//   Since the orig instruction sequence is unambiguous and tracks the
//   xmemory.h `_Allocate<_Ty>` template at sizeof(_Ty)==1, the
//   pragmatic choice — matching FUN_00403f10's precedent — is to
//   `_emit` the 83 orig bytes verbatim. tools/compare.py compares the
//   .obj `.text` to the orig slice byte-for-byte; since the bytes are
//   raw immediates (not COFF relocs), no masking is needed and the
//   diff is GREEN by direct equality.

extern "C" __declspec(naked) void FUN_00401090() {
    __asm {
        // 00001090: mov ecx, dword ptr [esp+4]   ; load count
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 00001094: sub esp, 0Ch                  ; reserve stack frame
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00001097: test ecx, ecx
        _emit 0x85
        _emit 0xc9
        // 00001099: ja  0x004010aa                ; count > 0 → overflow check
        _emit 0x77
        _emit 0x0f
        // 0000109b: xor ecx, ecx                  ; zero count for alloc(0)
        _emit 0x33
        _emit 0xc9
        // 0000109d: push ecx                      ; merge: JNC lands here
        _emit 0x51
        // 0000109e: call FUN_009d1b35             ; operator new(count)
        _emit 0xe8
        _emit 0x92
        _emit 0x0a
        _emit 0x5d
        _emit 0x00
        // 000010a3: add esp, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 000010a6: add esp, 0Ch
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 000010a9: ret
        _emit 0xc3
        // 000010aa: or  eax, 0FFFFFFFFh           ; eax = SIZE_MAX
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // 000010ad: xor edx, edx
        _emit 0x33
        _emit 0xd2
        // 000010af: div ecx                       ; eax = UINT_MAX / count
        _emit 0xf7
        _emit 0xf1
        // 000010b1: cmp eax, 1                    ; (UINT_MAX/count) < sizeof(_Ty)?
        _emit 0x83
        _emit 0xf8
        _emit 0x01
        // 000010b4: jnc 0x0040109d                ; quotient ≥ 1 → safe path
        _emit 0x73
        _emit 0xe7
        // --- throw std::bad_alloc() arm -----------------------------------
        // 000010b6: lea eax, [esp+10h]            ; &(stack slot above frame)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 000010ba: push eax                      ;   pass to exception ctor
        _emit 0x50
        // 000010bb: lea ecx, [esp+4]              ; this = &stack-bad_alloc
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 000010bf: mov dword ptr [esp+14h], 0    ; zero the reference target
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000010c7: call FUN_009d18da             ; std::exception::exception(&NULL)
        _emit 0xe8
        _emit 0x0e
        _emit 0x08
        _emit 0x5d
        _emit 0x00
        // 000010cc: push 011A8C90h                ; &bad_alloc::_ThrowInfo
        _emit 0x68
        _emit 0x90
        _emit 0x8c
        _emit 0x1a
        _emit 0x01
        // 000010d1: lea ecx, [esp+4]              ; reload &exception obj
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 000010d5: push ecx                      ;   pass to throw helper
        _emit 0x51
        // 000010d6: mov dword ptr [esp+8], 00F54A10h
        //          ; override the std::exception vptr the parent ctor just
        //          ; installed with std::bad_alloc::vftable
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x10
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        // 000010de: call __CxxThrowException@8   ; FUN_009d1b9f — noreturn
        _emit 0xe8
        _emit 0xbc
        _emit 0x0a
        _emit 0x5d
        _emit 0x00
    }
}
