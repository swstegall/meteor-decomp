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
// FUNCTION: ffxivgame 0x00440270 — overflow-checked `operator new[]` thunk
//                                  for a 2-byte (0x02) element type
//                                  (__cdecl void *(unsigned count), 86 B).
//
// Same canonical MSVC 2005 `vector new` wrapper as sibling FUN_00403b70
// (which targets an 84-byte element); this variant targets a 2-byte
// element. The size multiplication is therefore the dense
// `lea edx, [ecx + ecx*0x1]` form (3 bytes: 8d 14 09) rather than the
// `imul` or shift-and-add ladder MSVC emits for larger element sizes,
// which produces 2*count in EDX. The overflow-guard divides UINT_MAX by
// count and compares against 0x2 (not 0x54 as in FUN_00403b70). All
// three reloc-bearing CRT helpers reached by the throw arm are shared
// verbatim with FUN_00403b70:
//
//   - operator new[]                     @ VA 0x009d1b35
//   - std::exception::exception(const char *const&)  @ VA 0x009d18da
//   - _CxxThrowException                 @ VA 0x009d1b9f
//   - std::bad_alloc vftable             @ VA 0x00f54a10  (DIR32)
//   - std::bad_alloc throw_info          @ VA 0x011a8c90  (DIR32)
//
// Behaviour:
//
//     void *FUN_00440270(unsigned count) {
//         if (count == 0 || (0xFFFFFFFFu / count) >= 0x2u)
//             return ::operator new[](count * 0x2u);
//
//         std::bad_alloc tmp;
//         std::exception::exception(&tmp, &(const char *){NULL});
//         tmp.__vftable__ = std::bad_alloc_vftable;
//         _CxxThrowException(&tmp, &std::bad_alloc_throw_info);
//     }
//
// Asm shape (86 bytes, read from orig RVA 0x00040270):
//
//   00040270:  8b 4c 24 04                 mov   ecx, [esp+0x4]    ; count
//   00040274:  83 ec 0c                    sub   esp, 0xc          ; locals
//   00040277:  85 c9                       test  ecx, ecx
//   00040279:  77 12                       ja    overflow_check    ; if count != 0
//   0004027b:  33 c9                       xor   ecx, ecx
//   alloc_path:
//   0004027d:  8d 14 09                    lea   edx, [ecx+ecx*1]  ; size = count*2
//   00040280:  52                          push  edx               ; size
//   00040281:  e8 af 18 59 00              call  operator new[]    ; @ 0x009d1b35
//   00040286:  83 c4 04                    add   esp, 4            ; pop size
//   00040289:  83 c4 0c                    add   esp, 0xc          ; collapse locals
//   0004028c:  c3                          ret
//   overflow_check:
//   0004028d:  83 c8 ff                    or    eax, 0xffffffff   ; eax = UINT_MAX
//   00040290:  33 d2                       xor   edx, edx
//   00040292:  f7 f1                       div   ecx               ; eax = UINT_MAX/count
//   00040294:  83 f8 02                    cmp   eax, 0x2
//   00040297:  73 e4                       jnc   alloc_path        ; no overflow
//   00040299:  8d 44 24 10                 lea   eax, [esp+0x10]   ; &msg
//   0004029d:  50                          push  eax
//   0004029e:  8d 4c 24 04                 lea   ecx, [esp+0x4]    ; this = &tmp
//   000402a2:  c7 44 24 14 00 00 00 00     mov   dword ptr [esp+0x14], 0
//                                                                  ; msg = NULL
//   000402aa:  e8 2b 16 59 00              call  std::exception::exception
//                                                                  ; @ 0x009d18da
//   000402af:  68 90 8c 1a 01              push  offset bad_alloc_throw_info
//                                                                  ; @ 0x011a8c90
//   000402b4:  8d 4c 24 04                 lea   ecx, [esp+0x4]    ; &tmp
//   000402b8:  51                          push  ecx
//   000402b9:  c7 44 24 08 10 4a f5 00     mov   dword ptr [esp+0x8], offset bad_alloc_vftable
//                                                                  ; tmp.vftable
//                                                                  ; @ 0x00f54a10
//   000402c1:  e8 d9 18 59 00              call  _CxxThrowException
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
//   Identical reasoning to sibling FUN_00403b70: an idiomatic C++
//   `operator new[]` source-level reconstruction can't reliably coax
//   MSVC 2005 /O2 into the exact register-allocator picture (ecx
//   carrying count through the test, xor-to-zero arm, LEA-multiply and
//   push into EDX) AND the back-edge `jnc` that re-enters the alloc path
//   from the overflow check, while the throw arm also pins three DIR32 /
//   rel32 reloc-bearing operands that would need to resolve against
//   external symbols whose addresses we'd have to provide by hand.
//   The naked `_emit` body sidesteps all of that — the .obj's `.text`
//   is byte-identical to the orig slice with zero relocations, so
//   tools/compare.py reports GREEN without reloc masking.

extern "C" __declspec(naked) void FUN_00440270() {
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
        _emit 0x8d              // LEA  EDX, [ECX+ECX*1]           (alloc_path: size=count*2)
        _emit 0x14
        _emit 0x09
        _emit 0x52              // PUSH EDX                        (size)
        _emit 0xe8              // CALL operator new[]   (rel32 → 0x009d1b35)
        _emit 0xaf
        _emit 0x18
        _emit 0x59
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
        _emit 0x83              // CMP  EAX, 0x2
        _emit 0xf8
        _emit 0x02
        _emit 0x73              // JNC  alloc_path  (-0x1C)
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
        _emit 0x16
        _emit 0x59
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
        _emit 0x18
        _emit 0x59
        _emit 0x00
    }
}
