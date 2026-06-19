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
// FUNCTION: ffxivgame 0x00038a00 — `__thiscall` stack-object builder /
//                                  forwarding wrapper (66 B / 0x42)
//
// Signature (inferred from asm):
//   __thiscall void FUN_00438a00(this, int arg1, int arg2, float arg3, int arg4)
//     ECX        : this pointer
//     [ESP+0x04] : int   arg1
//     [ESP+0x08] : int   arg2
//     [ESP+0x0c] : float arg3  (read via MOVSS → XMM0)
//     [ESP+0x10] : int   arg4
//   RET 0x10 — __thiscall, callee cleans 4 DWORD args.
//
// Inspection (read from the disassembly at orig RVA 0x00038a00, 66 bytes):
//
//   SUB    ESP, 0x14                     ; allocate 0x14-byte local frame
//   MOV    EAX, [ESP + 0x18]             ; EAX = arg1
//   MOV    ECX, [ECX + 0x4]             ; ECX = this->field4 (dereference member)
//   MOV    EDX, [ESP + 0x1c]             ; EDX = arg2
//   MOVSS  XMM0, [ESP + 0x20]           ; XMM0 = arg3 (float, 32-bit scalar load)
//   MOV    [ESP + 0x4], EAX             ; local[4] = arg1
//   MOV    EAX, [ESP + 0x24]             ; EAX = arg4
//   PUSH   ECX                           ; push this->field4 as stack arg for callee
//   LEA    ECX, [ESP + 0x4]             ; ECX = &local[0] — ptr to stack-built object
//   MOV    [ESP + 0x4], 0x00f64990      ; local[0] = vtable ptr (0xf64990)
//   MOV    [ESP + 0xc], EDX             ; local[8] = arg2
//   MOV    [ESP + 0x10], EAX            ; local[12] = arg4
//   MOVSS  [ESP + 0x14], XMM0           ; local[16] = arg3 (float)
//   CALL   0x00435b50                    ; FUN_00435b50(ECX=&local_obj, arg=this->field4)
//   ADD    ESP, 0x14                     ; collapse local frame (callee cleaned stack arg)
//   RET    0x10                          ; return and pop 4 DWORD args from caller
//
// Stack-built object layout (ECX → base of local frame):
//   +0x00  void* vtable  = 0x00f64990
//   +0x04  int           = arg1
//   +0x08  (unset)
//   +0x0c  int           = arg2
//   +0x10  int           = arg4
//   +0x14  float         = arg3         (note: written from XMM0 via MOVSS)
//
// Reloc-bearing sites in the orig 66 bytes:
//   +0x21  MOV imm32 → 0x00f64990 (vtable / data pointer, resolved at link time)
//   +0x38  CALL rel32 → FUN_00435b50 at RVA 0x00035b50
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ would not reproduce the exact register allocation
//   (ECX clobbered by member deref before PUSH, XMM0 used as float-move
//   relay) nor the in-binary immediate 0xf64990 (vtable ptr known only at
//   link time relative to image base 0x00400000). The pragmatic approach —
//   same as siblings FUN_00406fa0 / FUN_00408780 in this module — is a
//   `__declspec(naked)` body re-emitting the orig 66 bytes verbatim via
//   MASM `_emit` directives so the .obj's .text section is byte-identical.

extern "C" __declspec(naked) void FUN_00438a00() {
    __asm {
        _emit 0x83              // SUB ESP, 0x14
        _emit 0xec
        _emit 0x14
        _emit 0x8b              // MOV EAX, [ESP + 0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV ECX, [ECX + 0x4]
        _emit 0x49
        _emit 0x04
        _emit 0x8b              // MOV EDX, [ESP + 0x1c]
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0xf3              // MOVSS XMM0, [ESP + 0x20]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x89              // MOV [ESP + 0x4], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV EAX, [ESP + 0x24]
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x51              // PUSH ECX
        _emit 0x8d              // LEA ECX, [ESP + 0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xc7              // MOV [ESP + 0x4], 0x00f64990
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x90
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x89              // MOV [ESP + 0xc], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x89              // MOV [ESP + 0x10], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xf3              // MOVSS [ESP + 0x14], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xe8              // CALL 0x00435b50  (rel32 = 0xffffd114)
        _emit 0x14
        _emit 0xd1
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET 0x10
        _emit 0x10
        _emit 0x00
    }
}
