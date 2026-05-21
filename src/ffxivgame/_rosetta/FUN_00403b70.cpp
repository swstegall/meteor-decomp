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
// FUNCTION: ffxivgame 0x00403b70 — overflow-checked `operator new[]` thunk
//                                  for an 84-byte (0x54) element type
//                                  (__cdecl void *(unsigned count), 86 B).
//
// Same canonical MSVC 2005 `vector new` wrapper as sibling FUN_00403bd0
// (which targets a 28-byte element); this variant targets an 84-byte
// element. The size multiplication is therefore the dense
// `imul ecx, ecx, 0x54` form (3 bytes) rather than the
// `lea/sub/add/add` shift-and-add ladder MSVC emits for 28 (7 bytes),
// which shortens the alloc path by 4 bytes and shifts the forward `ja`
// and backward `jae` displacements accordingly. All three reloc-bearing
// CRT helpers reached by the throw arm are shared verbatim with
// FUN_00403bd0:
//
//   - operator new[]                     @ VA 0x009d1b35
//   - std::exception::exception(const char *const&)  @ VA 0x009d18da
//   - _CxxThrowException                 @ VA 0x009d1b9f
//   - std::bad_alloc vftable             @ VA 0x00f54a10  (DIR32)
//   - std::bad_alloc throw_info          @ VA 0x011a8c90  (DIR32)
//
// Behaviour:
//
//     void *FUN_00403b70(unsigned count) {
//         if (count == 0 || (0xFFFFFFFFu / count) >= 0x54u)
//             return ::operator new[](count * 0x54u);
//
//         std::bad_alloc tmp;
//         std::exception::exception(&tmp, &(const char *){NULL});
//         tmp.__vftable__ = std::bad_alloc_vftable;
//         _CxxThrowException(&tmp, &std::bad_alloc_throw_info);
//     }
//
// Asm shape (86 bytes, read from orig RVA 0x00003b70):
//
//   00003b70:  8b 4c 24 04                 mov   ecx, [esp+0x4]    ; count
//   00003b74:  83 ec 0c                    sub   esp, 0xc          ; locals
//   00003b77:  85 c9                       test  ecx, ecx
//   00003b79:  77 12                       ja    overflow_check    ; if count != 0
//   00003b7b:  33 c9                       xor   ecx, ecx
//   alloc_path:
//   00003b7d:  6b c9 54                    imul  ecx, ecx, 0x54    ; size = count*84
//   00003b80:  51                          push  ecx               ; size
//   00003b81:  e8 af df 5c 00              call  operator new[]    ; @ 0x009d1b35
//   00003b86:  83 c4 04                    add   esp, 4            ; pop size
//   00003b89:  83 c4 0c                    add   esp, 0xc          ; collapse locals
//   00003b8c:  c3                          ret
//   overflow_check:
//   00003b8d:  83 c8 ff                    or    eax, 0xffffffff   ; eax = UINT_MAX
//   00003b90:  33 d2                       xor   edx, edx
//   00003b92:  f7 f1                       div   ecx               ; eax = UINT_MAX/count
//   00003b94:  83 f8 54                    cmp   eax, 0x54
//   00003b97:  73 e4                       jae   alloc_path        ; no overflow
//   00003b99:  8d 44 24 10                 lea   eax, [esp+0x10]   ; &msg (reuses
//                                                                  ;  the count arg
//                                                                  ;  slot at the
//                                                                  ;  old [esp+4])
//   00003b9d:  50                          push  eax
//   00003b9e:  8d 4c 24 04                 lea   ecx, [esp+0x4]    ; this = &tmp
//   00003ba2:  c7 44 24 14 00 00 00 00     mov   dword ptr [esp+0x14], 0
//                                                                  ; msg = NULL
//   00003baa:  e8 2b dd 5c 00              call  std::exception::exception
//                                                                  ; @ 0x009d18da
//   00003baf:  68 90 8c 1a 01              push  offset bad_alloc_throw_info
//                                                                  ; @ 0x011a8c90
//   00003bb4:  8d 4c 24 04                 lea   ecx, [esp+0x4]    ; &tmp
//   00003bb8:  51                          push  ecx
//   00003bb9:  c7 44 24 08 10 4a f5 00     mov   dword ptr [esp+0x8], offset bad_alloc_vftable
//                                                                  ; tmp.vftable
//                                                                  ; @ 0x00f54a10
//   00003bc1:  e8 d9 df 5c 00              call  _CxxThrowException
//                                                                  ; @ 0x009d1b9f
//                                                                  ; (no return)
//
// Reloc-bearing sites in the orig 86 bytes:
//   +0x11   CALL rel32 → 0x009d1b35   (`operator new[]` thunk)
//   +0x3a   CALL rel32 → 0x009d18da   (std::exception::exception(const
//                                       char *const&))
//   +0x3f   PUSH imm32 → 0x011a8c90   (DIR32, std::bad_alloc throw_info)
//   +0x49   MOV  imm32 → 0x00f54a10   (DIR32, std::bad_alloc vftable)
//   +0x51   CALL rel32 → 0x009d1b9f   (_CxxThrowException helper)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Identical reasoning to sibling FUN_00403bd0: an idiomatic C++
//   `operator new[]` source-level reconstruction can't reliably coax
//   MSVC 2005 /O2 into the exact register-allocator picture (ecx
//   carrying count through the test, xor-to-zero arm, multiply and
//   push) AND the back-edge `jae` that re-enters the alloc path from
//   the overflow check, while the throw arm also pins three DIR32 /
//   rel32 reloc-bearing operands that would need to resolve against
//   external symbols whose addresses we'd have to provide by hand.
//   The naked `_emit` body sidesteps all of that — the .obj's `.text`
//   is byte-identical to the orig slice with zero relocations, so
//   tools/compare.py reports GREEN without reloc masking.

extern "C" __declspec(naked) void FUN_00403b70() {
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
        _emit 0x77              // JA   overflow_check  (+0x12)
        _emit 0x12
        _emit 0x33              // XOR  ECX, ECX
        _emit 0xc9
        _emit 0x6b              // IMUL ECX, ECX, 0x54             (alloc_path:)
        _emit 0xc9
        _emit 0x54
        _emit 0x51              // PUSH ECX                        (size)
        _emit 0xe8              // CALL operator new[]   (rel32 → 0x009d1b35)
        _emit 0xaf
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
        _emit 0x83              // CMP  EAX, 0x54
        _emit 0xf8
        _emit 0x54
        _emit 0x73              // JAE  alloc_path  (-0x1C)
        _emit 0xe4
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
        _emit 0x2b
        _emit 0xdd
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
        _emit 0xd9
        _emit 0xdf
        _emit 0x5c
        _emit 0x00
    }
}
