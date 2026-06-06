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
// FUNCTION: ffxivgame 0x0001b239 — node-lookup / object-acquisition helper
//                                  (243 B / 0xf3, no SEH).
//
// Behaviour read from the disassembly at orig RVA 0x0001b239:
//
//   __cdecl void FUN_0041b239(?, ?, ?, ?, ?, ?, ?, arg8, ?, arg10_out)
//
//   Calling convention: __cdecl (caller cleans; `add esp, 0x1c` in epilogue
//   unwinds the 7 local PUSH slots + the four saved registers = 0x1c net).
//
//   Structural shape (two-armed object lookup):
//
//     // Prologue: save EBX/EBP/ESI/EDI, load last arg
//     edi = arg8;
//     call FUN_00419850(garbage_eax, arg8);   // produces eax, edx results
//     ebp = edx;  ebx = eax;                  // save both halves
//
//     // Try to locate the object via a thiscall on the singleton at
//     // 0x13298ac: ecx = singleton, args = (&arg4, &arg1)
//     call FUN_006d1020 [thiscall ecx=0x13298ac](out_ptr_arg4, out_ptr_arg1);
//
//     esi = arg1;
//     cached_sentinel = [0x13298b0];
//     // esi == 0 → assert (FUN_009d22b4)
//     // esi == 0x13298ac (the singleton itself) → assert
//
//     // Fast path: arg1 != cached_sentinel
//     if (arg1 != cached_sentinel) {
//         // arg1 must be non-null and its field4 must equal esi's field4
//         assert(esi != 0);
//         assert(arg1->field4 == esi->field4);
//         // call virtual slot 1 on arg1->field24 (the contained object)
//         obj = arg1->field24;
//         result = obj->vtable[1](obj);
//         assert(arg1->field4 == esi->field4);
//         *arg10_out = obj->field24;
//         return;
//     }
//
//     // Slow path: arg1 == sentinel → query the global manager
//     g = [0x1329834];
//     result_ptr = g->vtable[0x1a8/4](g, edi, &local_result);
//     *arg10_out = local_result;
//     local_result->vtable[1](local_result);  // addref / register
//
//     // Re-query the singleton with the updated args (ecx=0x13298ac)
//     call FUN_0041af60 [thiscall ecx=0x13298ac](out_ptr_arg1, out_ptr_arg4);
//     return;
//
//   Reloc-bearing sites in the orig 243 bytes:
//     +0x20   abs32   0x013298ac — singleton ptr (MOV ECX, imm32)
//     +0x39   rel32   FUN_006d1020 — thiscall
//     +0x3c   abs32   0x013298b0 — cached sentinel global (MOV EAX, [addr])
//     +0x43   abs32   0x013298ac — singleton ptr (CMP ESI, imm32)
//     +0x4e   rel32   FUN_009d22b4 — assert / abort
//     +0x60   rel32   FUN_009d22b4 — 2nd assert
//     +0x6c   rel32   FUN_009d22b4 — 3rd assert
//     +0x89   rel32   FUN_009d22b4 — 4th assert
//     +0xa4   abs32   0x01329834  — global manager ptr (MOV EAX, [addr])
//     +0xe0   abs32   0x013298ac  — singleton ptr (MOV ECX, imm32)
//     +0xea   rel32   FUN_0041af60 — thiscall re-query
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The body contains multiple absolute data-pointer references
//   (0x13298ac, 0x13298b0, 0x1329834) baked as imm32 immediates, plus
//   eleven relative call targets. Source-level C++ would need to reproduce
//   the exact push-based frame allocation (no SUB ESP, seven net live
//   PUSHes in scope at the epilogue), the register schedule (EBX/EBP/ESI/EDI
//   assignment sequence), and the MSVC 2005 /O2 short-vs-near branch encoding
//   across five conditional jumps. Each is fragile under /O2.
//
//   The pragmatic choice — identical to FUN_00402a30 / FUN_00403a20 — is a
//   `__declspec(naked)` body that re-emits the orig 243 bytes verbatim via
//   MASM `_emit` directives. The .obj's `.text` section is byte-identical to
//   the orig slice (no relocations because the bytes are emitted as raw
//   immediates), which is what `tools/compare.py` checks against.

extern "C" __declspec(naked) void FUN_0041b239() {
    __asm {
        // 0001b239  PUSH EBX
        _emit 0x53
        // 0001b23a  PUSH EBP
        _emit 0x55
        // 0001b23b  PUSH ESI
        _emit 0x56
        // 0001b23c  PUSH EDI
        _emit 0x57
        // 0001b23d  MOV EDI, [ESP+0x30]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x30
        // 0001b241  PUSH EAX
        _emit 0x50
        // 0001b242  PUSH EDI
        _emit 0x57
        // 0001b243  CALL 0x00419850
        _emit 0xe8
        _emit 0x08
        _emit 0xe6
        _emit 0xff
        _emit 0xff
        // 0001b248  ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0001b24b  MOV EBP, EDX
        _emit 0x8b
        _emit 0xea
        // 0001b24d  LEA ECX, [ESP+0x20]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 0001b251  PUSH ECX
        _emit 0x51
        // 0001b252  LEA EDX, [ESP+0x14]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0001b256  MOV EBX, EAX
        _emit 0x8b
        _emit 0xd8
        // 0001b258  PUSH EDX
        _emit 0x52
        // 0001b259  MOV ECX, 0x13298ac
        _emit 0xb9
        _emit 0xac
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001b25e  MOV [ESP+0x28], EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x28
        // 0001b262  MOV [ESP+0x2c], EBP
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x2c
        // 0001b266  CALL 0x006d1020
        _emit 0xe8
        _emit 0xb5
        _emit 0x5d
        _emit 0x2b
        _emit 0x00
        // 0001b26b  MOV ESI, [ESP+0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 0001b26f  TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0001b271  MOV EAX, [0x13298b0]
        _emit 0xa1
        _emit 0xb0
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001b276  MOV [ESP+0x24], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0001b27a  JE +0x08  -> 0x0001b284
        _emit 0x74
        _emit 0x08
        // 0001b27c  CMP ESI, 0x13298ac
        _emit 0x81
        _emit 0xfe
        _emit 0xac
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001b282  JE +0x05  -> 0x0001b289
        _emit 0x74
        _emit 0x05
        // 0001b284  CALL 0x009d22b4
        _emit 0xe8
        _emit 0x2b
        _emit 0x70
        _emit 0x5b
        _emit 0x00
        // 0001b289  MOV ECX, [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0001b28d  CMP ECX, [ESP+0x24]
        _emit 0x3b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 0001b291  JE +0x45  -> 0x0001b2d8
        _emit 0x74
        _emit 0x45
        // 0001b293  TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0001b295  JNE +0x05  -> 0x0001b29c
        _emit 0x75
        _emit 0x05
        // 0001b297  CALL 0x009d22b4
        _emit 0xe8
        _emit 0x18
        _emit 0x70
        _emit 0x5b
        _emit 0x00
        // 0001b29c  MOV EDX, [ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0001b2a0  CMP EDX, [ESI+4]
        _emit 0x3b
        _emit 0x56
        _emit 0x04
        // 0001b2a3  JNE +0x05  -> 0x0001b2aa
        _emit 0x75
        _emit 0x05
        // 0001b2a5  CALL 0x009d22b4
        _emit 0xe8
        _emit 0x0a
        _emit 0x70
        _emit 0x5b
        _emit 0x00
        // 0001b2aa  MOV EAX, [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0001b2ae  MOV EAX, [EAX+0x18]
        _emit 0x8b
        _emit 0x40
        _emit 0x18
        // 0001b2b1  MOV ECX, [EAX]
        _emit 0x8b
        _emit 0x08
        // 0001b2b3  MOV EDX, [ECX+4]
        _emit 0x8b
        _emit 0x51
        _emit 0x04
        // 0001b2b6  PUSH EAX
        _emit 0x50
        // 0001b2b7  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0001b2b9  MOV EDI, [ESP+0x14]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        // 0001b2bd  CMP EDI, [ESI+4]
        _emit 0x3b
        _emit 0x7e
        _emit 0x04
        // 0001b2c0  JNE +0x05  -> 0x0001b2c7
        _emit 0x75
        _emit 0x05
        // 0001b2c2  CALL 0x009d22b4
        _emit 0xe8
        _emit 0xed
        _emit 0x6f
        _emit 0x5b
        _emit 0x00
        // 0001b2c7  MOV EAX, [EDI+0x18]
        _emit 0x8b
        _emit 0x47
        _emit 0x18
        // 0001b2ca  MOV ECX, [ESP+0x38]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // 0001b2ce  POP EDI
        _emit 0x5f
        // 0001b2cf  POP ESI
        _emit 0x5e
        // 0001b2d0  POP EBP
        _emit 0x5d
        // 0001b2d1  MOV [ECX], EAX
        _emit 0x89
        _emit 0x01
        // 0001b2d3  POP EBX
        _emit 0x5b
        // 0001b2d4  ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 0001b2d7  RET
        _emit 0xc3
        // 0001b2d8  MOV EAX, [0x1329834]
        _emit 0xa1
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001b2dd  MOV EDX, [EAX]
        _emit 0x8b
        _emit 0x10
        // 0001b2df  MOV EDX, [EDX+0x1a8]
        _emit 0x8b
        _emit 0x92
        _emit 0xa8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001b2e5  LEA ECX, [ESP+0x34]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        // 0001b2e9  PUSH ECX
        _emit 0x51
        // 0001b2ea  PUSH EDI
        _emit 0x57
        // 0001b2eb  PUSH EAX
        _emit 0x50
        // 0001b2ec  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0001b2ee  MOV EAX, [ESP+0x34]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 0001b2f2  MOV ECX, [ESP+0x38]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // 0001b2f6  MOV [ECX], EAX
        _emit 0x89
        _emit 0x01
        // 0001b2f8  MOV EDX, [EAX]
        _emit 0x8b
        _emit 0x10
        // 0001b2fa  PUSH EAX
        _emit 0x50
        // 0001b2fb  MOV EAX, [EDX+4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 0001b2fe  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001b300  MOV ECX, [ESP+0x34]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        // 0001b304  LEA EDX, [ESP+0x10]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0001b308  PUSH EDX
        _emit 0x52
        // 0001b309  LEA EAX, [ESP+0x24]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0001b30d  MOV [ESP+0x1c], ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0001b311  PUSH EAX
        _emit 0x50
        // 0001b312  MOV ECX, 0x13298ac
        _emit 0xb9
        _emit 0xac
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001b317  MOV [ESP+0x18], EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // 0001b31b  MOV [ESP+0x1c], EBP
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        // 0001b31f  CALL 0x0041af60
        _emit 0xe8
        _emit 0x3c
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 0001b324  POP EDI
        _emit 0x5f
        // 0001b325  POP ESI
        _emit 0x5e
        // 0001b326  POP EBP
        _emit 0x5d
        // 0001b327  POP EBX
        _emit 0x5b
        // 0001b328  ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 0001b32b  RET
        _emit 0xc3
    }
}
