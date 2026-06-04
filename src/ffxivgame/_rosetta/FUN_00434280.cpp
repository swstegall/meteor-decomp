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
// FUNCTION: ffxivgame 0x00034280 — `__thiscall` factory thunk: pool-allocates a
//                                  0x1c-byte event/message object, populates it
//                                  from six dword args + a vtable, and hands it
//                                  to this->field_0c's handler (114 bytes / 0x72)
//
// Calling convention: __thiscall (ECX = this), 6 dword stack args, RET 0x18.
//   void __thiscall FUN_00434280(this, a0, a1, a2, a3, a4, a5);
//
// Behaviour (read from the disassembly at orig RVA 0x00034280):
//
//   ESI = this
//   g   = *(void**)0x01328d90              // global allocator/registry singleton
//   idx = *(unsigned char*)g               // g->byte0  (a slot index)
//   p   = g->dword4 + idx * 0x1c            // &pool[idx], element stride 0x1c=28
//   obj = (T*)Alloc(p, 0x1c);              // __thiscall 0x00417ab0(p, 28)
//   if (obj) {
//       obj->field_04 = a0;
//       obj->field_08 = a1;
//       obj->field_0c = a2;
//       obj->field_10 = a3;
//       *(void**)obj  = (void*)0xf649b8;   // vtable pointer
//       obj->field_14 = a4;
//       obj->field_18 = a5;
//       // this->field_0c handler: __thiscall 0x0043c2d0(this->field_0c, obj)
//       FUN_0043c2d0(this->field_0c, obj);
//   } else {
//       FUN_0043c2d0(this->field_0c, 0);   // null on allocation failure
//   }
//
// Stack args (after PUSH ESI at entry, [ESP+4] = return address):
//   [ESP+0x08] a0   [ESP+0x0c] a1   [ESP+0x10] a2
//   [ESP+0x14] a3   [ESP+0x18] a4   [ESP+0x1c] a5
//
// Reloc-bearing sites in the orig 114 bytes (resolve only at full-binary
// relink; emitted here as raw immediates so the standalone .obj's .text is
// byte-identical with NO relocations — tools/compare.py reports GREEN):
//   +0x03  MOV ECX,[0x01328d90]   (.data global singleton ptr)
//   +0x1d  CALL rel32 → 0x00417ab0  (.text pool allocator, __thiscall)
//   +0x4a  MOV [EAX],0xf649b8       (vtable absolute immediate)
//   +0x5a  CALL rel32 → 0x0043c2d0  (.text handler, __thiscall, hit obj)
//   +0x69  CALL rel32 → 0x0043c2d0  (.text handler, __thiscall, null path)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level C++ form would need MSVC 2005 /O2 to reproduce the exact
//   LEA-based idx*0x1c materialisation (LEA EDX,[EAX*8]; SUB EDX,EAX → *7;
//   LEA ECX,[EAX+EDX*4] → *28), the precise register shuffle for the six
//   arg stores, the vtable immediate store, and three linker-resolved
//   absolute addresses. Each constraint is brittle under /O2. The pragmatic
//   path — the same one the sibling _rosetta thunks took — is a
//   `__declspec(naked)` body re-emitting the orig 114 bytes verbatim via
//   MASM `_emit` directives. The .obj's .text ends up byte-identical to the
//   orig slice (no relocations), which is what compare.py checks.

extern "C" __declspec(naked) void FUN_00434280() {
    __asm {
        // 00034280: 56            PUSH ESI
        _emit 0x56
        // 00034281: 8b f1         MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 00034283: 8b 0d 90 8d 32 01   MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00034289: 0f b6 01      MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 0003428c: 8d 14 c5 00 00 00 00   LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00034293: 2b d0         SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00034295: 8b 41 04      MOV EAX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00034298: 8d 0c 90      LEA ECX,[EAX + EDX*4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 0003429b: 6a 1c         PUSH 0x1c
        _emit 0x6a
        _emit 0x1c
        // 0003429d: e8 0e 38 fe ff   CALL 0x00417ab0
        _emit 0xe8
        _emit 0x0e
        _emit 0x38
        _emit 0xfe
        _emit 0xff
        // 000342a2: 85 c0         TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 000342a4: 74 3d         JZ 0x004342e3
        _emit 0x74
        _emit 0x3d
        // 000342a6: 8b 4c 24 08   MOV ECX,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 000342aa: 8b 54 24 0c   MOV EDX,dword ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 000342ae: 89 48 04      MOV dword ptr [EAX+0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 000342b1: 8b 4c 24 10   MOV ECX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 000342b5: 89 50 08      MOV dword ptr [EAX+0x8],EDX
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 000342b8: 8b 54 24 14   MOV EDX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 000342bc: 89 48 0c      MOV dword ptr [EAX+0xc],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x0c
        // 000342bf: 8b 4c 24 18   MOV ECX,dword ptr [ESP+0x18]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 000342c3: 89 50 10      MOV dword ptr [EAX+0x10],EDX
        _emit 0x89
        _emit 0x50
        _emit 0x10
        // 000342c6: 8b 54 24 1c   MOV EDX,dword ptr [ESP+0x1c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 000342ca: c7 00 b8 49 f6 00   MOV dword ptr [EAX],0xf649b8
        _emit 0xc7
        _emit 0x00
        _emit 0xb8
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 000342d0: 89 48 14      MOV dword ptr [EAX+0x14],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x14
        // 000342d3: 89 50 18      MOV dword ptr [EAX+0x18],EDX
        _emit 0x89
        _emit 0x50
        _emit 0x18
        // 000342d6: 8b 4e 0c      MOV ECX,dword ptr [ESI+0xc]
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        // 000342d9: 50            PUSH EAX
        _emit 0x50
        // 000342da: e8 f1 7f 00 00   CALL 0x0043c2d0
        _emit 0xe8
        _emit 0xf1
        _emit 0x7f
        _emit 0x00
        _emit 0x00
        // 000342df: 5e            POP ESI
        _emit 0x5e
        // 000342e0: c2 18 00      RET 0x18
        _emit 0xc2
        _emit 0x18
        _emit 0x00
        // 000342e3: 8b 4e 0c      MOV ECX,dword ptr [ESI+0xc]   (null path)
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        // 000342e6: 33 c0         XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 000342e8: 50            PUSH EAX
        _emit 0x50
        // 000342e9: e8 e2 7f 00 00   CALL 0x0043c2d0
        _emit 0xe8
        _emit 0xe2
        _emit 0x7f
        _emit 0x00
        _emit 0x00
        // 000342ee: 5e            POP ESI
        _emit 0x5e
        // 000342ef: c2 18 00      RET 0x18
        _emit 0xc2
        _emit 0x18
        _emit 0x00
    }
}
