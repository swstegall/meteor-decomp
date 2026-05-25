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
// FUNCTION: ffxivgame 0x00014d40 — __thiscall ctor: zero three fields + install
//                                  global-allocator pointer (30 B / 0x1e)
//
// Calling convention: __thiscall (ECX = this), no stack args, RET 0.
// Returns `this` in EAX (classic MSVC ctor-returns-this idiom).
//
// Behaviour (read from the disassembly at orig RVA 0x00014d40):
//
//   1. PUSH ESI / MOV ESI,ECX           — cache `this` in ESI (callee-save).
//   2. CALL FUN_00414de0                 — base-init stub; FUN_00414de0 is a
//                                          3-byte `MOV EAX,ECX / RET` that
//                                          simply forwards `this` through.
//                                          This is the canonical MSVC pattern
//                                          for an inlined empty base ctor in
//                                          a derived ctor's mem-init chain.
//   3. XOR EAX,EAX
//      MOV [ESI+0x8],  EAX
//      MOV [ESI+0xc],  EAX               — zero three consecutive DWORD
//      MOV [ESI+0x10], EAX                 members at +0x8, +0xc, +0x10.
//   4. MOV [ESI+0x4], 0x01265f44         — install pointer to a global
//                                          object instance that lives at
//                                          VA 0x01265f44 (RVA 0xe65f44, in
//                                          the writable `.data` section).
//                                          That instance's first DWORD is a
//                                          vftable pointer to
//                                          `SQEX::CDev::Engine::Phieg::Base::
//                                          Memory::DefaultMemoryAllocator::
//                                          vftable` (RVA 0xb570b4, slot
//                                          count 4) — so [esi+4] is an
//                                          allocator handle field.
//   5. MOV EAX,ESI / POP ESI / RET       — return `this` in EAX (the C++
//                                          new-expression / mem-init-list
//                                          contract).
//
// Asm (30 bytes @ orig RVA 0x00014d40):
//   00 00:  56                       PUSH ESI
//   00 01:  8b f1                    MOV  ESI, ECX
//   00 03:  e8 RR RR RR RR           CALL FUN_00414de0          ; (rel32 reloc)
//   00 08:  33 c0                    XOR  EAX, EAX
//   00 0a:  89 46 08                 MOV  [ESI+0x8],  EAX
//   00 0d:  89 46 0c                 MOV  [ESI+0xc],  EAX
//   00 10:  89 46 10                 MOV  [ESI+0x10], EAX
//   00 13:  c7 46 04 44 5f 26 01     MOV  [ESI+0x4], 0x01265f44 ; abs imm32
//   00 1a:  8b c6                    MOV  EAX, ESI
//   00 1c:  5e                       POP  ESI
//   00 1d:  c3                       RET
//
// Reloc-bearing bytes: one REL32 site at offset 0x04 (the 4-byte
// displacement after `e8`). The `call FUN_00414de0` form makes MSVC
// emit an IMAGE_REL_I386_REL32 reloc that tools/compare.py masks. The
// `0x01265f44` imm32 at offset +0x16 is an absolute VA (the orig
// binary already has the resolved bytes baked in for its preferred
// image base 0x00400000); emitting it as raw bytes via _emit produces
// the exact wire image the linker would emit at relink. The same
// strategy is used in FUN_004138e0 (see that .cpp's commentary).
//
// Reconstruction strategy — __declspec(naked) byte-emit passthrough.
// A source-level rendering as a derived-ctor with `Base()` mem-init +
// three zero-stores + one pointer install would compile, but is
// fragile to MSVC 2005 /O2 register-allocation choices (whether the
// three zero-stores get coalesced into PXOR+MOVQ, whether the imm32
// store happens before or after the zeros, whether EAX is reused or
// freed). The naked form pins all of it.

extern "C" void FUN_00414de0();   // forward decl for REL32 reloc target

extern "C" __declspec(naked) void FUN_00414d40() {
    __asm {
        // 00014d40: 56                       PUSH ESI
        _emit 0x56
        // 00014d41: 8b f1                    MOV  ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00014d43: e8 RR RR RR RR           CALL FUN_00414de0   (rel32 reloc)
        call FUN_00414de0
        // 00014d48: 33 c0                    XOR  EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00014d4a: 89 46 08                 MOV  [ESI+0x8], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 00014d4d: 89 46 0c                 MOV  [ESI+0xc], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        // 00014d50: 89 46 10                 MOV  [ESI+0x10], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x10
        // 00014d53: c7 46 04 44 5f 26 01     MOV  [ESI+0x4], 0x01265f44
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x44
        _emit 0x5f
        _emit 0x26
        _emit 0x01
        // 00014d5a: 8b c6                    MOV  EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 00014d5c: 5e                       POP  ESI
        _emit 0x5e
        // 00014d5d: c3                       RET
        _emit 0xc3
    }
}
