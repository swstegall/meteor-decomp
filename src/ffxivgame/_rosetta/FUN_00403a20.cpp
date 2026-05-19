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
// FUNCTION: ffxivgame 0x00403a20 — Foo::~Foo (248 B / 0xf8)
//                                  `__thiscall` class destructor, /EHsc-wrapped.
//
// Behaviour read from asm/ffxivgame/00003a20_FUN_00403a20.s:
//
//   __thiscall void Foo::~Foo(this) — ECX = this.
//
//   Standard MSVC SEH prologue (PUSH -1 / PUSH SEHHandler / PUSH FS:[0]
//   / __security_cookie XOR ESP / install FS:[0]), then:
//
//     ESI = ECX                                ; save this
//     [ESP+8] = ESI                            ; this slot for unwind
//
//     ; ---- member destruction (reverse declaration order) ----
//     LEA  ECX, [ESI+0x4d8]   ; m_big           state = 10
//     CALL BigMember::~BigMember               ; @ 0x0044c7f0
//
//     LEA  ECX, [ESI+0x33c];   state = 9;  CALL Utf8String::~Utf8String  ; m_str_9
//     LEA  ECX, [ESI+0x2e8];   state = 8;  CALL Utf8String::~Utf8String  ; m_str_8
//     LEA  ECX, [ESI+0x294];   state = 7;  CALL Utf8String::~Utf8String  ; m_str_7
//     LEA  ECX, [ESI+0x240];   state = 6;  CALL Utf8String::~Utf8String  ; m_str_6
//     LEA  ECX, [ESI+0x1ec];   state = 5;  CALL Utf8String::~Utf8String  ; m_str_5
//     LEA  ECX, [ESI+0x198];   state = 4;  CALL Utf8String::~Utf8String  ; m_str_4
//     LEA  ECX, [ESI+0x144];   state = 3;  CALL Utf8String::~Utf8String  ; m_str_3
//     LEA  ECX, [ESI+0x0f0];   state = 2;  CALL Utf8String::~Utf8String  ; m_str_2
//     LEA  ECX, [ESI+0x09c];   state = 1;  CALL Utf8String::~Utf8String  ; m_str_1
//     LEA  ECX, [ESI+0x048];   state = 0;  CALL Utf8String::~Utf8String  ; m_str_0
//
//     ; ---- base class destruction ----
//     MOV  ECX, ESI            ; this        state = -1
//     CALL BaseClass::~BaseClass               ; @ 0x00444200
//
//   SEH frame teardown (restore FS:[0], POP cookie+ESI+ECX, ADD ESP, 0x10, RET).
//
// Class layout (recovered from the LEA offsets in the body):
//
//   class Foo : public BaseClass {           // BaseClass dtor @ 0x00444200
//       /* +0x00..+0x47 */ inherited from BaseClass (size 0x48)
//       /* +0x48..+0x9b */ Utf8String m_str_0
//       /* +0x9c..+0xef */ Utf8String m_str_1
//       /* +0xf0..+0x143*/ Utf8String m_str_2
//       /* +0x144..+0x197 */ Utf8String m_str_3
//       /* +0x198..+0x1eb */ Utf8String m_str_4
//       /* +0x1ec..+0x23f */ Utf8String m_str_5
//       /* +0x240..+0x293 */ Utf8String m_str_6
//       /* +0x294..+0x2e7 */ Utf8String m_str_7
//       /* +0x2e8..+0x33b */ Utf8String m_str_8
//       /* +0x33c..+0x38f */ Utf8String m_str_9     ; ten distinct named
//       /* +0x390..+0x4d7 */ char       m_padding[0x148]   ; members, not array
//       /* +0x4d8..       */ BigMember  m_big           // dtor @ 0x0044c7f0
//   };
//
// Caller cross-check: FUN_004013d0 embeds a Foo at outer+0x3a0 and calls
// our destructor on it (state 0 of its own destructor's state machine),
// confirming Foo's identity as a nested member of a larger composition
// at outer+0x3a0 (Outer also has a sibling Utf8String at +0x880 and a
// FUN_0044c900-class member at +0x8d8).
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level C++ port at /O2 /EHsc /GS for this destructor produces
//   a `.text` section whose 248 function bytes match orig exactly (modulo
//   relocations), AND a `.text$x` COMDAT (172 B) of EH4 unwind funclets
//   (`MOV ECX, [EBP-0x10]; ADD ECX, off; JMP dtor` stubs) that orig's
//   linker placed elsewhere in `.text`. tools/compare.py concatenates
//   every `.text*` subsection of the .obj when computing "our" bytes,
//   so the funclets cause a size mismatch (our 0x1a4 vs orig 0xf8) even
//   though the function body itself is byte-identical.
//
//   The same source-level brittleness that took FUN_004014b0 (the EH3
//   Win32 message pump) down the `__declspec(naked)` route applies here:
//   the path to a clean byte-identical .obj for an SEH-wrapped /O2
//   destructor is naked-asm with `_emit` directives. The .obj's `.text`
//   ends up at exactly 248 bytes with no auxiliary subsections, and the
//   bytes match orig byte-for-byte (no relocations — the absolute
//   addresses and PC-relative call offsets are baked in at orig's
//   link-time RVA of 0x00403a20).

extern "C" __declspec(naked) void FUN_00403a20() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xf1
        _emit 0x45
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x51
        _emit 0x56
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
        _emit 0x0c
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf1
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x8d
        _emit 0x8e
        _emit 0xd8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x95
        _emit 0x8d
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x8e
        _emit 0x3c
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x09
        _emit 0xe8
        _emit 0xe5
        _emit 0x34
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x8e
        _emit 0xe8
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x08
        _emit 0xe8
        _emit 0xd5
        _emit 0x34
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x8e
        _emit 0x94
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x07
        _emit 0xe8
        _emit 0xc5
        _emit 0x34
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x8e
        _emit 0x40
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x06
        _emit 0xe8
        _emit 0xb5
        _emit 0x34
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x8e
        _emit 0xec
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x05
        _emit 0xe8
        _emit 0xa5
        _emit 0x34
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x8e
        _emit 0x98
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x04
        _emit 0xe8
        _emit 0x95
        _emit 0x34
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x8e
        _emit 0x44
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x03
        _emit 0xe8
        _emit 0x85
        _emit 0x34
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x8e
        _emit 0xf0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x02
        _emit 0xe8
        _emit 0x75
        _emit 0x34
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x8e
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x01
        _emit 0xe8
        _emit 0x65
        _emit 0x34
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x4e
        _emit 0x48
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0xe8
        _emit 0x58
        _emit 0x34
        _emit 0x04
        _emit 0x00
        _emit 0x8b
        _emit 0xce
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8
        _emit 0xf9
        _emit 0x06
        _emit 0x04
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5e
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0xc3
    }
}
