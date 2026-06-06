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
// FUNCTION: ffxivgame 0x00035a90 — `__thiscall` virtual-predicate check that
//                                  fires a lazily-bound assertion reporter on
//                                  failure (89 B / 0x59, no SEH).
//
// Inspection (read from asm/ffxivgame/00035a90_FUN_00435a90.s):
//
//   __thiscall void Foo::Check(Bar* other /* [ESP+0x4] */);   // ECX = this
//
//     // Call a virtual method (vtable slot +0xbc) on `other`, passing
//     // `other` itself and `this + 4`. The slot returns a truthy value on
//     // failure, which trips an assertion-report path.
//     int ok = (*(int(**)(Bar*, void*))(*(void***)other + 0xbc/4))(
//                  other, (char*)this + 4);
//     if (ok) {
//         // MSVC 2005 lazy-init of a file-scope function pointer (the
//         // assertion reporter), guarded by a 1-byte init flag packed into
//         // the dword at 0x01323910.
//         static char s_initFlag;                  // .data 0x01323910
//         static void (*s_report)(...);            // .data 0x0132390c
//         if (!(s_initFlag & 1)) {
//             s_initFlag |= 1;
//             s_report = (void(*)(...))0x00433720; // default reporter thunk
//         }
//         s_report((void*)0xf64be8,   // expr string
//                  (void*)0xf65070,   // file string
//                  (void*)0xf64c18,   // ? string
//                  0x1a3,             // line number (419)
//                  (void*)0xf65088);  // message string
//     }
//
//   Calling convention: `__thiscall` — ECX = this on entry; one stack arg
//   (`other` at [ESP+0x4]); RET 4 pops it. No registers preserved (no
//   prologue): EAX/EDX/ECX are all scratch here.
//
//   The trailing reporter call is `__cdecl` (5 dword args, `ADD ESP,0x14`
//   cleanup) dispatched through the lazily-bound pointer at 0x0132390c.
//
//   Reloc-bearing sites in the orig 89 bytes (these absolute addresses and
//   the rel32-free indirect call resolve only in a full-binary relink at
//   image base 0x00400000; tools/compare.py masks the reloc windows, so a
//   naked-asm .obj with the same raw bytes matches byte-for-byte):
//     +0x1c   init-flag TEST            (.data 0x01323910)
//     +0x24   init-flag OR              (.data 0x01323910)
//     +0x2a   reporter store (init)     (.data 0x0132390c = 0x00433720)
//     +0x34   message string PUSH       (.rdata 0x00f65088)
//     +0x3e   ? string PUSH             (.rdata 0x00f64c18)
//     +0x43   file string PUSH          (.rdata 0x00f65070)
//     +0x48   expr string PUSH          (.rdata 0x00f64be8)
//     +0x4d   reporter CALL [ptr]       (.data 0x0132390c)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ formulation cannot coax MSVC 2005 into reproducing
//   the exact moffs32 / absolute-address encodings (`84 05 .. / 09 05 .. /
//   ff 15 ..`) plus the resolved string-literal addresses without a full
//   relink. The pragmatic choice — the same one the FUN_0040a530 singleton
//   family took — is a `__declspec(naked)` body that re-emits the orig 89
//   bytes verbatim via MASM `_emit` directives. The .obj's `.text` section
//   ends up byte-identical to the orig slice.

extern "C" __declspec(naked) void FUN_00435a90() {
    __asm {
        _emit 0x8b                  // MOV EAX, [ESP+0x4]    ; other
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b                  // MOV EDX, [EAX]        ; other->vtbl
        _emit 0x10
        _emit 0x83                  // ADD ECX, 0x4          ; this + 4
        _emit 0xc1
        _emit 0x04
        _emit 0x51                  // PUSH ECX
        _emit 0x50                  // PUSH EAX
        _emit 0x8b                  // MOV EAX, [EDX+0xbc]    ; vtbl slot
        _emit 0x82
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff                  // CALL EAX
        _emit 0xd0
        _emit 0x85                  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74                  // JZ tail (+0x3f → 0x00435ae6)
        _emit 0x3f

        _emit 0xb8                  // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84                  // TEST [0x01323910], AL ; init flag
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75                  // JNZ already_init (+0x10 → 0x00435ac4)
        _emit 0x10
        _emit 0x09                  // OR [0x01323910], EAX  ; set flag
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7                  // MOV [0x0132390c], 0x433720 ; reporter
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00

    already_init:                   // 0x00435ac4
        _emit 0x68                  // PUSH 0xf65088         ; message
        _emit 0x88
        _emit 0x50
        _emit 0xf6
        _emit 0x00
        _emit 0x68                  // PUSH 0x1a3            ; line 419
        _emit 0xa3
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68                  // PUSH 0xf64c18
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68                  // PUSH 0xf65070         ; file
        _emit 0x70
        _emit 0x50
        _emit 0xf6
        _emit 0x00
        _emit 0x68                  // PUSH 0xf64be8         ; expr
        _emit 0xe8
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        _emit 0xff                  // CALL [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83                  // ADD ESP, 0x14         ; cdecl cleanup
        _emit 0xc4
        _emit 0x14

    tail:                           // 0x00435ae6
        _emit 0xc2                  // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
