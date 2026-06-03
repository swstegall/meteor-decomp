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
// FUNCTION: ffxivgame 0x00694b1d — virtual-dispatch search loop, hot path
//                                  (81 B / 0x51, no frame, callee-saved
//                                   ebx/esi/edi + inherited-frame ebp).
//
// Context: this fragment is the non-NULL hot path of a two-part function.
// The guard (at 0xa94b10–0xa94b1c, a separate Ghidra node) does:
//
//   push ebp                    ; save caller's ebp
//   mov  ebp, [esp+8]           ; ebp = first arg (container/search key)
//   test ebp, ebp
//   jne  0xa94b1d               ; non-NULL → fall into this fragment
//   xor  eax, eax               ; NULL arg → return 0
//   pop  ebp
//   ret
//
// When 0xa94b1d is reached, ebp already holds the first argument and the
// guard's `push ebp` is live on the stack.  The combined function's
// epilogue (pop edi / pop esi / pop ebx / pop ebp / ret) unwinds all four
// callee-saves including the ebp pushed by the guard.
//
// Behaviour:
//   1. Save ebx, esi, edi.
//   2. Call FUN_00a886e0 → returns a pointer; dispatch vtable[0xC/4]
//      to get an inner container object (→ esi).
//   3. Call esi->vtable[0x40/4]() to get the element count (→ ebx).
//   4. Iterate i = 0 .. count-1:
//        item = esi->vtable[0x38/4](i)     [thiscall, 1 arg]
//        if (item) {
//            result = item->vtable[0xF4/4](ebp)  [thiscall, 1 arg]
//            if (result != 0) return result   ← early exit
//        }
//   5. Return 0.
//
// The `push ebp` at +0x3c passes the inherited argument (the container/
// search-key pointer loaded by the guard) to the innermost virtual call.
// The matching `pop ebp` at +0x4f is the epilogue cleanup of the guard's
// frame push — NOT a local push within this fragment.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   MSVC 2005 never emits pop ebp without a matching push ebp in the same
//   function prologue.  This fragment straddles a Ghidra-split boundary:
//   the push ebp lives in the guard node (0xa94b10) while the pop ebp
//   lives in this node's epilogue.  No source-level C++ can reproduce
//   this shape for just the 81 bytes beginning at 0xa94b1d.  The naked
//   passthrough is the only viable match strategy for this fragment.

extern "C" __declspec(naked) void FUN_00a94b1d() {
    __asm {
        // 0xa94b1d: push ebx
        _emit 0x53
        // 0xa94b1e: push esi
        _emit 0x56
        // 0xa94b1f: push edi
        _emit 0x57
        // 0xa94b20: call FUN_00a886e0  (e8 rel32)
        _emit 0xe8
        _emit 0xbb
        _emit 0x3b
        _emit 0xff
        _emit 0xff
        // 0xa94b25: mov edx, [eax]
        _emit 0x8b
        _emit 0x10
        // 0xa94b27: mov ecx, eax
        _emit 0x8b
        _emit 0xc8
        // 0xa94b29: mov eax, [edx+0xc]
        _emit 0x8b
        _emit 0x42
        _emit 0x0c
        // 0xa94b2c: call eax
        _emit 0xff
        _emit 0xd0
        // 0xa94b2e: mov esi, eax
        _emit 0x8b
        _emit 0xf0
        // 0xa94b30: mov edx, [esi]
        _emit 0x8b
        _emit 0x16
        // 0xa94b32: mov eax, [edx+0x40]
        _emit 0x8b
        _emit 0x42
        _emit 0x40
        // 0xa94b35: mov ecx, esi
        _emit 0x8b
        _emit 0xce
        // 0xa94b37: call eax
        _emit 0xff
        _emit 0xd0
        // 0xa94b39: mov ebx, eax
        _emit 0x8b
        _emit 0xd8
        // 0xa94b3b: xor edi, edi
        _emit 0x33
        _emit 0xff
        // 0xa94b3d: test ebx, ebx
        _emit 0x85
        _emit 0xdb
        // 0xa94b3f: jbe +0x26  (→ 0xa94b67 xor eax,eax)
        _emit 0x76
        _emit 0x26
        // ---- loop body ----
        // 0xa94b41: mov edx, [esi]
        _emit 0x8b
        _emit 0x16
        // 0xa94b43: mov eax, [edx+0x38]
        _emit 0x8b
        _emit 0x42
        _emit 0x38
        // 0xa94b46: push edi
        _emit 0x57
        // 0xa94b47: mov ecx, esi
        _emit 0x8b
        _emit 0xce
        // 0xa94b49: call eax  (esi->vtable[14](edi))
        _emit 0xff
        _emit 0xd0
        // 0xa94b4b: test eax, eax
        _emit 0x85
        _emit 0xc0
        // 0xa94b4d: je +0x11  (→ 0xa94b60 loop_next)
        _emit 0x74
        _emit 0x11
        // ---- item != NULL arm ----
        // 0xa94b4f: mov edx, [eax]
        _emit 0x8b
        _emit 0x10
        // 0xa94b51: mov ecx, eax
        _emit 0x8b
        _emit 0xc8
        // 0xa94b53: mov eax, [edx+0xf4]
        _emit 0x8b
        _emit 0x82
        _emit 0xf4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0xa94b59: push ebp  (pass inherited arg to vtable[61])
        _emit 0x55
        // 0xa94b5a: call eax  (item->vtable[61](ebp))
        _emit 0xff
        _emit 0xd0
        // 0xa94b5c: test eax, eax
        _emit 0x85
        _emit 0xc0
        // 0xa94b5e: jne +9  (→ 0xa94b69 epilogue, return non-zero)
        _emit 0x75
        _emit 0x09
        // ---- loop_next ----
        // 0xa94b60: add edi, 1
        _emit 0x83
        _emit 0xc7
        _emit 0x01
        // 0xa94b63: cmp edi, ebx
        _emit 0x3b
        _emit 0xfb
        // 0xa94b65: jb -0x26  (→ 0xa94b41 loop top)
        _emit 0x72
        _emit 0xda
        // 0xa94b67: xor eax, eax  (return 0)
        _emit 0x33
        _emit 0xc0
        // ---- epilogue ----
        // 0xa94b69: pop edi
        _emit 0x5f
        // 0xa94b6a: pop esi
        _emit 0x5e
        // 0xa94b6b: pop ebx
        _emit 0x5b
        // 0xa94b6c: pop ebp   (restores the push from guard at 0xa94b10)
        _emit 0x5d
        // 0xa94b6d: ret
        _emit 0xc3
    }
}
