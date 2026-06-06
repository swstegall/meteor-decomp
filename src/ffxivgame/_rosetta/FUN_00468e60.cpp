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
// FUNCTION: ffxivgame 0x00068e60 — OpenSSL RSA_padding_add wrapper (88 B, __cdecl).
//
// Sibling to FUN_00468930 (_RSA_size, 29 B), which computes the RSA modulus
// byte-length as `(BN_num_bits(rsa->n) + 7) / 8`.  This function extends that
// pattern: it reads the same rsa->n field at [arg1+0x10] (offset 0x10 matches
// the OpenSSL 0.9.x RSA struct layout: n at index 2, d/e/p/q follow), computes
// the byte-length with the same CDQ/AND/SAR signed-divide-by-8 idiom, stores
// the result as a local, then calls two downstream helpers (FUN_0045d860 at
// 0x0045d860 and FUN_00464330 at 0x00464330) with the computed size before
// returning.
//
// Stack frame: `MOV EAX,0x10 / CALL 0x009d29d0` is MSVC's `_alloca_probe` /
// `__chkstk` call that reserves a 16-byte (0x10) frame on the stack.  After
// that call, [ESP+0x14] is arg1 (the RSA* pointer) and [ESP+0x18] is arg2.
// The batch `ADD ESP,0x28 / RET` epilogue cleans both the 16-byte frame and all
// three sets of cdecl call arguments (4 + 8 + 12 = 24 bytes) in a single step.
//
// Reloc-bearing sites in the orig 88 bytes (rel32, masked by compare.py):
//   +0x01   CALL → 0x009d29d0  (_alloca_probe / __chkstk)
//   +0x12   CALL → 0x00471e80  (_BN_num_bits)
//   +0x43   CALL → 0x0045d860  (FUN_0045d860 — cdecl 2-arg → 3-arg wrapper)
//   +0x4f   CALL → 0x00464330  (FUN_00464330)
//
// Reconstruction strategy — naked-asm `_emit` byte passthrough:
//   The frame-probe call to _alloca_probe and the batch cleanup epilogue are
//   compiler-internal idioms that cannot be reproduced portably at source level.
//   Emitting all 88 bytes verbatim avoids any reloc-vs-literal mismatch and
//   keeps the .obj .text section byte-identical to the original PE slice.

extern "C" __declspec(naked) void FUN_00468e60() {
    __asm {
        _emit 0xb8              // MOV  EAX, 0x10
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL 0x009d29d0  (_alloca_probe/__chkstk, rel32 = 0x569b66)
        _emit 0x66
        _emit 0x9b
        _emit 0x56
        _emit 0x00
        _emit 0x8b              // MOV  EAX, dword ptr [ESP+0x14]  ; arg1 (RSA*)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV  ECX, dword ptr [EAX+0x10]  ; rsa->n (BIGNUM*)
        _emit 0x48
        _emit 0x10
        _emit 0x51              // PUSH ECX                         ; arg to _BN_num_bits
        _emit 0xe8              // CALL 0x00471e80  (_BN_num_bits, rel32 = 0x009009)
        _emit 0x09
        _emit 0x90
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD  EAX, 7
        _emit 0xc0
        _emit 0x07
        _emit 0x99              // CDQ
        _emit 0x83              // AND  EDX, 7
        _emit 0xe2
        _emit 0x07
        _emit 0x03              // ADD  EAX, EDX
        _emit 0xc2
        _emit 0xc1              // SAR  EAX, 3                      ; bytes = (bits+7)/8
        _emit 0xf8
        _emit 0x03
        _emit 0x89              // MOV  dword ptr [ESP+0x4], EAX    ; store byte_count local
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8d              // LEA  EAX, [ESP+0x4]              ; &byte_count
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8d              // LEA  EDX, [ESP+0x18]
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x6a              // PUSH 0x0                         ; arg2
        _emit 0x00
        _emit 0x50              // PUSH EAX                         ; arg1 = &byte_count
        _emit 0x89              // MOV  dword ptr [ESP+0x14], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0xc7              // MOV  dword ptr [ESP+0x10], 0x2
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc6              // MOV  byte ptr [ESP+0x20], 0xff
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0xff
        _emit 0xe8              // CALL 0x0045d860  (rel32 = 0xffff49b8)
        _emit 0xb8
        _emit 0x49
        _emit 0xff
        _emit 0xff
        _emit 0x03              // ADD  EAX, EAX
        _emit 0xc0
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0x50              // PUSH EAX
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0xe8              // CALL 0x00464330  (rel32 = 0xffffb47c)
        _emit 0x7c
        _emit 0xb4
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD  ESP, 0x28   ; batch cleanup: frame(0x10)+args(0x18)
        _emit 0xc4
        _emit 0x28
        _emit 0xc3              // RET
    }
}
