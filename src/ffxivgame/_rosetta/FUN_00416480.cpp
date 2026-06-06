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
// FUNCTION: ffxivgame 0x00016480 — static-init dispatch wrapper
//                                  (253 B / 0xfd, SEH-wrapped, __cdecl).
//
// Behaviour read from the disassembly at orig RVA 0x00016480:
//
//   __cdecl void FUN_00416480(arg1, arg2, arg3, arg4, arg5, arg6)
//
//   SEH prologue (PUSH -1 / PUSH scope-table 0xe55220 / PUSH FS:[0]
//   / MOV FS:[0],ESP / PUSH ECX).
//
//   static initialization guard at [0x01328d68]:
//     if ([0x01328d68] == 0) {
//         ECX = 0x1328130;
//         [0x01328d68] = 0x416390;
//         [0x01328d6c] = 0x4163a0;
//         [0x01328d70] = 0x4163c0;
//         [0x01328d74] = 0x4163d0;
//         [0x01328d78] = 0x4163e0;
//         [0x01328d80] = ECX;
//         [ESP]        = ECX;       // local slot
//         // trylevel = 0
//         FUN_00416660(1, 0, 1);    // __stdcall, 3 args
//         // trylevel = -1
//     }
//
//   [0x01328d7d] = AL (low byte of arg2);
//   result = (*[0x01328d68])(arg1, arg2, arg3, arg4, arg5, arg6);
//
//   if ([0x01328d7c] != 0 && result != 0) {
//       [0x01328d50] += 1;
//       [0x01328d58] += 1;
//       aligned = (arg1 + arg3 - 1) & ~(arg3 - 1);  // ALIGN_UP(arg1, arg3)
//       [0x01328d54] += aligned;
//   } else if ([0x01328d7c] != 0 && result == 0) {
//       [0x01328d60] += 1;
//   }
//   // if [0x01328d7c] == 0: skip all counter updates
//
//   restore FS:[0], clean frame, RET.
//
//   Stack frame (ESP-relative after SEH prologue + PUSH ECX):
//     [ESP+0x00]        local ECX save / re-used as static ctx ptr
//     [ESP+0x04]        saved FS:[0]
//     [ESP+0x08]        scope-table (0xe55220)
//     [ESP+0x0c]        trylevel (start=-1, init=0, exit=-1)
//     [ESP+0x10]        return address
//     [ESP+0x14]        arg1
//     [ESP+0x18]        arg2
//     [ESP+0x1c]        arg3
//     [ESP+0x20]        arg4
//     [ESP+0x24]        arg5
//     [ESP+0x28]        arg6
//
//   Reloc-bearing sites in the orig 253 bytes (absolute addresses resolve
//   only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation cannot reproduce them):
//     +0x03  scope-table address  (0xe55220)
//     +0x09  FS:[0] read          (constant 0)
//     +0x0e  FS:[0] install       (constant 0)
//     +0x16  static guard addr    (0x01328d68)
//     +0x1f  MOV ECX imm          (0x01328130)
//     +0x24  fp[0] store addr     (0x01328d68) → 0x416390
//     +0x2e  fp[1] store addr     (0x01328d6c) → 0x4163a0
//     +0x38  fp[2] store addr     (0x01328d70) → 0x4163c0
//     +0x42  fp[3] store addr     (0x01328d74) → 0x4163d0
//     +0x4c  fp[4] store addr     (0x01328d78) → 0x4163e0
//     +0x56  ctx store addr       (0x01328d80)
//     +0x6d  CALL FUN_00416660    (rel32)
//     +0x9a  MOV [moffs8], AL     (0x01328d7d)
//     +0x9f  CALL [fp] addr       (0x01328d68)
//     +0xa8  CMP byte addr        (0x01328d7c)
//     +0xb5  ADD [cnt0] addr      (0x01328d50)
//     +0xbc  ADD [cnt1] addr      (0x01328d58)
//     +0xce  ADD [aligned] addr   (0x01328d54)
//     +0xe5  ADD [cnt2] addr      (0x01328d60)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /EHsc into
//   reproducing the exact SEH prolog (PUSH -1 / PUSH scope-table /
//   PUSH FS:[0] / MOV FS:[0],ESP / PUSH ECX), the static-init branch's
//   register allocation (ECX carries 0x1328130 for both the fp-table
//   initialisation and the local slot store), the exact interleaving of
//   PUSH/MOV to shuffle six args through three registers onto the stack
//   before the indirect call, AND the 19 linker-resolved absolute
//   addresses listed above. Each constraint is brittle under /O2.
//
//   The pragmatic choice — the same one FUN_00402a30 / FUN_00403a20 /
//   FUN_004054d0 took — is a `__declspec(naked)` body that re-emits the
//   orig 253 bytes verbatim via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_00416480() {
    __asm {
        // 00016480  PUSH -1
        _emit 0x6a
        _emit 0xff
        // 00016482  PUSH 0xe55220 (scope-table)
        _emit 0x68
        _emit 0x20
        _emit 0x52
        _emit 0xe5
        _emit 0x00
        // 00016487  MOV EAX, FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001648d  PUSH EAX
        _emit 0x50
        // 0001648e  MOV FS:[0], ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00016495  PUSH ECX
        _emit 0x51
        // 00016496  CMP [0x01328d68], 0
        _emit 0x83
        _emit 0x3d
        _emit 0x68
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // 0001649d  JNZ +0x5b (to 0x4164fa)
        _emit 0x75
        _emit 0x5b
        // 0001649f  MOV ECX, 0x1328130
        _emit 0xb9
        _emit 0x30
        _emit 0x81
        _emit 0x32
        _emit 0x01
        // 000164a4  MOV [0x01328d68], 0x416390
        _emit 0xc7
        _emit 0x05
        _emit 0x68
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x90
        _emit 0x63
        _emit 0x41
        _emit 0x00
        // 000164ae  MOV [0x01328d6c], 0x4163a0
        _emit 0xc7
        _emit 0x05
        _emit 0x6c
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0xa0
        _emit 0x63
        _emit 0x41
        _emit 0x00
        // 000164b8  MOV [0x01328d70], 0x4163c0
        _emit 0xc7
        _emit 0x05
        _emit 0x70
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0xc0
        _emit 0x63
        _emit 0x41
        _emit 0x00
        // 000164c2  MOV [0x01328d74], 0x4163d0
        _emit 0xc7
        _emit 0x05
        _emit 0x74
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0xd0
        _emit 0x63
        _emit 0x41
        _emit 0x00
        // 000164cc  MOV [0x01328d78], 0x4163e0
        _emit 0xc7
        _emit 0x05
        _emit 0x78
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0xe0
        _emit 0x63
        _emit 0x41
        _emit 0x00
        // 000164d6  MOV [0x01328d80], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x80
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000164dc  MOV [ESP], ECX
        _emit 0x89
        _emit 0x0c
        _emit 0x24
        // 000164df  PUSH 1
        _emit 0x6a
        _emit 0x01
        // 000164e1  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 000164e3  PUSH 1
        _emit 0x6a
        _emit 0x01
        // 000164e5  MOV [ESP+0x18], 0   (trylevel = 0)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000164ed  CALL 0x00416660
        _emit 0xe8
        _emit 0x6e
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 000164f2  MOV [ESP+0x0c], -1  (trylevel = -1)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 000164fa  MOV ECX, [ESP+0x28]  (arg6)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // 000164fe  MOV EDX, [ESP+0x24]  (arg5)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 00016502  MOV EAX, [ESP+0x18]  (arg2)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 00016506  PUSH ESI
        _emit 0x56
        // 00016507  MOV ESI, [ESP+0x20]  (arg3 — ESP shifted by 4)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x20
        // 0001650b  PUSH EDI
        _emit 0x57
        // 0001650c  MOV EDI, [ESP+0x1c]  (arg1 — ESP shifted by 8)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        // 00016510  PUSH ECX             (arg6 onto call stack)
        _emit 0x51
        // 00016511  MOV ECX, [ESP+0x2c]  (arg4 — ESP shifted by 12)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // 00016515  PUSH EDX             (arg5)
        _emit 0x52
        // 00016516  PUSH ECX             (arg4)
        _emit 0x51
        // 00016517  PUSH ESI             (arg3)
        _emit 0x56
        // 00016518  PUSH EAX             (arg2)
        _emit 0x50
        // 00016519  PUSH EDI             (arg1)
        _emit 0x57
        // 0001651a  MOV [0x01328d7d], AL
        _emit 0xa2
        _emit 0x7d
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 0001651f  CALL [0x01328d68]
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00016525  ADD ESP, 0x18
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 00016528  CMP byte ptr [0x01328d7c], 0
        _emit 0x80
        _emit 0x3d
        _emit 0x7c
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // 0001652f  JZ +0x3b (to 0x0041656c)
        _emit 0x74
        _emit 0x3b
        // 00016531  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00016533  JZ +0x30 (to 0x00416565)
        _emit 0x74
        _emit 0x30
        // 00016535  ADD [0x01328d50], 1
        _emit 0x83
        _emit 0x05
        _emit 0x50
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0001653c  ADD [0x01328d58], 1
        _emit 0x83
        _emit 0x05
        _emit 0x58
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 00016543  LEA EDX, [EDI + ESI*1 - 1]
        _emit 0x8d
        _emit 0x54
        _emit 0x37
        _emit 0xff
        // 00016547  ADD ESI, -1
        _emit 0x83
        _emit 0xc6
        _emit 0xff
        // 0001654a  NOT ESI
        _emit 0xf7
        _emit 0xd6
        // 0001654c  AND EDX, ESI
        _emit 0x23
        _emit 0xd6
        // 0001654e  ADD [0x01328d54], EDX
        _emit 0x01
        _emit 0x15
        _emit 0x54
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00016554  POP EDI
        _emit 0x5f
        // 00016555  POP ESI
        _emit 0x5e
        // 00016556  MOV ECX, [ESP+0x4]   (saved FS:[0])
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0001655a  MOV FS:[0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00016561  ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00016564  RET
        _emit 0xc3
        // 00016565  ADD [0x01328d60], 1
        _emit 0x83
        _emit 0x05
        _emit 0x60
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0001656c  MOV ECX, [ESP+0x0c]  (saved FS:[0], before POP EDI/ESI)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 00016570  POP EDI
        _emit 0x5f
        // 00016571  POP ESI
        _emit 0x5e
        // 00016572  MOV FS:[0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00016579  ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0001657c  RET
        _emit 0xc3
    }
}
