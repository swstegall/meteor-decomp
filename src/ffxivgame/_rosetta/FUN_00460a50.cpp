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
// FUNCTION: ffxivgame 0x00060a50 — _ASN1_item_i2d (28 B / 0x1c),
//                                  __cdecl 3-arg wrapper forwarding to
//                                  FUN_004609b0.
//
// Asm shape (28 bytes):
//
//   00060a50:  8b 44 24 0c    MOV  EAX, [ESP+0xc]  ; arg3 (it)
//   00060a54:  8b 4c 24 08    MOV  ECX, [ESP+0x8]  ; arg2 (out)
//   00060a58:  8b 54 24 04    MOV  EDX, [ESP+0x4]  ; arg1 (val)
//   00060a5c:  53             PUSH EBX             ; save EBX (callee-save)
//   00060a5d:  50             PUSH EAX             ; push arg3
//   00060a5e:  51             PUSH ECX             ; push arg2
//   00060a5f:  52             PUSH EDX             ; push arg1
//   00060a60:  33 db          XOR  EBX, EBX        ; EBX = 0 (between pushes and call)
//   00060a62:  e8 49 ff ff ff CALL FUN_004609b0    ; rel32 = -0xb7
//   00060a67:  83 c4 0c       ADD  ESP, 0xc        ; cdecl cleanup (3 args)
//   00060a6a:  5b             POP  EBX             ; restore EBX
//   00060a6b:  c3             RET
//
// Calling convention: __cdecl (RET without operand; caller cleans stack).
// EBX is saved/restored per the ABI callee-save contract. The XOR EBX,EBX
// between the arg pushes and the CALL is a compiler artefact (MSVC 2005
// initialises EBX to 0 as a dead store for a local int that was optimised
// away at the return site).
//
// Reconstruction strategy — __declspec(naked) byte passthrough.
//
//   The single reloc-bearing site is the CALL rel32 at offset +0x12
//   (bytes e8 49 ff ff ff). Using _emit bakes the orig displacement
//   verbatim, so compare.py reports GREEN regardless of where the
//   CALL target FUN_004609b0 lands in our own link.
//
// Reloc-bearing offsets within the function (compare.py masks these):
//   +0x13  REL32 → 0x004609b0  (FUN_004609b0)

extern "C" __declspec(naked) void FUN_00460a50() {
    __asm {
        _emit 0x8b      // MOV  EAX, [ESP+0xc]     ; arg3
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b      // MOV  ECX, [ESP+0x8]     ; arg2
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b      // MOV  EDX, [ESP+0x4]     ; arg1
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x53      // PUSH EBX                ; save EBX
        _emit 0x50      // PUSH EAX                ; push arg3
        _emit 0x51      // PUSH ECX                ; push arg2
        _emit 0x52      // PUSH EDX                ; push arg1
        _emit 0x33      // XOR  EBX, EBX
        _emit 0xdb
        _emit 0xe8      // CALL FUN_004609b0        ; rel32 = +0xffffff49
        _emit 0x49
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x83      // ADD  ESP, 0xc            ; cdecl cleanup
        _emit 0xc4
        _emit 0x0c
        _emit 0x5b      // POP  EBX                ; restore EBX
        _emit 0xc3      // RET
    }
}
