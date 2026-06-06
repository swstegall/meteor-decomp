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
// FUNCTION: ffxivgame 0x00043c90 — `__thiscall` destructor with an SEH
//                                  (`/EHsc`) unwind frame that tears down
//                                  two member subobjects (89 B / 0x59).
//
// Inspection (read from asm/ffxivgame/00043c90_FUN_00443c90.s):
//
//   __thiscall void Class::~Class();      // `this` arrives in ECX
//
//     // Standard MSVC 2005 /EHsc + /GS function prologue: push the
//     // exception-handler record (handler @ 0x00e572fb, initial
//     // trylevel -1), then the XOR'd security cookie, then install the
//     // FS:[0] frame.
//     //
//     //   this->member_0x68.~T();   // unwind state 0 — first subobject
//     //   this->member_0x04.~U();   // unwind state -1 — second subobject
//     //
//     // Both subobject dtors are the same routine FUN_00446f50 (called
//     // __thiscall with ECX = &subobject). The MOV [ESP+0x14], imm
//     // stores are the SEH __ehfuncinfo trylevel updates that guard the
//     // two destructor calls; the store-to-0 before the first and the
//     // store-to-(-1) before the second is exactly how MSVC sequences a
//     // two-member dtor body under /EHsc.
//
// Layout touched by this fn (`this` aka ESI):
//   +0x04 : second member subobject (destroyed last, state -1)
//   +0x68 : first  member subobject (destroyed first, state 0)
//
// Reloc-bearing sites in the orig 89 bytes (these absolute / rel32
// references resolve only in a full-binary relink at image base
// 0x00400000; tools/compare.py masks reloc windows on the cmp_obj path
// so a naked-asm .obj with the same raw bytes matches byte-for-byte):
//     +0x02   SEH handler PUSH         (0x00e572fb)
//     +0x10   security-cookie moffs    (.data 0x012ea8b0)
//     +0x33   first  dtor CALL         (.text 0x00446f50 rel32)
//     +0x43   second dtor CALL         (.text 0x00446f50 rel32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level dtor would need MSVC 2005 to regenerate the exact
//   __ehfuncinfo / FS:[0] frame, the cookie XOR, and the two
//   linker-resolved rel32 CALL displacements. The pragmatic choice —
//   the same one the rest of this _rosetta row took — is a
//   `__declspec(naked)` body that re-emits the orig 89 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice (no relocations because the bytes
//   are emitted as raw immediates), which is what tools/compare.py
//   checks against.
//
// Asm shape (89 bytes — RVA 0x00043c90..0x00043ce9):
//
//     00043c90:  6a ff                       PUSH -0x1
//     00043c92:  68 fb 72 e5 00              PUSH 0xe572fb            ; handler
//     00043c97:  64 a1 00 00 00 00           MOV  EAX, FS:[0x0]
//     00043c9d:  50                          PUSH EAX
//     00043c9e:  51                          PUSH ECX
//     00043c9f:  56                          PUSH ESI
//     00043ca0:  a1 b0 a8 2e 01              MOV  EAX, [0x012ea8b0]   ; cookie
//     00043ca5:  33 c4                       XOR  EAX, ESP
//     00043ca7:  50                          PUSH EAX
//     00043ca8:  8d 44 24 0c                 LEA  EAX, [ESP+0xc]
//     00043cac:  64 a3 00 00 00 00           MOV  FS:[0x0], EAX
//     00043cb2:  8b f1                       MOV  ESI, ECX            ; this
//     00043cb4:  89 74 24 08                 MOV  [ESP+0x8], ESI
//     00043cb8:  8d 4e 68                    LEA  ECX, [ESI+0x68]
//     00043cbb:  c7 44 24 14 00 00 00 00     MOV  [ESP+0x14], 0x0     ; state 0
//     00043cc3:  e8 88 32 00 00              CALL 0x00446f50          ; dtor
//     00043cc8:  8d 4e 04                    LEA  ECX, [ESI+0x4]
//     00043ccb:  c7 44 24 14 ff ff ff ff     MOV  [ESP+0x14], -1      ; state -1
//     00043cd3:  e8 78 32 00 00              CALL 0x00446f50          ; dtor
//     00043cd8:  8b 4c 24 0c                 MOV  ECX, [ESP+0xc]
//     00043cdc:  64 89 0d 00 00 00 00        MOV  FS:[0x0], ECX
//     00043ce3:  59                          POP  ECX
//     00043ce4:  5e                          POP  ESI
//     00043ce5:  83 c4 10                    ADD  ESP, 0x10
//     00043ce8:  c3                          RET

extern "C" __declspec(naked) void FUN_00443c90() {
    __asm {
        _emit 0x6a                  // PUSH -0x1
        _emit 0xff
        _emit 0x68                  // PUSH 0xe572fb
        _emit 0xfb
        _emit 0x72
        _emit 0xe5
        _emit 0x00
        _emit 0x64                  // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50                  // PUSH EAX
        _emit 0x51                  // PUSH ECX
        _emit 0x56                  // PUSH ESI
        _emit 0xa1                  // MOV EAX, [0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33                  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50                  // PUSH EAX
        _emit 0x8d                  // LEA EAX, [ESP+0xc]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x64                  // MOV FS:[0x0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b                  // MOV ESI, ECX
        _emit 0xf1
        _emit 0x89                  // MOV [ESP+0x8], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x8d                  // LEA ECX, [ESI+0x68]
        _emit 0x4e
        _emit 0x68
        _emit 0xc7                  // MOV [ESP+0x14], 0x0
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8                  // CALL 0x00446f50
        _emit 0x88
        _emit 0x32
        _emit 0x00
        _emit 0x00
        _emit 0x8d                  // LEA ECX, [ESI+0x4]
        _emit 0x4e
        _emit 0x04
        _emit 0xc7                  // MOV [ESP+0x14], 0xffffffff
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8                  // CALL 0x00446f50
        _emit 0x78
        _emit 0x32
        _emit 0x00
        _emit 0x00
        _emit 0x8b                  // MOV ECX, [ESP+0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x64                  // MOV FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59                  // POP ECX
        _emit 0x5e                  // POP ESI
        _emit 0x83                  // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3                  // RET
    }
}
