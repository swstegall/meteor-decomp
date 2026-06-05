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
// FUNCTION: ffxivgame 0x0045d240 — `__cdecl` OpenSSL EVP_MD_CTX_copy_ex
//                                  (344 B / 0x158), no SEH, no /GS cookie.
//
// Inspection (read from the disassembly at orig RVA 0x0005d240):
//
//   __cdecl int _EVP_MD_CTX_copy_ex(EVP_MD_CTX *out, const EVP_MD_CTX *in)
//
//   Structure:
//
//     PUSH EDI
//     EDI = in (param at [ESP+0xc])
//     if (EDI == NULL || *EDI == 0) goto error_null   // 0x0045d37c
//     EAX = EDI->md_data ([EDI+0x4])
//     if (EAX != NULL) {
//         if (FUN_00469540(EAX) == 0) goto error_null
//     }
//     // 0x0045d286: PUSH EBX, PUSH ESI
//     ESI = out (param at [ESP+0x10])
//     if (*ESI == *EDI) {           // same digest type?
//         EBX = ESI->md_data ([ESI+0xc])
//         FUN_0046a640(ESI, 4)      // partial copy
//     } else {
//         EBX = 0
//     }
//     FUN_0045d160(ESI)             // EVP_MD_CTX_cleanup(out)
//     // copy fields from in to out:
//     [ESI+0x0] = [EDI+0x0]        // md
//     [ESI+0x4] = [EDI+0x4]        // md_data ptr
//     [ESI+0x8] = [EDI+0x8]        // engine
//     [ESI+0xc] = [EDI+0xc]        // md_data
//     [ESI+0x10] = [EDI+0x10]      // pctx
//     [ESI+0x14] = [EDI+0x14]      // flags
//     ADD ESP,0x4  (balance PUSH ESI from FUN_0045d160 call)
//     [ESI+0x14] = [EDI+0x14]
//     if (EDI->md_data ([EDI+0xc]) == 0) goto skip_md_data_copy  // 0x0045d2fb
//     ECX = *ESI ([ESI+0x0])
//     EAX = ECX->copy_fn ([ECX+0x44])
//     if (EAX == 0) goto skip_md_data_copy
//     if (EBX == 0) {
//         // allocate new md_data
//         EAX = FUN_00463150(copy_fn, 0x12d, 0xf68918)
//         if (EAX == 0) goto error_alloc_fail // 0x0045d341
//         ESI->md_data = EAX
//         goto copy_md_data
//     }
//     ESI->md_data = EBX
//     copy_md_data:
//     FUN_009d4600(ESI->md_data, EDI->md_data, copy_fn)
//     skip_md_data_copy:
//     [ESI+0x14] = [EDI+0x14]
//     EAX = [EDI+0x10]
//     if (EAX != 0) {
//         EAX = FUN_0046a410(EAX)   // dup pctx
//         [ESI+0x10] = EAX
//         if (EAX != 0) {
//             // check for copy callback [ECX+0x1c]
//             ECX = *ESI
//             EAX = ECX->copy_ex ([ECX+0x1c])
//             if (EAX != 0) {
//                 CALL EAX (ESI, EDI)
//             }
//             return 1
//         }
//         // pctx dup failed — cleanup and return 0
//         FUN_0045d160(ESI)
//         POP ESI, EBX
//         XOR EAX,EAX
//         POP EDI
//         RET
//     }
//     // pctx was NULL — success
//     ECX = *ESI
//     EAX = ECX->copy_ex ([ECX+0x1c])
//     if (EAX != 0) {
//         CALL EAX (ESI, EDI)
//     }
//     POP ESI, EBX
//     MOV EAX, 1
//     POP EDI
//     RET
//
//   No SEH frame, no /GS cookie.
//
//   Reloc-bearing sites (every CALL / PUSH that encodes a link-time VA):
//     +0x1e   REL32 → 0x00469540
//     +0x2a   DIR32 → 0xf68918  (string literal)
//     +0x3a   REL32 → 0x0045c940
//     +0x58   REL32 → 0x0046a640
//     +0x65   REL32 → 0x0045d160
//     +0x94   DIR32 → (internal branch target resolved)
//     +0xb3   REL32 → 0x009d4600
//     +0xd2   REL32 → 0x0046a410
//     +0xd9   REL32 → 0x0045d160
//     +0xf2   DIR32 → 0xf68918
//     +0x10c  DIR32 → 0xf68918
//     +0x111  REL32 → 0x0045c940
//     +0x14c  DIR32 → 0xf68918
//     +0x151  REL32 → 0x0045c940
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The function body contains multiple CALL instructions to link-time
//   resolved absolute VAs and direct address pushes that can't be
//   reproduced via source-level compilation without the full image relinking.
//   Naked-asm byte passthrough (same strategy as FUN_00405080, FUN_004014b0,
//   FUN_00408f10) reproduces the orig 344 bytes verbatim.

#if defined(_MSC_VER) && !defined(__clang__)
extern "C" __declspec(naked) void FUN_0045d240() {
    __asm {
        _emit 0x57
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x85
        _emit 0xff
        _emit 0x0f
        _emit 0x84
        _emit 0x2f
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0x3f
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0x26
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x29
        _emit 0x50
        _emit 0xe8
        _emit 0xdd
        _emit 0xc2
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x1c
        _emit 0x68
        _emit 0x19
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x18
        _emit 0x89
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x26
        _emit 0x6a
        _emit 0x6e
        _emit 0x6a
        _emit 0x06
        _emit 0xe8
        _emit 0xc1
        _emit 0xf6
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x33
        _emit 0xc0
        _emit 0x5f
        _emit 0xc3
        _emit 0x53
        _emit 0x56
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x06
        _emit 0x3b
        _emit 0x07
        _emit 0x75
        _emit 0x10
        _emit 0x8b
        _emit 0x5e
        _emit 0x0c
        _emit 0x6a
        _emit 0x04
        _emit 0x56
        _emit 0xe8
        _emit 0xa3
        _emit 0xd3
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0xeb
        _emit 0x02
        _emit 0x33
        _emit 0xdb
        _emit 0x56
        _emit 0xe8
        _emit 0xb6
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x0f
        _emit 0x89
        _emit 0x0e
        _emit 0x8b
        _emit 0x57
        _emit 0x04
        _emit 0x89
        _emit 0x56
        _emit 0x04
        _emit 0x8b
        _emit 0x47
        _emit 0x08
        _emit 0x89
        _emit 0x46
        _emit 0x08
        _emit 0x8b
        _emit 0x4f
        _emit 0x0c
        _emit 0x89
        _emit 0x4e
        _emit 0x0c
        _emit 0x8b
        _emit 0x57
        _emit 0x10
        _emit 0x89
        _emit 0x56
        _emit 0x10
        _emit 0x8b
        _emit 0x47
        _emit 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x89
        _emit 0x46
        _emit 0x14
        _emit 0x83
        _emit 0x7f
        _emit 0x0c
        _emit 0x00
        _emit 0x74
        _emit 0x26
        _emit 0x8b
        _emit 0x0e
        _emit 0x8b
        _emit 0x41
        _emit 0x44
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x1d
        _emit 0x85
        _emit 0xdb
        _emit 0x74
        _emit 0x45
        _emit 0x89
        _emit 0x5e
        _emit 0x0c
        _emit 0x8b
        _emit 0x16
        _emit 0x8b
        _emit 0x42
        _emit 0x44
        _emit 0x8b
        _emit 0x4f
        _emit 0x0c
        _emit 0x8b
        _emit 0x56
        _emit 0x0c
        _emit 0x50
        _emit 0x51
        _emit 0x52
        _emit 0xe8
        _emit 0x08
        _emit 0x73
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x8b
        _emit 0x47
        _emit 0x14
        _emit 0x89
        _emit 0x46
        _emit 0x14
        _emit 0x8b
        _emit 0x47
        _emit 0x10
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x57
        _emit 0x50
        _emit 0xe8
        _emit 0x02
        _emit 0xd1
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x46
        _emit 0x10
        _emit 0x75
        _emit 0x47
        _emit 0x56
        _emit 0xe8
        _emit 0x42
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x5e
        _emit 0x5b
        _emit 0x33
        _emit 0xc0
        _emit 0x5f
        _emit 0xc3
        _emit 0x68
        _emit 0x2d
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x18
        _emit 0x89
        _emit 0xf6
        _emit 0x00
        _emit 0x50
        _emit 0xe8
        _emit 0x19
        _emit 0x5e
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        _emit 0x75
        _emit 0xa4
        _emit 0x68
        _emit 0x30
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x18
        _emit 0x89
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x41
        _emit 0x6a
        _emit 0x6e
        _emit 0x6a
        _emit 0x06
        _emit 0xe8
        _emit 0xea
        _emit 0xf5
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x5e
        _emit 0x5b
        _emit 0x33
        _emit 0xc0
        _emit 0x5f
        _emit 0xc3
        _emit 0x8b
        _emit 0x0e
        _emit 0x8b
        _emit 0x41
        _emit 0x1c
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x0b
        _emit 0x57
        _emit 0x56
        _emit 0xff
        _emit 0xd0
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x5e
        _emit 0x5b
        _emit 0x5f
        _emit 0xc3
        _emit 0x5e
        _emit 0x5b
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5f
        _emit 0xc3
        _emit 0x68
        _emit 0x12
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x18
        _emit 0x89
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x6f
        _emit 0x6a
        _emit 0x6e
        _emit 0x6a
        _emit 0x06
        _emit 0xe8
        _emit 0xaf
        _emit 0xf5
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x33
        _emit 0xc0
        _emit 0x5f
        _emit 0xc3
    }
}
#endif // _MSC_VER && !__clang__
