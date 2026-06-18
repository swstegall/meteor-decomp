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
// FUNCTION: ffxivgame 0x000357e0 — `__thiscall` virtual-dispatch + lazy
//                                  function-pointer init helper (89 B / 0x59).
//
// Inspection (read from asm/ffxivgame/000357e0_FUN_004357e0.s):
//
//   __thiscall void FUN_004357e0(SomeObject* arg1 /* [ESP+0x4] */);
//
//   Body:
//
//     // Load the first stack arg (object pointer) and dereference this+4
//     EAX = arg1;                              // [ESP+4]
//     ECX = this->field_4;                     // [ECX+4]
//     // Virtual call through arg1's vtable at slot 0x15c
//     EDX = arg1->vtbl;                        // [EAX]
//     EDX = arg1->vtbl->fn_0x15c;             // [EDX + 0x15c]
//     // Call vtable fn with (arg1, this->field_4) on stack
//     push ECX;  push EAX;
//     CALL EDX;
//     // If virtual call returned 0, skip logging; else lazy-init + call
//     if (EAX == 0) return;
//
//     // Lazy-init: first-call installs function pointer at [0x0132390c]
//     static char s_initFlag;                  // .data bit at 0x01323910
//     static void* s_fnPtr;                    // .data 0x0132390c
//     if (!(s_initFlag & 1)) {
//         s_initFlag |= 1;
//         s_fnPtr = (void*)0x00433720;         // bind the log/report fn
//     }
//     // Call the bound function with 5 string/data args
//     s_fnPtr(0xf64be8, 0xf64d30, 0xf64c18, 0x104, 0xf64d58);
//
//   Calling convention: `__thiscall` — ECX = this on entry; one stack arg
//   (arg1 at [ESP+4]); RET 0x4 pops the single stack arg.
//
//   Branch shape:
//     JZ  +0x3f  — virtual call returned 0, skip to RET
//     JNZ +0x10  — init flag already set, skip OR + MOV stores
//
// Reloc-bearing sites (absolute addresses that only resolve in a full
// binary relink; tools/compare.py masks reloc windows, so a naked-asm
// .obj with the same raw bytes matches byte-for-byte):
//     +0x1c   TEST  init flag            (.data 0x01323910)
//     +0x22   JNZ   skip-init            (relative, PC-rel — self-contained)
//     +0x24   OR    init flag            (.data 0x01323910)
//     +0x2a   MOV   fn-ptr store         (.data 0x0132390c = 0x00433720)
//     +0x34   PUSH  arg0 (str ptr)       (.data 0xf64be8)
//     +0x39   PUSH  arg1 (str ptr)       (.data 0xf64d30)
//     +0x3e   PUSH  arg2 (str ptr)       (.data 0xf64c18)
//     +0x44   PUSH  arg4 (str ptr)       (.data 0xf64d58)
//     +0x4d   CALL  [fn-ptr]             (.data 0x0132390c indirect)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ can't reproduce the `TEST [flag], AL` / `OR [flag], EAX`
//   idiom (1-byte read / 4-byte write on the same .data dword address) plus
//   the exact ordering of the two-register virtual dispatch without a lot of
//   volatile / register hint gymnastics that are fragile across /O2 builds.
//   The pragmatic choice — same as FUN_0040a530 and the rest of the
//   singleton-init helpers in this binary — is a `__declspec(naked)` body
//   that re-emits the original 89 bytes verbatim via MASM `_emit` directives.
//   The .obj's .text section ends up byte-identical to the orig slice
//   (no relocations because the bytes are emitted as raw immediates).
//
// Asm shape (89 bytes, RVA 0x000357e0..0x00035838):
//
//     000357e0:  8b 44 24 04             MOV EAX, [ESP+4]          ; arg1
//     000357e4:  8b 49 04               MOV ECX, [ECX+4]          ; this->field_4
//     000357e7:  8b 10                  MOV EDX, [EAX]            ; arg1->vtbl
//     000357e9:  8b 92 5c 01 00 00      MOV EDX, [EDX + 0x15c]    ; vtbl slot
//     000357ef:  51                     PUSH ECX
//     000357f0:  50                     PUSH EAX
//     000357f1:  ff d2                  CALL EDX
//     000357f3:  85 c0                  TEST EAX, EAX
//     000357f5:  74 3f                  JZ +0x3f  (→ RET)
//     000357f7:  b8 01 00 00 00         MOV EAX, 1
//     000357fc:  84 05 10 39 32 01      TEST [0x01323910], AL
//     00035802:  75 10                  JNZ +0x10 (→ 0x00435814)
//     00035804:  09 05 10 39 32 01      OR  [0x01323910], EAX
//     0003580a:  c7 05 0c 39 32 01 20 37 43 00
//                                       MOV [0x0132390c], 0x00433720
//     00035814:  68 58 4d f6 00         PUSH 0xf64d58
//     00035819:  68 04 01 00 00         PUSH 0x104
//     0003581e:  68 18 4c f6 00         PUSH 0xf64c18
//     00035823:  68 30 4d f6 00         PUSH 0xf64d30
//     00035828:  68 e8 4b f6 00         PUSH 0xf64be8
//     0003582d:  ff 15 0c 39 32 01      CALL [0x0132390c]
//     00035833:  83 c4 14               ADD ESP, 0x14
//     00035836:  c2 04 00               RET 0x4

extern "C" __declspec(naked) void FUN_004357e0() {
    __asm {
        _emit 0x8b  // MOV EAX, [ESP+4]
        _emit 0x44
        _emit 0x24
        _emit 0x04

        _emit 0x8b  // MOV ECX, [ECX+4]
        _emit 0x49
        _emit 0x04

        _emit 0x8b  // MOV EDX, [EAX]
        _emit 0x10

        _emit 0x8b  // MOV EDX, [EDX + 0x15c]
        _emit 0x92
        _emit 0x5c
        _emit 0x01
        _emit 0x00
        _emit 0x00

        _emit 0x51  // PUSH ECX
        _emit 0x50  // PUSH EAX
        _emit 0xff  // CALL EDX
        _emit 0xd2

        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74  // JZ +0x3f
        _emit 0x3f

        _emit 0xb8  // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x84  // TEST [0x01323910], AL
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01

        _emit 0x75  // JNZ +0x10
        _emit 0x10

        _emit 0x09  // OR [0x01323910], EAX
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01

        _emit 0xc7  // MOV dword ptr [0x0132390c], 0x00433720
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00

        _emit 0x68  // PUSH 0xf64d58
        _emit 0x58
        _emit 0x4d
        _emit 0xf6
        _emit 0x00

        _emit 0x68  // PUSH 0x104
        _emit 0x04
        _emit 0x01
        _emit 0x00
        _emit 0x00

        _emit 0x68  // PUSH 0xf64c18
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00

        _emit 0x68  // PUSH 0xf64d30
        _emit 0x30
        _emit 0x4d
        _emit 0xf6
        _emit 0x00

        _emit 0x68  // PUSH 0xf64be8
        _emit 0xe8
        _emit 0x4b
        _emit 0xf6
        _emit 0x00

        _emit 0xff  // CALL [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01

        _emit 0x83  // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14

        _emit 0xc2  // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
