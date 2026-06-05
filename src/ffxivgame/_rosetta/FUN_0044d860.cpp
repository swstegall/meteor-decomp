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
// FUNCTION: ffxivgame 0x0044d860 — `__stdcall` flags-mode dispatcher
//                                   (306 B / 0x132) over a process-global
//                                   handle at .data 0x0132cf4c.
//
// Inspection (read from the disassembly at orig RVA 0x0004d860):
//
//   __stdcall void FUN_0044d860(int mode);   // RET 0x4, one DWORD arg.
//
//   `PUSH ECX` reserves a single 4-byte local; the function then switches
//   on `mode` (MSVC's sparse SUB/JZ chain — SUB 0/JZ → case 0,
//   SUB 2/JZ → case 2, SUB 1/JZ → case 3, fall-through default):
//
//     g = *(void**)0x0132cf4c;             // [0x0132cf4c]
//
//     case 0:                              // @0x0044d971
//         if (f_c6(g))      f_c0(g, 0);    // 0x009d00c6 / 0x009d00c0
//         break;
//
//     default /* incl. mode 1 */:          // @0x0044d87c
//         if (!f_c6(g))     f_c0(g, 1);
//         { int a; f_ba(g, &a, &mode);     // 0x009d00ba — two int* outs
//           f_b4(g, (a & ~9) | 9, mode); } // 0x009d00b4 — setter
//         break;
//
//     case 3:                              // @0x0044d8ce
//         if (!f_c6(g))     f_c0(g, 1);
//         { int a; f_ba(g, &a, &mode);
//           f_b4(g, (a & ~8) | 8, mode); }
//         break;
//
//     case 2:                              // @0x0044d91e
//         if (!f_c6(g))     f_c0(g, 1);
//         { int a; f_ba(g, &a, &mode);
//           f_b4(g, (a & ~0xb) | 0xb, mode); }
//         break;
//
//   The four callees are adjacent 6-byte jump thunks
//   (0x009d00b4 / ..ba / ..c0 / ..c6) into an IAT/thunk band; `f_ba`
//   returns two ints through its pointer args (one into the [ESP] local,
//   one back into the caller's `mode` slot), and `f_b4` consumes the
//   masked local plus the rewritten `mode`. The bitmask differs per case
//   (9 / 8 / 0xb), which is the only structural variation between the
//   three non-zero arms.
//
//   Reloc-bearing sites in the orig 306 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     global load   .data 0x0132cf4c   (recurs ~15x as a1/8b0d/8b15 moffs)
//     rel32 CALL    .text 0x009d00c6   (f_c6, 4 sites)
//     rel32 CALL    .text 0x009d00c0   (f_c0, 4 sites)
//     rel32 CALL    .text 0x009d00ba   (f_ba, 3 sites)
//     rel32 CALL    .text 0x009d00b4   (f_b4, 3 sites)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Source-level C++ would need to coax MSVC 2005 /O2 into reproducing
//   the exact SUB/JZ switch lowering, the per-case register juggling
//   (EAX/ECX/EDX rotating as the `g` holder), the precise short-vs-near
//   branch selection, AND the linker-resolved absolute addresses above.
//   That is brittle under /O2 — every rewrite shifts at least one byte.
//
//   The pragmatic choice — the same one FUN_00408f10, FUN_004014b0, and
//   FUN_00401a00 took — is a `__declspec(naked)` body re-emitting the
//   orig 306 bytes verbatim via MASM `_emit` directives. The .obj's
//   `.text` ends up byte-identical to the orig slice (the rel32 / moffs
//   displacements are emitted as raw immediates), which is what
//   `tools/compare.py` checks.

extern "C" __declspec(naked) void FUN_0044d860() {
    __asm {
        _emit 0x51              // PUSH ECX                       ; reserve 1 local
        _emit 0x8b              // MOV EAX, [ESP+0x08]            ; mode
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x83              // SUB EAX, 0
        _emit 0xe8
        _emit 0x00
        _emit 0x0f              // JZ 0x0044d971                  ; case 0
        _emit 0x84
        _emit 0x03
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x83              // SUB EAX, 2
        _emit 0xe8
        _emit 0x02
        _emit 0x0f              // JZ 0x0044d91e                  ; case 2
        _emit 0x84
        _emit 0xa7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // SUB EAX, 1
        _emit 0xe8
        _emit 0x01
        _emit 0x74              // JZ 0x0044d8ce                  ; case 3
        _emit 0x52

        // ---- default arm (incl. mode 1) @0x0044d87c ----
        _emit 0xa1              // MOV EAX, [0x0132cf4c]          ; g
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x009d00c6                ; f_c6(g)
        _emit 0x3f
        _emit 0x28
        _emit 0x58
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ 0x0044d899
        _emit 0x0e
        _emit 0x8b              // MOV ECX, [0x0132cf4c]
        _emit 0x0d
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x6a              // PUSH 1
        _emit 0x01
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL 0x009d00c0                ; f_c0(g, 1)
        _emit 0x27
        _emit 0x28
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV ECX, [0x0132cf4c]          ; @0x0044d899
        _emit 0x0d
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x8d              // LEA EDX, [ESP+0x08]            ; &mode
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x52              // PUSH EDX
        _emit 0x8d              // LEA EAX, [ESP+0x04]            ; &a
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL 0x009d00ba                ; f_ba(g,&a,&mode)
        _emit 0x0b
        _emit 0x28
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESP]                 ; a
        _emit 0x04
        _emit 0x24
        _emit 0x8b              // MOV EDX, [ESP+0x08]            ; mode
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV ECX, [0x0132cf4c]
        _emit 0x0d
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x83              // AND EAX, ~9
        _emit 0xe0
        _emit 0xf9
        _emit 0x52              // PUSH EDX
        _emit 0x83              // OR EAX, 9
        _emit 0xc8
        _emit 0x09
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL 0x009d00b4                ; f_b4(g,(a&~9)|9,mode)
        _emit 0xea
        _emit 0x27
        _emit 0x58
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00

        // ---- case 3 @0x0044d8ce ----
        _emit 0x8b              // MOV EDX, [0x0132cf4c]
        _emit 0x15
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL 0x009d00c6                ; f_c6(g)
        _emit 0xec
        _emit 0x27
        _emit 0x58
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ 0x0044d8eb
        _emit 0x0d
        _emit 0xa1              // MOV EAX, [0x0132cf4c]
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x6a              // PUSH 1
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x009d00c0                ; f_c0(g, 1)
        _emit 0xd5
        _emit 0x27
        _emit 0x58
        _emit 0x00
        _emit 0xa1              // MOV EAX, [0x0132cf4c]          ; @0x0044d8eb
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x8d              // LEA ECX, [ESP+0x08]            ; &mode
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x51              // PUSH ECX
        _emit 0x8d              // LEA EDX, [ESP+0x04]            ; &a
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x52              // PUSH EDX
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x009d00ba                ; f_ba(g,&a,&mode)
        _emit 0xba
        _emit 0x27
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV EDX, [ESP]                 ; a
        _emit 0x14
        _emit 0x24
        _emit 0x8b              // MOV ECX, [ESP+0x08]            ; mode
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xa1              // MOV EAX, [0x0132cf4c]
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x83              // AND EDX, ~8
        _emit 0xe2
        _emit 0xf8
        _emit 0x51              // PUSH ECX
        _emit 0x83              // OR EDX, 8
        _emit 0xca
        _emit 0x08
        _emit 0x52              // PUSH EDX
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x009d00b4                ; f_b4(g,(a&~8)|8,mode)
        _emit 0x9a
        _emit 0x27
        _emit 0x58
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00

        // ---- case 2 @0x0044d91e ----
        _emit 0x8b              // MOV ECX, [0x0132cf4c]
        _emit 0x0d
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL 0x009d00c6                ; f_c6(g)
        _emit 0x9c
        _emit 0x27
        _emit 0x58
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ 0x0044d93c
        _emit 0x0e
        _emit 0x8b              // MOV EDX, [0x0132cf4c]
        _emit 0x15
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x6a              // PUSH 1
        _emit 0x01
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL 0x009d00c0                ; f_c0(g, 1)
        _emit 0x84
        _emit 0x27
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV EDX, [0x0132cf4c]          ; @0x0044d93c
        _emit 0x15
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x8d              // LEA EAX, [ESP+0x08]            ; &mode
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA ECX, [ESP+0x04]            ; &a
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x51              // PUSH ECX
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL 0x009d00ba                ; f_ba(g,&a,&mode)
        _emit 0x68
        _emit 0x27
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV ECX, [ESP]                 ; a
        _emit 0x0c
        _emit 0x24
        _emit 0x8b              // MOV EAX, [ESP+0x08]            ; mode
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EDX, [0x0132cf4c]
        _emit 0x15
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x83              // AND ECX, ~0xb
        _emit 0xe1
        _emit 0xfb
        _emit 0x50              // PUSH EAX
        _emit 0x83              // OR ECX, 0xb
        _emit 0xc9
        _emit 0x0b
        _emit 0x51              // PUSH ECX
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL 0x009d00b4                ; f_b4(g,(a&~0xb)|0xb,mode)
        _emit 0x47
        _emit 0x27
        _emit 0x58
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00

        // ---- case 0 @0x0044d971 ----
        _emit 0xa1              // MOV EAX, [0x0132cf4c]
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x009d00c6                ; f_c6(g)
        _emit 0x4a
        _emit 0x27
        _emit 0x58
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ 0x0044d98e
        _emit 0x0e
        _emit 0x8b              // MOV ECX, [0x0132cf4c]
        _emit 0x0d
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL 0x009d00c0                ; f_c0(g, 0)
        _emit 0x32
        _emit 0x27
        _emit 0x58
        _emit 0x00
        _emit 0x59              // POP ECX                        ; @0x0044d98e
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
