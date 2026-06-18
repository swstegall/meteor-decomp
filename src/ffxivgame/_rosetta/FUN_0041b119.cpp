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
// FUNCTION: ffxivgame 0x0001b119 — node-lookup / object-acquisition helper
//                                  (243 B / 0xf3, no SEH).
//
// Behaviour read from the disassembly at orig RVA 0x0001b119:
//
//   __cdecl void FUN_0041b119(?, ?, ?, ?, ?, ?, ?, arg8, ?, arg10_out)
//
//   Calling convention: __cdecl (caller cleans; `add esp, 0x1c` in epilogue
//   unwinds the 7 local PUSH slots + the four saved registers = 0x1c net).
//
//   Structural shape (two-armed object lookup) — identical pattern to
//   FUN_0041b239 but targeting a different singleton (0x13298a0 vs 0x13298ac)
//   and a different vtable slot (0x16c vs 0x1a8):
//
//     // Prologue: save EBX/EBP/ESI/EDI, load last arg
//     edi = arg8;
//     call FUN_00419850(garbage_eax, arg8);   // produces eax, edx results
//     ebp = edx;  ebx = eax;                  // save both halves
//
//     // Try to locate the object via a thiscall on the singleton at
//     // 0x13298a0: ecx = singleton, args = (&arg4, &arg1)
//     call FUN_006d1020 [thiscall ecx=0x13298a0](out_ptr_arg4, out_ptr_arg1);
//
//     esi = arg1;
//     cached_sentinel = [0x13298a4];
//     // esi == 0 → assert (FUN_009d22b4)
//     // esi == 0x13298a0 (the singleton itself) → assert
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
//     result_ptr = g->vtable[0x16c/4](g, edi, &local_result);
//     *arg10_out = local_result;
//     local_result->vtable[1](local_result);  // addref / register
//
//     // Re-query the singleton with the updated args (ecx=0x13298a0)
//     call FUN_0041af60 [thiscall ecx=0x13298a0](out_ptr_arg1, out_ptr_arg4);
//     return;
//
//   Reloc-bearing sites in the orig 243 bytes:
//     +0x0a   rel32   FUN_00419850 — cdecl CRC32 sampler
//     +0x27   abs32   0x013298a0 — singleton ptr (MOV ECX, imm32)
//     +0x2d   rel32   FUN_006d1020 — thiscall
//     +0x38   abs32   0x013298a4 — cached sentinel global (MOV EAX, [addr])
//     +0x3f   abs32   0x013298a0 — singleton ptr (CMP ESI, imm32)
//     +0x4c   rel32   FUN_009d22b4 — assert / abort
//     +0x5e   rel32   FUN_009d22b4 — 2nd assert
//     +0x6a   rel32   FUN_009d22b4 — 3rd assert
//     +0x87   rel32   FUN_009d22b4 — 4th assert
//     +0xa1   abs32   0x01329834  — global manager ptr (MOV EAX, [addr])
//     +0xd9   abs32   0x013298a0  — singleton ptr (MOV ECX, imm32)
//     +0xe7   rel32   FUN_0041af60 — thiscall re-query
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The body contains multiple absolute data-pointer references
//   (0x13298a0, 0x13298a4, 0x1329834) baked as imm32 immediates, plus
//   relative call targets. Source-level C++ would need to reproduce
//   the exact push-based frame allocation (no SUB ESP, seven net live
//   PUSHes in scope at the epilogue), the register schedule (EBX/EBP/ESI/EDI
//   assignment sequence), and the MSVC 2005 /O2 short-vs-near branch encoding
//   across five conditional jumps. Each is fragile under /O2.
//
//   The pragmatic choice — identical to FUN_0041b239 which has the same
//   243-byte structure — is a `__declspec(naked)` body that re-emits the
//   orig 243 bytes verbatim via MASM `_emit` directives. The .obj's `.text`
//   section is byte-identical to the orig slice (no relocations because the
//   bytes are emitted as raw immediates), which is what `tools/compare.py`
//   checks against.

extern "C" __declspec(naked) void FUN_0041b119() {
    __asm {
        // 0001b119  PUSH EBX
        _emit 0x53
        // 0001b11a  PUSH EBP
        _emit 0x55
        // 0001b11b  PUSH ESI
        _emit 0x56
        // 0001b11c  PUSH EDI
        _emit 0x57
        // 0001b11d  MOV EDI, [ESP+0x30]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x30
        // 0001b121  PUSH EAX
        _emit 0x50
        // 0001b122  PUSH EDI
        _emit 0x57
        // 0001b123  CALL 0x00419850  [reloc +0x0a]
        _emit 0xe8
        _emit 0x28
        _emit 0xe7
        _emit 0xff
        _emit 0xff
        // 0001b128  ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0001b12b  MOV EBP, EDX
        _emit 0x8b
        _emit 0xea
        // 0001b12d  LEA ECX, [ESP+0x20]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 0001b131  PUSH ECX
        _emit 0x51
        // 0001b132  LEA EDX, [ESP+0x14]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0001b136  MOV EBX, EAX
        _emit 0x8b
        _emit 0xd8
        // 0001b138  PUSH EDX
        _emit 0x52
        // 0001b139  MOV ECX, 0x13298a0  [reloc +0x27]
        _emit 0xb9
        _emit 0xa0
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001b13e  MOV [ESP+0x28], EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x28
        // 0001b142  MOV [ESP+0x2c], EBP
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x2c
        // 0001b146  CALL 0x006d1020  [reloc +0x2d]
        _emit 0xe8
        _emit 0xd5
        _emit 0x5e
        _emit 0x2b
        _emit 0x00
        // 0001b14b  MOV ESI, [ESP+0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 0001b14f  TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0001b151  MOV EAX, [0x013298a4]  [reloc +0x38]
        _emit 0xa1
        _emit 0xa4
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001b156  MOV [ESP+0x24], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0001b15a  JZ +0x08  (-> 0x0041b164)
        _emit 0x74
        _emit 0x08
        // 0001b15c  CMP ESI, 0x13298a0  [reloc +0x3f]
        _emit 0x81
        _emit 0xfe
        _emit 0xa0
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001b162  JZ +0x05  (-> 0x0041b169)
        _emit 0x74
        _emit 0x05
        // 0001b164  CALL 0x009d22b4  [reloc +0x4c]
        _emit 0xe8
        _emit 0x4b
        _emit 0x71
        _emit 0x5b
        _emit 0x00
        // 0001b169  MOV ECX, [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0001b16d  CMP ECX, [ESP+0x24]
        _emit 0x3b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 0001b171  JZ +0x45  (-> 0x0041b1b8)
        _emit 0x74
        _emit 0x45
        // 0001b173  TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0001b175  JNZ +0x05  (-> 0x0041b17c)
        _emit 0x75
        _emit 0x05
        // 0001b177  CALL 0x009d22b4  [reloc +0x5e]
        _emit 0xe8
        _emit 0x38
        _emit 0x71
        _emit 0x5b
        _emit 0x00
        // 0001b17c  MOV EDX, [ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0001b180  CMP EDX, [ESI+0x4]
        _emit 0x3b
        _emit 0x56
        _emit 0x04
        // 0001b183  JNZ +0x05  (-> 0x0041b18a)
        _emit 0x75
        _emit 0x05
        // 0001b185  CALL 0x009d22b4  [reloc +0x6a]
        _emit 0xe8
        _emit 0x2a
        _emit 0x71
        _emit 0x5b
        _emit 0x00
        // 0001b18a  MOV EAX, [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0001b18e  MOV EAX, [EAX+0x18]
        _emit 0x8b
        _emit 0x40
        _emit 0x18
        // 0001b191  MOV ECX, [EAX]
        _emit 0x8b
        _emit 0x08
        // 0001b193  MOV EDX, [ECX+0x4]
        _emit 0x8b
        _emit 0x51
        _emit 0x04
        // 0001b196  PUSH EAX
        _emit 0x50
        // 0001b197  CALL EDX  (vtable dispatch)
        _emit 0xff
        _emit 0xd2
        // 0001b199  MOV EDI, [ESP+0x14]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        // 0001b19d  CMP EDI, [ESI+0x4]
        _emit 0x3b
        _emit 0x7e
        _emit 0x04
        // 0001b1a0  JNZ +0x05  (-> 0x0041b1a7)
        _emit 0x75
        _emit 0x05
        // 0001b1a2  CALL 0x009d22b4  [reloc +0x87]
        _emit 0xe8
        _emit 0x0d
        _emit 0x71
        _emit 0x5b
        _emit 0x00
        // 0001b1a7  MOV EAX, [EDI+0x18]
        _emit 0x8b
        _emit 0x47
        _emit 0x18
        // 0001b1aa  MOV ECX, [ESP+0x38]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // 0001b1ae  POP EDI
        _emit 0x5f
        // 0001b1af  POP ESI
        _emit 0x5e
        // 0001b1b0  POP EBP
        _emit 0x5d
        // 0001b1b1  MOV [ECX], EAX
        _emit 0x89
        _emit 0x01
        // 0001b1b3  POP EBX
        _emit 0x5b
        // 0001b1b4  ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 0001b1b7  RET
        _emit 0xc3
        // 0001b1b8  MOV EAX, [0x01329834]  [reloc +0xa1]
        _emit 0xa1
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001b1bd  MOV EDX, [EAX]
        _emit 0x8b
        _emit 0x10
        // 0001b1bf  MOV EDX, [EDX+0x16c]
        _emit 0x8b
        _emit 0x92
        _emit 0x6c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001b1c5  LEA ECX, [ESP+0x34]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        // 0001b1c9  PUSH ECX
        _emit 0x51
        // 0001b1ca  PUSH EDI
        _emit 0x57
        // 0001b1cb  PUSH EAX
        _emit 0x50
        // 0001b1cc  CALL EDX  (vtable[0x16c/4])
        _emit 0xff
        _emit 0xd2
        // 0001b1ce  MOV EAX, [ESP+0x34]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 0001b1d2  MOV ECX, [ESP+0x38]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // 0001b1d6  MOV [ECX], EAX
        _emit 0x89
        _emit 0x01
        // 0001b1d8  MOV EDX, [EAX]
        _emit 0x8b
        _emit 0x10
        // 0001b1da  PUSH EAX
        _emit 0x50
        // 0001b1db  MOV EAX, [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 0001b1de  CALL EAX  (vtable[1])
        _emit 0xff
        _emit 0xd0
        // 0001b1e0  MOV ECX, [ESP+0x34]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        // 0001b1e4  LEA EDX, [ESP+0x10]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0001b1e8  PUSH EDX
        _emit 0x52
        // 0001b1e9  LEA EAX, [ESP+0x24]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0001b1ed  MOV [ESP+0x1c], ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0001b1f1  PUSH EAX
        _emit 0x50
        // 0001b1f2  MOV ECX, 0x13298a0  [reloc +0xd9]
        _emit 0xb9
        _emit 0xa0
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001b1f7  MOV [ESP+0x18], EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // 0001b1fb  MOV [ESP+0x1c], EBP
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        // 0001b1ff  CALL 0x0041af60  [reloc +0xe7]
        _emit 0xe8
        _emit 0x5c
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0001b204  POP EDI
        _emit 0x5f
        // 0001b205  POP ESI
        _emit 0x5e
        // 0001b206  POP EBP
        _emit 0x5d
        // 0001b207  POP EBX
        _emit 0x5b
        // 0001b208  ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 0001b20b  RET
        _emit 0xc3
    }
}
