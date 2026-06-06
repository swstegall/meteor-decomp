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
// FUNCTION: ffxivgame 0x00460b70 — struct-dispatch / vtable resolver (54 bytes)
//
// Non-standard ABI: EDX = output pointer (in-register, not loaded from stack);
//                   [ESP+4] = struct pointer (single stack arg, caller-cleaned).
//
// Layout of the struct at [ESP+4] (inferred):
//   +0x00  byte     tag          (compared to 0x5)
//   +0x04  int      kind         (compared to 0x1)
//   +0x10  void*    ptr          (pointer to a "method table" or inner struct)
//   +0x14  int      value        (returned via *EDX on the inline path)
//
// Layout of the inner struct at *(struct+0x10):
//   +0x10  void*    fn           (function pointer; called as fn(EDX, EAX))
//
// Logic (inferred):
//   1. If arg == NULL                        → *out = 0
//   2. inner = arg->ptr
//   3. If inner == NULL:
//        if arg->tag == 5                    → *out = 0
//        if arg->kind != 1                   → *out = 0
//        *out = arg->value                   (inline value path)
//   4. fn = inner->fn
//   5. If fn == NULL                         → *out = 0
//   6. fn(out, arg)   [pushes EAX then EDX → callee sees EDX as arg1, EAX as arg2]
//
// Calling convention: non-standard / naked.  No callee-saved registers, no
// stack frame.  All three exit paths use bare RET (C3); no `ret n` → the
// caller is responsible for the [ESP+4] stack slot.
//
// No external call relocations — the only CALL is an indirect `CALL ECX`
// (register) whose target address is resolved at runtime.  tools/compare.py
// therefore needs no reloc masking here.

extern "C" __declspec(naked) void FUN_00460b70() {
    __asm {
        mov  eax, dword ptr [esp + 4]
        test eax, eax
        jz   b70_null_ret
        mov  ecx, dword ptr [eax + 0x10]
        test ecx, ecx
        jz   b70_no_fn_ptr
        mov  ecx, dword ptr [ecx + 0x10]
        test ecx, ecx
        jz   b70_null_ret
        push eax
        push edx
        call ecx
        add  esp, 8
        ret
    b70_no_fn_ptr:
        cmp  byte ptr [eax], 5
        jz   b70_null_ret
        cmp  dword ptr [eax + 4], 1
        jnz  b70_null_ret
        mov  eax, dword ptr [eax + 0x14]
        mov  dword ptr [edx], eax
        ret
    b70_null_ret:
        mov  dword ptr [edx], 0
        ret
    }
}
