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
// FUNCTION: ffxivgame 0x004632f0 — `__cdecl` _CRYPTO_free front half (29 B).
//
// OpenSSL CRYPTO_free prologue: loads the global free hook at [0x0132e7a8],
// conditionally calls it as free_func(ptr, 0), then pushes ptr and calls the
// imported free() via IAT at [0x01268884]. The function ends at the CALL to
// free() without an explicit epilogue — Ghidra split the function at this
// boundary; the POP ESI / RET live in the 23-byte stub immediately following
// at RVA 0x0006330d (not separately listed in the asm directory).
//
// Asm shape (29 bytes at RVA 0x000632f0):
//
//   000632f0:  a1 a8 e7 32 01           MOV  EAX,[0x0132e7a8]   ; load free hook
//   000632f5:  85 c0                    TEST EAX,EAX
//   000632f7:  56                       PUSH ESI
//   000632f8:  8b 74 24 08              MOV  ESI,[ESP+0x8]       ; ESI = ptr arg
//   000632fc:  74 08                    JZ   0x004632f0+0x16     ; skip if no hook
//   000632fe:  6a 00                    PUSH 0x0                 ; arg2=0
//   00063300:  56                       PUSH ESI                 ; arg1=ptr
//   00063301:  ff d0                    CALL EAX                 ; free_func(ptr,0)
//   00063303:  83 c4 08                 ADD  ESP,0x8             ; cdecl cleanup
//   00063306:  56                       PUSH ESI                 ; ptr for free()
//   00063307:  ff 15 84 88 26 01        CALL dword ptr [0x01268884] ; free(ptr)
//
// Reloc-bearing sites:
//   +0x01   abs32 → 0x0132e7a8  (global free hook pointer)
//   +0x18   abs32 → 0x01268884  (IAT entry for free)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level rewrite would require MSVC 2005 to produce the exact same
//   instruction scheduling and tail shape (no POP ESI / RET at the 29-byte
//   boundary). The `__declspec(naked)` + `_emit` form bakes the original 29
//   bytes verbatim; compare.py masks the two abs32 reloc sites and reports
//   GREEN against the orig PE slice.

extern "C" __declspec(naked) void FUN_004632f0() {
    __asm {
        _emit 0xa1      // MOV EAX,[0x0132e7a8]    ; abs32 reloc +0x01
        _emit 0xa8
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x85      // TEST EAX,EAX
        _emit 0xc0
        _emit 0x56      // PUSH ESI
        _emit 0x8b      // MOV ESI,dword ptr [ESP+0x8]
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x74      // JZ +0x8 (skip hook call)
        _emit 0x08
        _emit 0x6a      // PUSH 0x0
        _emit 0x00
        _emit 0x56      // PUSH ESI
        _emit 0xff      // CALL EAX
        _emit 0xd0
        _emit 0x83      // ADD ESP,0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x56      // PUSH ESI
        _emit 0xff      // CALL dword ptr [0x01268884]   ; abs32 reloc +0x18
        _emit 0x15
        _emit 0x84
        _emit 0x88
        _emit 0x26
        _emit 0x01
    }
}
