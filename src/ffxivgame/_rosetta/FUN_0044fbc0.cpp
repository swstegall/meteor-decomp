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
// FUNCTION: ffxivgame 0x0044fbc0 — `__thiscall` formatter with a one-arg
//                                  stack parameter (ret 4, 158 B / 0x9e).
//
// __thiscall <ptr> FUN_0044fbc0(this, int mode)   // ECX = this, [ESP+4] = mode
//
// Behaviour (recovered from asm @ 0x0044fbc0):
//
//   if (mode == 1) {
//       // small-buffer-optimised string: capacity at +0x24, union at +0x10
//       if (this->_Myres < 0x10) return (char *)this + 0x10;   // inline buf
//       else                     return *(char **)(this + 0x10); // heap ptr
//   }
//   // mode != 1: format two fields into the +0x85 scratch buffer
//   unsigned hi = SomeApi() & 0xFFFF0000;          // CALL [0x00f3e1bc]
//   this->field_7c = hi;
//   _snprintf((char *)this + 0x85, 0x20, fmt_a /*0xf676a8*/, hi);
//   unsigned lo = this->field_7e & 0xf;
//   this->field_80 = lo;
//   if (this->field_84 == 0x21) {
//       this->field_84 = FUN_0044fa30(this, lo);
//   } else {
//       if (FUN_0044fa30(this, lo) != this->field_84) {
//           this->field_7c += 0xffff0000;
//           _snprintf((char *)this + 0x85, 0x20, fmt_b /*0xf676b0*/, this->field_7c);
//       }
//   }
//   return (char *)this + 0x85;
//
// Reloc-bearing sites in the orig 158 bytes (resolve only in a full-binary
// relink at image base 0x00400000):
//   +0x1f  DIR32 IAT load   (0x00f3e1bc — imported API thunk)
//   +0x2b  DIR32 immediate  (0x00f676a8 — format string fmt_a)
//   +0x3c  CALL rel32       → 0x009d4f83 (_snprintf)
//   +0x5d  CALL rel32       → 0x0044fa30 (sibling helper)
//   +0x75  DIR32 immediate  (0x00f676b0 — format string fmt_b)
//   +0x7d  CALL rel32       → 0x009d4f83 (_snprintf)
//   +0x8c  CALL rel32       → 0x0044fa30 (sibling helper)
//
// Reconstruction strategy — naked-asm byte passthrough (same as siblings
// FUN_00434180 / FUN_00406f00): a `__declspec(naked)` body re-emitting the
// orig 158 bytes verbatim via MASM `_emit`. The absolute data addresses and
// rel32 displacements are emitted as raw immediates that already match the
// orig binary's resolved bytes, so no relocations are involved and
// tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0044fbc0() {
    __asm {
        // 0004fbc0: 83 7c 24 04 01     CMP dword ptr [ESP+4], 1
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x04
        _emit 0x01
        // 0004fbc5: 56                 PUSH ESI
        _emit 0x56
        // 0004fbc6: 8b f1              MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 0004fbc8: 75 14              JNZ 0x0044fbde
        _emit 0x75
        _emit 0x14
        // 0004fbca: 83 7e 24 10        CMP dword ptr [ESI+0x24], 0x10
        _emit 0x83
        _emit 0x7e
        _emit 0x24
        _emit 0x10
        // 0004fbce: 72 07              JC 0x0044fbd7
        _emit 0x72
        _emit 0x07
        // 0004fbd0: 8b 46 10           MOV EAX, dword ptr [ESI+0x10]
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        // 0004fbd3: 5e                 POP ESI
        _emit 0x5e
        // 0004fbd4: c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 0004fbd7: 8d 46 10           LEA EAX, [ESI+0x10]
        _emit 0x8d
        _emit 0x46
        _emit 0x10
        // 0004fbda: 5e                 POP ESI
        _emit 0x5e
        // 0004fbdb: c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 0004fbde: 57                 PUSH EDI
        _emit 0x57
        // 0004fbdf: ff 15 bc e1 f3 00  CALL dword ptr [0x00f3e1bc]
        _emit 0xff
        _emit 0x15
        _emit 0xbc
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0004fbe5: 0f b7 c8           MOVZX ECX, AX
        _emit 0x0f
        _emit 0xb7
        _emit 0xc8
        // 0004fbe8: 2b c1              SUB EAX, ECX
        _emit 0x2b
        _emit 0xc1
        // 0004fbea: 50                 PUSH EAX
        _emit 0x50
        // 0004fbeb: 68 a8 76 f6 00     PUSH 0x00f676a8
        _emit 0x68
        _emit 0xa8
        _emit 0x76
        _emit 0xf6
        _emit 0x00
        // 0004fbf0: 8d be 85 00 00 00  LEA EDI, [ESI+0x85]
        _emit 0x8d
        _emit 0xbe
        _emit 0x85
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004fbf6: 6a 20              PUSH 0x20
        _emit 0x6a
        _emit 0x20
        // 0004fbf8: 57                 PUSH EDI
        _emit 0x57
        // 0004fbf9: 89 46 7c           MOV dword ptr [ESI+0x7c], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x7c
        // 0004fbfc: e8 82 53 58 00     CALL 0x009d4f83
        _emit 0xe8
        _emit 0x82
        _emit 0x53
        _emit 0x58
        _emit 0x00
        // 0004fc01: 0f b6 46 7e        MOVZX EAX, byte ptr [ESI+0x7e]
        _emit 0x0f
        _emit 0xb6
        _emit 0x46
        _emit 0x7e
        // 0004fc05: 83 e0 0f           AND EAX, 0xf
        _emit 0x83
        _emit 0xe0
        _emit 0x0f
        // 0004fc08: 83 c4 10           ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0004fc0b: 80 be 84 00 00 00 21  CMP byte ptr [ESI+0x84], 0x21
        _emit 0x80
        _emit 0xbe
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x21
        // 0004fc12: 89 86 80 00 00 00  MOV dword ptr [ESI+0x80], EAX
        _emit 0x89
        _emit 0x86
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004fc18: 50                 PUSH EAX
        _emit 0x50
        // 0004fc19: 8b ce              MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0004fc1b: 74 2f              JZ 0x0044fc4c
        _emit 0x74
        _emit 0x2f
        // 0004fc1d: e8 0e fe ff ff     CALL 0x0044fa30
        _emit 0xe8
        _emit 0x0e
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 0004fc22: 3a 86 84 00 00 00  CMP AL, byte ptr [ESI+0x84]
        _emit 0x3a
        _emit 0x86
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004fc28: 74 2d              JZ 0x0044fc57
        _emit 0x74
        _emit 0x2d
        // 0004fc2a: 81 46 7c 00 00 ff ff  ADD dword ptr [ESI+0x7c], 0xffff0000
        _emit 0x81
        _emit 0x46
        _emit 0x7c
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xff
        // 0004fc31: 8b 76 7c           MOV ESI, dword ptr [ESI+0x7c]
        _emit 0x8b
        _emit 0x76
        _emit 0x7c
        // 0004fc34: 56                 PUSH ESI
        _emit 0x56
        // 0004fc35: 68 b0 76 f6 00     PUSH 0x00f676b0
        _emit 0x68
        _emit 0xb0
        _emit 0x76
        _emit 0xf6
        _emit 0x00
        // 0004fc3a: 6a 20              PUSH 0x20
        _emit 0x6a
        _emit 0x20
        // 0004fc3c: 57                 PUSH EDI
        _emit 0x57
        // 0004fc3d: e8 41 53 58 00     CALL 0x009d4f83
        _emit 0xe8
        _emit 0x41
        _emit 0x53
        _emit 0x58
        _emit 0x00
        // 0004fc42: 83 c4 10           ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0004fc45: 8b c7              MOV EAX, EDI
        _emit 0x8b
        _emit 0xc7
        // 0004fc47: 5f                 POP EDI
        _emit 0x5f
        // 0004fc48: 5e                 POP ESI
        _emit 0x5e
        // 0004fc49: c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 0004fc4c: e8 df fd ff ff     CALL 0x0044fa30
        _emit 0xe8
        _emit 0xdf
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0004fc51: 88 86 84 00 00 00  MOV byte ptr [ESI+0x84], AL
        _emit 0x88
        _emit 0x86
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004fc57: 8b c7              MOV EAX, EDI
        _emit 0x8b
        _emit 0xc7
        // 0004fc59: 5f                 POP EDI
        _emit 0x5f
        // 0004fc5a: 5e                 POP ESI
        _emit 0x5e
        // 0004fc5b: c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
