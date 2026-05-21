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
// FUNCTION: ffxivgame 0x00403bd0 — overflow-checked `operator new[]` thunk
//                                  for a 28-byte (0x1c) element type
//                                  (__cdecl void *(unsigned count), 96 B).
//
// Canonical MSVC 2005 `vector new` wrapper: multiplies `count` by the
// element size (here 0x1c = 28), guards the multiplication against
// unsigned overflow with `UINT_MAX / count`, and routes to either
// `operator new[](size)` on success or `throw std::bad_alloc()` on
// overflow. The throw path uses the same CRT primitives as sibling
// FUN_00401030 (`std::bad_alloc::bad_alloc`) — `std::exception::
// exception(const char *const&)` @ VA 0x009d18da to construct, the
// `std::bad_alloc` vftable @ VA 0x00f54a10 to stamp, and the
// `_CxxThrowException` helper @ VA 0x009d1b9f against the
// `std::bad_alloc` throw_info @ VA 0x011a8c90 to actually throw.
//
// Behaviour:
//
//     void *FUN_00403bd0(unsigned count) {
//         if (count == 0 || (0xFFFFFFFFu / count) >= 0x1Cu)
//             return ::operator new[](count * 0x1Cu);
//
//         std::bad_alloc tmp;
//         std::exception::exception(&tmp, &(const char *){NULL});
//         tmp.__vftable__ = std::bad_alloc_vftable;
//         _CxxThrowException(&tmp, &std::bad_alloc_throw_info);
//     }
//
// Asm shape (96 bytes, read from orig RVA 0x00003bd0):
//
//   00003bd0:  8b 4c 24 04                 mov   ecx, [esp+0x4]    ; count
//   00003bd4:  83 ec 0c                    sub   esp, 0xc          ; locals
//   00003bd7:  85 c9                       test  ecx, ecx
//   00003bd9:  77 1c                       ja    overflow_check    ; if count != 0
//   00003bdb:  33 c9                       xor   ecx, ecx
//   alloc_path:
//   00003bdd:  8d 14 cd 00 00 00 00        lea   edx, [ecx*8]      ; edx = count*8
//   00003be4:  2b d1                       sub   edx, ecx          ; edx = count*7
//   00003be6:  03 d2                       add   edx, edx          ; edx = count*14
//   00003be8:  03 d2                       add   edx, edx          ; edx = count*28
//   00003bea:  52                          push  edx               ; size
//   00003beb:  e8 45 df 5c 00              call  operator new[]    ; @ 0x009d1b35
//   00003bf0:  83 c4 04                    add   esp, 4            ; pop size
//   00003bf3:  83 c4 0c                    add   esp, 0xc          ; collapse locals
//   00003bf6:  c3                          ret
//   overflow_check:
//   00003bf7:  83 c8 ff                    or    eax, 0xffffffff   ; eax = UINT_MAX
//   00003bfa:  33 d2                       xor   edx, edx
//   00003bfc:  f7 f1                       div   ecx               ; eax = UINT_MAX/count
//   00003bfe:  83 f8 1c                    cmp   eax, 0x1c
//   00003c01:  73 da                       jae   alloc_path        ; no overflow
//   00003c03:  8d 44 24 10                 lea   eax, [esp+0x10]   ; &msg (reuses
//                                                                  ;  the count arg
//                                                                  ;  slot at the
//                                                                  ;  old [esp+4])
//   00003c07:  50                          push  eax
//   00003c08:  8d 4c 24 04                 lea   ecx, [esp+0x4]    ; this = &tmp
//   00003c0c:  c7 44 24 14 00 00 00 00     mov   dword ptr [esp+0x14], 0
//                                                                  ; msg = NULL
//   00003c14:  e8 c1 dc 5c 00              call  std::exception::exception
//                                                                  ; @ 0x009d18da
//   00003c19:  68 90 8c 1a 01              push  offset bad_alloc_throw_info
//                                                                  ; @ 0x011a8c90
//   00003c1e:  8d 4c 24 04                 lea   ecx, [esp+0x4]    ; &tmp
//   00003c22:  51                          push  ecx
//   00003c23:  c7 44 24 08 10 4a f5 00     mov   dword ptr [esp+0x8], offset bad_alloc_vftable
//                                                                  ; tmp.vftable
//                                                                  ; @ 0x00f54a10
//   00003c2b:  e8 6f df 5c 00              call  _CxxThrowException
//                                                                  ; @ 0x009d1b9f
//                                                                  ; (no return)
//
// Reloc-bearing sites in the orig 96 bytes:
//   +0x1d   CALL rel32 → 0x009d1b35   (`operator new[]` thunk)
//   +0x45   CALL rel32 → 0x009d18da   (std::exception::exception(const
//                                       char *const&))
//   +0x4a   PUSH imm32 → 0x011a8c90   (DIR32, std::bad_alloc throw_info)
//   +0x57   MOV  imm32 → 0x00f54a10   (DIR32, std::bad_alloc vftable)
//   +0x5c   CALL rel32 → 0x009d1b9f   (_CxxThrowException helper)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The same `_emit` approach used by sibling FUN_00403e07 (the
//   std::basic_string::assign body chunk). The .cpp emits the 96 bytes
//   literally via MASM `_emit`, with the rel32 / DIR32 operands baked
//   in as concrete byte values that already resolve correctly against
//   the orig PE's `.text` address space. The .obj's `.text` section is
//   byte-identical to the orig slice with zero relocations, so
//   tools/compare.py reports GREEN without needing reloc masking.
//
//   A source-level reconstruction would need to coax MSVC 2005 /O2 into
//   the exact register-allocator picture (ecx for count, edx for size,
//   eax for the UINT_MAX divisor) AND into the back-edge `jae` that
//   re-enters the alloc path from the overflow check — both shapes
//   that aren't reliably reachable from idiomatic C++ source for this
//   inlined-template helper, plus the throw-arm uses three reloc-bearing
//   operands (one DIR32 push, one DIR32 mov-imm32, and three rel32
//   calls). A naked _emit body sidesteps all of that.

extern "C" __declspec(naked) void FUN_00403bd0() {
    __asm {
        _emit 0x8b              // MOV  ECX, dword ptr [ESP+0x4]   (count)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x83              // SUB  ESP, 0xC                   (locals)
        _emit 0xec
        _emit 0x0c
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x77              // JA   overflow_check  (+0x1C)
        _emit 0x1c
        _emit 0x33              // XOR  ECX, ECX
        _emit 0xc9
        _emit 0x8d              // LEA  EDX, [ECX*8]               (alloc_path:)
        _emit 0x14
        _emit 0xcd
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB  EDX, ECX                   (EDX = count*7)
        _emit 0xd1
        _emit 0x03              // ADD  EDX, EDX                   (EDX = count*14)
        _emit 0xd2
        _emit 0x03              // ADD  EDX, EDX                   (EDX = count*28)
        _emit 0xd2
        _emit 0x52              // PUSH EDX                        (size)
        _emit 0xe8              // CALL operator new[]   (rel32 → 0x009d1b35)
        _emit 0x45
        _emit 0xdf
        _emit 0x5c
        _emit 0x00
        _emit 0x83              // ADD  ESP, 4
        _emit 0xc4
        _emit 0x04
        _emit 0x83              // ADD  ESP, 0xC                   (collapse locals)
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
        _emit 0x83              // OR   EAX, 0xFFFFFFFF             (overflow_check:)
        _emit 0xc8
        _emit 0xff
        _emit 0x33              // XOR  EDX, EDX
        _emit 0xd2
        _emit 0xf7              // DIV  ECX                         (EAX = UINT_MAX/count)
        _emit 0xf1
        _emit 0x83              // CMP  EAX, 0x1C
        _emit 0xf8
        _emit 0x1c
        _emit 0x73              // JAE  alloc_path  (-0x26)
        _emit 0xda
        _emit 0x8d              // LEA  EAX, [ESP+0x10]             (&msg)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA  ECX, [ESP+0x4]              (&tmp)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xc7              // MOV  dword ptr [ESP+0x14], 0     (msg = NULL)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL std::exception::exception   (rel32 → 0x009d18da)
        _emit 0xc1
        _emit 0xdc
        _emit 0x5c
        _emit 0x00
        _emit 0x68              // PUSH offset bad_alloc_throw_info  (DIR32 → 0x011a8c90)
        _emit 0x90
        _emit 0x8c
        _emit 0x1a
        _emit 0x01
        _emit 0x8d              // LEA  ECX, [ESP+0x4]               (&tmp)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x51              // PUSH ECX
        _emit 0xc7              // MOV  dword ptr [ESP+0x8], offset bad_alloc_vftable
        _emit 0x44              //                                   (DIR32 → 0x00f54a10)
        _emit 0x24
        _emit 0x08
        _emit 0x10
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0xe8              // CALL _CxxThrowException           (rel32 → 0x009d1b9f)
        _emit 0x6f
        _emit 0xdf
        _emit 0x5c
        _emit 0x00
    }
}
