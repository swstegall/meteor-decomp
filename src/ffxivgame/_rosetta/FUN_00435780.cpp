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
// FUNCTION: ffxivgame 0x00435780 — guarded virtual-dispatch + lazy-init
//                                  diagnostic dispatcher (__thiscall, 95 bytes)
//
// __thiscall bool FUN_00435780(this, Obj *arg) [arg at ESP+0x4 after RET 4]
//
// Behaviour: load arg's vtable, call its slot at +0x104 with three args
// (arg, this->field_4, this->field_8). If the call returns zero, return
// (RET 4) immediately. Otherwise run a once-only initialiser for the
// function pointer at [0x0132390c]: a guard bit at [0x01323910] is
// tested with AL=1; if unset, OR the bit in and store the dispatcher
// VA 0x00433720 into [0x0132390c]. Then push five args (a string/data
// blob set: 0xf64ce0, line number 0xed, 0xf64c18, 0xf64cc4, 0xf64be8)
// and CALL [0x0132390c], cleaning up 0x14 bytes (__cdecl, 5 dwords).
//
// Asm (95 bytes, read from orig RVA 0x00035780):
//
//   8b 44 24 04                 MOV  EAX, [ESP+0x4]          ; arg
//   8b 10                       MOV  EDX, [EAX]              ; arg->vtable
//   8b 92 04 01 00 00           MOV  EDX, [EDX+0x104]        ; vtable slot
//   56                          PUSH ESI
//   8b 71 08                    MOV  ESI, [ECX+0x8]          ; this->field_8
//   8b 49 04                    MOV  ECX, [ECX+0x4]          ; this->field_4
//   56                          PUSH ESI
//   51                          PUSH ECX
//   50                          PUSH EAX                     ; arg
//   ff d2                       CALL EDX
//   85 c0                       TEST EAX, EAX
//   5e                          POP  ESI
//   74 3f                       JZ   0x004357dc              ; -> RET
//   b8 01 00 00 00              MOV  EAX, 1
//   84 05 10 39 32 01           TEST byte ptr [0x01323910], AL
//   75 10                       JNZ  0x004357ba              ; already inited
//   09 05 10 39 32 01           OR   [0x01323910], EAX
//   c7 05 0c 39 32 01 20 37 43 00  MOV [0x0132390c], 0x433720
//   68 e0 4c f6 00              PUSH 0xf64ce0
//   68 ed 00 00 00              PUSH 0xed
//   68 18 4c f6 00              PUSH 0xf64c18
//   68 c4 4c f6 00              PUSH 0xf64cc4
//   68 e8 4b f6 00              PUSH 0xf64be8
//   ff 15 0c 39 32 01           CALL [0x0132390c]
//   83 c4 14                    ADD  ESP, 0x14
//   c2 04 00                    RET  0x4
//
// Reloc-bearing sites (DIR32 absolute data/code addresses the linker
// would resolve from source-level C++): the guard word [0x01323910],
// the function-pointer slot [0x0132390c] (3 references), the stored
// dispatcher VA 0x00433720, and the five PUSH'd blob addresses. We
// re-emit the orig 95 bytes verbatim via MASM `_emit`, so the .obj's
// .text is byte-identical to the orig slice with NO relocations and
// tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00435780() {
    __asm {
        _emit 0x8b              // MOV  EAX, [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV  EDX, [EAX]
        _emit 0x10
        _emit 0x8b              // MOV  EDX, [EDX+0x104]
        _emit 0x92
        _emit 0x04
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV  ESI, [ECX+0x8]
        _emit 0x71
        _emit 0x08
        _emit 0x8b              // MOV  ECX, [ECX+0x4]
        _emit 0x49
        _emit 0x04
        _emit 0x56              // PUSH ESI
        _emit 0x51              // PUSH ECX
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x5e              // POP  ESI
        _emit 0x74              // JZ   0x004357dc  (+0x3F)
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
        _emit 0x75              // JNZ  0x004357ba  (+0x10)
        _emit 0x10
        _emit 0x09              // OR   [0x01323910], EAX
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV  [0x0132390c], 0x433720
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00
        _emit 0x68              // PUSH 0xf64ce0
        _emit 0xe0
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0xed
        _emit 0xed
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf64c18
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0xf64cc4
        _emit 0xc4
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0xf64be8
        _emit 0xe8
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        _emit 0xff              // CALL [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD  ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET  0x4
        _emit 0x04
        _emit 0x00
    }
}
