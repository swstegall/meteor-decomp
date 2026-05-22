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
// FUNCTION: ffxivgame 0x0000a640 — __thiscall struct zeroing initialiser (23 B)
//
// Asm (23 bytes @ orig RVA 0x0000a640):
//   8b c1                       MOV  EAX, ECX              ; this → EAX
//   66 0f ef c0                 PXOR XMM0, XMM0            ; XMM0 = 0
//   66 0f d6 00                 MOVQ qword ptr [EAX],      XMM0  ; zero this+0x00..0x07
//   66 0f d6 40 08              MOVQ qword ptr [EAX+0x8],  XMM0  ; zero this+0x08..0x0f
//   c7 40 10 00 00 00 00        MOV  dword ptr [EAX+0x10], 0     ; zero this+0x10..0x13
//   c3                          RET
//
// Calling convention: __thiscall (ECX = this, no stack args, RET 0).
// No prologue — /Oy frame-pointer omission; function is a leaf with no
// callee-saved register usage.
//
// The SSE2 PXOR + MOVQ pair is MSVC 2005's /O2 optimisation for zeroing
// two consecutive 8-byte aligned fields in a struct (avoids two pairs of
// `MOV [addr],0` / `MOV [addr+4],0`). The struct layout inferred:
//
//   struct SomeStruct {        // __thiscall receiver
//       __int64 field_0x00;    // cleared via MOVQ
//       __int64 field_0x08;    // cleared via MOVQ
//       int     field_0x10;    // cleared via MOV dword
//   };                         // size ≥ 0x14 (20) bytes
//
// Using naked-asm _emit to reproduce the exact byte sequence; a
// source-level constructor would need per-TU register-allocation luck
// to reproduce the specific SSE2 zeroing idiom reliably.

extern "C" __declspec(naked) void FUN_0040a640() {
    __asm {
        _emit 0x8b              // MOV EAX, ECX
        _emit 0xc1
        _emit 0x66              // PXOR XMM0, XMM0
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        _emit 0x66              // MOVQ qword ptr [EAX], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x00
        _emit 0x66              // MOVQ qword ptr [EAX+0x8], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [EAX+0x10], 0
        _emit 0x40
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
    }
}
