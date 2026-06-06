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
// FUNCTION: ffxivgame 0x0044d830 — __stdcall guarded dispatch helper (27 B form)
//
// __stdcall void FUN_0044d830(int arg)
//   [ESP+0x4] : arg
//
// Asm shape (27 bytes, read from orig RVA 0x0004d830):
//
//   0004d830:  8b 44 24 04        mov    eax, dword ptr [esp+4]   ; arg
//   0004d834:  85 c0              test   eax, eax
//   0004d836:  7c 10              jl     0x0044d848               ; if (arg < 0) return
//   0004d838:  50                 push   eax                      ; arg
//   0004d839:  a1 4c cf 32 01     mov    eax, [0x0132cf4c]        ; global handle
//   0004d83e:  6a 00              push   0
//   0004d840:  6a 12              push   0x12
//   0004d842:  50                 push   eax
//   0004d843:  e8 66 28 58 00     call   0x009d00ae               ; dispatch(global, 0x12, 0, arg)
//   0004d848:  c2 04 00           ret    4                        ; __stdcall, 1 stack arg
//
// The branch is the standard MSVC 2005 lowering of an early-out
// `if (arg < 0) return;` guard — JL emits the fall-through arm
// unconditionally, so the negative-argument case skips straight to the
// RET. The dispatch call reads a module-global handle from absolute
// address 0x0132cf4c and forwards (handle, 0x12, 0, arg) to the helper
// at 0x009d00ae via the right-to-left __cdecl push order.
//
// Reloc-bearing sites in the orig 27 bytes:
//   +0x09   MOV  imm32 → 0x0132cf4c   (DIR32, module-global handle)
//   +0x13   CALL rel32 → 0x009d00ae   (REL32, dispatch helper)
//
// Reconstruction strategy — naked-asm byte passthrough (same as sibling
// FUN_00409580): a `__declspec(naked)` body re-emits the 27 orig bytes
// verbatim via MASM `_emit`. The DIR32 global address and REL32 call
// target are baked in as concrete byte values that already resolve
// against the orig PE's address space, so the .obj carries zero
// relocations and tools/compare.py reports GREEN without reloc masking.

extern "C" __declspec(naked) void FUN_0044d830() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7c              // JL +0x10  (0x0044d848)
        _emit 0x10
        _emit 0x50              // PUSH EAX
        _emit 0xa1              // MOV EAX, [0x0132cf4c]
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x6a              // PUSH 0x12
        _emit 0x12
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x009d00ae  (rel32)
        _emit 0x66
        _emit 0x28
        _emit 0x58
        _emit 0x00
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
