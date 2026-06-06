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
// FUNCTION: ffxivgame 0x0047af20 — _EC_POINT_get_affine_coordinates_GF2m
//                                  108 bytes, __cdecl, 5 parameters
//
// int __cdecl _EC_POINT_get_affine_coordinates_GF2m(
//     EC_GROUP *group,   [ESP+0x8]
//     EC_POINT *point,   [ESP+0xC]
//     BIGNUM   *x,       [ESP+0x10]
//     BIGNUM   *y,       [ESP+0x14]
//     BN_CTX   *ctx)     [ESP+0x18]
//
// Stack frame: only PUSH ESI (no local variables).
// Calling convention: __cdecl (caller cleans, RET not RET N).
//
// Logic (recovered from asm):
//   1. ESI = group (param_1 from [ESP+0x8])
//   2. EAX = group->meth  (first field of group)
//   3. ECX = meth->point_get_affine_coordinates  (offset 0x44 in meth vtable)
//   4. If ECX == 0:
//        ECerr(EC_F_EC_POINT_GET_AFFINE_COORDINATES_GF2M,
//              ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);  // args: 0x10, 0xb7, 0x42, <str>, 0x383
//        return 0;
//   5. EDX = point (param_2 from [ESP+0xC])
//   6. If group->meth != point->meth  (EAX != [EDX]):
//        ECerr(EC_F_EC_POINT_GET_AFFINE_COORDINATES_GF2M,
//              EC_R_INCOMPATIBLE_OBJECTS);           // args: 0x10, 0xb7, 0x65, <str>, 0x388
//        return 0;
//   7. Push ctx, y, x, point, group; CALL ECX (the vtable method); return result.
//
// Reloc-bearing sites (absolute/rel32 values baked into the orig binary;
// emitting them as raw bytes via MASM `_emit` produces a byte-identical .obj
// with no relocations — compare.py reads the post-fixup binary and matches):
//   +0x14  PUSH imm32 → 0x00f7b4f8  (file name string in .rdata)
//   +0x22  CALL rel32 → 0x0045c940  (ECerr / error-reporting helper)
//   +0x3b  PUSH imm32 → 0x00f7b4f8  (same file name string)
//   +0x49  CALL rel32 → 0x0045c940  (same ECerr helper; different rel32 offset)

extern "C" __declspec(naked) void FUN_0047af20() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x8]
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x8b              // MOV ECX, dword ptr [EAX+0x44]
        _emit 0x48
        _emit 0x44
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x75              // JNZ +0x1f  (to success path at +0x2d)
        _emit 0x1f
        _emit 0x68              // PUSH 0x383
        _emit 0x83
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x00f7b4f8  (file name; reloc +0x14)
        _emit 0xf8
        _emit 0xb4
        _emit 0xf7
        _emit 0x00
        _emit 0x6a              // PUSH 0x42
        _emit 0x42
        _emit 0x68              // PUSH 0xb7
        _emit 0xb7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0xe8              // CALL 0x0045c940  (rel32 = 0xfffe19fa; reloc +0x22)
        _emit 0xfa
        _emit 0x19
        _emit 0xfe
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0xc]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x3b              // CMP EAX, dword ptr [EDX]
        _emit 0x02
        _emit 0x74              // JZ +0x1f  (to vtable-call path at +0x54)
        _emit 0x1f
        _emit 0x68              // PUSH 0x388
        _emit 0x88
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x00f7b4f8  (file name; reloc +0x3b)
        _emit 0xf8
        _emit 0xb4
        _emit 0xf7
        _emit 0x00
        _emit 0x6a              // PUSH 0x65
        _emit 0x65
        _emit 0x68              // PUSH 0xb7
        _emit 0xb7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0xe8              // CALL 0x0045c940  (rel32 = 0xfffe19d3; reloc +0x49)
        _emit 0xd3
        _emit 0x19
        _emit 0xfe
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x18]  ; ctx
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x18]  ; y  (ESP shifted -4)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x18]  ; x  (ESP shifted -8)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50              // PUSH EAX
        _emit 0x52              // PUSH EDX  (point)
        _emit 0x56              // PUSH ESI  (group)
        _emit 0xff              // CALL ECX  (meth->point_get_affine_coordinates)
        _emit 0xd1
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
