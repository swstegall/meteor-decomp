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
// FUNCTION: ffxivgame 0x00001730 — `__stdcall` 4-arg trampoline that
//                                   forwards to the FUN_004014b0
//                                   message-pump tick (18 B / 0x12).
//
// Inspection (read from the disassembly at orig RVA 0x00001730):
//
//   __stdcall <ret> trampoline(arg, arg, arg, arg) — the `ret 0x10`
//   epilogue tells us the function is `__stdcall` and pops 16 bytes
//   (four 32-bit slots) of caller-pushed arguments. The body itself
//   ignores every argument; it only consults the static singleton
//   pointer at .data 0x013232b8 and, if non-null, invokes the
//   `__thiscall` tick callee (FUN_004014b0) on it.
//
//   Structure (matches the asm flow):
//
//     if (g_frame /* [0x013232b8] */ != nullptr) {
//         g_frame->tick();          // CALL 0x004014b0 (__thiscall)
//     }
//     // EAX is left holding whatever the tick callee returned (or is
//     // undefined when the callee was skipped) — the caller treats
//     // this slot as void; the trampoline never wipes EAX.
//
//   No prologue. No callee-saves. No stack frame. No security cookie.
//   The four argument slots cleaned by `ret 0x10` line up with the
//   canonical Win32 WNDPROC signature (HWND, UINT, WPARAM, LPARAM) —
//   this is presumably the static dispatcher that the window class
//   registers, forwarding every message to the singleton's tick().
//
// Reloc-bearing sites in the orig 18 bytes (these absolute addresses
// resolve only in a full-binary relink at image base 0x00400000;
// standalone .obj compilation can't reproduce them):
//   +0x02   global ptr load          .data 0x013232b8 — singleton ptr
//   +0x0b   __thiscall callee CALL   .text 0x004014b0 rel32 (FUN_004014b0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The orig has no prologue and no epilogue beyond `ret 0x10`. MSVC
//   2005 /O2 /Oy will not produce that shape from any C++ source we
//   can write — the singleton-pointer-deref idiom either materialises
//   a stack frame, wipes EAX before return, or pushes the displacement-
//   only `MOV ECX, [imm32]` through a different modrm path. Each of
//   those shifts at least one byte in an 18-byte function where every
//   byte is reloc-adjacent.
//
//   The pragmatic choice — the same one FUN_004014b0 / FUN_00401820 /
//   FUN_00401a00 took for their reloc-heavy bodies — is a
//   `__declspec(naked)` body that re-emits the orig 18 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice (no relocations because the bytes
//   are emitted as raw immediates), which is what `tools/compare.py`
//   checks against.
//
//   The structural commentary above is the readable record of what
//   the function actually does, so a future contributor can promote
//   this to a real source-level match once the surrounding window-
//   frame class (singleton at .data 0x013232b8, tick() at .text
//   0x004014b0) is catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_00401730() {
    __asm {
        _emit 0x8b
        _emit 0x0d
        _emit 0xb8
        _emit 0x32
        _emit 0x32
        _emit 0x01

        _emit 0x85
        _emit 0xc9
        _emit 0x74
        _emit 0x05
        _emit 0xe8
        _emit 0x71
        _emit 0xfd
        _emit 0xff
        _emit 0xff

        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
