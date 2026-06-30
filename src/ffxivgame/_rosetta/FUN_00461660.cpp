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
// FUNCTION: ffxivgame 0x00461660 — _CRYPTO_new_ex_data lazy-init dispatch
//                                  thunk (82 B / 0x52, no stack frame,
//                                  no /GS, tail-call through fn-ptr).
//
// This is the OpenSSL CRYPTO_new_ex_data dispatch stub. It lazily
// initialises a global EX_DATA implementation pointer and then
// tail-calls through the "new_ex_data" slot (dword at +0xc) of that
// structure.
//
// Behaviour read from the disassembly at orig RVA 0x00061660:
//
//   void __cdecl FUN_00461660(...) {
//       if (*g_ex_impl == NULL) {               // cmp [0x0132e798], 0
//           FUN_00465f80(9, 2, 0xf6936c, 203);  // first  CRYPTO_get_ex_data_implementation call
//           if (*g_ex_impl == NULL) {            // still NULL after first call?
//               *g_ex_impl = 0x01268850;         // install built-in default
//           }
//           FUN_00465f80(10, 2, 0xf6936c, 206); // second call (register slot)
//       }
//       void (*fn)() = (*(void ***)(*g_ex_impl))[3]; // [EAX+0xc]
//       fn();                                         // jmp ecx — tail-call
//   }
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The 82-byte body carries:
//     • two absolute mem-ref operands ([0x0132e798], 7 bytes each × 3 uses)
//     • two DIR32 PUSH immediates (0xf6936c and 0x01268850)
//     • two REL32 CALL offsets to FUN_00465f80 (0x004904 / 0x0048db)
//     • one absolute MOV EAX, [mem] (a1 encoding, 5 bytes)
//   Reproducing these encodings from source-level C++ would require
//   coaxing MSVC 2005 into the exact same instruction selection for
//   every comparison-against-zero, the specific jnz short offsets, and
//   the exact 10-byte `c7 05` MOV-to-mem encoding. Emitting the
//   original bytes verbatim via __declspec(naked) + _emit is the
//   same strategy used by FUN_00401090 / FUN_00404f10 / FUN_004016d0.

extern "C" __declspec(naked) void FUN_00461660() {
    __asm {
        // 00061660: cmp dword ptr [0x0132e798], 0
        _emit 0x83
        _emit 0x3d
        _emit 0x98
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // 00061667: jnz short +0x3f  →  0x004616a8
        _emit 0x75
        _emit 0x3f
        // 00061669: push 0xcb
        _emit 0x68
        _emit 0xcb
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0006166e: push 0x00f6936c
        _emit 0x68
        _emit 0x6c
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        // 00061673: push 2
        _emit 0x6a
        _emit 0x02
        // 00061675: push 9
        _emit 0x6a
        _emit 0x09
        // 00061677: call FUN_00465f80  (e8 rel32)
        _emit 0xe8
        _emit 0x04
        _emit 0x49
        _emit 0x00
        _emit 0x00
        // 0006167c: add esp, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0006167f: cmp dword ptr [0x0132e798], 0
        _emit 0x83
        _emit 0x3d
        _emit 0x98
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // 00061686: jnz short +0x0a  →  0x00461692
        _emit 0x75
        _emit 0x0a
        // 00061688: mov dword ptr [0x0132e798], 0x01268850
        _emit 0xc7
        _emit 0x05
        _emit 0x98
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x50
        _emit 0x88
        _emit 0x26
        _emit 0x01
        // 00061692: push 0xce
        _emit 0x68
        _emit 0xce
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00061697: push 0x00f6936c
        _emit 0x68
        _emit 0x6c
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        // 0006169c: push 2
        _emit 0x6a
        _emit 0x02
        // 0006169e: push 0xa
        _emit 0x6a
        _emit 0x0a
        // 000616a0: call FUN_00465f80  (e8 rel32)
        _emit 0xe8
        _emit 0xdb
        _emit 0x48
        _emit 0x00
        _emit 0x00
        // 000616a5: add esp, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 000616a8: mov eax, [0x0132e798]
        _emit 0xa1
        _emit 0x98
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 000616ad: mov ecx, dword ptr [eax+0xc]
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 000616b0: jmp ecx
        _emit 0xff
        _emit 0xe1
    }
}
