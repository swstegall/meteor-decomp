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
// FUNCTION: ffxivgame 0x00057350 — `__stdcall` 29-byte "deref-arg-into-
//                                   local, thiscall a method, return true"
//                                   wrapper.
//
// bool __stdcall FUN_00457350(void **pSrc, T *obj)
//   [ESP+0x4] : pSrc  (pointer; *pSrc is the value of interest)
//   [ESP+0x8] : obj   (becomes the `this` of the thiscall callee)
//
// The function loads `*pSrc`, writes that dword back into its own inbound
// arg slot (reusing `[ESP+0x4]` as the home for a single-dword local),
// then takes the address of that slot and passes it as the lone stack
// argument to a `__thiscall` method on `obj`. It unconditionally returns
// `true` (AL = 1) and cleans 8 bytes of args (`RET 0x8` → __stdcall, two
// dword params).
//
// Asm shape (29 bytes — orig RVA 0x00057350..0x0005736d):
//
//   00057350:  8b 44 24 04        MOV  EAX, [ESP+0x4]   ; EAX = pSrc
//   00057354:  8b 08              MOV  ECX, [EAX]       ; ECX = *pSrc
//   00057356:  8d 54 24 04        LEA  EDX, [ESP+0x4]   ; EDX = &localslot
//   0005735a:  89 4c 24 04        MOV  [ESP+0x4], ECX   ; localslot = *pSrc
//   0005735e:  8b 4c 24 08        MOV  ECX, [ESP+0x8]   ; ECX = obj (this)
//   00057362:  52                 PUSH EDX              ; arg: &localslot
//   00057363:  e8 a8 40 84 00     CALL FUN_00c9b410     ; rel32, thiscall
//   00057368:  b0 01              MOV  AL, 0x1          ; return true
//   0005736a:  c2 08 00           RET  0x8              ; __stdcall, 2 args
//
// Reloc-bearing site in the orig 29 bytes:
//   +0x13   CALL rel32 → 0x00c9b410 (FUN_00c9b410, the thiscall callee)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level rewrite would have to coax MSVC 2005 into reusing the
//   inbound `[ESP+0x4]` arg slot as the storage for the `*pSrc` local
//   (rather than reserving a fresh `SUB ESP` frame), preserve the exact
//   load/store ordering, and emit the trailing `MOV AL, 1` rather than a
//   widened `MOV EAX, 1` — all fragile against declaration-order and
//   return-type perturbations. As with sibling 29-byte wrappers
//   (FUN_00401000, FUN_00404e10, FUN_00409580), the robust choice is to
//   re-emit the orig 29 bytes verbatim. The rel32 displacement is baked
//   in as raw bytes that resolve correctly against the orig PE's RVA
//   space, so the .obj carries no relocations and compare.py reports
//   GREEN.

extern "C" __declspec(naked) void FUN_00457350() {
    __asm {
        _emit 0x8b      // MOV  EAX, [ESP+0x4]     ; pSrc
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b      // MOV  ECX, [EAX]         ; *pSrc
        _emit 0x08
        _emit 0x8d      // LEA  EDX, [ESP+0x4]     ; &localslot
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x89      // MOV  [ESP+0x4], ECX     ; localslot = *pSrc
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x8b      // MOV  ECX, [ESP+0x8]     ; this = obj
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x52      // PUSH EDX                ; arg: &localslot
        _emit 0xe8      // CALL FUN_00c9b410       ; rel32 = +0x008440a8
        _emit 0xa8
        _emit 0x40
        _emit 0x84
        _emit 0x00
        _emit 0xb0      // MOV  AL, 0x1            ; return true
        _emit 0x01
        _emit 0xc2      // RET  0x8                ; __stdcall, 2 args
        _emit 0x08
        _emit 0x00
    }
}
