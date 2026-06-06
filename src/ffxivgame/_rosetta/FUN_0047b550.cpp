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
// FUNCTION: ffxivgame 0x0047b550 — EC_POINT_have_precompute_mult dispatch (37 B)
//
// OpenSSL EC_POINT_have_precompute_mult: if the group method table has no
// custom mul (slot +0x74), tail-calls the default implementation
// FUN_004998e0; otherwise, if have_precompute_mult (slot +0x7c) is non-null,
// tail-calls it; else returns 0.
//
// EC_METHOD layout (relevant offsets; GFp-only build):
//   +0x64  is_on_curve           (confirmed by FUN_0047b310)
//   +0x68  point_cmp
//   +0x6c  make_affine
//   +0x70  points_make_affine
//   +0x74  mul                   <- checked here (null => use default)
//   +0x78  precompute_mult
//   +0x7c  have_precompute_mult  <- called if non-null
//
// EC_GROUP layout:
//   +0x00  EC_METHOD *meth
//
// Signature:
//   int __cdecl FUN_0047b550(EC_GROUP *group)
//
// Calling convention: __cdecl (plain RET, caller cleans).  One declared arg,
// but as a tail-call forwarder the callee (default or have_precompute_mult)
// sees every argument originally placed on the stack by the outer caller.
//
// Register allocation (MSVC 2005 /O2 /Oy, no frame pointer):
//   ECX = group (param_1) — loaded from [ESP+4], kept alive so it can be
//         spilled back to [ESP+4] before each tail JMP
//   EAX = group->meth initially, then overwritten with the function pointer
//         loaded from meth+0x7c for the second dispatch
//
// Control flow:
//   1. ECX = [ESP+4]  (group)
//   2. EAX = [ECX]    (group->meth)
//   3. CMP [EAX+0x74], 0  — test mul pointer
//   4. JNZ  →  load have_precompute_mult path
//   5. (mul==0) MOV [ESP+4],ECX; JMP FUN_004998e0   (tail-call default)
//   6. EAX = [EAX+0x7c]  (have_precompute_mult)
//   7. TEST EAX,EAX
//   8. JZ  →  XOR EAX,EAX; RET  (return 0)
//   9. MOV [ESP+4],ECX; JMP EAX   (tail-call have_precompute_mult)

struct EC_METHOD_b550 {
    char    _pad[0x74];
    void   *mul;                   // offset 0x74
    void   *precompute_mult;       // offset 0x78
    void   *have_precompute_mult;  // offset 0x7c
};

struct EC_GROUP_b550 {
    EC_METHOD_b550 *meth;          // offset 0x00
};

typedef int (*EC_fn_b550)(EC_GROUP_b550 *);

extern "C" int __cdecl FUN_004998e0(EC_GROUP_b550 *group);

extern "C" int __cdecl FUN_0047b550(EC_GROUP_b550 *param_1)
{
    EC_METHOD_b550 *meth = param_1->meth;
    if (meth->mul == 0) {
        return FUN_004998e0(param_1);
    }
    EC_fn_b550 fn = (EC_fn_b550)meth->have_precompute_mult;
    if (fn != 0) {
        return fn(param_1);
    }
    return 0;
}
