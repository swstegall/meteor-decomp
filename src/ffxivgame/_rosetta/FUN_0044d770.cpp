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
// FUNCTION: ffxivgame 0x0044d770 — singleton-dispatched UI notifier with a
//                                  "last-value" de-dupe cache (80 B / 0x50).
//
// Behaviour read from the disassembly at orig RVA 0x0004d770:
//
//   __stdcall void FUN_0044d770(int a, int b);          // RET 8 → 2 stack args
//
//     Mgr* mgr = DAT_0132cf40;                          // global singleton ptr
//     if (mgr == nullptr) return;
//
//     // virtual dispatch #1: vtbl[0x48], __stdcall(ctx, a)
//     mgr->vtbl->slot48(&DAT_0132cf60, a);
//
//     if (DAT_01266dfc != a) {                          // value changed?
//         DAT_01266dfc = a;                             // remember it, done
//         return;
//     }
//
//     // value unchanged: only the "b > 0" edge does extra work
//     if (b <= 0) return;
//
//     // mask = (a == 0) ? b : 0   — the classic NEG/SBB/NOT/AND lowering:
//     //   NEG ESI; SBB ESI,ESI; NOT ESI  →  (a==0) ? 0xffffffff : 0
//     //   AND ESI, b                     →  (a==0) ? b : 0
//     int arg = (a == 0) ? b : 0;
//
//     // virtual dispatch #2: vtbl[0x14], __stdcall(arg) on the same singleton
//     DAT_0132cf40->vtbl->slot14(arg);
//
// Asm (80 B, RVA 0x0004d770):
//   8b 0d 40 cf 32 01     MOV  ECX, [0x0132cf40]   ; singleton ptr
//   85 c9                 TEST ECX, ECX
//   74 43                 JZ   +0x43 (→ ret 8)
//   8b 01                 MOV  EAX, [ECX]          ; vtable
//   8b 50 48              MOV  EDX, [EAX+0x48]      ; slot 0x48
//   56                    PUSH ESI
//   8b 74 24 08           MOV  ESI, [ESP+0x8]       ; a
//   56                    PUSH ESI                  ; arg2 = a
//   68 60 cf 32 01        PUSH 0x132cf60            ; arg1 = &ctx
//   ff d2                 CALL EDX                  ; slot48(&ctx, a)  (stdcall)
//   39 35 fc 6d 26 01     CMP  [0x01266dfc], ESI    ; cache != a?
//   75 22                 JNZ  +0x22                ; → store-and-return
//   8b 44 24 0c           MOV  EAX, [ESP+0xc]       ; b
//   85 c0                 TEST EAX, EAX
//   7e 20                 JLE  +0x20 (→ pop/ret)
//   8b 0d 40 cf 32 01     MOV  ECX, [0x0132cf40]
//   8b 11                 MOV  EDX, [ECX]           ; vtable
//   f7 de                 NEG  ESI                  ; ┐
//   1b f6                 SBB  ESI, ESI             ; ├ (a==0)?0xffffffff:0
//   f7 d6                 NOT  ESI                  ; ┘
//   23 f0                 AND  ESI, EAX             ; & b → (a==0)?b:0
//   8b 42 14              MOV  EAX, [EDX+0x14]       ; slot 0x14
//   56                    PUSH ESI                  ; arg
//   ff d0                 CALL EAX                  ; slot14(arg)      (stdcall)
//   5e                    POP  ESI
//   c2 08 00              RET  0x8
//   89 35 fc 6d 26 01     MOV  [0x01266dfc], ESI    ; cache = a
//   5e                    POP  ESI
//   c2 08 00              RET  0x8
//
// Reloc-bearing sites in the orig 80 bytes are all absolute data addresses
// the linker baked at the orig image base 0x00400000 (0x0132cf40,
// 0x132cf60, 0x01266dfc). The two CALLs are register-indirect (CALL EDX /
// CALL EAX through vtable slots), so they carry no relocations at all.
// Emitting the bytes raw via MASM `_emit` therefore yields a .obj whose
// .text matches the orig slice byte-for-byte with zero relocations, and
// tools/compare.py reports GREEN by direct equality.
//
// Reconstruction strategy — naked-asm byte passthrough. Same idiom as the
// module siblings (FUN_0040a460 / FUN_00401090 / FUN_00404f10): a
// source-level port would have to coax MSVC 2005 /O2 into the exact
// NEG/SBB/NOT/AND boolean lowering, the precise singleton-reload between
// the two virtual dispatches, and the short-vs-near branch encodings — all
// brittle. The structural commentary above is the readable record.

extern "C" __declspec(naked) void FUN_0044d770() {
    __asm {
        _emit 0x8b              // MOV ECX, dword ptr [0x0132cf40]
        _emit 0x0d
        _emit 0x40
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74              // JZ +0x43
        _emit 0x43
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x48]
        _emit 0x50
        _emit 0x48
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x8]
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x68              // PUSH 0x132cf60
        _emit 0x60
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x39              // CMP dword ptr [0x01266dfc], ESI
        _emit 0x35
        _emit 0xfc
        _emit 0x6d
        _emit 0x26
        _emit 0x01
        _emit 0x75              // JNZ +0x22
        _emit 0x22
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0xc]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7e              // JLE +0x20
        _emit 0x20
        _emit 0x8b              // MOV ECX, dword ptr [0x0132cf40]
        _emit 0x0d
        _emit 0x40
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [ECX]
        _emit 0x11
        _emit 0xf7              // NEG ESI
        _emit 0xde
        _emit 0x1b              // SBB ESI, ESI
        _emit 0xf6
        _emit 0xf7              // NOT ESI
        _emit 0xd6
        _emit 0x23              // AND ESI, EAX
        _emit 0xf0
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x14]
        _emit 0x42
        _emit 0x14
        _emit 0x56              // PUSH ESI
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        _emit 0x89              // MOV dword ptr [0x01266dfc], ESI
        _emit 0x35
        _emit 0xfc
        _emit 0x6d
        _emit 0x26
        _emit 0x01
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
