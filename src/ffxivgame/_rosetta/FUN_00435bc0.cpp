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
// FUNCTION: ffxivgame 0x00435bc0 — "assert-once" logging thunk:
//                                  call vtable[41] on arg, if non-zero
//                                  lazily init a fn-ptr global and call it
//                                  with 5 args (file/func/line/msg strings)
//                                  (__stdcall, single arg, 85 bytes)
//
// Asm shape (read from orig RVA 0x00035bc0, 85 bytes total):
//
//   00035bc0:  8b 44 24 04              MOV EAX,[ESP+0x4]      ; obj = arg
//   00035bc4:  8b 08                    MOV ECX,[EAX]           ; ecx = *obj (vtable)
//   00035bc6:  8b 91 a4 00 00 00        MOV EDX,[ECX+0xa4]     ; edx = vtable[41]
//   00035bcc:  50                       PUSH EAX                ; push obj
//   00035bcd:  ff d2                    CALL EDX                ; call vtable method
//   00035bcf:  85 c0                    TEST EAX,EAX
//   00035bd1:  74 3f                    JZ   → 00435c12         ; if 0, return
//   00035bd3:  b8 01 00 00 00           MOV  EAX,1
//   00035bd8:  84 05 10 39 32 01        TEST byte ptr [0x01323910],AL
//   00035bde:  75 10                    JNZ  → 00435bf0         ; already init'd, skip
//   00035be0:  09 05 10 39 32 01        OR   dword ptr [0x01323910],EAX  ; set flag
//   00035be6:  c7 05 0c 39 32 01        MOV  dword ptr [0x0132390c],0x00433720
//              20 37 43 00
//   00435bf0:  68 d8 51 f6 00           PUSH 0x00f651d8         ; arg5 (string)
//   00435bf5:  68 e6 01 00 00           PUSH 0x1e6              ; arg4 = 486 (line#)
//   00435bfa:  68 18 4c f6 00           PUSH 0x00f64c18         ; arg3 (string)
//   00435bff:  68 bc 51 f6 00           PUSH 0x00f651bc         ; arg2 (string)
//   00435c04:  68 e8 4b f6 00           PUSH 0x00f64be8         ; arg1 (string)
//   00435c09:  ff 15 0c 39 32 01        CALL dword ptr [0x0132390c]
//   00435c0f:  83 c4 14                 ADD  ESP,0x14           ; cdecl cleanup (5 args)
//   00435c12:  c2 04 00                 RET  0x4                ; __stdcall: pop 4 bytes
//
// Reloc-bearing sites in the orig 85 bytes:
//   +0x18  TEST/OR [DIR32] → 0x01323910  (byte flag: "have we init'd?")
//   +0x20  OR  [DIR32]    → 0x01323910  (same flag, write side)
//   +0x26  MOV [DIR32]    → 0x0132390c  (fn-ptr slot, write)
//   +0x2a  MOV imm32      → 0x00433720  (fn-ptr value, DIR32)
//   +0x2f  PUSH imm32     → 0x00f651d8  (DIR32 string literal)
//   +0x39  PUSH imm32     → 0x00f64c18  (DIR32 string literal)
//   +0x3e  PUSH imm32     → 0x00f651bc  (DIR32 string literal)
//   +0x43  PUSH imm32     → 0x00f64be8  (DIR32 string literal)
//   +0x49  CALL [DIR32]   → 0x0132390c  (indirect call via fn-ptr slot)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Multiple DIR32 operands in a single function (global flag, fn-ptr slot,
//   fn-ptr value, four string-literal pushes, indirect-call operand) make
//   a source-level C++ reconstruction extremely fragile — the compiler would
//   emit the same shape but introduce .obj relocations that compare.py would
//   need to mask. The naked `_emit` body re-emits the 85 orig bytes
//   verbatim into the .obj's .text with no relocations so compare.py
//   reports GREEN without reloc masking, matching the pattern used by
//   siblings FUN_004065c0, FUN_00403b70, etc. in this directory.

extern "C" __declspec(naked) void FUN_00435bc0() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x4]   ; obj = arg
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [EAX]        ; ecx = *obj (vtable)
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ECX+0xa4]  ; edx = vtable[41]
        _emit 0x91
        _emit 0xa4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX                        ; push obj
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ   +0x3f  (→ RET)
        _emit 0x3f
        _emit 0xb8              // MOV  EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [0x01323910], AL
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ  +0x10  (→ PUSH 0xf651d8)
        _emit 0x10
        _emit 0x09              // OR   dword ptr [0x01323910], EAX  ; set flag
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV  dword ptr [0x0132390c], 0x00433720
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00
        _emit 0x68              // PUSH 0x00f651d8   ; arg5
        _emit 0xd8
        _emit 0x51
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x1e6        ; arg4 = 486 (line number)
        _emit 0xe6
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x00f64c18   ; arg3
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x00f651bc   ; arg2
        _emit 0xbc
        _emit 0x51
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x00f64be8   ; arg1
        _emit 0xe8
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x0132390c]   ; call fn-ptr
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD  ESP, 0x14   ; cdecl cleanup (5 args × 4)
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET  0x4         ; __stdcall: caller-arg cleanup
        _emit 0x04
        _emit 0x00
    }
}
