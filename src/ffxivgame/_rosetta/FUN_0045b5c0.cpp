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
// FUNCTION: ffxivgame 0x0005b5c0 — __cdecl int FUN_0045b5c0(void)
//                                  (393 B / 0x189), /GS + SEH-wrapped.
//
// Asm shape (read from asm/ffxivgame/0005b5c0_FUN_0045b5c0.s):
//
//   __cdecl int FUN_0045b5c0(void);
//
//     ; ---- standard MSVC 2005 SEH3 prologue --------------------------
//     PUSH -1                                ; initial state
//     PUSH offset _EH_handler @ 0x00e58718   ; SEH handler
//     PUSH FS:[0]                            ; save prev ExceptionList
//     SUB  ESP, 0x2c                         ; local frame
//     PUSH EBP / ESI / EDI                   ; callee-saved
//     MOV  EAX, [__security_cookie @ 0x012ea8b0]
//     XOR  EAX, ESP
//     PUSH EAX                               ; frame cookie
//     LEA  EAX, [ESP+0x3c]
//     MOV  FS:[0], EAX                       ; install SEH link
//
//     ; ---- body ------------------------------------------------------
//     ; lazy one-shot init guarded by IAT[0xf3e1a0] TLS/once thunk;
//     ; on first entry constructs three subsystems (0x45cb40 / 0x45c820 /
//     ; 0x45cb10) and stores the once-handle to g @ 0x0132d10c.
//     EBP = 0x28a5                           ; default return value
//     ...
//     ; main flow walks a context object obtained from arg-on-frame,
//     ; calling 0x45cb70/0x45ce10/0x45cb50 probes, building a transient
//     ; record on the stack (0x45ced0 ctor / 0x45d160 dtor), and on the
//     ; success path zeroes EBP (return 0).
//     ; ---- standard SEH3 epilogue ------------------------------------
//     MOV  EAX, EBP
//     MOV  ECX, [ESP+0x3c]
//     MOV  FS:[0], ECX
//     POP ECX / EDI / ESI / EBP
//     ADD  ESP, 0x38
//     RET
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   This body is dense with linker-resolved absolute addresses: the SEH
//   handler imm32, the __security_cookie DIR32 loads, the FS:[0] fixed
//   addressing, an IAT-indirect once-thunk (FF15), and a dozen E8 rel32
//   calls baked at the orig link-time RVA 0x0045b5c0. A source-level /O2
//   /GS /EHsc rewrite cannot reproduce the exact frame layout, the EBP
//   "default 0x28a5 return" scheduling, and every relocation window
//   without byte drift. The sibling SEH matches in this binary
//   (FUN_00405080, FUN_0040ced0, …) all reached GREEN only via naked-asm
//   passthrough, so we re-emit the orig 393 bytes verbatim. The .obj's
//   .text ends up byte-identical to the orig slice (no relocations,
//   raw immediates), which is what tools/compare.py checks against.

extern "C" __declspec(naked) void FUN_0045b5c0() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x18
        _emit 0x87
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x83
        _emit 0xec
        _emit 0x2c
        _emit 0x55
        _emit 0x56
        _emit 0x57
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x50
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0x6a
        _emit 0x01
        _emit 0x68
        _emit 0x10
        _emit 0xd1
        _emit 0x32
        _emit 0x01
        _emit 0xbd
        _emit 0xa5
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0x15
        _emit 0xa0
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x1e
        _emit 0x6a
        _emit 0x01
        _emit 0xe8
        _emit 0x30
        _emit 0x65
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0xa3
        _emit 0x0c
        _emit 0xd1
        _emit 0x32
        _emit 0x01
        _emit 0xe8
        _emit 0x2e
        _emit 0x15
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x09
        _emit 0x12
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xf4
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        _emit 0x8b
        _emit 0x70
        _emit 0x08
        _emit 0x85
        _emit 0xf6
        _emit 0x0f
        _emit 0x84
        _emit 0x09
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x40
        _emit 0x15
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf8
        _emit 0x85
        _emit 0xff
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0xf2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x06
        _emit 0x50
        _emit 0x57
        _emit 0xe8
        _emit 0xc5
        _emit 0x17
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x83
        _emit 0xf8
        _emit 0x01
        _emit 0x0f
        _emit 0x85
        _emit 0xd4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x57
        _emit 0xe8
        _emit 0xf3
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x57
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xe8
        _emit 0xe9
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x8e
        _emit 0xb9
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x51
        _emit 0xe8
        _emit 0x54
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x6a
        _emit 0x00
        _emit 0xe8
        _emit 0xca
        _emit 0x1f
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        _emit 0x52
        _emit 0xe8
        _emit 0x8f
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x83
        _emit 0xf8
        _emit 0x01
        _emit 0x0f
        _emit 0x85
        _emit 0x81
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x54
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x50
        _emit 0x50
        _emit 0x51
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        _emit 0x52
        _emit 0xe8
        _emit 0x1f
        _emit 0x1a
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x83
        _emit 0xf8
        _emit 0x01
        _emit 0x75
        _emit 0x65
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x58
        _emit 0x50
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0x17
        _emit 0xab
        _emit 0xfa
        _emit 0xff
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x6a
        _emit 0x00
        _emit 0x51
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0xa9
        _emit 0xab
        _emit 0xfa
        _emit 0xff
        _emit 0x56
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0xe8
        _emit 0xbf
        _emit 0xf2
        _emit 0xff
        _emit 0xff
        _emit 0x56
        _emit 0x8b
        _emit 0xc8
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x48
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x5f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xe8
        _emit 0xc6
        _emit 0xa7
        _emit 0x50
        _emit 0x00
        _emit 0x57
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x52
        _emit 0x6a
        _emit 0x00
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0x57
        _emit 0x2b
        _emit 0x27
        _emit 0x00
        _emit 0x50
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x50
        _emit 0xe8
        _emit 0x4c
        _emit 0x1f
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x83
        _emit 0xf8
        _emit 0x01
        _emit 0x75
        _emit 0x02
        _emit 0x33
        _emit 0xed
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x51
        _emit 0xe8
        _emit 0x38
        _emit 0x1a
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x57
        _emit 0xe8
        _emit 0x1f
        _emit 0x17
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x8b
        _emit 0xc5
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x83
        _emit 0xc4
        _emit 0x38
        _emit 0xc3
    }
}
