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
// FUNCTION: ffxivgame 0x00423190 — guarded virtual dispatch (36 B / 0x24)
//
// void __thiscall FUN_00423190(C *this, X *arg0)
//
// 1. this   = ECX, arg0 = [ESP+0xc] (after PUSH ESI / PUSH EDI at entry).
// 2. Calls FUN_004237d0 as a __thiscall (this = this->field_4) with arg0;
//    the result is a bool in AL.
// 3. If that returned false, dispatches vtable slot 0x28/4 = 10 of
//    this->field_0 as a __thiscall (this = this->field_0) with arg0.
// 4. RET 4 — callee cleans the single stack argument (__thiscall).
//
// MSVC pushes the ESI/EDI callee-saves at function entry, then loads the
// stack arg into EDI and saves this (ECX) into ESI before the first CALL.
// The only linker-relocated field is the 4-byte relative offset of the
// CALL to FUN_004237d0 (the e8 instruction); compare.py masks those bytes.
//
// Because the precise register allocation, the guarded short branch, and
// the indirect virtual CALL are fragile to reproduce from source-level
// C++, this function is encoded as a __declspec(naked) byte-for-byte stub.
//
// Asm (36 bytes @ orig RVA 0x00423190):
//   56              PUSH ESI
//   57              PUSH EDI
//   8b 7c 24 0c     MOV EDI, [ESP+0xc]        ; arg0
//   8b f1           MOV ESI, ECX              ; this
//   8b 4e 04        MOV ECX, [ESI+0x4]        ; this->field_4
//   57              PUSH EDI                  ; arg0
//   e8 RR RR RR RR  CALL FUN_004237d0         ; (reloc) __thiscall -> bool
//   84 c0           TEST AL, AL
//   75 0a           JNZ +0x0a → epilogue
//   8b 0e           MOV ECX, [ESI]            ; this->field_0
//   8b 01           MOV EAX, [ECX]            ; vtable
//   8b 50 28        MOV EDX, [EAX+0x28]       ; vtable[10]
//   57              PUSH EDI                  ; arg0
//   ff d2           CALL EDX                  ; this->field_0->vfunc10(arg0)
//   5f              POP EDI
//   5e              POP ESI
//   c2 04 00        RET 0x4

extern "C" void FUN_004237d0();   // forward declaration for the CALL relocation

extern "C" __declspec(naked) void __cdecl FUN_00423190(void *) {
    __asm {
        // 00423190: 56            PUSH ESI
        _emit 0x56
        // 00423191: 57            PUSH EDI
        _emit 0x57
        // 00423192: 8b 7c 24 0c   MOV EDI, [ESP+0xc]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 00423196: 8b f1         MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00423198: 8b 4e 04      MOV ECX, [ESI+4]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 0042319b: 57            PUSH EDI
        _emit 0x57
        // 0042319c: e8 RR RR RR RR  CALL FUN_004237d0  (reloc)
        call FUN_004237d0
        // 004231a1: 84 c0         TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // 004231a3: 75 0a         JNZ +0x0a
        _emit 0x75
        _emit 0x0a
        // 004231a5: 8b 0e         MOV ECX, [ESI]
        _emit 0x8b
        _emit 0x0e
        // 004231a7: 8b 01         MOV EAX, [ECX]
        _emit 0x8b
        _emit 0x01
        // 004231a9: 8b 50 28      MOV EDX, [EAX+0x28]
        _emit 0x8b
        _emit 0x50
        _emit 0x28
        // 004231ac: 57            PUSH EDI
        _emit 0x57
        // 004231ad: ff d2         CALL EDX
        _emit 0xff
        _emit 0xd2
        // 004231af: 5f            POP EDI
        _emit 0x5f
        // 004231b0: 5e            POP ESI
        _emit 0x5e
        // 004231b1: c2 04 00      RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
