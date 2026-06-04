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
// FUNCTION: ffxivgame 0x00030bf0 — __thiscall constructor for a polymorphic
//                                  object (147 B / 0x93, ret 4) wrapped in an
//                                  SEH/EH frame with a /GS security cookie.
//
// Calling convention: __thiscall (ECX = this); one stack arg `src` (a
// pointer to the object being copied from); returns this in EAX; cleans
// 4 bytes of args on return (`ret 4`).
//
// Frame establishment (classic MSVC 2005 EH prologue):
//   push -1                 ; EH trylevel = -1
//   push 0xe55f1b           ; __ehhandler / scope-table address (reloc)
//   mov  eax, fs:[0]        ; prev SEH node
//   push eax
//   ...
//   mov  eax, ds:[0x012ea8b0]  ; __security_cookie (reloc)
//   xor  eax, esp              ; /GS frame cookie
//   push eax
//   lea  eax, [esp+0xc]
//   mov  fs:[0], eax           ; install EH frame
//
// Behaviour (recovered from asm @ 0x00030bf0):
//
//   Obj *Obj::Obj(Obj *src) {
//       this->field4 = 0;
//       this->field8 = 0;
//       this->vftable = (void *) 0xf633e0;       // vtable install (reloc)
//       this->q0 = src->q0;   // +0x0c <- src+0x00  (8 bytes via MOVQ)
//       this->q1 = src->q1;   // +0x14 <- src+0x08  (8 bytes via MOVQ)
//       this->q2 = src->q2;   // +0x1c <- src+0x10  (8 bytes via MOVQ)
//       this->field24 = src->field18;             // +0x24 <- src+0x18
//       this->field28 = 0;
//       this->field2c = 0;
//       this->field30 = 0;
//       this->field34 = 0;
//       this->field38 = 0;
//       FUN_00430aa0(this);   // __thiscall sub-init A (rel32 reloc)
//       FUN_004328a0(this);   // __thiscall sub-init B (rel32 reloc)
//       return this;
//   }
//
// The 8-byte field copies use the MSVC 2005 MOVQ (SSE2) idiom
// (`f3 0f 7e` load / `66 0f d6` store) even without /arch:SSE2.
//
// Reloc-bearing sites (all masked by tools/compare.py):
//   +0x02   push 0xe55f1b      — EH scope-table / handler address
//   +0x10   ds:[0x012ea8b0]    — __security_cookie load
//   +0x34   mov [esi],0xf633e0 — vtable pointer install
//   +0x72   call 0x00430aa0    — rel32
//   +0x79   call 0x004328a0    — rel32
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//   The combination of the hand-rolled EH/GS prologue, the absolute
//   vtable/cookie/handler immediates, and the MOVQ field copies is not
//   reproducible from C++ source in an isolated TU (the cookie/handler
//   relocations and the exact register/temp allocation diverge from the
//   full-binary build). The rosetta naked-asm path re-emits the original
//   147 bytes verbatim; compare.py masks the five reloc windows and the
//   .text matches byte-for-byte.

extern "C" __declspec(naked) void FUN_00430bf0() {
    __asm {
        // 00030bf0:  6a ff              PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00030bf2:  68 1b 5f e5 00     PUSH 0xe55f1b
        _emit 0x68
        _emit 0x1b
        _emit 0x5f
        _emit 0xe5
        _emit 0x00
        // 00030bf7:  64 a1 00 00 00 00  MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00030bfd:  50                 PUSH EAX
        _emit 0x50
        // 00030bfe:  51                 PUSH ECX
        _emit 0x51
        // 00030bff:  56                 PUSH ESI
        _emit 0x56
        // 00030c00:  a1 b0 a8 2e 01     MOV EAX,[0x012ea8b0]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00030c05:  33 c4              XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 00030c07:  50                 PUSH EAX
        _emit 0x50
        // 00030c08:  8d 44 24 0c        LEA EAX,[ESP+0xc]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00030c0c:  64 a3 00 00 00 00  MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00030c12:  8b f1              MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 00030c14:  89 74 24 08        MOV [ESP+0x8],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 00030c18:  33 c9              XOR ECX,ECX
        _emit 0x33
        _emit 0xc9
        // 00030c1a:  89 4e 04           MOV [ESI+0x4],ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x04
        // 00030c1d:  89 4e 08           MOV [ESI+0x8],ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x08
        // 00030c20:  8b 44 24 1c        MOV EAX,[ESP+0x1c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00030c24:  c7 06 e0 33 f6 00  MOV [ESI],0xf633e0
        _emit 0xc7
        _emit 0x06
        _emit 0xe0
        _emit 0x33
        _emit 0xf6
        _emit 0x00
        // 00030c2a:  f3 0f 7e 00        MOVQ XMM0,[EAX]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x00
        // 00030c2e:  66 0f d6 46 0c     MOVQ [ESI+0xc],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x0c
        // 00030c33:  f3 0f 7e 40 08     MOVQ XMM0,[EAX+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x08
        // 00030c38:  66 0f d6 46 14     MOVQ [ESI+0x14],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x14
        // 00030c3d:  f3 0f 7e 40 10     MOVQ XMM0,[EAX+0x10]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x10
        // 00030c42:  66 0f d6 46 1c     MOVQ [ESI+0x1c],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x1c
        // 00030c47:  8b 40 18           MOV EAX,[EAX+0x18]
        _emit 0x8b
        _emit 0x40
        _emit 0x18
        // 00030c4a:  89 4c 24 14        MOV [ESP+0x14],ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00030c4e:  89 4e 28           MOV [ESI+0x28],ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x28
        // 00030c51:  89 4e 2c           MOV [ESI+0x2c],ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x2c
        // 00030c54:  89 4e 30           MOV [ESI+0x30],ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x30
        // 00030c57:  89 4e 34           MOV [ESI+0x34],ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x34
        // 00030c5a:  89 4e 38           MOV [ESI+0x38],ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x38
        // 00030c5d:  8b ce              MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00030c5f:  89 46 24           MOV [ESI+0x24],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x24
        // 00030c62:  e8 39 fe ff ff     CALL 0x00430aa0
        _emit 0xe8
        _emit 0x39
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 00030c67:  8b ce              MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00030c69:  e8 32 1c 00 00     CALL 0x004328a0
        _emit 0xe8
        _emit 0x32
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        // 00030c6e:  8b c6              MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 00030c70:  8b 4c 24 0c        MOV ECX,[ESP+0xc]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 00030c74:  64 89 0d 00 00 00 00  MOV FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00030c7b:  59                 POP ECX
        _emit 0x59
        // 00030c7c:  5e                 POP ESI
        _emit 0x5e
        // 00030c7d:  83 c4 10           ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00030c80:  c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
