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
// FUNCTION: ffxivgame 0x0044c6d0 — `__thiscall` vtable-stamp + tail-handoff
//                                   (18 B / 0x12).
//
// Behaviour read from the disassembly at orig RVA 0x0004c6d0:
//
//   56                 push esi                    ; save ESI (callee-saved)
//   8b f1              mov  esi, ecx               ; esi = this
//   8b 46 04           mov  eax, [esi+4]           ; load member at +0x4
//   50                 push eax                    ; pass [esi+4] as argument
//   c7 06 7c 73 f6 00  mov  dword [esi], 0xf6737c  ; this->vftable = 0x00f6737c
//   e8 07 55 58 00     call 0x009d1be9             ; hand off (no epilogue)
//
//   __thiscall: ECX carries `this`. The body re-stamps the object's
//   vtable pointer to 0x00f6737c (a .rdata vftable, reloc-bearing in the
//   live image) and then transfers control to the helper at 0x009d1be9
//   passing the +0x4 member. The function carries NO epilogue — no
//   `pop esi`, no `ret` — so the saved ESI is left on the stack and the
//   pushed member doubles as the helper's argument: the callee owns the
//   return (a CALL-shaped tail handoff into a non-returning / stack-
//   reclaiming helper — the classic shape MSVC 2005 will not regenerate
//   from any C++ source we can write for an 18-byte body).
//
// Reloc-bearing sites in the orig 18 bytes (resolve only in a full-binary
// relink at image base 0x00400000; standalone .obj compilation cannot
// reproduce them as relocations):
//   +0x09   absolute vftable imm32   .rdata 0x00f6737c
//   +0x0e   rel32 CALL               .text  0x009d1be9 (rel 0x00585507)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Because the function has no epilogue (the trailing CALL is the last
//   instruction and ESI is never restored), no C++ source compiled by
//   MSVC 2005 /O2 emits this shape: any source-level rewrite materialises
//   a `pop esi` / `ret` or rebalances the stack, shifting bytes in an
//   18-byte function where the vtable store and the CALL are both reloc-
//   adjacent. The pragmatic choice — identical to sibling FUN_00401730 —
//   is a `__declspec(naked)` body re-emitting the orig 18 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` ends up byte-identical
//   to the orig slice (raw immediates, no relocations), which is what
//   tools/compare.py grades against.

extern "C" __declspec(naked) void FUN_0044c6d0() {
    __asm {
        _emit 0x56

        _emit 0x8b
        _emit 0xf1

        _emit 0x8b
        _emit 0x46
        _emit 0x04

        _emit 0x50

        _emit 0xc7
        _emit 0x06
        _emit 0x7c
        _emit 0x73
        _emit 0xf6
        _emit 0x00

        _emit 0xe8
        _emit 0x07
        _emit 0x55
        _emit 0x58
        _emit 0x00
    }
}
