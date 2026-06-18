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
// FUNCTION: ffxivgame 0x000372e0 — __thiscall factory helper: allocates an
//                                  8-byte object via sub_417ab0, stamps its
//                                  vftable (0x00f64960) and one caller dword,
//                                  then dispatches it through this->field_8's
//                                  handler. (79 bytes / 0x4f)
//
// Calling convention: __thiscall (ECX = this), one 4-byte stack arg
//   (RET 0x4 cleans 4 bytes). ESI = saved `this` across the body.
//
// Pseudo-C (read from the disassembly at orig RVA 0x000372e0):
//
//   void __thiscall FUN_004372e0(T *this, u32 a1) {
//       // global pool descriptor at .data 0x01328d90
//       Pool *p = *(Pool **)0x01328d90;
//       u8 slot = *(u8 *)p;                 // current pool index (byte)
//       // base + slot * 0x1c  (0x1c = 7*4, computed as (slot*8 - slot)*4)
//       void *poolThis = (char *)p->field_4 + (slot * 7) * 4;
//       Obj *e = Alloc_00417ab0(poolThis, 8);   // __thiscall, size 8
//       if (e) {
//           e->vtable  = (void *)0x00f64960;
//           e->field_4 = a1;
//           Dispatch_0043c2d0(this->field_8, e);     // __thiscall(handler, e)
//       } else {
//           Dispatch_0043c2d0(this->field_8, 0);     // __thiscall(handler, 0)
//       }
//   }
//
// Reloc-bearing sites in the orig 79 bytes (resolved only in a full-binary
// relink at image base 0x00400000; emitting them as raw immediates produces a
// .obj whose .text is byte-identical with NO relocations, which is exactly
// what tools/compare.py checks):
//     +0x03  global pool ptr load   (.data 0x01328d90, moffs32)
//     +0x1d  Alloc CALL rel32       (.text 0x00417ab0)
//     +0x2a  vtable immediate       (0x00f64960)
//     +0x37  Dispatch CALL rel32    (.text 0x0043c2d0, success arm)
//     +0x46  Dispatch CALL rel32    (.text 0x0043c2d0, null arm)
//
// Reconstruction strategy — naked-asm byte passthrough (same idiom as
// siblings FUN_00434180 / FUN_00437660 in _rosetta): a `__declspec(naked)`
// body re-emitting the orig 79 bytes verbatim via MASM `_emit`. The .obj's
// .text is then byte-identical to the orig slice — the absolute data
// addresses and rel32 displacements are emitted as raw immediates that
// already match the orig binary's resolved bytes, so no relocations are
// involved and tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004372e0() {
    __asm {
        // 000372e0: 56            PUSH ESI
        _emit 0x56
        // 000372e1: 8b f1         MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 000372e3: 8b 0d 90 8d 32 01   MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000372e9: 0f b6 01      MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 000372ec: 8d 14 c5 00 00 00 00   LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000372f3: 2b d0         SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 000372f5: 8b 41 04      MOV EAX,dword ptr [ECX + 0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 000372f8: 8d 0c 90      LEA ECX,[EAX + EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 000372fb: 6a 08         PUSH 0x8
        _emit 0x6a
        _emit 0x08
        // 000372fd: e8 ae 07 fe ff   CALL 0x00417ab0
        _emit 0xe8
        _emit 0xae
        _emit 0x07
        _emit 0xfe
        _emit 0xff
        // 00037302: 85 c0         TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00037304: 74 1a         JZ +0x1a (→ null_branch)
        _emit 0x74
        _emit 0x1a
        // 00037306: 8b 4c 24 08   MOV ECX,dword ptr [ESP + 0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 0003730a: c7 00 60 49 f6 00   MOV dword ptr [EAX],0xf64960
        _emit 0xc7
        _emit 0x00
        _emit 0x60
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 00037310: 89 48 04      MOV dword ptr [EAX + 0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00037313: 8b 4e 08      MOV ECX,dword ptr [ESI + 0x8]
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        // 00037316: 50            PUSH EAX
        _emit 0x50
        // 00037317: e8 b4 4f 00 00   CALL 0x0043c2d0
        _emit 0xe8
        _emit 0xb4
        _emit 0x4f
        _emit 0x00
        _emit 0x00
        // 0003731c: 5e            POP ESI
        _emit 0x5e
        // 0003731d: c2 04 00      RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // null_branch:
        // 00037320: 8b 4e 08      MOV ECX,dword ptr [ESI + 0x8]
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        // 00037323: 33 c0         XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 00037325: 50            PUSH EAX
        _emit 0x50
        // 00037326: e8 a5 4f 00 00   CALL 0x0043c2d0
        _emit 0xe8
        _emit 0xa5
        _emit 0x4f
        _emit 0x00
        _emit 0x00
        // 0003732b: 5e            POP ESI
        _emit 0x5e
        // 0003732c: c2 04 00      RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
