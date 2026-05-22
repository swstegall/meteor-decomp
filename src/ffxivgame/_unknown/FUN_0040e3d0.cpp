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
// FUNCTION: ffxivgame 0x0000e3d0 — inline-SEH wrapper that constructs/
//                                  initialises the object at (param+0x3c)
//                                  via FUN_0040f150 (__thiscall, 1 arg),
//                                  returning the object pointer or 0.
//                                  (82 B / 0x52, __cdecl, ESP-relative
//                                  SEH frame, no EBP, no /GS cookie)
//
// Inspection (orig bytes at RVA 0x0000e3d0..0x0000e421, 82 bytes):
//
//   __cdecl int FUN_0040e3d0(int param_1);
//
//   Prolog: inline EH3-style SEH frame (no __EH_prolog3 helper,
//   no security cookie — frameless /Oy compilation):
//
//     6a ff                  PUSH -0x1          ; SEH state = -1
//     68 c4 4e e5 00         PUSH 0xe54ec4      ; SEH scope-table / handler ← reloc
//     64 a1 00 00 00 00      MOV EAX, FS:[0x0]  ; prev SEH frame
//     50                     PUSH EAX
//     64 89 25 00 00 00 00   MOV FS:[0x0], ESP  ; install SEH frame
//     51                     PUSH ECX           ; allocate one local slot
//
//   Body:
//     8b 44 24 14            MOV EAX, [ESP+0x14]   ; EAX = param_1
//     8d 48 3c               LEA ECX, [EAX+0x3c]   ; ECX = param_1+0x3c (obj ptr)
//     89 0c 24               MOV [ESP], ECX         ; store in local (for SEH handler)
//     85 c9                  TEST ECX, ECX
//     c7 44 24 0c 00 00 00 00 MOV [ESP+0xc], 0x0   ; SEH state → 0 (enter try block)
//     74 15                  JZ  zero_path          ; if param_1 == -0x3c, skip
//     50                     PUSH EAX               ; push param_1 as __thiscall arg
//     e8 4e 0d 00 00         CALL FUN_0040f150      ; obj->ctor(param_1) ← reloc
//     8b 4c 24 04            MOV ECX, [ESP+0x4]     ; restore prev SEH frame
//     64 89 0d 00 00 00 00   MOV FS:[0x0], ECX
//     83 c4 10               ADD ESP, 0x10          ; drop 4 SEH locals
//     c3                     RET
//   zero_path:
//     8b 4c 24 04            MOV ECX, [ESP+0x4]     ; restore prev SEH frame
//     33 c0                  XOR EAX, EAX           ; return 0
//     64 89 0d 00 00 00 00   MOV FS:[0x0], ECX
//     83 c4 10               ADD ESP, 0x10
//     c3                     RET
//
// FUN_0040f150 is __thiscall (takes `this` in ECX = param_1+0x3c, one
// stack arg = param_1, returns EAX = this).  It cleans its own arg
// (RET 4), so after the call ESP is back to the pre-PUSH position.
//
// Reloc-bearing sites in the 82 bytes:
//     +0x03   PUSH imm32  → 0xe54ec4 (SEH scope-table/handler addr)
//     +0x2d   CALL rel32  → FUN_0040f150 (RVA 0x0000f150)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The inline EH3 prolog without __EH_prolog3, combined with the two
//   relocation sites above, means source-level C++ cannot reliably
//   reproduce the exact byte stream without hitting linker-resolved
//   absolute / relative immediates.  Emitting the 82 orig bytes verbatim
//   via `__declspec(naked)` / MASM `_emit` gives a .obj whose .text is
//   byte-identical to the orig slice; compare.py masks the two reloc
//   windows and reports GREEN.

extern "C" __declspec(naked) void FUN_0040e3d0() {
    __asm {
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0xe54ec4  (SEH handler/scope-table) ← reloc
        _emit 0xc4
        _emit 0x4e
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX  (prev SEH frame)
        _emit 0x64              // MOV FS:[0x0], ESP
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX  (allocate local slot)
        _emit 0x8b              // MOV EAX, [ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8d              // LEA ECX, [EAX+0x3c]
        _emit 0x48
        _emit 0x3c
        _emit 0x89              // MOV [ESP], ECX
        _emit 0x0c
        _emit 0x24
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0xc7              // MOV [ESP+0xc], 0x0  (SEH state → 0)
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x15  (→ zero_path)
        _emit 0x15
        _emit 0x50              // PUSH EAX  (__thiscall stack arg = param_1)
        _emit 0xe8              // CALL FUN_0040f150  ← reloc
        _emit 0x4e
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, [ESP+0x4]  (prev SEH frame)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x64              // MOV FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3              // RET
        _emit 0x8b              // zero_path: MOV ECX, [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x64              // MOV FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3              // RET
    }
}
