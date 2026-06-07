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
// FUNCTION: ffxivgame 0x00021e90 — unknown constructor / init method
//                                  (384 B / 0x180, __thiscall, SEH4 + /GS).
//
// Calling convention: __thiscall (ECX = this).  Returns `this` in EAX.
//
// SEH4 prologue layout (post-push ESP = base):
//   base+0x00  GS cookie (XOR'd with ESP, popped by POP ECX in epilogue)
//   base+0x04  EDI save
//   base+0x08  ESI save (= this)
//   base+0x0C  EBP save
//   base+0x10  EBX save
//   base+0x14  local[0] — saved this (MOV [ESP+0x14],ESI)
//   base+0x18  local[1]
//   base+0x1C  local[2] — alloc/init result spill
//   base+0x20  local[3]
//   base+0x24  local[4]
//   base+0x28  prev FS:[0]           ─┐
//   base+0x2C  SEH handler (0xe55aad) ├─ EH_RECORD installed at FS:[0]
//   base+0x30  SEH state (−1→0→1→2)  ─┘
//   base+0x34  return address
//
// Body (informal):
//   1. this->field_0x08 (word) = 0.
//   2. ECX = this+0x140; CALL 0x00a10030 (sub-object ctor returning ptr).
//      Wire the returned ptr into a circular 3-pointer list at
//      [this+0x140+0x04/0x00/0x08]; zero [this+0x140+0x08].
//   3. ECX = this+0x1ac; set SEH state=0; CALL 0x00a3caa0 (second ctor).
//      Same circular-list init for [this+0x1ac+0x04/0x00/0x08].
//   4. Zero this->field_0x00, _0x04, _0x0c.
//   5. Push arg from [this+0x140+0x04+0x04]; ECX=this+0x140; SEH state=1;
//      CALL 0x00c2bb10 (some list-removal step).
//      Re-wire [this+0x140+0x04] circular; advance EDI to that ptr.
//   6. PUSH 0xf599d8; PUSH 0x10; LEA ECX,[ESP+0x28] (local object);
//      zero fields this+0x150, 0x164..0x1a4, two floats at 0x178/0x17c,
//      byte at 0x180; CALL 0x0040e2d0 (__cdecl helper).
//   7. PUSH EAX (return from step 6); PUSH 0x14; save EAX to local;
//      CALL 0x00419c40 (alloc/ctor with 2 args); ADD ESP,8.
//      SEH state=2.
//   8. If result != NULL: ECX=result; CALL 0x00432850; store EAX at
//      this+0x1a8.  Else store 0 at this+0x1a8.
//   9. Return this in EAX; restore FS:[0]; pop saved regs;
//      ADD ESP,0x20; RET.
//
// Reloc-bearing sites (VA-dependent, cannot reproduce in standalone .obj):
//   +0x02   DIR32 0xe55aad          (PUSH SEH handler)
//   +0x08   moffs  FS:[0x0]         (MOV EAX,FS:[0])
//   +0x0f   DIR32 0x012ea8b0        (MOV EAX,[__security_cookie])
//   +0x18   moffs  FS:[0x0]         (MOV FS:[0],EAX)
//   +0x2b   REL32 → 0x00a10030     (CALL sub-object ctor 1)
//   +0x57   REL32 → 0x00a3caa0     (CALL sub-object ctor 2)
//   +0x9d   REL32 → 0x00c2bb10     (CALL list helper)
//   +0xb9   DIR32 0xf599d8          (PUSH address arg)
//   +0x134  REL32 → 0x0040e2d0     (CALL __cdecl helper)
//   +0x13a  REL32 → 0x00419c40     (CALL alloc/ctor)
//   +0x157  REL32 → 0x00432850     (CALL ctor on result)
//   +0x170  moffs  FS:[0x0]         (MOV FS:[0],ECX restore)
//
// Reconstruction strategy — naked-asm byte passthrough (same as siblings).

extern "C" __declspec(naked) void FUN_00421e90() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xad
        _emit 0x5a
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x83
        _emit 0xec
        _emit 0x14
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x57
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x50
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf1
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x8d
        _emit 0xbe
        _emit 0x40
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xdb
        _emit 0x8b
        _emit 0xcf
        _emit 0x66
        _emit 0x89
        _emit 0x5e
        _emit 0x08
        _emit 0xe8
        _emit 0x60
        _emit 0xe1
        _emit 0x5e
        _emit 0x00
        _emit 0x89
        _emit 0x47
        _emit 0x04
        _emit 0xc6
        _emit 0x40
        _emit 0x11
        _emit 0x01
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        _emit 0x89
        _emit 0x40
        _emit 0x04
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        _emit 0x89
        _emit 0x00
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        _emit 0x89
        _emit 0x40
        _emit 0x08
        _emit 0x89
        _emit 0x5f
        _emit 0x08
        _emit 0x8d
        _emit 0xae
        _emit 0xac
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xcd
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x30
        _emit 0xe8
        _emit 0xa4
        _emit 0xab
        _emit 0x61
        _emit 0x00
        _emit 0x89
        _emit 0x45
        _emit 0x04
        _emit 0xc6
        _emit 0x40
        _emit 0x15
        _emit 0x01
        _emit 0x8b
        _emit 0x45
        _emit 0x04
        _emit 0x89
        _emit 0x40
        _emit 0x04
        _emit 0x8b
        _emit 0x45
        _emit 0x04
        _emit 0x89
        _emit 0x00
        _emit 0x8b
        _emit 0x45
        _emit 0x04
        _emit 0x89
        _emit 0x40
        _emit 0x08
        _emit 0x89
        _emit 0x5d
        _emit 0x08
        _emit 0x89
        _emit 0x1e
        _emit 0x89
        _emit 0x5e
        _emit 0x04
        _emit 0x89
        _emit 0x5e
        _emit 0x0c
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        _emit 0x51
        _emit 0x8b
        _emit 0xcf
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x01
        _emit 0xe8
        _emit 0xde
        _emit 0x9b
        _emit 0x80
        _emit 0x00
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        _emit 0x89
        _emit 0x40
        _emit 0x04
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        _emit 0x89
        _emit 0x5f
        _emit 0x08
        _emit 0x89
        _emit 0x00
        _emit 0x8b
        _emit 0x7f
        _emit 0x04
        _emit 0x89
        _emit 0x7f
        _emit 0x08
        _emit 0x68
        _emit 0xd8
        _emit 0x99
        _emit 0xf5
        _emit 0x00
        _emit 0x6a
        _emit 0x10
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x89
        _emit 0x9e
        _emit 0x50
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x9e
        _emit 0x64
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x9e
        _emit 0x68
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x9e
        _emit 0x6c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x9e
        _emit 0x70
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x9e
        _emit 0x74
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x86
        _emit 0x78
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x86
        _emit 0x7c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x88
        _emit 0x9e
        _emit 0x80
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x9e
        _emit 0x84
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x9e
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x9e
        _emit 0x8c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x9e
        _emit 0x90
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x9e
        _emit 0x94
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x9e
        _emit 0x98
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x9e
        _emit 0x9c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x9e
        _emit 0xa0
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x9e
        _emit 0xa4
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x07
        _emit 0xc3
        _emit 0xfe
        _emit 0xff
        _emit 0x50
        _emit 0x6a
        _emit 0x14
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0xe8
        _emit 0x6b
        _emit 0x7c
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x3b
        _emit 0xc3
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x02
        _emit 0x74
        _emit 0x0f
        _emit 0x8b
        _emit 0xc8
        _emit 0xe8
        _emit 0x64
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x89
        _emit 0x86
        _emit 0xa8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x06
        _emit 0x89
        _emit 0x9e
        _emit 0xa8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xc6
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        _emit 0xc3
    }
}
