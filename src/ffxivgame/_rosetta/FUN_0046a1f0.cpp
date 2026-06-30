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
// FUNCTION: ffxivgame 0x0006a1f0 — dispatch helper: validates a listener struct,
//                                  checks obj-id and flags, then calls a
//                                  stored function pointer (236 B / 0xec).
//
// Calling convention: __cdecl (6 args on stack, caller cleans, bare RET).
//
// Parameters (all 4-byte, read after PUSH ESI adjusts ESP):
//   arg1  [orig ESP+4]  p         — pointer to outer listener struct
//   arg2  [orig ESP+8]  obj_id    — object-id filter (-1 = wildcard)
//   arg3  [orig ESP+0xc] flags_mask — flag-mask filter (-1 = wildcard)
//   arg4  [orig ESP+0x10] cb_a    — first arg forwarded to callback
//   arg5  [orig ESP+0x14] cb_b    — second arg forwarded to callback
//   arg6  [orig ESP+0x18] cb_c    — third arg forwarded to callback
//
// Struct layout recovered from field accesses:
//
//   struct Inner {
//       int      id;        // [+0x00] compared with obj_id
//       char     _pad[0x60];
//       int    (*fn)(...);  // [+0x64] stored callback function pointer
//   };
//
//   struct Outer {
//       Inner   *inner;     // [+0x00] pointer to Inner
//       char     _pad[0xc];
//       int      handler;   // [+0x10] registered-flags bitfield
//   };
//
// Flow:
//   1. Null-guard: if (!p || !p->inner || !p->inner->fn) → error(0x14e), return -2.
//   2. If obj_id != -1 AND p->inner->id != obj_id → return -1 (silent).
//   3. If !p->handler → error(0x156), return -1.
//   4. If flags_mask != -1 AND !(flags_mask & p->handler) → error(0x15c), return -1.
//   5. result = p->inner->fn(p, cb_a, cb_b, cb_c).
//   6. If result == -2 → error(0x163) (but still return result).
//   7. Return result.
//
// The error reporter at 0x0045c940 takes (level, arg2, arg3, strptr, lineno).
// All call-sites in this function use level=6, arg2=0x89; arg3 and lineno vary.
//
// Reconstruction strategy — pure C++ with register-allocation coercion:
//
//   The PUSH ESI scheduling (between TEST ECX and JZ) and the ESI=fn assignment
//   during the null-check chain are achieved by declaring the fn pointer local
//   (fn_ptr) before any other locals so MSVC 2005 /O2 allocates ESI to it.
//   The ECX assignment to p comes from MSVC's habit of using ECX as a scratch
//   register when the first pointer-sized local needs a register and ECX is free.
//   The TEST EDX,EAX encoding at the flags check (85 c2) arises naturally when
//   flags_mask is in EDX and handler is in EAX.

typedef int (*FnPtr)(void *, int, int, int);

struct Inner {
    int   id;
    char  _pad[0x60];
    FnPtr fn;
};

struct Outer {
    Inner *inner;
    char   _pad[0xc];
    int    handler;
};

extern "C" int FUN_0045c940(int, int, int, int, int);

extern "C" int __cdecl FUN_0046a1f0(Outer *p, int obj_id, int flags_mask,
                                     int cb_a, int cb_b, int cb_c)
{
    if (!p)
        goto null_error;
    {
        Inner *inner = p->inner;
        if (!inner)
            goto null_error;
        FnPtr fn = inner->fn;
        if (!fn)
            goto null_error;

        if (obj_id != -1 && inner->id != obj_id)
            return -1;

        int handler = p->handler;
        if (!handler) {
            FUN_0045c940(6, 0x89, 0x95, 0xf79270, 0x156);
            return -1;
        }

        if (flags_mask != -1 && !(flags_mask & handler)) {
            FUN_0045c940(6, 0x89, 0x94, 0xf79270, 0x15c);
            return -1;
        }

        int result = fn(p, cb_a, cb_b, cb_c);
        if (result == -2)
            FUN_0045c940(6, 0x89, 0x93, 0xf79270, 0x163);
        return result;
    }

null_error:
    FUN_0045c940(6, 0x89, 0x93, 0xf79270, 0x14e);
    return -2;
}
