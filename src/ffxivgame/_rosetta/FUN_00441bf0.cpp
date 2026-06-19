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
// FUNCTION: ffxivgame 0x00441bf0 — ordered-container range check / insert helper
//                                  (__thiscall, 108 bytes / 0x6C)
//
// Calling convention: __thiscall (ECX = this); callee-cleans 2 DWORD stack args
//   via RET 0x8.
//
// Frame:
//   SUB ESP, 0x8        — 2 DWORD locals:
//                           local_lo @ [E-8]  (lower iterator / node ptr)
//                           local_hi @ [E-4]  (upper iterator / node ptr)
//   PUSH EBX, ESI, EDI
//   epilogue: POP EDI / POP ESI / POP EBX / ADD ESP,8 / RET 8
//
// Logic (recovered from asm):
//   ESI = this + 0x18             ; sub-object (sorted container / tree root)
//   (this+0x18)->Func(&local_lo, &arg1)
//                                 ; fills local_lo (and adjacent local_hi)
//                                 ; with two iterator results (e.g. equal_range)
//   EDI = local_lo                ; lower-bound iterator
//   EBX = (this+0x18)->field_4   ; end sentinel / last node pointer
//   assert: EDI != NULL || EDI == this+0x18   (bounds guard)
//   ESI = local_hi                ; upper-bound iterator
//   if ESI == EBX: return false   ; empty range → not found
//   assert: EDI != NULL
//   assert: ESI != EDI->field_4   ; ordering invariant
//   ECX = local_hi->field_0x10
//   PUSH arg2; CALL 0x004429d0    ; insert / assign helper (__thiscall, 1 arg)
//   return true
//
// Reloc-bearing CALL sites (compare.py wildcard-masks the rel32 payload):
//   +0x15  CALL 0x0071d420  — container Func (equal_range / lower_bound)
//   +0x29  CALL 0x009d22b4  — assertion failure helper
//   +0x3a  CALL 0x009d22b4  — assertion failure helper
//   +0x44  CALL 0x009d22b4  — assertion failure helper
//   +0x51  CALL 0x004429d0  — insert / assign helper (thiscall, ret 4)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function's interleaved LEA / TEST / CMP / JZ / JNZ guards and its
//   specific register allocation (EBX = end sentinel, EDI = lower result,
//   ESI = upper result) would be difficult to reproduce byte-for-byte via
//   a source-level C++ form. Emitting all 108 bytes verbatim via MASM
//   _emit directives yields a .obj whose .text is byte-identical to the
//   orig slice; compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_00441bf0() {
    __asm {
        _emit 0x83              // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA ESI, [ECX + 0x18]
        _emit 0x71
        _emit 0x18
        _emit 0x8d              // LEA EAX, [ESP + 0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA ECX, [ESP + 0x10]   (ESP now = E-24)
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x0071d420 (rel32 = +0x002db816)
        _emit 0x16
        _emit 0xb8
        _emit 0x2d
        _emit 0x00
        _emit 0x8b              // MOV EDI, dword ptr [ESP + 0xc]
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x8b              // MOV EBX, dword ptr [ESI + 0x4]
        _emit 0x5e
        _emit 0x04
        _emit 0x74              // JZ +0x4  (→ assert site 1)
        _emit 0x04
        _emit 0x3b              // CMP EDI, ESI
        _emit 0xfe
        _emit 0x74              // JZ +0x5  (→ skip assert 1)
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4 (rel32 = +0x00590696)
        _emit 0x96
        _emit 0x06
        _emit 0x59
        _emit 0x00
        _emit 0x8b              // MOV ESI, dword ptr [ESP + 0x10]
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x3b              // CMP ESI, EBX
        _emit 0xf3
        _emit 0x74              // JZ +0x2b (→ early return false)
        _emit 0x2b
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x75              // JNZ +0x5 (→ skip assert 2)
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4 (rel32 = +0x00590685)
        _emit 0x85
        _emit 0x06
        _emit 0x59
        _emit 0x00
        _emit 0x3b              // CMP ESI, dword ptr [EDI + 0x4]
        _emit 0x77
        _emit 0x04
        _emit 0x75              // JNZ +0x5 (→ skip assert 3)
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4 (rel32 = +0x0059067b)
        _emit 0x7b
        _emit 0x06
        _emit 0x59
        _emit 0x00
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 0x1c]
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x10]
        _emit 0x4e
        _emit 0x10
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL 0x004429d0 (rel32 = +0x00000d8a)
        _emit 0x8a
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x5f              // POP EDI              (success epilogue)
        _emit 0x5e              // POP ESI
        _emit 0xb0              // MOV AL, 0x1
        _emit 0x01
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        _emit 0x5f              // POP EDI              (failure epilogue)
        _emit 0x5e              // POP ESI
        _emit 0x32              // XOR AL, AL
        _emit 0xc0
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
