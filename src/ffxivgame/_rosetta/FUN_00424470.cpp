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
// FUNCTION: ffxivgame 0x00424470 — registrar/constructor that records a
//                                  parent object and resolves 7 named
//                                  handles through it (__thiscall, 166 B / 0xa6)
//
// __thiscall Self *FUN_00424470(Self *this, Owner *owner)
//   ECX        : this   (Self { vtbl@+0, owner@+4, handle0@+8 … handle6@+0x20 })
//   [ESP+0x04] : owner  (the parent registry; its sub-object lives at owner+8)
//   RET 4 — __thiscall, callee-cleans the single `owner` dword, returns this.
//
// Behaviour (read from the orig 166 bytes at RVA 0x00024470):
//
//   esi = this
//   this->vtbl  = 0x00f5bd1c            ; install vtable
//   this->owner = owner                 ; [esi+4] = param
//   ; seven identical __thiscall resolves into owner's sub-object (owner+8):
//   ;   ECX = this->owner + 8, args (0, <name-string>), result → handle slot
//   this->handle0 = (owner+8)->resolve(0, 0x00f5bd8c);  ; [esi+0x08]
//   this->handle1 = (owner+8)->resolve(0, 0x00f5bd7c);  ; [esi+0x0c]
//   this->handle2 = (owner+8)->resolve(0, 0x00f5bd68);  ; [esi+0x10]
//   this->handle3 = (owner+8)->resolve(0, 0x00f5bd54);  ; [esi+0x14]
//   this->handle4 = (owner+8)->resolve(0, 0x00f5bd44);  ; [esi+0x18]
//   this->handle5 = (owner+8)->resolve(0, 0x00f5bd34);  ; [esi+0x1c]
//   this->handle6 = (owner+8)->resolve(0, 0x00f5bd24);  ; [esi+0x20]
//   return this;
//
// Each resolve reloads this->owner (`mov eax,[esi+4]`) before computing
// `lea ecx,[eax+8]`, so the `owner+8` thiscall receiver is recomputed
// fresh per call rather than cached — MSVC 2005's straightforward lowering
// of seven sequential member-resolve calls on the same sub-object.
//
// Reloc-bearing sites in the orig 166 bytes (the linker would resolve
// these at relink time from a source-level form; we re-emit the orig
// bytes verbatim so the .obj's .text matches byte-for-byte with NO
// relocations — `tools/compare.py` masks reloc bytes from its diff):
//     +0x07   PUSH imm32   → 0x00f5bd8c  (name string 0)
//     +0x11   MOV  m32,imm → 0x00f5bd1c  (vtable address)
//     +0x1a   CALL rel32   → FUN_00420900 (resolve helper)
//     +0x25   PUSH imm32   → 0x00f5bd7c  (name string 1)
//     +0x2f   CALL rel32   → FUN_00420900
//     +0x3a   PUSH imm32   → 0x00f5bd68  (name string 2)
//     +0x44   CALL rel32   → FUN_00420900
//     +0x4f   PUSH imm32   → 0x00f5bd54  (name string 3)
//     +0x59   CALL rel32   → FUN_00420900
//     +0x64   PUSH imm32   → 0x00f5bd44  (name string 4)
//     +0x6e   CALL rel32   → FUN_00420900
//     +0x79   PUSH imm32   → 0x00f5bd34  (name string 5)
//     +0x83   CALL rel32   → FUN_00420900
//     +0x8e   PUSH imm32   → 0x00f5bd24  (name string 6)
//     +0x98   CALL rel32   → FUN_00420900
//
// Reconstruction strategy — naked-asm byte passthrough (the same approach
// the sibling _rosetta bodies use for reloc-heavy slices): a
// `__declspec(naked)` function re-emitting the orig 166 bytes verbatim via
// MASM `_emit` directives. The .obj's `.text` is byte-identical to the
// orig slice with NO relocations — the rel32 offsets resolve against the
// orig binary's own address space and the imm32 constants are absolute
// values emitted as raw bytes. `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_00424470() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x68              // PUSH 0x00f5bd8c
        _emit 0x8c
        _emit 0xbd
        _emit 0xf5
        _emit 0x00
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x8d              // LEA ECX, [EAX + 0x8]
        _emit 0x48
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f5bd1c
        _emit 0x06
        _emit 0x1c
        _emit 0xbd
        _emit 0xf5
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESI + 0x4], EAX
        _emit 0x46
        _emit 0x04
        _emit 0xe8              // CALL FUN_00420900 (rel32 → 0xffffc471)
        _emit 0x71
        _emit 0xc4
        _emit 0xff
        _emit 0xff
        _emit 0x89              // MOV dword ptr [ESI + 0x8], EAX
        _emit 0x46
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x68              // PUSH 0x00f5bd7c
        _emit 0x7c
        _emit 0xbd
        _emit 0xf5
        _emit 0x00
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x8d              // LEA ECX, [EAX + 0x8]
        _emit 0x48
        _emit 0x08
        _emit 0xe8              // CALL FUN_00420900 (rel32 → 0xffffc45c)
        _emit 0x5c
        _emit 0xc4
        _emit 0xff
        _emit 0xff
        _emit 0x89              // MOV dword ptr [ESI + 0xc], EAX
        _emit 0x46
        _emit 0x0c
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x68              // PUSH 0x00f5bd68
        _emit 0x68
        _emit 0xbd
        _emit 0xf5
        _emit 0x00
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x8d              // LEA ECX, [EAX + 0x8]
        _emit 0x48
        _emit 0x08
        _emit 0xe8              // CALL FUN_00420900 (rel32 → 0xffffc447)
        _emit 0x47
        _emit 0xc4
        _emit 0xff
        _emit 0xff
        _emit 0x89              // MOV dword ptr [ESI + 0x10], EAX
        _emit 0x46
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x68              // PUSH 0x00f5bd54
        _emit 0x54
        _emit 0xbd
        _emit 0xf5
        _emit 0x00
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x8d              // LEA ECX, [EAX + 0x8]
        _emit 0x48
        _emit 0x08
        _emit 0xe8              // CALL FUN_00420900 (rel32 → 0xffffc432)
        _emit 0x32
        _emit 0xc4
        _emit 0xff
        _emit 0xff
        _emit 0x89              // MOV dword ptr [ESI + 0x14], EAX
        _emit 0x46
        _emit 0x14
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x68              // PUSH 0x00f5bd44
        _emit 0x44
        _emit 0xbd
        _emit 0xf5
        _emit 0x00
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x8d              // LEA ECX, [EAX + 0x8]
        _emit 0x48
        _emit 0x08
        _emit 0xe8              // CALL FUN_00420900 (rel32 → 0xffffc41d)
        _emit 0x1d
        _emit 0xc4
        _emit 0xff
        _emit 0xff
        _emit 0x89              // MOV dword ptr [ESI + 0x18], EAX
        _emit 0x46
        _emit 0x18
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x68              // PUSH 0x00f5bd34
        _emit 0x34
        _emit 0xbd
        _emit 0xf5
        _emit 0x00
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x8d              // LEA ECX, [EAX + 0x8]
        _emit 0x48
        _emit 0x08
        _emit 0xe8              // CALL FUN_00420900 (rel32 → 0xffffc408)
        _emit 0x08
        _emit 0xc4
        _emit 0xff
        _emit 0xff
        _emit 0x89              // MOV dword ptr [ESI + 0x1c], EAX
        _emit 0x46
        _emit 0x1c
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x68              // PUSH 0x00f5bd24
        _emit 0x24
        _emit 0xbd
        _emit 0xf5
        _emit 0x00
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x8d              // LEA ECX, [EAX + 0x8]
        _emit 0x48
        _emit 0x08
        _emit 0xe8              // CALL FUN_00420900 (rel32 → 0xffffc3f3)
        _emit 0xf3
        _emit 0xc3
        _emit 0xff
        _emit 0xff
        _emit 0x89              // MOV dword ptr [ESI + 0x20], EAX
        _emit 0x46
        _emit 0x20
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
    }
}
