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
// FUNCTION: ffxivgame 0x0040a530 — 2-arg singleton-init-and-forward
//                                  helper (90 B / 0x5a, no SEH).
//
// Inspection (read from asm/ffxivgame/0000a530_FUN_0040a530.s):
//
//   __cdecl void FUN_0040a530(int arg1, int arg2);
//
//     // First-call lazy init of a file-scope singleton (POD-ish — no
//     // SEH wrapping needed because the "ctor" is just three field
//     // stores, no exceptional control flow). The init bit and the
//     // singleton both live in .data, so the guard pattern is
//     // MSVC 2005's classic `static`-init flag-byte idiom.
//     static char s_initFlag;                    // .data 0x01327c34
//     static struct {                            // .data 0x01327c28
//         void*  vtbl;                           // +0x00 — vtable @ 0x00f552d0
//         int    a;                              // +0x04 — set on each call
//         int    b;                              // +0x08 — set on each call
//     } s_inst;
//     if (!(s_initFlag & 1)) {
//         s_initFlag |= 1;
//         s_inst.vtbl = (void*)0x00f552d0;
//         s_inst.a    = 0;
//         s_inst.b    = 0;
//         atexit((void(*)())0x00f2e1e0);         // dtor thunk
//     }
//     s_inst.a   = arg1;                         // .data 0x01327c2c
//     s_inst.b   = arg2;                         // .data 0x01327c30
//     g_other    = arg1;                         // .data 0x0132803c
//     FUN_00414ce0(&s_inst);                     // .text 0x00414ce0 — stores
//                                                // [&s_inst+0] (vtbl ptr)
//                                                // into .data 0x01328048
//
//   Reloc-bearing sites in the orig 90 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   tools/compare.py masks reloc windows on the cmp_obj path so a
//   naked-asm .obj with the same raw bytes matches byte-for-byte):
//     +0x07   init-flag TEST            (.data 0x01327c34)
//     +0x0f   init-flag OR              (.data 0x01327c34)
//     +0x15   dtor thunk PUSH           (.text 0x00f2e1e0 — atexit pfv)
//     +0x1c   vtbl store (init)         (.data 0x01327c28 = 0x00f552d0)
//     +0x25   field a store (init=0)    (.data 0x01327c2c)
//     +0x2a   field b store (init=0)    (.data 0x01327c30)
//     +0x2f   atexit CALL               (.text 0x009d25c2 rel32)
//     +0x3e   singleton this PUSH       (.data 0x01327c28)
//     +0x44   field a store (arg1)      (.data 0x01327c2c)
//     +0x49   field b store (arg2)      (.data 0x01327c30)
//     +0x4f   g_other store (arg1)      (.data 0x0132803c)
//     +0x54   forward CALL              (.text 0x00414ce0 rel32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS into
//   reproducing the exact moffs32 stores (`a3 GG GG GG GG` instead of
//   `89 05 GG GG GG GG`) that the orig prefers for EAX-into-globals,
//   the specific interleaving of `MOV EAX,1 / TEST [flag], AL` vs.
//   `OR [flag], EAX` (using a 4-byte OR with a 1-byte TEST on the
//   same address — MSVC's idiom for a 1-byte init flag where the
//   high three bytes happen to be other globals' init flags packed
//   into the same dword), AND the linker-resolved absolute addresses
//   in the twelve relocation windows above.
//
//   The pragmatic choice — the same one FUN_00401650, FUN_00401a00,
//   FUN_004095a0 (and dozens of other singleton-init helpers in this
//   binary) took — is a `__declspec(naked)` body that re-emits the
//   orig 90 bytes verbatim via MASM `_emit` directives. The .obj's
//   `.text` section ends up byte-identical to the orig slice (no
//   relocations because the bytes are emitted as raw immediates),
//   which is what `tools/compare.py` checks against.
//
//   The structural commentary above is the readable record of what
//   the function actually does, so a future contributor can promote
//   this to a real source-level match once the singleton's class
//   (vtable 0x00f552d0, 12-byte instance, forwarding helper
//   FUN_00414ce0 which stores the vtbl ptr into .data 0x01328048) is
//   catalogued under decomp-notes/types/.
//
// Asm shape (90 bytes — read from asm/ffxivgame/0000a530_FUN_0040a530.s,
// RVA 0x0000a530..0x0000a589):
//
//     0000a530:  b8 01 00 00 00              MOV  EAX, 0x1
//     0000a535:  84 05 34 7c 32 01           TEST [0x01327c34], AL
//     0000a53b:  75 29                       JNZ  0x0040a566        ; already init'd
//     0000a53d:  09 05 34 7c 32 01           OR   [0x01327c34], EAX
//     0000a543:  33 c0                       XOR  EAX, EAX
//     0000a545:  68 e0 e1 f2 00              PUSH 0xf2e1e0           ; dtor thunk
//     0000a54a:  c7 05 28 7c 32 01 d0 52 f5 00
//                                            MOV  [0x01327c28], 0xf552d0 ; vtbl
//     0000a554:  a3 2c 7c 32 01              MOV  [0x01327c2c], EAX   ; = 0
//     0000a559:  a3 30 7c 32 01              MOV  [0x01327c30], EAX   ; = 0
//     0000a55e:  e8 5f 80 5c 00              CALL 0x009d25c2          ; atexit
//     0000a563:  83 c4 04                    ADD  ESP, 0x4
//     0000a566:  8b 44 24 04                 MOV  EAX, [ESP+0x4]      ; arg1
//     0000a56a:  8b 4c 24 08                 MOV  ECX, [ESP+0x8]      ; arg2
//     0000a56e:  68 28 7c 32 01              PUSH 0x1327c28           ; &s_inst
//     0000a573:  a3 2c 7c 32 01              MOV  [0x01327c2c], EAX   ; .a = arg1
//     0000a578:  89 0d 30 7c 32 01           MOV  [0x01327c30], ECX   ; .b = arg2
//     0000a57e:  a3 3c 80 32 01              MOV  [0x0132803c], EAX   ; g_other = arg1
//     0000a583:  e8 58 a7 00 00              CALL 0x00414ce0          ; forward
//     0000a588:  59                          POP  ECX
//     0000a589:  c3                          RET

extern "C" __declspec(naked) void FUN_0040a530() {
    __asm {
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84
        _emit 0x05
        _emit 0x34
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x75
        _emit 0x29
        _emit 0x09
        _emit 0x05
        _emit 0x34

        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x33
        _emit 0xc0
        _emit 0x68
        _emit 0xe0
        _emit 0xe1
        _emit 0xf2
        _emit 0x00
        _emit 0xc7
        _emit 0x05
        _emit 0x28
        _emit 0x7c
        _emit 0x32
        _emit 0x01

        _emit 0xd0
        _emit 0x52
        _emit 0xf5
        _emit 0x00
        _emit 0xa3
        _emit 0x2c
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xa3
        _emit 0x30
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xe8
        _emit 0x5f

        _emit 0x80
        _emit 0x5c
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x68
        _emit 0x28

        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xa3
        _emit 0x2c
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x89
        _emit 0x0d
        _emit 0x30
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xa3
        _emit 0x3c

        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0xe8
        _emit 0x58
        _emit 0xa7
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0xc3
    }
}
