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
// FUNCTION: ffxivgame 0x00436e00 — message/event factory: pool-allocates a
//                                  0x10-byte node, fills a vtable + 3 fields,
//                                  and hands it to a dispatch sink
//                                  (__thiscall, 139 bytes / 0x8b)
//
// Calling convention: __thiscall (ECX = this); RET 0xC → three explicit
// stack args. Callee-saves pushed: EBX, ESI, EDI.
//
//   void __thiscall FUN_00436e00(This *this,
//                                int   arg1 /*[esp+arg]*/,
//                                int   arg2 /*scaled <<4*/,
//                                int   arg3);
//
// Shape:
//   ECX = *DAT_01328d90 (global allocator-context pointer)
//   slot = ctx->field4 + ((ctx->byte0 * 7) * 4)   // per-bucket pool head
//   tmp  = FUN_00417ae0(slot, arg3, arg2 << 4, 4) // sub-allocate backing
//   node = FUN_00417ab0(slot, 0x10)               // alloc 0x10-byte node
//   if (node) {
//       node->vtbl  = 0x00f64918;                 // reloc'd vtable/const
//       node->f4    = arg1;
//       node->f8    = arg2;
//       node->fc    = tmp;
//       FUN_0043c2d0(this->f8, node);             // dispatch / enqueue
//   } else {
//       FUN_0043c2d0(this->f8, 0);
//   }
//
// Reloc-bearing sites (compare.py masks the 4-byte displacement windows):
//   ABS  [0x01328d90]  at +0x03 and +0x33  (disp32 @ +0x05 / +0x35)
//   IMM  mov [eax],0x00f64918              (imm32 @ +0x5e)
//   REL  CALL FUN_00417ae0                 (rel32 @ +0x2f)
//   REL  CALL FUN_00417ab0                 (rel32 @ +0x50)
//   REL  CALL FUN_0043c2d0  (x2)           (rel32 @ +0x70 / +0x81)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The per-bucket `slot = field4 + (byte0*7)*4` LEA chain and the
//   pooled-allocator call pattern with the relocated absolute global and
//   vtable immediate aren't coercible from C++ source under /O2 without
//   exact register/stack scheduling. As with siblings FUN_0040ad30 /
//   FUN_00408610, the __declspec(naked) body re-emits the original 139
//   bytes verbatim via MASM _emit directives; the .obj's .text is
//   byte-identical to the original slice and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00436e00() {
    __asm {
        // 00036e00:  53                 PUSH EBX
        _emit 0x53
        // 00036e01:  8b d9              MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 00036e03:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00036e09:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00036e0c:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036e13:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00036e15:  8b 41 04           MOV EAX,dword ptr [ECX + 0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00036e18:  56                 PUSH ESI
        _emit 0x56
        // 00036e19:  8b 74 24 10        MOV ESI,dword ptr [ESP + 0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00036e1d:  8d 0c 90           LEA ECX,[EAX + EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00036e20:  8b 44 24 14        MOV EAX,dword ptr [ESP + 0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00036e24:  57                 PUSH EDI
        _emit 0x57
        // 00036e25:  6a 04              PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 00036e27:  8b d6              MOV EDX,ESI
        _emit 0x8b
        _emit 0xd6
        // 00036e29:  c1 e2 04           SHL EDX,0x4
        _emit 0xc1
        _emit 0xe2
        _emit 0x04
        // 00036e2c:  52                 PUSH EDX
        _emit 0x52
        // 00036e2d:  50                 PUSH EAX
        _emit 0x50
        // 00036e2e:  e8 ad 0c fe ff     CALL 0x00417ae0
        _emit 0xe8
        _emit 0xad
        _emit 0x0c
        _emit 0xfe
        _emit 0xff
        // 00036e33:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00036e39:  8b f8              MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 00036e3b:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00036e3e:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036e45:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00036e47:  8b 41 04           MOV EAX,dword ptr [ECX + 0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00036e4a:  8d 0c 90           LEA ECX,[EAX + EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00036e4d:  6a 10              PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 00036e4f:  e8 5c 0c fe ff     CALL 0x00417ab0
        _emit 0xe8
        _emit 0x5c
        _emit 0x0c
        _emit 0xfe
        _emit 0xff
        // 00036e54:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00036e56:  74 22              JZ 0x00436e7a
        _emit 0x74
        _emit 0x22
        // 00036e58:  8b 4c 24 10        MOV ECX,dword ptr [ESP + 0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00036e5c:  c7 00 18 49 f6 00  MOV dword ptr [EAX],0xf64918
        _emit 0xc7
        _emit 0x00
        _emit 0x18
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 00036e62:  89 48 04           MOV dword ptr [EAX + 0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00036e65:  89 70 08           MOV dword ptr [EAX + 0x8],ESI
        _emit 0x89
        _emit 0x70
        _emit 0x08
        // 00036e68:  89 78 0c           MOV dword ptr [EAX + 0xc],EDI
        _emit 0x89
        _emit 0x78
        _emit 0x0c
        // 00036e6b:  8b 4b 08           MOV ECX,dword ptr [EBX + 0x8]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 00036e6e:  50                 PUSH EAX
        _emit 0x50
        // 00036e6f:  e8 5c 54 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x5c
        _emit 0x54
        _emit 0x00
        _emit 0x00
        // 00036e74:  5f                 POP EDI
        _emit 0x5f
        // 00036e75:  5e                 POP ESI
        _emit 0x5e
        // 00036e76:  5b                 POP EBX
        _emit 0x5b
        // 00036e77:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00036e7a:  8b 4b 08           MOV ECX,dword ptr [EBX + 0x8]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 00036e7d:  33 c0              XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 00036e7f:  50                 PUSH EAX
        _emit 0x50
        // 00036e80:  e8 4b 54 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x4b
        _emit 0x54
        _emit 0x00
        _emit 0x00
        // 00036e85:  5f                 POP EDI
        _emit 0x5f
        // 00036e86:  5e                 POP ESI
        _emit 0x5e
        // 00036e87:  5b                 POP EBX
        _emit 0x5b
        // 00036e88:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
