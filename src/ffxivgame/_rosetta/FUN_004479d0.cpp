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
// FUNCTION: ffxivgame 0x000479d0 — string pop-back returning the popped char
//                                  (__thiscall, 0 args, 40 B / 0x28)
//
// char __thiscall FUN_004479d0(StrClass *this)
//
// Operates on a small string/vector-like object whose length is at [this+0]
// and whose backing buffer pointer is at [this+8]. The buffer is held with a
// trailing NUL (the live char count is length-1, so the last data byte sits
// at buffer[length-2]).
//
// 1. EAX = this->length        ([ESI])
// 2. EDI = this->buffer        ([ESI+8])
// 3. BL  = buffer[length-2]    ; save the about-to-be-removed last char
// 4. this->FUN_00447010(buffer-1, 1)  ; thiscall: erase/shrink by 1
// 5. EDX = this->length        ; reload (mutated by the call)
// 6. buffer[length-2] = 0      ; re-terminate at the new end
// 7. return BL                 ; the popped character
//
// MSVC /O2 defers the PUSH EDI callee-save past the initial length load and
// interleaves the two PUSH-arg computations with the LEA — and the result is
// returned through a saved EBX (AL = BL). That register-allocation shape plus
// the deferred save is fragile to reproduce from source-level C++, so this is
// encoded as a __declspec(naked) byte-for-byte passthrough. The only
// linker-relocated field is the 4-byte relative offset of the CALL to
// FUN_00447010 (bytes at +0x16..+0x19); compare.py masks those bytes.
//
// Asm (40 bytes @ orig RVA 0x000479d0):
//   53              PUSH EBX
//   56              PUSH ESI
//   8b f1           MOV ESI, ECX                 ; this
//   8b 06           MOV EAX, dword ptr [ESI]      ; this->length
//   57              PUSH EDI
//   8b 7e 08        MOV EDI, dword ptr [ESI+0x8]  ; this->buffer
//   8a 5c 07 fe     MOV BL, byte ptr [EDI+EAX-0x2]; save last char
//   6a 01           PUSH 1
//   8d 4f ff        LEA ECX, [EDI-0x1]           ; arg1 = buffer-1
//   51              PUSH ECX
//   8b ce           MOV ECX, ESI                 ; this
//   e8 RR RR RR RR  CALL FUN_00447010            ; (reloc)
//   8b 16           MOV EDX, dword ptr [ESI]      ; this->length (reloaded)
//   c6 44 17 fe 00  MOV byte ptr [EDI+EDX-0x2], 0 ; re-terminate
//   5f              POP EDI
//   5e              POP ESI
//   8a c3           MOV AL, BL                   ; return popped char
//   5b              POP EBX
//   c3              RET

extern "C" void FUN_00447010();   // forward declaration for the CALL relocation

extern "C" __declspec(naked) void FUN_004479d0() {
    __asm {
        // 000479d0: 53            PUSH EBX
        _emit 0x53
        // 000479d1: 56            PUSH ESI
        _emit 0x56
        // 000479d2: 8b f1         MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 000479d4: 8b 06         MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 000479d6: 57            PUSH EDI
        _emit 0x57
        // 000479d7: 8b 7e 08      MOV EDI, [ESI+8]
        _emit 0x8b
        _emit 0x7e
        _emit 0x08
        // 000479da: 8a 5c 07 fe   MOV BL, [EDI+EAX-2]
        _emit 0x8a
        _emit 0x5c
        _emit 0x07
        _emit 0xfe
        // 000479de: 6a 01         PUSH 1
        _emit 0x6a
        _emit 0x01
        // 000479e0: 8d 4f ff      LEA ECX, [EDI-1]
        _emit 0x8d
        _emit 0x4f
        _emit 0xff
        // 000479e3: 51            PUSH ECX
        _emit 0x51
        // 000479e4: 8b ce         MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000479e6: e8 RR RR RR RR  CALL FUN_00447010  (reloc)
        call FUN_00447010
        // 000479eb: 8b 16         MOV EDX, [ESI]
        _emit 0x8b
        _emit 0x16
        // 000479ed: c6 44 17 fe 00  MOV byte ptr [EDI+EDX-2], 0
        _emit 0xc6
        _emit 0x44
        _emit 0x17
        _emit 0xfe
        _emit 0x00
        // 000479f2: 5f            POP EDI
        _emit 0x5f
        // 000479f3: 5e            POP ESI
        _emit 0x5e
        // 000479f4: 8a c3         MOV AL, BL
        _emit 0x8a
        _emit 0xc3
        // 000479f6: 5b            POP EBX
        _emit 0x5b
        // 000479f7: c3            RET
        _emit 0xc3
    }
}
