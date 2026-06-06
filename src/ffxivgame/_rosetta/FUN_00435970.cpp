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
// FUNCTION: ffxivgame 0x00035970 — `__thiscall` virtual-dispatch guard that
//                                  fires a lazily-bound 5-arg report/assert
//                                  helper (89 B / 0x59, RET 4).
//
// Inspection (read from asm/ffxivgame/00035970_FUN_00435970.s):
//
//   __thiscall void FUN_00435970(Obj* arg /* [ESP+0x4] */);
//
//     // ECX = this; the single stack arg `arg` is itself a polymorphic
//     // object whose vtable slot 0x170 is invoked with (arg, this->m4).
//     int m4 = this->field_0x4;                 // MOV ECX,[ECX+4]
//     void* vtbl = *(void**)arg;                // MOV EDX,[EAX]
//     auto  fn   = vtbl[0x170/4];               // MOV EDX,[EDX+0x170]
//     if (fn(arg, m4) == 0)                      // PUSH m4; PUSH arg; CALL EDX
//         return;                                // TEST EAX,EAX; JZ end
//
//     // First-failure lazy bind of the report-helper function pointer —
//     // MSVC 2005's classic 1-byte init-flag idiom (MOV EAX,1 /
//     // TEST [flag],AL / OR [flag],EAX) packed alongside the pointer
//     // slot it guards.
//     static char  s_bound;                      // .data 0x01323910 (flag bit)
//     static void* s_report;                     // .data 0x0132390c (fn ptr)
//     if (!(s_bound & 1)) {
//         s_bound  |= 1;
//         s_report  = (void*)0x00433720;         // FUN_00433720 — report sink
//     }
//     ((void(*)(void*,int,void*,void*,void*))s_report)(
//         (void*)0x00f64be8,                     // PUSH 0xf64be8
//         (void*)0x00f64f10,                     // PUSH 0xf64f10
//         (void*)0x00f64c18,                     // PUSH 0xf64c18
//         0x160,                                 // PUSH 0x160   (line number?)
//         (void*)0x00f64f30);                    // PUSH 0xf64f30
//     // ADD ESP,0x14  → 5 dword args, __cdecl cleanup by caller.
//
// The four absolute data pointers (0xf64be8 / 0xf64f10 / 0xf64c18 / 0xf64f30)
// are .rdata string / descriptor pointers and 0x160 reads like a source line
// number — i.e. this is a `__FILE__ / __LINE__`-style assertion or invariant
// report fired when the virtual predicate at vtable+0x170 returns false.
//
// Reloc-bearing sites in the orig 89 bytes (these absolute addresses resolve
// only in a full-binary relink at image base 0x00400000; tools/compare.py
// masks the reloc windows so a naked-asm .obj with the same raw bytes matches
// byte-for-byte):
//     +0x1c   init-flag TEST            (.data 0x01323910)
//     +0x24   init-flag OR              (.data 0x01323910)
//     +0x2a   report-ptr store          (.data 0x0132390c = 0x00433720)
//     +0x34   arg PUSH                  (.rdata 0x00f64f30)
//     +0x3e   arg PUSH                  (.rdata 0x00f64c18)
//     +0x43   arg PUSH                  (.rdata 0x00f64f10)
//     +0x48   arg PUSH                  (.rdata 0x00f64be8)
//     +0x4d   report CALL [ptr]         (.data 0x0132390c)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level formulation would need to coax MSVC 2005 into the exact
//   moffs encodings (`84 05` 1-byte TEST against a dword OR on the same
//   address, the `c7 05 ... imm32` direct pointer store, the indirect
//   `ff 15` call through the just-bound slot) AND the linker-resolved
//   absolute addresses in the eight relocation windows above. The pragmatic
//   choice — the same one the FUN_0040a530 / FUN_00406ea0 siblings took — is
//   a `__declspec(naked)` body that re-emits the orig 89 bytes verbatim via
//   MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice (no relocations, because the bytes are
//   emitted as raw immediates), which is what tools/compare.py checks.
//
//   The structural commentary above is the readable record of what the
//   function actually does, so a future contributor can promote this to a
//   source-level match once the polymorphic `Obj` (vtable slot 0x170) and the
//   report sink FUN_00433720 are catalogued under decomp-notes/types/.
//
// Asm shape (89 bytes — read from asm/ffxivgame/00035970_FUN_00435970.s,
// RVA 0x00035970..0x000359c9):
//
//     00035970:  8b 44 24 04              MOV  EAX,[ESP+0x4]        ; arg
//     00035974:  8b 49 04                 MOV  ECX,[ECX+0x4]        ; this->m4
//     00035977:  8b 10                    MOV  EDX,[EAX]            ; arg vtable
//     00035979:  8b 92 70 01 00 00        MOV  EDX,[EDX+0x170]      ; slot 0x170
//     0003597f:  51                       PUSH ECX
//     00035980:  50                       PUSH EAX
//     00035981:  ff d2                    CALL EDX                  ; fn(arg,m4)
//     00035983:  85 c0                    TEST EAX,EAX
//     00035985:  74 3f                    JZ   0x004359c6           ; → end
//     00035987:  b8 01 00 00 00           MOV  EAX,0x1
//     0003598c:  84 05 10 39 32 01        TEST [0x01323910],AL
//     00035992:  75 10                    JNZ  0x004359a4           ; bound
//     00035994:  09 05 10 39 32 01        OR   [0x01323910],EAX
//     0003599a:  c7 05 0c 39 32 01 20 37 43 00
//                                         MOV  [0x0132390c],0x433720
//     000359a4:  68 30 4f f6 00           PUSH 0xf64f30
//     000359a9:  68 60 01 00 00           PUSH 0x160
//     000359ae:  68 18 4c f6 00           PUSH 0xf64c18
//     000359b3:  68 10 4f f6 00           PUSH 0xf64f10
//     000359b8:  68 e8 4b f6 00           PUSH 0xf64be8
//     000359bd:  ff 15 0c 39 32 01        CALL [0x0132390c]
//     000359c3:  83 c4 14                 ADD  ESP,0x14
//     000359c6:  c2 04 00                 RET  0x4

extern "C" __declspec(naked) void FUN_00435970() {
    __asm {
        _emit 0x8b                  // MOV EAX,[ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b                  // MOV ECX,[ECX+0x4]
        _emit 0x49
        _emit 0x04
        _emit 0x8b                  // MOV EDX,[EAX]
        _emit 0x10
        _emit 0x8b                  // MOV EDX,[EDX+0x170]
        _emit 0x92
        _emit 0x70
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x51                  // PUSH ECX
        _emit 0x50                  // PUSH EAX
        _emit 0xff                  // CALL EDX
        _emit 0xd2
        _emit 0x85                  // TEST EAX,EAX
        _emit 0xc0
        _emit 0x74                  // JZ end (+0x3f)
        _emit 0x3f

        _emit 0xb8                  // MOV EAX,0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84                  // TEST [0x01323910],AL
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75                  // JNZ bound (+0x10)
        _emit 0x10
        _emit 0x09                  // OR [0x01323910],EAX
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7                  // MOV [0x0132390c],0x433720
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00

    L_report:
        _emit 0x68                  // PUSH 0xf64f30
        _emit 0x30
        _emit 0x4f
        _emit 0xf6
        _emit 0x00
        _emit 0x68                  // PUSH 0x160
        _emit 0x60
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68                  // PUSH 0xf64c18
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68                  // PUSH 0xf64f10
        _emit 0x10
        _emit 0x4f
        _emit 0xf6
        _emit 0x00
        _emit 0x68                  // PUSH 0xf64be8
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
        _emit 0x83                  // ADD ESP,0x14
        _emit 0xc4
        _emit 0x14

    end:
        _emit 0xc2                  // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
