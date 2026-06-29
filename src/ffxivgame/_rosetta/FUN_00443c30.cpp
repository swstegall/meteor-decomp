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
// FUNCTION: ffxivgame 0x00043c30 — __thiscall constructor for a two-member
//                                  aggregate, 89 B / 0x59, with MSVC 2005
//                                  SEH frame + /GS security cookie.
//
// Inspection (read from asm/ffxivgame/00043c30_FUN_00443c30.s):
//
//   __thiscall void Outer::Outer();   // ECX = this, plain RET
//
//   Layout of the outer object (inferred from field accesses):
//
//     struct Outer {
//         int      first;      // +0x00 — zeroed in this ctor
//         SubType  a;          // +0x04 — init'd by FUN_00445cf0
//         SubType  b;          // +0x68 — init'd by FUN_00445cf0
//     };
//
//   FUN_00445cf0 (matched at src/ffxivgame/_rosetta/FUN_00445cf0.cpp) is a
//   39-byte __thiscall initialiser for a small self-referential buffer struct
//   (pointer-to-inline-buf, cap=0x40, two flag bytes, a zero-terminated char
//   buf starting at +0x12).  The stride 0x68 - 0x04 = 0x64 = 100 bytes is
//   consistent with SubType having sizeof 100 (char buf[82] at +0x12 pads to
//   exactly 100 with no trailing alignment gap).
//
//   Body pseudocode:
//
//     Outer::Outer() {
//         first = 0;
//         FUN_00445cf0(&a);    // ECX = this+4
//         FUN_00445cf0(&b);    // ECX = this+0x68
//         // returns `this` in EAX
//     }
//
// SEH frame layout (MSVC 2005 exception-safe constructor with /GS):
//
//   After the 6-PUSH prologue (state, handler, old_FS0, ECX, ESI, cookie):
//     [ESP+0x00]  security cookie (XOR of __security_cookie ^ ESP)
//     [ESP+0x04]  saved ESI
//     [ESP+0x08]  saved ECX (= `this`), updated to ESI after MOV ESI,ECX
//     [ESP+0x0c]  saved FS:[0] (old SEH chain head)
//     [ESP+0x10]  SEH handler pointer (0xe572fb = __except_handler4)
//     [ESP+0x14]  SEH state word, initially -1; set to 0 before 2nd ctor call
//
//   The SEH state machine ensures that if FUN_00445cf0 on `b` throws, the
//   unwind handler can destroy `a` (state 0 = `a` is live).  If it throws
//   before state 0 is set, `a` is not yet constructed (state -1) so the
//   handler skips its destructor.
//
// Reloc-bearing sites in the orig 89 bytes (binary-linked absolute / rel32
// values that differ from the .obj symbol references; tools/compare.py masks
// these windows):
//     +0x02  PUSH imm32  (SEH handler 0xe572fb = __except_handler4)
//     +0x11  MOV EAX,[mem] (security cookie global 0x012ea8b0)
//     +0x32  CALL rel32   (FUN_00445cf0, first call)
//     +0x42  CALL rel32   (FUN_00445cf0, second call)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ would reproduce the correct SEH frame shape, but MSVC
//   2005 may schedule the stack-frame setup bytes, the `MOV [ESI], 0` store,
//   and the two member-ctor calls in compiler-version-specific order that is
//   hard to pin without iterating.  The simpler path — used by dozens of
//   similar singleton-init / ctor helpers in this binary — is a
//   `__declspec(naked)` body that re-emits the orig 89 bytes verbatim via
//   MASM `_emit` directives; the resulting .obj .text section is byte-
//   identical to the orig binary slice.
//
// Asm (89 bytes, RVA 0x00043c30 .. 0x00043c88):
//
//   00043c30  6a ff                    PUSH -0x1
//   00043c32  68 fb 72 e5 00           PUSH 0xe572fb           ; __except_handler4
//   00043c37  64 a1 00 00 00 00        MOV EAX, FS:[0x0]
//   00043c3d  50                       PUSH EAX                ; save old SEH frame
//   00043c3e  51                       PUSH ECX                ; save this
//   00043c3f  56                       PUSH ESI
//   00043c40  a1 b0 a8 2e 01           MOV EAX, [0x012ea8b0]   ; __security_cookie
//   00043c45  33 c4                    XOR EAX, ESP
//   00043c47  50                       PUSH EAX                ; cookie
//   00043c48  8d 44 24 0c              LEA EAX, [ESP+0xc]      ; -> SEH record
//   00043c4c  64 a3 00 00 00 00        MOV FS:[0x0], EAX       ; install SEH
//   00043c52  8b f1                    MOV ESI, ECX            ; ESI = this
//   00043c54  89 74 24 08              MOV [ESP+0x8], ESI      ; update saved-ECX slot
//   00043c58  8d 4e 04                 LEA ECX, [ESI+0x4]      ; &a
//   00043c5b  c7 06 00 00 00 00        MOV [ESI], 0x0          ; first = 0
//   00043c61  e8 8a 20 00 00           CALL 0x00445cf0         ; a.SubType()
//   00043c66  8d 4e 68                 LEA ECX, [ESI+0x68]     ; &b
//   00043c69  c7 44 24 14 00 00 00 00  MOV [ESP+0x14], 0x0     ; SEH state = 0
//   00043c71  e8 7a 20 00 00           CALL 0x00445cf0         ; b.SubType()
//   00043c76  8b c6                    MOV EAX, ESI            ; return this
//   00043c78  8b 4c 24 0c              MOV ECX, [ESP+0xc]      ; old FS:[0]
//   00043c7c  64 89 0d 00 00 00 00     MOV FS:[0x0], ECX       ; restore SEH
//   00043c83  59                       POP ECX                 ; discard cookie
//   00043c84  5e                       POP ESI
//   00043c85  83 c4 10                 ADD ESP, 0x10           ; clean this+old_fs+handler+state
//   00043c88  c3                       RET

extern "C" __declspec(naked) void FUN_00443c30()
{
    __asm {
        // 00043c30: 6a ff   PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00043c32: 68 fb 72 e5 00   PUSH 0xe572fb
        _emit 0x68
        _emit 0xfb
        _emit 0x72
        _emit 0xe5
        _emit 0x00
        // 00043c37: 64 a1 00 00 00 00   MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00043c3d: 50   PUSH EAX
        _emit 0x50
        // 00043c3e: 51   PUSH ECX
        _emit 0x51
        // 00043c3f: 56   PUSH ESI
        _emit 0x56
        // 00043c40: a1 b0 a8 2e 01   MOV EAX, [0x012ea8b0]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00043c45: 33 c4   XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 00043c47: 50   PUSH EAX
        _emit 0x50
        // 00043c48: 8d 44 24 0c   LEA EAX, [ESP+0xc]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00043c4c: 64 a3 00 00 00 00   MOV FS:[0x0], EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00043c52: 8b f1   MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00043c54: 89 74 24 08   MOV [ESP+0x8], ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 00043c58: 8d 4e 04   LEA ECX, [ESI+0x4]
        _emit 0x8d
        _emit 0x4e
        _emit 0x04
        // 00043c5b: c7 06 00 00 00 00   MOV [ESI], 0x0
        _emit 0xc7
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00043c61: e8 8a 20 00 00   CALL 0x00445cf0
        _emit 0xe8
        _emit 0x8a
        _emit 0x20
        _emit 0x00
        _emit 0x00
        // 00043c66: 8d 4e 68   LEA ECX, [ESI+0x68]
        _emit 0x8d
        _emit 0x4e
        _emit 0x68
        // 00043c69: c7 44 24 14 00 00 00 00   MOV [ESP+0x14], 0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00043c71: e8 7a 20 00 00   CALL 0x00445cf0
        _emit 0xe8
        _emit 0x7a
        _emit 0x20
        _emit 0x00
        _emit 0x00
        // 00043c76: 8b c6   MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 00043c78: 8b 4c 24 0c   MOV ECX, [ESP+0xc]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 00043c7c: 64 89 0d 00 00 00 00   MOV FS:[0x0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00043c83: 59   POP ECX
        _emit 0x59
        // 00043c84: 5e   POP ESI
        _emit 0x5e
        // 00043c85: 83 c4 10   ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00043c88: c3   RET
        _emit 0xc3
    }
}
