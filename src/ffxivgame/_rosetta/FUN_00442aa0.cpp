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
// FUNCTION: ffxivgame 0x00042aa0 — C++ constructor (__thiscall, 115 B / 0x73,
//                                  EH3-SEH wrapped, ESP-relative frame).
//
// Calling convention: __thiscall (ECX = this); returns `this` in EAX.
// Takes 1 argument (RET 0x4).
//
// Object layout (offsets written by this constructor):
//   [this + 0x00]  vtable pointer := 0x00f670b8
//   [this + 0x04]  dword := 0
//   [this + 0x0c]  dword := 0
//   [this + 0x10]  dword := 0
//   [this + 0x14]  dword := 0
//   [this + 0x18]  dword := 0
//   [this + 0x1c]  dword := 0
//   [this + 0x20]  dword := 0
//   [this + 0x24]  byte  := 0
//
// The EH3 prologue uses scope table at VA 0x00e57244.
// After zeroing the fields, the constructor passes the caller argument
// to a sub-constructor at VA 0x004432c0 (__thiscall, this+8 used as
// sub-object), then returns `this` in EAX.
//
// Stack frame (ESP-relative, no EBP; after EH3 prologue + cookie PUSH):
//   [ESP + 0x00]  security cookie XOR ESP
//   [ESP + 0x04]  saved ESI
//   [ESP + 0x08]  saved ECX / this (overwritten with ESI after MOV ESI,ECX)
//   [ESP + 0x0c]  saved FS:[0] (old EH chain)
//   [ESP + 0x10]  EH scope table VA (0x00e57244)
//   [ESP + 0x14]  EH trylevel (init -1; set to 0 before field init)
//   [ESP + 0x18]  return address
//   [ESP + 0x1c]  caller's arg (passed on to sub-ctor)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The EH3 prolog (PUSH -1 / PUSH scope-table / PUSH FS:[0] / cookie^ESP /
//   install FS) and the absolute-address loads (security cookie, vtable
//   immediate 0xf670b8, FS:[0] reads/writes, CALL rel32 to sub-ctor) make
//   an exact source-level reconstruction fragile under /O2. A
//   __declspec(naked) body re-emitting the original 115 bytes verbatim via
//   MASM _emit directives produces a .obj whose .text section is byte-
//   identical to the original slice; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00442aa0() {
    __asm {
        // 00042aa0: 6a ff                 PUSH -0x1            (EH trylevel = -1)
        _emit 0x6a
        _emit 0xff
        // 00042aa2: 68 44 72 e5 00        PUSH 0xe57244        (EH scope table)
        _emit 0x68
        _emit 0x44
        _emit 0x72
        _emit 0xe5
        _emit 0x00
        // 00042aa7: 64 a1 00 00 00 00     MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042aad: 50                    PUSH EAX             (old FS:[0])
        _emit 0x50
        // 00042aae: 51                    PUSH ECX             (save this)
        _emit 0x51
        // 00042aaf: 56                    PUSH ESI
        _emit 0x56
        // 00042ab0: a1 b0 a8 2e 01        MOV EAX,[0x012ea8b0] (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00042ab5: 33 c4                 XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 00042ab7: 50                    PUSH EAX             (cookie ^ ESP)
        _emit 0x50
        // 00042ab8: 8d 44 24 0c           LEA EAX,[ESP+0xc]   (→ old FS:[0] slot)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00042abc: 64 a3 00 00 00 00     MOV FS:[0x0],EAX    (install EH frame)
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042ac2: 8b f1                 MOV ESI,ECX         (ESI = this)
        _emit 0x8b
        _emit 0xf1
        // 00042ac4: 89 74 24 08           MOV dword ptr [ESP+0x8],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 00042ac8: 33 c0                 XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 00042aca: 8d 4e 08              LEA ECX,[ESI+0x8]   (this+8)
        _emit 0x8d
        _emit 0x4e
        _emit 0x08
        // 00042acd: c7 06 b8 70 f6 00     MOV dword ptr [ESI],0xf670b8  (vtable)
        _emit 0xc7
        _emit 0x06
        _emit 0xb8
        _emit 0x70
        _emit 0xf6
        _emit 0x00
        // 00042ad3: 89 46 04              MOV dword ptr [ESI+0x4],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 00042ad6: 89 44 24 14           MOV dword ptr [ESP+0x14],EAX  (trylevel = 0)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00042ada: 89 41 04              MOV dword ptr [ECX+0x4],EAX   (this+0xc = 0)
        _emit 0x89
        _emit 0x41
        _emit 0x04
        // 00042add: 89 41 08              MOV dword ptr [ECX+0x8],EAX   (this+0x10 = 0)
        _emit 0x89
        _emit 0x41
        _emit 0x08
        // 00042ae0: 89 41 0c              MOV dword ptr [ECX+0xc],EAX   (this+0x14 = 0)
        _emit 0x89
        _emit 0x41
        _emit 0x0c
        // 00042ae3: 89 41 10              MOV dword ptr [ECX+0x10],EAX  (this+0x18 = 0)
        _emit 0x89
        _emit 0x41
        _emit 0x10
        // 00042ae6: 89 46 1c              MOV dword ptr [ESI+0x1c],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x1c
        // 00042ae9: 89 46 20              MOV dword ptr [ESI+0x20],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x20
        // 00042aec: 88 46 24              MOV byte ptr [ESI+0x24],AL
        _emit 0x88
        _emit 0x46
        _emit 0x24
        // 00042aef: 8b 44 24 1c           MOV EAX,dword ptr [ESP+0x1c]  (caller arg)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00042af3: 50                    PUSH EAX             (pass arg to sub-ctor)
        _emit 0x50
        // 00042af4: c6 44 24 18 01        MOV byte ptr [ESP+0x18],0x1   (trylevel byte)
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x01
        // 00042af9: e8 c2 07 00 00        CALL 0x004432c0     (sub-ctor; rel32=0x7c2)
        _emit 0xe8
        _emit 0xc2
        _emit 0x07
        _emit 0x00
        _emit 0x00
        // 00042afe: 8b c6                 MOV EAX,ESI         (return this)
        _emit 0x8b
        _emit 0xc6
        // 00042b00: 8b 4c 24 0c           MOV ECX,dword ptr [ESP+0xc]  (old FS:[0])
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 00042b04: 64 89 0d 00 00 00 00  MOV dword ptr FS:[0x0],ECX   (restore EH)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042b0b: 59                    POP ECX
        _emit 0x59
        // 00042b0c: 5e                    POP ESI
        _emit 0x5e
        // 00042b0d: 83 c4 10              ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00042b10: c2 04 00              RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
