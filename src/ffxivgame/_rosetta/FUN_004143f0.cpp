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
// FUNCTION: ffxivgame 0x000143f0 — two-level vtable dispatch + cdecl free
//                                   (__cdecl, 1 arg, 35 B / 0x23)
//
// void __cdecl FUN_004143f0(SomeObj *param_1)
//
// 1. Loads the first argument from [ESP+4] into ECX (no frame pointer).
// 2. Calls vtable slot 1 of *param_1 as a __thiscall (this = param_1).
//    Stores result in ESI.
// 3. Calls vtable slot 0 of result with arg 0 (__thiscall, ECX = result).
// 4. Calls FUN_009d56fd(result) as __cdecl and cleans up the stack.
//
// MSVC defers the PUSH ESI callee-save until after [ESP+0x4] has been
// loaded (an /O2 /Oy idiom for frame-pointer-omitted functions). The
// POP ESI / RET epilogue lives right after the cdecl-call cleanup —
// straightforward but structurally fragile to coax out of a source-level
// C++ snippet, so we emit the bytes as a __declspec(naked) function.
// The only linker-relocated field is the 4-byte rel32 of the CALL to
// FUN_009d56fd (bytes +0x1a..+0x1d); compare.py masks those bytes.
//
// Structurally near-identical to sibling FUN_004134b0 at 0x000134b0,
// minus the EDI save and with a __cdecl FUN_009d56fd(result) tail in
// place of a __thiscall on result->field_0x10.
//
// Asm (35 bytes @ orig RVA 0x000143f0):
//   8b 4c 24 04     MOV ECX, dword ptr [ESP+0x4]      ; param_1
//   8b 01           MOV EAX, dword ptr [ECX]           ; vtable of *param_1
//   8b 50 04        MOV EDX, dword ptr [EAX+0x4]       ; vtable[1]
//   56              PUSH ESI
//   ff d2           CALL EDX                           ; param_1->vtable[1]()
//   8b f0           MOV ESI, EAX                       ; result
//   8b 06           MOV EAX, dword ptr [ESI]           ; vtable of result
//   8b 10           MOV EDX, dword ptr [EAX]           ; vtable[0]
//   6a 00           PUSH 0
//   8b ce           MOV ECX, ESI                       ; this = result
//   ff d2           CALL EDX                           ; result->vtable[0](0)
//   56              PUSH ESI                           ; arg = result
//   e8 RR RR RR RR  CALL FUN_009d56fd                  ; (reloc)
//   83 c4 04        ADD ESP, 0x4
//   5e              POP ESI
//   c3              RET

extern "C" void FUN_009d56fd(void *);  // forward declaration for the CALL relocation

extern "C" __declspec(naked) void __cdecl FUN_004143f0(void *) {
    __asm {
        // 000143f0: 8b 4c 24 04   MOV ECX, [ESP+4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 000143f4: 8b 01         MOV EAX, [ECX]
        _emit 0x8b
        _emit 0x01
        // 000143f6: 8b 50 04      MOV EDX, [EAX+4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000143f9: 56            PUSH ESI
        _emit 0x56
        // 000143fa: ff d2         CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000143fc: 8b f0         MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 000143fe: 8b 06         MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 00014400: 8b 10         MOV EDX, [EAX]
        _emit 0x8b
        _emit 0x10
        // 00014402: 6a 00         PUSH 0
        _emit 0x6a
        _emit 0x00
        // 00014404: 8b ce         MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00014406: ff d2         CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00014408: 56            PUSH ESI
        _emit 0x56
        // 00014409: e8 RR RR RR RR  CALL FUN_009d56fd  (reloc)
        call FUN_009d56fd
        // 0001440e: 83 c4 04      ADD ESP, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00014411: 5e            POP ESI
        _emit 0x5e
        // 00014412: c3            RET
        _emit 0xc3
    }
}
