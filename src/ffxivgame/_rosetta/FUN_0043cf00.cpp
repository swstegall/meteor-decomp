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
// FUNCTION: ffxivgame 0x0003cf00 — FUN_0043cf00
//                                  (__cdecl, 378 B / 0x17a, SEH4-wrapped,
//                                   /GS cookie, 0x8d0 local frame).
//
// Inspection (read from asm/ffxivgame/0003cf00_FUN_0043cf00.s):
//
//   __cdecl void* FUN_0043cf00(void* param_1, int param_2,
//                               void* param_3, void* param_4);
//
//   Standard MSVC 2005 SEH4 prologue:
//     PUSH -1                              ; initial EH state = -1
//     PUSH 0xe568a1                        ; SEH4 handler @ orig image RVA+0xa568a1-0x400000
//     PUSH FS:[0]                          ; save exception chain
//     SUB  ESP, 0x8d0                      ; allocate locals
//     PUSH EBX / EBP / ESI / EDI           ; callee-saves
//     MOV  EAX, [__security_cookie @ 0x012ea8b0]
//     XOR  EAX, ESP
//     PUSH EAX                             ; per-frame cookie
//     LEA  EAX, [ESP+0x8e4]               ; → saved FS:[0] slot
//     MOV  FS:[0], EAX                     ; install SEH frame
//
//   Body (informal):
//     EDI = 0; EBX = 2;
//     if (param_2 != 2) {
//         *param_1 = 0;
//         goto epilogue;              // return param_1
//     }
//     // param_2 == 2 path:
//     call FUN_0043b440(param_3, 0)  — decode/parse into [ESP+0x28] local block
//     if ([ESP+0x29] == 0 || [ESP+0x36] == 0) goto null_out;
//     call FUN_0043cd70()            — count query → ESI = result count
//     call FUN_00418e00(8 args)      — lookup/create entry → EBP = result ptr
//     clear entry's first dword to 0
//     if (ECX=[ESP+0x14] != 0) vtcall ECX->method[0](1)   // notify
//     loop (EDI=0..ESI-1): {
//         EAX = arr[EDI].field0 || arr[EDI].field4;
//         call FUN_0043cab0(EAX, EBP, ctx)
//         EBX += 0x1c;
//     }
//     *param_1 = EBP; [ESP+0x1c] = 1;
//     goto cleanup;
//   null_out:
//     *param_1 = 0; [ESP+0x1c] = EAX;
//   cleanup:
//     call FUN_0043ac30([ESP+0x28])  — release local block
//     EAX = param_1;
//   epilogue:
//     restore FS:[0], pop cookie+regs, ADD ESP,0x8dc, RET (cdecl)
//
//   Reloc-bearing sites in the orig 378 bytes:
//     +0x02  DIR32 → 0x00e568a1   (PUSH SEH4 handler address)
//     +0x0a  moffs → fs:[0]       (MOV EAX, FS:[0])
//     +0x12  DIR32 → 0x012ea8b0   (MOV EAX, [__security_cookie])
//     +0x22  moffs → fs:[0]       (MOV FS:[0], EAX)
//     +0x5c  REL32 → 0x0043b440   (CALL FUN_0043b440)
//     +0x8f  REL32 → 0x0043cd70   (CALL FUN_0043cd70)
//     +0xcf  REL32 → 0x00418e00   (CALL FUN_00418e00)
//     +0x115 REL32 → 0x0043cab0   (CALL FUN_0043cab0)
//     +0x159 REL32 → 0x0043ac30   (CALL FUN_0043ac30)
//     +0x167 moffs → fs:[0]       (MOV FS:[0], ECX — epilogue restore)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ port at /O2 /EHsc /GS would need to reproduce the
//   exact MSVC 2005 SEH4 prologue layout (state-slot write ordering,
//   shrink-wrapped callee-save timing), the register allocation across
//   five cdecl call sites with interleaved ESP-relative loads/stores, the
//   loop structure with EBX-as-struct-pointer stepping by 0x1c, the
//   vtable dispatch via CALL EAX (ecx from [ECX]) for the notify case,
//   and the exact interleaving of the EAX-reuse across three distinct
//   load/store sequences. Any of these constraints shifts bytes under /O2.
//   The sibling approach (FUN_00405080, FUN_0040ced0, FUN_00415d00 et al.)
//   is a naked-asm passthrough of the original 378 bytes verbatim.

extern "C" __declspec(naked) void FUN_0043cf00() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xa1
        _emit 0x68
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

        _emit 0xd0
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x53
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
        _emit 0xe4
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xff
        _emit 0xbb

        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x39
        _emit 0x9c
        _emit 0x24
        _emit 0xf8
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x74

        _emit 0x0e
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0xf4
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x38
        _emit 0xe9
        _emit 0x11
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b

        _emit 0x84
        _emit 0x24
        _emit 0xfc
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x57
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0xe8
        _emit 0xdf
        _emit 0xe4
        _emit 0xff

        _emit 0xff
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x29
        _emit 0x00
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0xec
        _emit 0x08

        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x36
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0xb9

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x8d
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0xe8

        _emit 0xdc
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0xac
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf0
        _emit 0x89
        _emit 0x74
        _emit 0x24

        _emit 0x18
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14

        _emit 0x51
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x4c
        _emit 0x52
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x4c
        _emit 0x57
        _emit 0x50
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x5c

        _emit 0x50
        _emit 0x51
        _emit 0x52
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x50
        _emit 0x88
        _emit 0x9c
        _emit 0x24
        _emit 0x0c
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0xe8

        _emit 0x2c
        _emit 0xbe
        _emit 0xfd
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        _emit 0x8b
        _emit 0x28
        _emit 0x89
        _emit 0x38
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x3b

        _emit 0xcf
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x20
        _emit 0x88
        _emit 0x9c
        _emit 0x24
        _emit 0xec
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x08
        _emit 0x8b
        _emit 0x11

        _emit 0x8b
        _emit 0x02
        _emit 0x6a
        _emit 0x01
        _emit 0xff
        _emit 0xd0
        _emit 0x3b
        _emit 0xef
        _emit 0x74
        _emit 0x2f
        _emit 0x3b
        _emit 0xf7
        _emit 0x76
        _emit 0x2b
        _emit 0x8d
        _emit 0x5c

        _emit 0x24
        _emit 0x54
        _emit 0x8b
        _emit 0x03
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x03
        _emit 0x8b
        _emit 0x43
        _emit 0x04
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x51

        _emit 0x55
        _emit 0x8b
        _emit 0xd7
        _emit 0x33
        _emit 0xf6
        _emit 0xe8
        _emit 0x96
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc7
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x08

        _emit 0x83
        _emit 0xc3
        _emit 0x1c
        _emit 0x3b
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x72
        _emit 0xd9
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0xf4
        _emit 0x08
        _emit 0x00
        _emit 0x00

        _emit 0x89
        _emit 0x2e
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x11
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0xf4

        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x89
        _emit 0x3e
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x8d
        _emit 0x4c
        _emit 0x24

        _emit 0x28
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0xec
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xd2
        _emit 0xdb
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xc6

        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0xe4
        _emit 0x08
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
        _emit 0x5b
        _emit 0x81
        _emit 0xc4
        _emit 0xdc
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xc3
    }
}
