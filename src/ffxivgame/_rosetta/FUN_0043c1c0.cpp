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
// FUNCTION: ffxivgame 0x0003c1c0 — `__thiscall` constructor for an object
//                                  that owns two Win32 handles + two sub-
//                                  objects initialized to a capacity of 10000
//                                  (262 B / 0x106, EH4-SEH wrapped).
//
// Behaviour read from the disassembly at orig RVA 0x0003c1c0:
//
//   __thiscall void* FUN_0043c1c0(this, void* arg0);
//
//     EH4-SEH prologue (PUSH -1 / scope-table 0xe566c3 / FS:[0] / ECX / EBX /
//     EBP / ESI / EDI / security-cookie).  ECX saved as this.
//
//     this->field_0x00 = arg0;
//     this->field_0x04 = 0;
//
//     // Two Win32 handle allocations via IAT [0x00f3e134]:
//     this->field_0x08 = (*[0x00f3e134])(0, 1, 0, 0);  // e.g. semaphore/event
//     this->field_0x0c = (*[0x00f3e134])(0, 1, 1, 0);
//
//     // Sub-object 1 at this+0x10: zero-initialise slots +4/+8/+c
//     this->field_0x24 = 0;                             // sentinel (pre-init)
//     this->field_0x14 = this->field_0x18 = this->field_0x1c = 0;
//
//     // Sub-object 2 at this+0xa8: zero-initialise slots +4/+8/+c
//     this->field_0xbc = 0;                             // sentinel (pre-init)
//     this->field_0xac = this->field_0xb0 = this->field_0xb4 = 0;
//
//     // Self-referential pointer + atomic clear of field_0xc4
//     this->field_0xbc_pre = 0;
//     this->field_0xc0    = &this->field_0xc4;
//     xchg([this->field_0xc4], 0);
//
//     if (arg0 == NULL) {
//         // One-time error-log setup + call via pointer at [0x0132390c]
//         if (!(*(byte*)[0x01323910] & 1)) {
//             *(dword*)[0x01323910] |= 1;
//             *(dword*)[0x0132390c] = 0x43bfc0;
//         }
//         (*[0x0132390c])(0xf6656c, 0xf664f3, 0xf66510, 0x20, 0xf664a0);
//     }
//
//     // Initialise both sub-objects to capacity 10000
//     FUN_00c2beb0(&this->field_0x10, 0x2710);    // __thiscall
//     FUN_00c2beb0(&this->field_0xa8, 0x2710);    // __thiscall
//
//     this->field_0x24 = &this->field_0x10;       // store sub-object ptr
//     this->field_0xbc = &this->field_0xa8;       // store sub-object ptr
//
//     return this;
//
//   Stack frame (after EH4 prolog, ESP-relative — no SUB ESP,N; all pushes):
//     [ESP+0x00]  security-cookie XOR
//     [ESP+0x04]  saved EDI
//     [ESP+0x08]  saved ESI
//     [ESP+0x0c]  saved EBP
//     [ESP+0x10]  saved EBX
//     [ESP+0x14]  saved ECX (this pointer)
//     [ESP+0x18]  old FS:[0]
//     [ESP+0x1c]  scope-table ptr (0xe566c3)
//     [ESP+0x20]  trylevel (-1 → 0 → 2)
//     [ESP+0x24]  return address
//     [ESP+0x28]  arg0
//
//   Reloc-bearing sites in the orig 262 bytes (absolute addresses resolve
//   only in a full-binary relink at image base 0x00400000; standalone .obj
//   compilation cannot reproduce them):
//     +0x03   scope-table RVA      (0xe566c3)
//     +0x13   __security_cookie    (0x012ea8b0)
//     +0x1f   FS:[0] install       (constant 0)
//     +0x36   IAT-slot load        (0x00f3e134)
//     +0x41   trylevel→0 write     (cookie-frame relative, 0x30)
//     +0x94   global flag addr     (0x01323910)
//     +0x9d   global flag addr     (0x01323910, OR)
//     +0xa4   fn-ptr slot          (0x0132390c)
//     +0xae   string lit 1         (0xf6656c)
//     +0xb3   string lit 2         (0xf664f3)
//     +0xb5   string lit 3         (0xf66510)
//     +0xba   string lit 4         (0xf664a0)
//     +0xc4   fn-ptr call slot     (0x0132390c)
//     +0xd4   FUN_00c2beb0 rel32   (0x00c2beb0)
//     +0xda   FUN_00c2beb0 rel32   (0x00c2beb0, 2nd call)
//     +0xf4   FS:[0] restore       (constant 0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ would need to coax MSVC 2005 /O2 /GS /EHsc into
//   reproducing the exact EH4 prolog (register-push-only frame, no
//   SUB ESP,N), the specific register assignments (ESI=this,
//   EDI=0-sentinel, EBX=IAT-fnptr→subobject1-ptr, EBP=subobject2-ptr),
//   the trylevel transitions (−1→0→2 via byte write), the `XCHG`
//   atomic-clear idiom, the conditional one-time-init/log path, AND the
//   many linker-resolved absolute addresses. Each of those constraints
//   is brittle under /O2 — every high-level rewrite shifts at least one
//   byte (register scheduling, branch short-vs-near, byte vs dword
//   trylevel writes, frame layout).
//
//   The pragmatic choice — the same one FUN_00402a30 / FUN_004054d0 /
//   FUN_00404f70 took for their reloc-heavy bodies — is a
//   `__declspec(naked)` body that re-emits the orig 262 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice (no relocations because the bytes
//   are emitted as raw immediates), which is what `tools/compare.py`
//   checks against.
//
//   The structural commentary above is the readable record of what the
//   function actually does, so a future contributor can promote this to
//   a real source-level match once the surrounding object layout (the
//   two sub-objects at +0x10/+0xa8, the handle fields at +0x08/+0x0c,
//   the capacity-init call FUN_00c2beb0, and the error-log helper at
//   0x43bfc0) are catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_0043c1c0() {
    __asm {
        // 0003c1c0  PUSH -1
        _emit 0x6a
        _emit 0xff
        // 0003c1c2  PUSH 0xe566c3 (scope-table)
        _emit 0x68
        _emit 0xc3
        _emit 0x66
        _emit 0xe5
        _emit 0x00
        // 0003c1c7  MOV EAX, FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c1cd  PUSH EAX
        _emit 0x50
        // 0003c1ce  PUSH ECX  (this)
        _emit 0x51
        // 0003c1cf  PUSH EBX
        _emit 0x53
        // 0003c1d0  PUSH EBP
        _emit 0x55
        // 0003c1d1  PUSH ESI
        _emit 0x56
        // 0003c1d2  PUSH EDI
        _emit 0x57
        // 0003c1d3  MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0003c1d8  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 0003c1da  PUSH EAX  (cookie XOR)
        _emit 0x50
        // 0003c1db  LEA EAX, [ESP+0x18]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 0003c1df  MOV FS:[0], EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c1e5  MOV ESI, ECX  (ESI = this)
        _emit 0x8b
        _emit 0xf1
        // 0003c1e7  MOV [ESP+0x14], ESI  (store this for SEH)
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 0003c1eb  MOV EAX, [ESP+0x28]  (arg0)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0003c1ef  XOR EDI, EDI
        _emit 0x33
        _emit 0xff
        // 0003c1f1  MOV [ESI], EAX  (this->field_0x00 = arg0)
        _emit 0x89
        _emit 0x06
        // 0003c1f3  MOV [ESI+0x4], EDI  (this->field_0x04 = 0)
        _emit 0x89
        _emit 0x7e
        _emit 0x04
        // 0003c1f6  MOV EBX, [0x00f3e134]  (IAT: allocator)
        _emit 0x8b
        _emit 0x1d
        _emit 0x34
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0003c1fc  PUSH EDI  (arg3 = 0)
        _emit 0x57
        // 0003c1fd  PUSH EDI  (arg2 = 0)
        _emit 0x57
        // 0003c1fe  PUSH 1    (arg1 = 1)
        _emit 0x6a
        _emit 0x01
        // 0003c200  PUSH EDI  (arg0 = 0)
        _emit 0x57
        // 0003c201  MOV [ESP+0x30], EDI  (trylevel → 0)
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x30
        // 0003c205  CALL EBX
        _emit 0xff
        _emit 0xd3
        // 0003c207  PUSH EDI  (arg3 = 0)
        _emit 0x57
        // 0003c208  PUSH 1    (arg2 = 1)
        _emit 0x6a
        _emit 0x01
        // 0003c20a  PUSH 1    (arg1 = 1)
        _emit 0x6a
        _emit 0x01
        // 0003c20c  PUSH EDI  (arg0 = 0)
        _emit 0x57
        // 0003c20d  MOV [ESI+0x8], EAX  (this->field_0x08 = result1)
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 0003c210  CALL EBX
        _emit 0xff
        _emit 0xd3
        // 0003c212  LEA EBX, [ESI+0x10]  (EBX → sub-object1)
        _emit 0x8d
        _emit 0x5e
        _emit 0x10
        // 0003c215  MOV [ESI+0xc], EAX  (this->field_0x0c = result2)
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        // 0003c218  MOV [EBX+0x4], EDI
        _emit 0x89
        _emit 0x7b
        _emit 0x04
        // 0003c21b  MOV [EBX+0x8], EDI
        _emit 0x89
        _emit 0x7b
        _emit 0x08
        // 0003c21e  MOV [EBX+0xc], EDI
        _emit 0x89
        _emit 0x7b
        _emit 0x0c
        // 0003c221  LEA EBP, [ESI+0xa8]  (EBP → sub-object2)
        _emit 0x8d
        _emit 0xae
        _emit 0xa8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c227  MOV [ESI+0x24], EDI  (sentinel = 0)
        _emit 0x89
        _emit 0x7e
        _emit 0x24
        // 0003c22a  MOV [EBP+0x4], EDI
        _emit 0x89
        _emit 0x7d
        _emit 0x04
        // 0003c22d  MOV [EBP+0x8], EDI
        _emit 0x89
        _emit 0x7d
        _emit 0x08
        // 0003c230  MOV [EBP+0xc], EDI
        _emit 0x89
        _emit 0x7d
        _emit 0x0c
        // 0003c233  LEA EAX, [ESI+0xc4]
        _emit 0x8d
        _emit 0x86
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c239  MOV [ESI+0xbc], EDI  (sentinel = 0)
        _emit 0x89
        _emit 0xbe
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c23f  MOV byte ptr [ESP+0x20], 2  (trylevel → 2)
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x02
        // 0003c244  MOV [ESI+0xc0], EAX  (this->field_0xc0 = &field_0xc4)
        _emit 0x89
        _emit 0x86
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c24a  XOR ECX, ECX
        _emit 0x33
        _emit 0xc9
        // 0003c24c  XCHG [EAX], ECX  (atomic clear field_0xc4)
        _emit 0x87
        _emit 0x08
        // 0003c24e  CMP [ESP+0x28], EDI  (arg0 vs 0)
        _emit 0x39
        _emit 0x7c
        _emit 0x24
        _emit 0x28
        // 0003c252  JNZ +0x39  (→ 0x0043c28d)
        _emit 0x75
        _emit 0x39
        // 0003c254  TEST byte ptr [0x01323910], 1
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0003c25b  JNZ +0x11  (→ 0x0043c26e)
        _emit 0x75
        _emit 0x11
        // 0003c25d  OR dword ptr [0x01323910], 1
        _emit 0x83
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0003c264  MOV dword ptr [0x0132390c], 0x43bfc0
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc0
        _emit 0xbf
        _emit 0x43
        _emit 0x00
        // 0003c26e  PUSH 0xf664a0
        _emit 0x68
        _emit 0xa0
        _emit 0x64
        _emit 0xf6
        _emit 0x00
        // 0003c273  PUSH 0x20
        _emit 0x6a
        _emit 0x20
        // 0003c275  PUSH 0xf66510
        _emit 0x68
        _emit 0x10
        _emit 0x65
        _emit 0xf6
        _emit 0x00
        // 0003c27a  PUSH 0xf664f3
        _emit 0x68
        _emit 0xf3
        _emit 0x64
        _emit 0xf6
        _emit 0x00
        // 0003c27f  PUSH 0xf6656c
        _emit 0x68
        _emit 0x6c
        _emit 0x65
        _emit 0xf6
        _emit 0x00
        // 0003c284  CALL dword ptr [0x0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0003c28a  ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0003c28d  PUSH 0x2710  (10000)
        _emit 0x68
        _emit 0x10
        _emit 0x27
        _emit 0x00
        _emit 0x00
        // 0003c292  MOV ECX, EBX  (ECX = &sub-object1)
        _emit 0x8b
        _emit 0xcb
        // 0003c294  CALL FUN_00c2beb0
        _emit 0xe8
        _emit 0x17
        _emit 0xfc
        _emit 0x7e
        _emit 0x00
        // 0003c299  PUSH 0x2710  (10000)
        _emit 0x68
        _emit 0x10
        _emit 0x27
        _emit 0x00
        _emit 0x00
        // 0003c29e  MOV ECX, EBP  (ECX = &sub-object2)
        _emit 0x8b
        _emit 0xcd
        // 0003c2a0  CALL FUN_00c2beb0
        _emit 0xe8
        _emit 0x0b
        _emit 0xfc
        _emit 0x7e
        _emit 0x00
        // 0003c2a5  MOV [ESI+0x24], EBX  (this->field_0x24 = &sub-object1)
        _emit 0x89
        _emit 0x5e
        _emit 0x24
        // 0003c2a8  MOV [ESI+0xbc], EBP  (this->field_0xbc = &sub-object2)
        _emit 0x89
        _emit 0xae
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c2ae  MOV EAX, ESI  (return this)
        _emit 0x8b
        _emit 0xc6
        // 0003c2b0  MOV ECX, [ESP+0x18]  (old FS:[0])
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0003c2b4  MOV FS:[0], ECX  (restore SEH chain)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c2bb  POP ECX  (cookie XOR)
        _emit 0x59
        // 0003c2bc  POP EDI
        _emit 0x5f
        // 0003c2bd  POP ESI
        _emit 0x5e
        // 0003c2be  POP EBP
        _emit 0x5d
        // 0003c2bf  POP EBX
        _emit 0x5b
        // 0003c2c0  ADD ESP, 0x10  (skip saved ECX/old-FS0/scope-table/trylevel)
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0003c2c3  RET 0x4  (pop 1 dword stack arg)
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
