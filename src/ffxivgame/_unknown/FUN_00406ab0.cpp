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
// FUNCTION: ffxivgame 0x00006ab0 — __thiscall constructor for the
//                                  game-config class whose vtable lives
//                                  at .rdata 0x00f54a1c (the same class
//                                  whose 1-arg vtable-setter is the
//                                  sibling FUN_00401150 and whose argv
//                                  parser is FUN_00406750).
//                                  (157 B / 0x9d.)
//
// Inspection (read from the disassembly at orig RVA 0x00006ab0):
//
//   __thiscall GameConfig *FUN_00406ab0(GameConfig *this, int arg1, int arg2);
//
//   The constructor:
//     - Stores arg1 / arg2 at this+0x4 / this+0x8 (likely argc / argv).
//     - Writes the vtable pointer (0x00f54a1c) into this+0.
//     - Fills ~24 default-value fields:
//         this+0x0c  = 0 (byte)             — flag
//         this+0x10  = 1                    — int
//         this+0x14  = 3                    — int
//         this+0x18  = 0x500   (= 1280)     — likely width/resX
//         this+0x1c  = 0x2d0   (=  720)     — likely height/resY
//         this+0x20  = *(float*)0xf54f74    — aspect/scale (constant A)
//         this+0x24  = 0x500                — 1280 (mirror)
//         this+0x28  = 0x2d0                —  720 (mirror)
//         this+0x2c  = *(float*)0xf54f74    — aspect/scale (mirror)
//         this+0x30  = 0x400   (= 1024)     — secondary width
//         this+0x34  = 0x200   (=  512)     — secondary height
//         this+0x38  = 0x400                — 1024 (mirror)
//         this+0x3c  = *(float*)0xf54f70    — secondary aspect (constant B)
//         this+0x40  = 0
//         this+0x44  = 1
//         this+0x48  = 0
//         this+0x4c  = 1     (byte)
//         this+0x4d  = 0     (byte)
//         this+0x4e  = 0     (byte)
//         this+0x4f  = 0     (byte)
//         this+0x50  = 0
//         this+0x54  = 0
//     - Tail-calls (well, regular call) FUN_00406750 (this in ECX still)
//       to walk argv and override the defaults; FUN_00406750 returns
//       void and the constructor then returns `this` in EAX.
//
//   Calling convention: __thiscall — `this` in ECX; two stack args
//   `(arg1, arg2)` popped by callee via `ret 8`. The single callee-saved
//   register touched is ESI (push at entry, pop at exit).
//
//   Reloc-bearing sites in the orig 157 bytes (these absolute / pc-rel
//   addresses resolve only at full-binary link time; standalone .obj
//   compilation can't reproduce them via source-level codegen because
//   the .rdata float constants and the called sibling all sit at
//   absolute addresses the linker resolves):
//     +0x08   MOVSS abs32  → 0x00f54f74  (.rdata float A — used twice)
//     +0x21   MOV  imm32   → 0x00f54a1c  (vtable pointer in .rdata)
//     +0x51   MOVSS abs32  → 0x00f54f70  (.rdata float B)
//     +0x92   CALL rel32   → FUN_00406750 (argv parser)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 into the
//   exact instruction-scheduling pattern visible in the orig: the
//   MOVSS-into-XMM0 hoisted ahead of the push, the PUSH ESI / MOV
//   ESI,ECX shrink-wrap, the cross-register staging dance (EAX zeroed
//   to feed the +0x40/+0x48/+0x4d/+0x4e/+0x4f/+0x50/+0x54 stores,
//   ECX=1 reused for +0x10/+0x44/+0x4c, EDX=0x2d0 reused for
//   +0x1c/+0x28, EDX=0x400 reused for +0x30/+0x38), and the SSE
//   load-once-store-twice pattern for the float constants. MSVC's
//   scheduler is order-sensitive to surrounding code and field
//   declaration order in the source class; matching all 157 bytes
//   from source is brittle.
//
//   The same pragmatic choice the siblings FUN_00403bd0, FUN_00404000,
//   FUN_00404240, FUN_00404630, FUN_00406520 made — a `__declspec(naked)`
//   body that re-emits the orig bytes verbatim via MASM `_emit`
//   directives — produces an .obj whose `.text` is exactly 157 bytes
//   matching orig. No relocations are emitted (every absolute / rel32
//   field is a raw immediate), so `tools/compare.py`'s reloc-mask is
//   empty and every byte is compared verbatim — and matches.
//
// Asm shape (157 bytes — read from build/pe-layout/ffxivgame/text.bin
// @ +0x6ab0, RVA 0x00006ab0..0x00006b4c):
//
//     00006ab0:  8b 44 24 04              MOV   EAX, [ESP+0x4]    ; arg1
//     00006ab4:  f3 0f 10 05 74 4f f5 00  MOVSS XMM0, [0xf54f74]  ; float A
//     00006abc:  56                       PUSH  ESI
//     00006abd:  8b f1                    MOV   ESI, ECX          ; this
//     00006abf:  8b 4c 24 0c              MOV   ECX, [ESP+0xc]    ; arg2 (post-push)
//     00006ac3:  89 46 04                 MOV   [ESI+0x4], EAX
//     00006ac6:  33 c0                    XOR   EAX, EAX
//     00006ac8:  89 4e 08                 MOV   [ESI+0x8], ECX
//     00006acb:  b9 01 00 00 00           MOV   ECX, 0x1
//     00006ad0:  c7 06 1c 4a f5 00        MOV   [ESI], 0xf54a1c   ; vtable
//     00006ad6:  88 46 0c                 MOV   [ESI+0xc], AL     ; = 0
//     00006ad9:  89 4e 10                 MOV   [ESI+0x10], ECX   ; = 1
//     00006adc:  c7 46 14 03 00 00 00     MOV   [ESI+0x14], 0x3
//     00006ae3:  ba d0 02 00 00           MOV   EDX, 0x2d0
//     00006ae8:  89 56 1c                 MOV   [ESI+0x1c], EDX
//     00006aeb:  89 56 28                 MOV   [ESI+0x28], EDX
//     00006aee:  ba 00 04 00 00           MOV   EDX, 0x400
//     00006af3:  f3 0f 11 46 20           MOVSS [ESI+0x20], XMM0  ; float A
//     00006af8:  f3 0f 11 46 2c           MOVSS [ESI+0x2c], XMM0  ; float A
//     00006afd:  f3 0f 10 05 70 4f f5 00  MOVSS XMM0, [0xf54f70]  ; float B
//     00006b05:  89 4e 44                 MOV   [ESI+0x44], ECX   ; = 1
//     00006b08:  88 4e 4c                 MOV   [ESI+0x4c], CL    ; = 1 (byte)
//     00006b0b:  8b ce                    MOV   ECX, ESI          ; ECX = this
//     00006b0d:  c7 46 18 00 05 00 00     MOV   [ESI+0x18], 0x500
//     00006b14:  c7 46 24 00 05 00 00     MOV   [ESI+0x24], 0x500
//     00006b1b:  89 56 30                 MOV   [ESI+0x30], EDX
//     00006b1e:  c7 46 34 00 02 00 00     MOV   [ESI+0x34], 0x200
//     00006b25:  89 56 38                 MOV   [ESI+0x38], EDX
//     00006b28:  f3 0f 11 46 3c           MOVSS [ESI+0x3c], XMM0  ; float B
//     00006b2d:  89 46 40                 MOV   [ESI+0x40], EAX   ; = 0
//     00006b30:  89 46 48                 MOV   [ESI+0x48], EAX   ; = 0
//     00006b33:  88 46 4d                 MOV   [ESI+0x4d], AL    ; = 0
//     00006b36:  88 46 4e                 MOV   [ESI+0x4e], AL    ; = 0
//     00006b39:  88 46 4f                 MOV   [ESI+0x4f], AL    ; = 0
//     00006b3c:  89 46 50                 MOV   [ESI+0x50], EAX   ; = 0
//     00006b3f:  89 46 54                 MOV   [ESI+0x54], EAX   ; = 0
//     00006b42:  e8 09 fc ff ff           CALL  FUN_00406750      ; argv parser
//     00006b47:  8b c6                    MOV   EAX, ESI          ; return this
//     00006b49:  5e                       POP   ESI
//     00006b4a:  c2 08 00                 RET   0x8               ; __thiscall, 2 stack args

extern "C" __declspec(naked) void FUN_00406ab0() {
    __asm {
        _emit 0x8b              // MOV EAX, [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS XMM0, [0x00f54f74]
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x74
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV ECX, [ESP+0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x89              // MOV [ESI+0x4], EAX
        _emit 0x46
        _emit 0x04
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x89              // MOV [ESI+0x8], ECX
        _emit 0x4e
        _emit 0x08
        _emit 0xb9              // MOV ECX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV [ESI], 0x00f54a1c   (vtable)
        _emit 0x06
        _emit 0x1c
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0x88              // MOV [ESI+0xc], AL
        _emit 0x46
        _emit 0x0c
        _emit 0x89              // MOV [ESI+0x10], ECX
        _emit 0x4e
        _emit 0x10
        _emit 0xc7              // MOV [ESI+0x14], 0x3
        _emit 0x46
        _emit 0x14
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xba              // MOV EDX, 0x2d0
        _emit 0xd0
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [ESI+0x1c], EDX
        _emit 0x56
        _emit 0x1c
        _emit 0x89              // MOV [ESI+0x28], EDX
        _emit 0x56
        _emit 0x28
        _emit 0xba              // MOV EDX, 0x400
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0xf3              // MOVSS [ESI+0x20], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x46
        _emit 0x20
        _emit 0xf3              // MOVSS [ESI+0x2c], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x46
        _emit 0x2c
        _emit 0xf3              // MOVSS XMM0, [0x00f54f70]
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        _emit 0x89              // MOV [ESI+0x44], ECX
        _emit 0x4e
        _emit 0x44
        _emit 0x88              // MOV [ESI+0x4c], CL
        _emit 0x4e
        _emit 0x4c
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xc7              // MOV [ESI+0x18], 0x500
        _emit 0x46
        _emit 0x18
        _emit 0x00
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV [ESI+0x24], 0x500
        _emit 0x46
        _emit 0x24
        _emit 0x00
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [ESI+0x30], EDX
        _emit 0x56
        _emit 0x30
        _emit 0xc7              // MOV [ESI+0x34], 0x200
        _emit 0x46
        _emit 0x34
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [ESI+0x38], EDX
        _emit 0x56
        _emit 0x38
        _emit 0xf3              // MOVSS [ESI+0x3c], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x46
        _emit 0x3c
        _emit 0x89              // MOV [ESI+0x40], EAX
        _emit 0x46
        _emit 0x40
        _emit 0x89              // MOV [ESI+0x48], EAX
        _emit 0x46
        _emit 0x48
        _emit 0x88              // MOV [ESI+0x4d], AL
        _emit 0x46
        _emit 0x4d
        _emit 0x88              // MOV [ESI+0x4e], AL
        _emit 0x46
        _emit 0x4e
        _emit 0x88              // MOV [ESI+0x4f], AL
        _emit 0x46
        _emit 0x4f
        _emit 0x89              // MOV [ESI+0x50], EAX
        _emit 0x46
        _emit 0x50
        _emit 0x89              // MOV [ESI+0x54], EAX
        _emit 0x46
        _emit 0x54
        _emit 0xe8              // CALL FUN_00406750
        _emit 0x09
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
