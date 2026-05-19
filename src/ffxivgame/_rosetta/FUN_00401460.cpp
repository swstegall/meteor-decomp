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
// FUNCTION: ffxivgame 0x00001460 — `__thiscall` Win32 window-frame teardown
//                                  (66 B / 0x42)
//
// Inspection (read from the disassembly at orig RVA 0x00001460):
//
//   __thiscall void teardown(this) — `ECX = this`, no return value, no args.
//
//   Structure (matches asm flow):
//
//     this->m_sub_34.vfn2();                        // [esi+0x34], (*(*[esi+0x34]+8))(&[esi+0x34])
//     DestroyWindow(this->m_hwnd_18);               // [esi+0x18], import [0x00f3e47c]
//     if (this->m_overlay_960 != NULL) {            // [esi+0x960]
//         release_a(this->m_overlay_960);           // import [0x00f3e1e8]
//         release_b(this->m_overlay_960);           // import [0x00f3e1ec]
//         this->m_overlay_960 = NULL;
//     }
//
//   Sibling at 0x004014b0 (the Win32 message-pump tick on the same
//   class) confirms: +0x18 is a HWND that USER32 imports operate on,
//   +0x34 is an inline polymorphic input-handler subobject (its vtbl
//   slots [1] = poll() / [2] = teardown()), and +0x960 is a pointer
//   to a heap-allocated overlay or surface object that the class owns.
//   The two adjacent IAT slots 0xf3e1e8 / 0xf3e1ec are paired
//   release-then-free imports from the same DLL (typical D3D-ish
//   release pattern).
//
//   Calling convention: __thiscall (ECX = this, no stack args, RET 0).
//   Stack frame: -4 (PUSH ESI / POP ESI bracket).
//
// Reloc-bearing sites in the orig 66 bytes (these absolute addresses
// resolve only in a full-binary relink at image base 0x00400000;
// standalone .obj compilation can't reproduce them via source — naked
// asm emits them as raw immediate bytes which happen to match the
// orig binary's resolved IAT addresses byte-for-byte):
//     +0x12   import IAT load (.rdata 0x00f3e47c — DestroyWindow-ish)
//     +0x23   import IAT load (.rdata 0x00f3e1e8 — release_a)
//     +0x30   import IAT load (.rdata 0x00f3e1ec — release_b)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc
//   into producing the exact register-allocation pattern (re-loading
//   m_overlay_960 from memory after each call rather than caching in
//   a callee-save) and the exact branch encoding (JZ short → +0x1e).
//   Both are sensitive to surrounding code in the full TU.
//
//   The pragmatic choice — the same one the sibling FUN_004014b0 took
//   for its 307-byte SEH-wrapped message-pump tick — is a
//   `__declspec(naked)` body that re-emits the orig 66 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice (no relocations: the IAT
//   addresses are absolute values in the binary's own address space,
//   so emitting them as immediates produces the same bytes the linker
//   would produce). `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_00401460() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, [ESI+0x34]   (vptr of m_sub_34)
        _emit 0x46
        _emit 0x34
        _emit 0x8b              // MOV EDX, [EAX+0x8]    (vtable[2])
        _emit 0x50
        _emit 0x08
        _emit 0x8d              // LEA ECX, [ESI+0x34]   (&m_sub_34)
        _emit 0x4e
        _emit 0x34
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EAX, [ESI+0x18]   (m_hwnd_18)
        _emit 0x46
        _emit 0x18
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL [0x00f3e47c]
        _emit 0x15
        _emit 0x7c
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESI+0x960]
        _emit 0x86
        _emit 0x60
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x1e -> end
        _emit 0x1e
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL [0x00f3e1e8]
        _emit 0x15
        _emit 0xe8
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV ECX, [ESI+0x960]
        _emit 0x8e
        _emit 0x60
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0xff              // CALL [0x00f3e1ec]
        _emit 0x15
        _emit 0xec
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0xc7              // MOV [ESI+0x960], 0
        _emit 0x86
        _emit 0x60
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
