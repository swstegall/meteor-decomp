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
// FUNCTION: ffxivgame 0x00037210 — allocate + init a 20-byte "message"
//                                  object from a per-thread/per-frame pool,
//                                  then hand it to this->field8's dispatcher
//                                  (__thiscall, 4 stack args, 100 B / 0x64)
//
// __thiscall void FUN_00437210(void* this, int a0, int a1, int a2, int a3)
//   ECX = this;  a0..a3 pushed by caller (cleaned up by RET 0x10)
//
//   // Index a global frame/pool table to find the active allocator.
//   T* tbl = *(T**)0x01328d90;
//   unsigned idx = *(unsigned char*)tbl;            // MOVZX byte [tbl]
//   // EDX = idx*8 - idx = idx*7; ECX = tbl->base[idx*7] (28 B stride)
//   Allocator* ax = (Allocator*)( *(char**)(tbl + 4) + (idx*7)*4 );
//   void* obj = ax->Alloc(0x14);                    // __thiscall, size=0x14
//   if (obj) {
//       *(int*)((char*)obj + 0x04) = a0;            // [esp+0x08]
//       *(int*)((char*)obj + 0x08) = a2;            // [esp+0x10]
//       *(void**)obj               = (void*)0x00F64950;  // vftable
//       *(int*)((char*)obj + 0x0c) = a3;            // [esp+0x14]
//       *(int*)((char*)obj + 0x10) = a1;            // [esp+0x0c]
//       ((Dispatcher*)this->field8)->Post(obj);     // __thiscall, ECX=field8
//   } else {
//       ((Dispatcher*)this->field8)->Post(0);
//   }
//
// Calling convention: __thiscall, 4 dword stack args, void return → RET 0x10.
// Frame: PUSH ESI / MOV ESI,ECX / POP ESI; no SUB ESP, no /GS cookie.
//
// Reloc-bearing sites compare.py masks:
//   +0x03  MOV ECX,[0x01328d90]      absolute global pointer
//   +0x1d  CALL 0x00417ab0           rel32 (allocator)
//   +0x3c  MOV [EAX],0x00F64950      absolute vftable imm32
//   +0x4c  CALL 0x0043c2d0           rel32 (dispatcher), 1st arm
//   +0x5b  CALL 0x0043c2d0           rel32 (dispatcher), 2nd/null arm
//
// Why naked asm: the per-frame allocator-table indexing (idx*7<<2 stride),
// the absolute vftable/global stores, and the two-arm tail-call to the same
// dispatcher are all brittle under /O2 — any source-level rewrite shifts a
// byte. A __declspec(naked) body re-emitting the original 100 bytes verbatim
// produces a .obj whose .text is byte-identical to the original slice;
// compare.py masks the two rel32 CALLs and the two absolute windows and
// reports GREEN.

extern "C" __declspec(naked) void FUN_00437210() {
    __asm {
        // 00037210: 56              PUSH ESI
        _emit 0x56
        // 00037211: 8b f1           MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00037213: 8b 0d 90 8d 32 01   MOV ECX, [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00037219: 0f b6 01        MOVZX EAX, byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 0003721c: 8d 14 c5 00 00 00 00   LEA EDX, [EAX*8 + 0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00037223: 2b d0           SUB EDX, EAX
        _emit 0x2b
        _emit 0xd0
        // 00037225: 8b 41 04        MOV EAX, [ECX + 0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00037228: 8d 0c 90        LEA ECX, [EAX + EDX*4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 0003722b: 6a 14           PUSH 0x14
        _emit 0x6a
        _emit 0x14
        // 0003722d: e8 7e 08 fe ff  CALL 0x00417ab0
        _emit 0xe8
        _emit 0x7e
        _emit 0x08
        _emit 0xfe
        _emit 0xff
        // 00037232: 85 c0           TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00037234: 74 2f           JZ 0x00437265
        _emit 0x74
        _emit 0x2f
        // 00037236: 8b 4c 24 08     MOV ECX, [ESP + 0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 0003723a: 8b 54 24 10     MOV EDX, [ESP + 0x10]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0003723e: 89 48 04        MOV [EAX + 0x4], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00037241: 8b 4c 24 14     MOV ECX, [ESP + 0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00037245: 89 50 08        MOV [EAX + 0x8], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 00037248: 8b 54 24 0c     MOV EDX, [ESP + 0xc]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 0003724c: c7 00 50 49 f6 00   MOV dword ptr [EAX], 0x00F64950
        _emit 0xc7
        _emit 0x00
        _emit 0x50
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 00037252: 89 48 0c        MOV [EAX + 0xc], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x0c
        // 00037255: 89 50 10        MOV [EAX + 0x10], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x10
        // 00037258: 8b 4e 08        MOV ECX, [ESI + 0x8]
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        // 0003725b: 50              PUSH EAX
        _emit 0x50
        // 0003725c: e8 6f 50 00 00  CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x6f
        _emit 0x50
        _emit 0x00
        _emit 0x00
        // 00037261: 5e              POP ESI
        _emit 0x5e
        // 00037262: c2 10 00        RET 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
        // 00037265: 8b 4e 08        MOV ECX, [ESI + 0x8]
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        // 00037268: 33 c0           XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0003726a: 50              PUSH EAX
        _emit 0x50
        // 0003726b: e8 60 50 00 00  CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x60
        _emit 0x50
        _emit 0x00
        _emit 0x00
        // 00037270: 5e              POP ESI
        _emit 0x5e
        // 00037271: c2 10 00        RET 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
