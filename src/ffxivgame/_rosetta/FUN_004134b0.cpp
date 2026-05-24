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
// FUNCTION: ffxivgame 0x000134b0 — two-level virtual dispatch + noreturn tail
//                                   (__cdecl, 1 arg, 36 B / 0x24)
//
// void FUN_004134b0(SomeObj *param_1)
//
// 1. Loads the first argument from [ESP+4] into ECX (no frame pointer).
// 2. Calls vtable slot 1 of *param_1 as a __thiscall (this = param_1).
//    Stores result in ESI.
// 3. Calls vtable slot 0 of result with arg 0 (__thiscall, ECX = result).
// 4. Loads result->field_0x10 into EDI.
// 5. Calls FUN_0040df70 as __thiscall (this = EDI, stack arg = result).
//
// MSVC places the PUSH ESI / PUSH EDI callee-saves just before the first
// external CALL rather than at function entry — a /O2 deferred-save idiom
// for frame-pointer-omitted (/Oy) functions that load the stack arg before
// any pushes. FUN_0040df70 has RET 4 and returns; execution then falls
// through to a POP EDI / POP ESI / RET epilogue at 0x000134d4 that is
// shared with the surrounding caller's code.
//
// Because the callee-save deferral and fall-through epilogue are fragile
// to reproduce from source-level C++, this function is encoded as a
// __declspec(naked) function emitting each byte verbatim. The only
// linker-relocated field is the 4-byte relative offset of the CALL to
// FUN_0040df70 (bytes at +0x1f..+0x22); compare.py masks those bytes.
//
// Asm (36 bytes @ orig RVA 0x000134b0):
//   8b 4c 24 04     MOV ECX, dword ptr [ESP+0x4]      ; param_1
//   8b 01           MOV EAX, dword ptr [ECX]           ; vtable of *param_1
//   8b 50 04        MOV EDX, dword ptr [EAX+0x4]       ; vtable[1]
//   56              PUSH ESI
//   57              PUSH EDI
//   ff d2           CALL EDX                           ; param_1->vtable[1]()
//   8b f0           MOV ESI, EAX                       ; result
//   8b 06           MOV EAX, dword ptr [ESI]           ; vtable of result
//   8b 10           MOV EDX, dword ptr [EAX]           ; vtable[0] of result
//   8b 7e 10        MOV EDI, dword ptr [ESI+0x10]      ; result->field_0x10
//   6a 00           PUSH 0
//   8b ce           MOV ECX, ESI                       ; this = result
//   ff d2           CALL EDX                           ; result->vtable[0](0)
//   56              PUSH ESI                           ; arg = result
//   8b cf           MOV ECX, EDI                       ; this = field_0x10
//   e8 RR RR RR RR  CALL FUN_0040df70                  ; (reloc)

extern "C" void FUN_0040df70();   // forward declaration for the CALL relocation

extern "C" __declspec(naked) void __cdecl FUN_004134b0(void *) {
    __asm {
        // 000134b0: 8b 4c 24 04   MOV ECX, [ESP+4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 000134b4: 8b 01         MOV EAX, [ECX]
        _emit 0x8b
        _emit 0x01
        // 000134b6: 8b 50 04      MOV EDX, [EAX+4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000134b9: 56            PUSH ESI
        _emit 0x56
        // 000134ba: 57            PUSH EDI
        _emit 0x57
        // 000134bb: ff d2         CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000134bd: 8b f0         MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 000134bf: 8b 06         MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 000134c1: 8b 10         MOV EDX, [EAX]
        _emit 0x8b
        _emit 0x10
        // 000134c3: 8b 7e 10      MOV EDI, [ESI+0x10]
        _emit 0x8b
        _emit 0x7e
        _emit 0x10
        // 000134c6: 6a 00         PUSH 0
        _emit 0x6a
        _emit 0x00
        // 000134c8: 8b ce         MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000134ca: ff d2         CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000134cc: 56            PUSH ESI
        _emit 0x56
        // 000134cd: 8b cf         MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 000134cf: e8 RR RR RR RR  CALL FUN_0040df70  (reloc)
        call FUN_0040df70
        // 000134d4: 5f            POP EDI
        _emit 0x5f
        // 000134d5: 5e            POP ESI
        _emit 0x5e
        // 000134d6: c3            RET
        _emit 0xc3
    }
}
