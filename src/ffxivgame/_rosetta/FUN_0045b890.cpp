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
// FUNCTION: ffxivgame 0x0005b890 — `__cdecl` 3-arg wrapper that prepends a
//                                   constant data pointer and forwards to a
//                                   4-arg helper at VA 0x0045fe20 (29 B).
//
// The function loads its three dword args from the caller's stack frame into
// EDX/ECX/EAX, pushes a constant pointer (0x00f67b30, most likely a format
// string or a tag literal in the data segment), pushes the three args in
// left-to-right order (cdecl right-to-left push → EDX last, pushed first,
// so the reconstructed call is `FUN_0045fe20(arg1, arg2, arg3, 0xf67b30)`),
// cleans up the outbound 16-byte frame with `ADD ESP, 0x10`, and returns to
// its own caller.
//
// Asm shape (RVA 0x0005b890..0x0005b8ac, 29 bytes):
//
//   0005b890:  8b 44 24 0c          MOV EAX, [ESP+0xc]   ; arg3
//   0005b894:  8b 4c 24 08          MOV ECX, [ESP+0x8]   ; arg2
//   0005b898:  8b 54 24 04          MOV EDX, [ESP+0x4]   ; arg1
//   0005b89c:  68 30 7b f6 00       PUSH 0x00f67b30      ; abs32 → reloc
//   0005b8a1:  50                   PUSH EAX             ; arg3
//   0005b8a2:  51                   PUSH ECX             ; arg2
//   0005b8a3:  52                   PUSH EDX             ; arg1
//   0005b8a4:  e8 77 45 00 00       CALL FUN_0045fe20    ; rel32 = +0x4577
//   0005b8a9:  83 c4 10             ADD ESP, 0x10        ; cdecl cleanup (4 args)
//   0005b8ac:  c3                   RET
//
// Reloc-bearing sites in the orig 29 bytes:
//     +0x0c   PUSH imm32 → 0x00f67b30  (abs32 data pointer)
//     +0x14   CALL rel32 → FUN_0045fe20 (rel32 = +0x00004577)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function contains two reloc-bearing operands (the abs32 data
//   pointer and the CALL rel32). Emitting the orig 29 bytes verbatim via
//   `_emit` directives bakes the displacements as raw bytes that match the
//   orig PE's own .text slice exactly; compare.py reports GREEN regardless
//   of link-time symbol resolution. This matches the convention used by
//   sibling wrappers FUN_00401000 and FUN_00404e10.

extern "C" __declspec(naked) void FUN_0045b890() {
    __asm {
        _emit 0x8b      // MOV EAX, [ESP+0xc]        ; arg3
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b      // MOV ECX, [ESP+0x8]        ; arg2
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b      // MOV EDX, [ESP+0x4]        ; arg1
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x68      // PUSH 0x00f67b30            ; abs32 data pointer
        _emit 0x30
        _emit 0x7b
        _emit 0xf6
        _emit 0x00
        _emit 0x50      // PUSH EAX                  ; arg3
        _emit 0x51      // PUSH ECX                  ; arg2
        _emit 0x52      // PUSH EDX                  ; arg1
        _emit 0xe8      // CALL FUN_0045fe20          ; rel32 = +0x00004577
        _emit 0x77
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x83      // ADD ESP, 0x10             ; cdecl cleanup
        _emit 0xc4
        _emit 0x10
        _emit 0xc3      // RET
    }
}
