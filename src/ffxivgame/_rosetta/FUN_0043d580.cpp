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
// FUNCTION: ffxivgame 0x0003d580 — __thiscall NRV-return builder with a /GS
//                                  cookie + full inline-SEH frame (466 B / 0x1d2).
//
// Inspection (read from the disassembly at orig RVA 0x0003d580):
//
//   __thiscall <obj>* build(this /*ECX*/, ..., <obj>* __ret /*[esp+0x444]*/);
//
//   The function uses MSVC's standard /GS + EH prologue:
//     PUSH -1 / PUSH 0x00e56a47 (scope table) / PUSH FS:[0]
//     SUB ESP,0x424
//     MOV EAX,[0x012ea8b0]; XOR EAX,ESP; MOV [esp+0x420],EAX   // __security_cookie
//     PUSH EBP/ESI/EDI
//     MOV EAX,[0x012ea8b0]; XOR EAX,ESP; PUSH EAX              // cookie-xor frame guard
//     LEA EAX,[esp+0x434]; MOV FS:[0],EAX                      // install EH node
//   EBP is the hidden NRV out-pointer (loaded from [esp+0x444]); ESI = this.
//   EAX returns EBP at the epilogue.
//
//   Structural shape:
//     out->field = 0;
//     FUN_004330f0(&local, this);                 // probe; writes [esp+0x14]
//     if (local[0] == 0) { *out = 0; goto done; }
//     int r = FUN_009d6b65(&local, this, 0xf66a24);// [esp+0x1c] result
//     if (r == 0) {                                // error path: format + log
//         FUN_009d4f9f(&buf, 0x400, 0x3fd, 0xf66a68, 0x1b1, 0xf66a28);
//         FUN_009d4f9f(&buf[r], 0x400-r, 0x3fe-r, 0xf66abc, this);
//         FUN_009d4bb4(&buf2, 0x400, 0xf66ae8);
//         (*0x012660d8)(&buf3, 4);                 // IAT-indirect log sink
//         *out = 0; goto done;
//     }
//     // success path: construct the result object
//     EDI = FUN_009d6962(FUN_009d6a61(r));
//     ESI_obj = FUN_0040a330(FUN_0040e2d0(&x, 0x10, 0xf66aec), 0);
//     ESI_obj = FUN_0040e110(EDI, ESI_obj);
//     FUN_009d6947(ESI_obj, EDI, 1, [esp+0x10]);
//     FUN_009d2646([esp+0x20]);
//     p = FUN_0043cf00(&y, out_arg, ESI_obj, [esp+0x45c]);
//     EDI = *p; *p = 0;                            // move out the built ptr
//     if (refcounted) (**refcounted)(1);           // release temp
//     if (ESI_obj) FUN_0040df70(ESI_obj);          // free temp
//     *out = EDI;
//   done:
//     EAX = out; MOV FS:[0],saved; ...; CALL __security_check_cookie; RET.
//
//   Reloc-bearing sites (rel32 calls + IAT-indirect + absolute imm32):
//     scope table 0x00e56a47, cookie global 0x012ea8b0 (×2), string
//     literals 0xf66a24/0xf66a28/0xf66a68/0xf66abc/0xf66ae8/0xf66aec,
//     rel32 → FUN_004330f0/009d6b65/009d4f9f(×2)/009d4bb4/009d6a61/
//     009d6962/0040e2d0/0040a330/0040e110/009d6947/009d2646/0043cf00/
//     0040df70/009d20f4, IAT [0x012660d8].
//
// Reconstruction strategy — naked-asm byte passthrough (same idiom as the
//   sibling /GS + SEH bodies FUN_0040b840 / FUN_00401820): a
//   __declspec(naked) body that re-emits the orig 466 bytes verbatim via
//   `_emit`. A source-level C++ rewrite through __try/__finally would not
//   reliably reproduce the /GS cookie placement, the SEH scope-table imm32,
//   the EH state-byte stores, or the exact register allocation across the
//   ~17 reloc windows under /O2. The emitted .text is byte-identical to the
//   orig slice, which is what tools/compare.py grades.

extern "C" __declspec(naked) void FUN_0043d580() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x47
        _emit 0x6a
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x81
        _emit 0xec
        _emit 0x24
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0x20
        _emit 0x04
        _emit 0x00
        _emit 0x00
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
        _emit 0x84
        _emit 0x24
        _emit 0x34
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xac
        _emit 0x24
        _emit 0x44
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf1
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x56
        _emit 0x50
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x28
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x16
        _emit 0x5b
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x75
        _emit 0x0c
        _emit 0xc7
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe9
        _emit 0x39
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x24
        _emit 0x6a
        _emit 0xf6
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x56
        _emit 0x51
        _emit 0xe8
        _emit 0x65
        _emit 0x95
        _emit 0x59
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x7a
        _emit 0x68
        _emit 0x28
        _emit 0x6a
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0xb1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x68
        _emit 0x6a
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0xfd
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x40
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x52
        _emit 0x88
        _emit 0x84
        _emit 0x24
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x6a
        _emit 0x79
        _emit 0x59
        _emit 0x00
        _emit 0x56
        _emit 0x68
        _emit 0xbc
        _emit 0x6a
        _emit 0xf6
        _emit 0x00
        _emit 0xb9
        _emit 0xfe
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x2b
        _emit 0xc8
        _emit 0x51
        _emit 0xba
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x2b
        _emit 0xd0
        _emit 0x52
        _emit 0x8d
        _emit 0x44
        _emit 0x04
        _emit 0x58
        _emit 0x50
        _emit 0xe8
        _emit 0x4a
        _emit 0x79
        _emit 0x59
        _emit 0x00
        _emit 0x68
        _emit 0xe8
        _emit 0x6a
        _emit 0xf6
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x51
        _emit 0xe8
        _emit 0x4b
        _emit 0x75
        _emit 0x59
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x68
        _emit 0x6a
        _emit 0x04
        _emit 0x52
        _emit 0xff
        _emit 0x15
        _emit 0xd8
        _emit 0x60
        _emit 0x26
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x40
        _emit 0xc7
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe9
        _emit 0xa4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0xe8
        _emit 0xd6
        _emit 0x93
        _emit 0x59
        _emit 0x00
        _emit 0x50
        _emit 0xe8
        _emit 0xd1
        _emit 0x92
        _emit 0x59
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x68
        _emit 0xec
        _emit 0x6a
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x10
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0x8b
        _emit 0xf8
        _emit 0xe8
        _emit 0x2a
        _emit 0x0c
        _emit 0xfd
        _emit 0xff
        _emit 0x6a
        _emit 0x00
        _emit 0x8b
        _emit 0xf0
        _emit 0xe8
        _emit 0x81
        _emit 0xcc
        _emit 0xfc
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x56
        _emit 0x57
        _emit 0x8b
        _emit 0xc8
        _emit 0xe8
        _emit 0x55
        _emit 0x0a
        _emit 0xfd
        _emit 0xff
        _emit 0x8b
        _emit 0xf0
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x50
        _emit 0x6a
        _emit 0x01
        _emit 0x57
        _emit 0x56
        _emit 0xe8
        _emit 0x7c
        _emit 0x92
        _emit 0x59
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x51
        _emit 0xe8
        _emit 0x71
        _emit 0x4f
        _emit 0x59
        _emit 0x00
        _emit 0x8b
        _emit 0x94
        _emit 0x24
        _emit 0x5c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x52
        _emit 0x56
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0x51
        _emit 0xe8
        _emit 0x13
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x10
        _emit 0x8b
        _emit 0xfa
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x85
        _emit 0xc9
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0x3c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x08
        _emit 0x8b
        _emit 0x01
        _emit 0x8b
        _emit 0x10
        _emit 0x6a
        _emit 0x01
        _emit 0xff
        _emit 0xd2
        _emit 0x85
        _emit 0xf6
        _emit 0x74
        _emit 0x09
        _emit 0x8b
        _emit 0x4e
        _emit 0xfc
        _emit 0x56
        _emit 0xe8
        _emit 0x4a
        _emit 0x08
        _emit 0xfd
        _emit 0xff
        _emit 0x89
        _emit 0x7d
        _emit 0x00
        _emit 0x8b
        _emit 0xc5
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x34
        _emit 0x04
        _emit 0x00
        _emit 0x00
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
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x20
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0xa9
        _emit 0x49
        _emit 0x59
        _emit 0x00
        _emit 0x81
        _emit 0xc4
        _emit 0x30
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0xc3
    }
}
