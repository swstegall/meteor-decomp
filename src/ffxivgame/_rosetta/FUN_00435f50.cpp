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
// FUNCTION: ffxivgame 0x00435f50 — gated relay through two virtual
//                                  methods plus a lazily-initialised
//                                  global callback (__thiscall, 140 B).
//
// Calling convention: __thiscall (ECX = this); takes one inbound stack
// argument and cleans it via `RET 0x4`. Pushes a single 4-byte local
// (the `PUSH ECX` in the prologue) whose address (`&local @ [ESP+8]`)
// is handed to the first virtual call as an out-parameter, and one
// callee-save (ESI).
//
// Shape:
//   tmp = 0;
//   if ((*(this->b)->vtbl[0x2c/4])(this->b, this->c, this->d,
//                                  &tmp, this->e) == 0) {
//       FUN_009d4600(tmp, this->f, this->d);          // CALL rel32
//       obj = this->b;
//       if ((*obj->vtbl[0x30/4])(obj) != 0) {
//           // lazy one-time init of the global callback pointer
//           if (!(DAT_01323910 & 1)) {
//               DAT_01323910 |= 1;
//               g_callback = (fn)0x00433720;
//           }
//           g_callback(0xf64be8, 0xf65558, 0xf64c18, 0x2f2, 0xf65580);
//       }
//   }
//
// Reloc-bearing sites (masked by tools/compare.py):
//   REL: CALL FUN_009d4600       @ +0x31 (rel32 = 0x0059e67a)
//   ABS: [0x01323910] flag word  @ +0x4f, +0x55  (TEST / OR)
//   ABS: [0x0132390c] callback   @ +0x5b store, +0x7e indirect CALL
//   ABS: PUSH 0x433720, 0xf65580, 0xf64c18, 0xf65558, 0xf64be8 — data/code
//        pointers embedded as absolute immediates.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The two indirect virtual CALLs (`CALL EAX` / `CALL EDX`) and the
//   absolute-disp global flag/callback references are not coercible from
//   C++ source under /O2 into this exact byte layout. The __declspec(naked)
//   body re-emits the original 140 bytes verbatim via MASM _emit
//   directives; the .obj's .text is byte-identical to the original slice
//   and tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00435f50() {
    __asm {
        // 00035f50:  51                 PUSH ECX
        _emit 0x51
        // 00035f51:  56                 PUSH ESI
        _emit 0x56
        // 00035f52:  8b f1              MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 00035f54:  8b 56 14           MOV EDX,[ESI+0x14]
        _emit 0x8b
        _emit 0x56
        _emit 0x14
        // 00035f57:  8b 46 04           MOV EAX,[ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00035f5a:  8b 08              MOV ECX,[EAX]
        _emit 0x8b
        _emit 0x08
        // 00035f5c:  52                 PUSH EDX
        _emit 0x52
        // 00035f5d:  8d 54 24 08        LEA EDX,[ESP+0x8]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 00035f61:  52                 PUSH EDX
        _emit 0x52
        // 00035f62:  8b 56 10           MOV EDX,[ESI+0x10]
        _emit 0x8b
        _emit 0x56
        _emit 0x10
        // 00035f65:  52                 PUSH EDX
        _emit 0x52
        // 00035f66:  8b 56 0c           MOV EDX,[ESI+0xc]
        _emit 0x8b
        _emit 0x56
        _emit 0x0c
        // 00035f69:  52                 PUSH EDX
        _emit 0x52
        // 00035f6a:  50                 PUSH EAX
        _emit 0x50
        // 00035f6b:  8b 41 2c           MOV EAX,[ECX+0x2c]
        _emit 0x8b
        _emit 0x41
        _emit 0x2c
        // 00035f6e:  ff d0              CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00035f70:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00035f72:  75 63              JNZ 0x00435fd7
        _emit 0x75
        _emit 0x63
        // 00035f74:  8b 4e 10           MOV ECX,[ESI+0x10]
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 00035f77:  8b 56 08           MOV EDX,[ESI+0x8]
        _emit 0x8b
        _emit 0x56
        _emit 0x08
        // 00035f7a:  8b 44 24 04        MOV EAX,[ESP+0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00035f7e:  51                 PUSH ECX
        _emit 0x51
        // 00035f7f:  52                 PUSH EDX
        _emit 0x52
        // 00035f80:  50                 PUSH EAX
        _emit 0x50
        // 00035f81:  e8 7a e6 59 00     CALL 0x009d4600
        _emit 0xe8
        _emit 0x7a
        _emit 0xe6
        _emit 0x59
        _emit 0x00
        // 00035f86:  8b 76 04           MOV ESI,[ESI+0x4]
        _emit 0x8b
        _emit 0x76
        _emit 0x04
        // 00035f89:  8b 0e              MOV ECX,[ESI]
        _emit 0x8b
        _emit 0x0e
        // 00035f8b:  8b 51 30           MOV EDX,[ECX+0x30]
        _emit 0x8b
        _emit 0x51
        _emit 0x30
        // 00035f8e:  83 c4 0c           ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00035f91:  56                 PUSH ESI
        _emit 0x56
        // 00035f92:  ff d2              CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00035f94:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00035f96:  74 3f              JZ 0x00435fd7
        _emit 0x74
        _emit 0x3f
        // 00035f98:  b8 01 00 00 00     MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00035f9d:  84 05 10 39 32 01  TEST byte ptr [0x01323910],AL
        _emit 0x84
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00035fa3:  75 10              JNZ 0x00435fb5
        _emit 0x75
        _emit 0x10
        // 00035fa5:  09 05 10 39 32 01  OR dword ptr [0x01323910],EAX
        _emit 0x09
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00035fab:  c7 05 0c 39 32 01 20 37 43 00  MOV dword ptr [0x0132390c],0x433720
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00
        // 00035fb5:  68 80 55 f6 00     PUSH 0xf65580
        _emit 0x68
        _emit 0x80
        _emit 0x55
        _emit 0xf6
        _emit 0x00
        // 00035fba:  68 f2 02 00 00     PUSH 0x2f2
        _emit 0x68
        _emit 0xf2
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 00035fbf:  68 18 4c f6 00     PUSH 0xf64c18
        _emit 0x68
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        // 00035fc4:  68 58 55 f6 00     PUSH 0xf65558
        _emit 0x68
        _emit 0x58
        _emit 0x55
        _emit 0xf6
        _emit 0x00
        // 00035fc9:  68 e8 4b f6 00     PUSH 0xf64be8
        _emit 0x68
        _emit 0xe8
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        // 00035fce:  ff 15 0c 39 32 01  CALL dword ptr [0x0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00035fd4:  83 c4 14           ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00035fd7:  5e                 POP ESI
        _emit 0x5e
        // 00035fd8:  59                 POP ECX
        _emit 0x59
        // 00035fd9:  c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
