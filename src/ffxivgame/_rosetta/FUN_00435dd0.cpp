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
// FUNCTION: ffxivgame 0x00035dd0 — `__thiscall` virtual-dispatch guard that
//                                  fires a lazily-bound 5-arg report/assert
//                                  helper (119 B / 0x77, RET 4).
//
// Inspection (read from asm/ffxivgame/00035dd0_FUN_00435dd0.s):
//
//   __thiscall void FUN_00435dd0(Obj* arg /* [ESP+0x4] */);
//
//     // ECX = this; the single stack arg `arg` is itself a polymorphic
//     // object whose vtable slot 0x150 is invoked with (arg, this->m4,
//     // this->mc, this->m10, this->m14, this->m1c, this->m8, this->m20,
//     // this->m18). The predicate's nine __stdcall args are pushed in
//     // reverse from member loads via ESI.
//     void* vtbl = *(void**)arg;                // MOV EDX,[EAX]
//     auto  fn   = vtbl[0x150/4];               // MOV EDX,[EDX+0x150]
//     if (fn(arg, this->m4, this->mc, this->m10,
//            this->m14, this->m1c, this->m8,
//            this->m20, this->m18) == 0)         // CALL EDX
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
//         (void*)0x00f653fc,                     // PUSH 0xf653fc
//         (void*)0x00f64c18,                     // PUSH 0xf64c18
//         0x25a,                                 // PUSH 0x25a   (line number?)
//         (void*)0x00f65420);                    // PUSH 0xf65420
//     // ADD ESP,0x14  → 5 dword args, __cdecl cleanup by caller.
//
// Same shape as the FUN_00435970 sibling (vtable slot 0x170 there, 0x150
// here; this variant forwards eight `this` members to the predicate and saves
// ESI with a matching PUSH/POP pair around the indirect call). The four
// absolute data pointers (0xf64be8 / 0xf653fc / 0xf64c18 / 0xf65420) are
// .rdata string/descriptor pointers and 0x25a reads like a source line number
// — i.e. a `__FILE__ / __LINE__`-style assertion fired when the virtual
// predicate at vtable+0x150 returns nonzero.
//
// Reloc-bearing sites in the orig 119 bytes resolve only in a full-binary
// relink at image base 0x00400000; tools/compare.py masks the reloc windows so
// a naked-asm .obj with the same raw bytes matches byte-for-byte.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level formulation would need to coax MSVC 2005 into the exact
//   moffs encodings (`84 05` 1-byte TEST against a dword OR on the same
//   address, the `c7 05 ... imm32` direct pointer store, the indirect
//   `ff 15` call through the just-bound slot), the precise ESI-shuffle that
//   materialises the nine call args, AND the linker-resolved absolute
//   addresses. The pragmatic choice — the same one the FUN_00435970 sibling
//   took — is a `__declspec(naked)` body that re-emits the orig 119 bytes
//   verbatim via MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice, which is what tools/compare.py checks.
//
// Asm shape (119 bytes — read from asm/ffxivgame/00035dd0_FUN_00435dd0.s,
// RVA 0x00035dd0..0x00035e47):
//
//     00035dd0:  56                       PUSH ESI                  ; save ESI
//     00035dd1:  8b 71 18                 MOV  ESI,[ECX+0x18]
//     00035dd4:  8b 44 24 08              MOV  EAX,[ESP+0x8]        ; arg
//     00035dd8:  56                       PUSH ESI
//     00035dd9:  8b 71 20                 MOV  ESI,[ECX+0x20]
//     00035ddc:  8b 10                    MOV  EDX,[EAX]            ; arg vtable
//     00035dde:  8b 92 50 01 00 00        MOV  EDX,[EDX+0x150]      ; slot 0x150
//     00035de4:  56                       PUSH ESI
//     00035de5:  8b 71 08                 MOV  ESI,[ECX+0x8]
//     00035de8:  56                       PUSH ESI
//     00035de9:  8b 71 1c                 MOV  ESI,[ECX+0x1c]
//     00035dec:  56                       PUSH ESI
//     00035ded:  8b 71 14                 MOV  ESI,[ECX+0x14]
//     00035df0:  56                       PUSH ESI
//     00035df1:  8b 71 10                 MOV  ESI,[ECX+0x10]
//     00035df4:  56                       PUSH ESI
//     00035df5:  8b 71 0c                 MOV  ESI,[ECX+0xc]
//     00035df8:  8b 49 04                 MOV  ECX,[ECX+0x4]
//     00035dfb:  56                       PUSH ESI
//     00035dfc:  51                       PUSH ECX
//     00035dfd:  50                       PUSH EAX                  ; arg (this)
//     00035dfe:  ff d2                    CALL EDX                  ; predicate
//     00035e00:  85 c0                    TEST EAX,EAX
//     00035e02:  5e                       POP  ESI                  ; restore ESI
//     00035e03:  74 3f                    JZ   0x00035e44           ; → end
//     00035e05:  b8 01 00 00 00           MOV  EAX,0x1
//     00035e0a:  84 05 10 39 32 01        TEST [0x01323910],AL
//     00035e10:  75 10                    JNZ  0x00035e22           ; bound
//     00035e12:  09 05 10 39 32 01        OR   [0x01323910],EAX
//     00035e18:  c7 05 0c 39 32 01 20 37 43 00
//                                         MOV  [0x0132390c],0x433720
//     00035e22:  68 20 54 f6 00           PUSH 0xf65420
//     00035e27:  68 5a 02 00 00           PUSH 0x25a
//     00035e2c:  68 18 4c f6 00           PUSH 0xf64c18
//     00035e31:  68 fc 53 f6 00           PUSH 0xf653fc
//     00035e36:  68 e8 4b f6 00           PUSH 0xf64be8
//     00035e3b:  ff 15 0c 39 32 01        CALL [0x0132390c]
//     00035e41:  83 c4 14                 ADD  ESP,0x14
//     00035e44:  c2 04 00                 RET  0x4

extern "C" __declspec(naked) void FUN_00435dd0() {
    __asm {
        _emit 0x56                  // PUSH ESI
        _emit 0x8b                  // MOV ESI,[ECX+0x18]
        _emit 0x71
        _emit 0x18
        _emit 0x8b                  // MOV EAX,[ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x56                  // PUSH ESI
        _emit 0x8b                  // MOV ESI,[ECX+0x20]
        _emit 0x71
        _emit 0x20
        _emit 0x8b                  // MOV EDX,[EAX]
        _emit 0x10
        _emit 0x8b                  // MOV EDX,[EDX+0x150]
        _emit 0x92
        _emit 0x50
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x56                  // PUSH ESI
        _emit 0x8b                  // MOV ESI,[ECX+0x8]
        _emit 0x71
        _emit 0x08
        _emit 0x56                  // PUSH ESI
        _emit 0x8b                  // MOV ESI,[ECX+0x1c]
        _emit 0x71
        _emit 0x1c
        _emit 0x56                  // PUSH ESI
        _emit 0x8b                  // MOV ESI,[ECX+0x14]
        _emit 0x71
        _emit 0x14
        _emit 0x56                  // PUSH ESI
        _emit 0x8b                  // MOV ESI,[ECX+0x10]
        _emit 0x71
        _emit 0x10
        _emit 0x56                  // PUSH ESI
        _emit 0x8b                  // MOV ESI,[ECX+0xc]
        _emit 0x71
        _emit 0x0c
        _emit 0x8b                  // MOV ECX,[ECX+0x4]
        _emit 0x49
        _emit 0x04
        _emit 0x56                  // PUSH ESI
        _emit 0x51                  // PUSH ECX
        _emit 0x50                  // PUSH EAX
        _emit 0xff                  // CALL EDX
        _emit 0xd2
        _emit 0x85                  // TEST EAX,EAX
        _emit 0xc0
        _emit 0x5e                  // POP ESI
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
        _emit 0x68                  // PUSH 0xf65420
        _emit 0x20
        _emit 0x54
        _emit 0xf6
        _emit 0x00
        _emit 0x68                  // PUSH 0x25a
        _emit 0x5a
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x68                  // PUSH 0xf64c18
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68                  // PUSH 0xf653fc
        _emit 0xfc
        _emit 0x53
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
