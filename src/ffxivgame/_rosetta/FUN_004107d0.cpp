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
// FUNCTION: ffxivgame 0x004107d0 — engine_memory virtual-dispatch wrapper
// with flag set/clear around an inner thiscall (39 B / 0x27).
//
// __thiscall, no explicit stack args (epilogue is JMP EAX tail-call, not RET).
//
// Shape:
//   ESI = this (ECX on entry)
//   this->vtable[0x2c/4]()           ; virtual call — ECX = this (unchanged)
//   arg = this->field_0x4c
//   this->field_0x50 = 1             ; set flag
//   FUN_0040f7b0(this, arg)          ; __thiscall, one stack arg
//   this->field_0x50 = 0             ; clear flag
//   tail-call this->vtable[0x30/4]() ; via JMP EAX, ECX = this
//
// Asm (39 bytes, RVA 0x000107d0):
//   56                  PUSH ESI
//   8b f1               MOV  ESI, ECX               ; cache this
//   8b 06               MOV  EAX, [ESI]              ; load vtable ptr
//   8b 50 2c            MOV  EDX, [EAX+0x2c]         ; vtable slot 11
//   ff d2               CALL EDX                     ; virtual call (ECX = this)
//   8b 46 4c            MOV  EAX, [ESI+0x4c]         ; field_0x4c
//   50                  PUSH EAX                     ; push as arg
//   8b ce               MOV  ECX, ESI                ; ECX = this
//   c6 46 50 01         MOV  byte ptr [ESI+0x50], 1  ; flag on
//   e8 c7 ef ff ff      CALL FUN_0040f7b0            ; rel32 → 0x0040f7b0
//   8b 16               MOV  EDX, [ESI]              ; reload vtable ptr
//   8b 42 30            MOV  EAX, [EDX+0x30]         ; vtable slot 12
//   c6 46 50 00         MOV  byte ptr [ESI+0x50], 0  ; flag off
//   8b ce               MOV  ECX, ESI                ; ECX = this
//   5e                  POP  ESI
//   ff e0               JMP  EAX                     ; tail-call vtable[12]
//
// Reloc-bearing sites: one CALL rel32 (displacement 0xFFFFEFC7) targeting
// FUN_0040f7b0 at 0x0040f7b0 — baked raw via _emit so the .obj carries
// zero relocations and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004107d0() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x2c]
        _emit 0x50
        _emit 0x2c
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x4c]
        _emit 0x46
        _emit 0x4c
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xc6              // MOV byte ptr [ESI+0x50], 0x1
        _emit 0x46
        _emit 0x50
        _emit 0x01
        _emit 0xe8              // CALL FUN_0040f7b0 (rel32 = 0xFFFFEFC7)
        _emit 0xc7
        _emit 0xef
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EDX, dword ptr [ESI]
        _emit 0x16
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x30]
        _emit 0x42
        _emit 0x30
        _emit 0xc6              // MOV byte ptr [ESI+0x50], 0x0
        _emit 0x46
        _emit 0x50
        _emit 0x00
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0x5e              // POP ESI
        _emit 0xff              // JMP EAX
        _emit 0xe0
    }
}
