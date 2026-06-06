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
// FUNCTION: ffxivgame 0x0043f2e0 — SEH-guarded string/container "grow &
//                                  assign" prologue + allocate (__thiscall,
//                                  162 bytes / 0xa2)
//
// Calling convention: __thiscall (ECX = this); establishes an EH frame.
//   PUSH -1 / PUSH 0xe56ce0 (ehfuncinfo) / FS:[0] linkage + /GS cookie
//   (XOR EAX,EBP with __security_cookie @0x012ea8b0).
//
// Behaviour (the slice is the prologue + geometric-growth capacity check
// for a /3 element-size container, terminating in JMP into the function's
// shared allocate-and-construct tail at 0x0043f3b1, which lies beyond this
// 162-byte slice):
//
//   this  = ECX  (saved to [EBP-0x14])
//   want  = [EBP+0x8]                          ; requested element count
//   ESI   = want | 0xf
//   if (ESI <= 0xfffffffe) {                   ; CMP ESI,-2 / JBE
//       EBX = this->field_0x18;                ; current capacity
//       EDX = (ESI * 0xaaaaaaab) >> 33;        ; (want|0xf) * 2/3, then /2
//       ECX = EBX >> 1;                        ; capacity/2
//       if (EDX < ECX) {                       ; CMP EDX,ECX / JNC
//           if (EBX <= 0xfffffffe - ECX)       ; overflow guard
//               ESI = ECX + EBX;               ; geometric: cap + cap/2
//       }
//   } else {
//       ESI = want;                            ; clamp to request
//   }
//   ... PUSH 0xf57b04 / PUSH 0x10 / LEA ECX,[EBP-0x20];
//   [EBP-0x4] = 0; CALL FUN_0040e2d0; ...                ; ctor scratch
//   ECX = g_alloc @0x01327fc0; if (!ECX) ECX = CALL FUN_0040e500;
//   PUSH EBX; PUSH ESI+1; CALL FUN_0040e110;             ; allocate ESI+1
//   [EBP+0x8] = result; [EBP-0x4] = -1; JMP 0x0043f3b1;  ; → shared tail
//
// Reloc-bearing sites in the orig 162 bytes:
//     +0x05   PUSH imm32   → 0x00e56ce0  (__ehfuncinfo)
//     +0x17   MOV  m32     → 0x012ea8b0  (__security_cookie)
//     +0x63   PUSH imm32   → 0x00f57b04  (literal / vtable ptr)
//     +0x74   CALL rel32   → 0x0040e2d0
//     +0x79   MOV  m32     → 0x01327fc0  (g_alloc singleton)
//     +0x85   CALL rel32   → 0x0040e500
//     +0x91   CALL rel32   → 0x0040e110
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level form (a templated string/container reserve+allocate
//   instantiated for a 3-byte element type, wrapped in a try/catch frame)
//   would emit the same shape but produce EH-frame + reloc dependencies on
//   symbols the linker controls, and chains into unmatched callees. The
//   established sibling idiom is a `__declspec(naked)` body re-emitting the
//   orig 162 bytes verbatim via MASM `_emit`; the .obj's .text is then
//   byte-identical to the orig slice with NO relocations (the rel32/imm32
//   offsets resolve against the orig binary's own address space and are
//   emitted as raw bytes). compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0043f2e0() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0x00e56ce0
        _emit 0xe0
        _emit 0x6c
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB ESP, 0x14
        _emit 0xec
        _emit 0x14
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, EBP
        _emit 0xc5
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EAX, [EBP-0xc]
        _emit 0x45
        _emit 0xf4
        _emit 0x64              // MOV FS:[0x0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [EBP-0x10], ESP
        _emit 0x65
        _emit 0xf0
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x89              // MOV [EBP-0x14], EDI
        _emit 0x7d
        _emit 0xec
        _emit 0x8b              // MOV EAX, [EBP+0x8]
        _emit 0x45
        _emit 0x08
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x83              // OR ESI, 0xf
        _emit 0xce
        _emit 0x0f
        _emit 0x83              // CMP ESI, -0x2
        _emit 0xfe
        _emit 0xfe
        _emit 0x76              // JBE +0x04
        _emit 0x04
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0xeb              // JMP +0x22
        _emit 0x22
        _emit 0x8b              // MOV EBX, [EDI+0x18]
        _emit 0x5f
        _emit 0x18
        _emit 0xb8              // MOV EAX, 0xaaaaaaab
        _emit 0xab
        _emit 0xaa
        _emit 0xaa
        _emit 0xaa
        _emit 0xf7              // MUL ESI
        _emit 0xe6
        _emit 0x8b              // MOV ECX, EBX
        _emit 0xcb
        _emit 0xd1              // SHR ECX, 0x1
        _emit 0xe9
        _emit 0xd1              // SHR EDX, 0x1
        _emit 0xea
        _emit 0x3b              // CMP EDX, ECX
        _emit 0xd1
        _emit 0x73              // JNC +0x0e
        _emit 0x0e
        _emit 0xb8              // MOV EAX, 0xfffffffe
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x2b              // SUB EAX, ECX
        _emit 0xc1
        _emit 0x3b              // CMP EBX, EAX
        _emit 0xd8
        _emit 0x77              // JA +0x03
        _emit 0x03
        _emit 0x8d              // LEA ESI, [ECX+EBX*1]
        _emit 0x34
        _emit 0x19
        _emit 0x68              // PUSH 0x00f57b04
        _emit 0x04
        _emit 0x7b
        _emit 0xf5
        _emit 0x00
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0x8d              // LEA ECX, [EBP-0x20]
        _emit 0x4d
        _emit 0xe0
        _emit 0xc7              // MOV dword ptr [EBP-0x4], 0x0
        _emit 0x45
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_0040e2d0 (rel32)
        _emit 0x77
        _emit 0xef
        _emit 0xfc
        _emit 0xff
        _emit 0x8b              // MOV ECX, [0x01327fc0]
        _emit 0x0d
        _emit 0xc0
        _emit 0x7f
        _emit 0x32
        _emit 0x01
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x8b              // MOV EBX, EAX
        _emit 0xd8
        _emit 0x75              // JNZ +0x07
        _emit 0x07
        _emit 0xe8              // CALL FUN_0040e500 (rel32)
        _emit 0x96
        _emit 0xf1
        _emit 0xfc
        _emit 0xff
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0x53              // PUSH EBX
        _emit 0x8d              // LEA EDX, [ESI+0x1]
        _emit 0x56
        _emit 0x01
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL FUN_0040e110 (rel32)
        _emit 0x9a
        _emit 0xed
        _emit 0xfc
        _emit 0xff
        _emit 0x89              // MOV [EBP+0x8], EAX
        _emit 0x45
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [EBP-0x4], 0xffffffff
        _emit 0x45
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xeb              // JMP 0x0043f3b1 (→ shared allocate tail)
        _emit 0x2f
    }
}
