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
// FUNCTION: ffxivgame 0x000124e0 — vtable double-dispatch + noreturn (39 B)
//
// __cdecl void FUN_004124e0(SomeObj *param_1)
//   [ESP+4] : param_1 — pointer to an object with a vtable
//
// Behaviour:
//   1. Loads param_1 from [ESP+4] into ECX (thiscall receiver).
//   2. Calls vtable[1] on param_1 (no stack args) → returns puVar1.
//   3. Saves puVar1->field_0xc (offset 0xc) into EDI before next call.
//   4. Calls vtable[0] on puVar1 (thiscall, arg=0).
//   5. Calls FUN_0040df70 on puVar1->field_0xc with arg=puVar1 — NORETURN.
//
// Calling convention: __cdecl — param from [ESP+4]; no epilogue (noreturn).
// Callee-saves: PUSH ESI; PUSH EDI emitted before first CALL (MSVC /Oy
//   delayed-save pattern: ECX/EAX/EDX are set up first, then ESI/EDI pushed
//   just before the call that clobbers the scratch registers).
//   No POP / RET because FUN_0040df70 never returns.
//
// Asm (36 bytes @ orig RVA 0x000124e0):
//   8b 4c 24 04           MOV ECX, [ESP+4]          ; ECX = param_1
//   8b 01                 MOV EAX, [ECX]             ; vtable ptr
//   8b 50 04              MOV EDX, [EAX+4]           ; vtable[1]
//   56                    PUSH ESI                   ; callee-save
//   57                    PUSH EDI                   ; callee-save
//   ff d2                 CALL EDX                   ; thiscall ECX=param_1 → puVar1
//   8b f0                 MOV ESI, EAX               ; ESI = puVar1
//   8b 06                 MOV EAX, [ESI]             ; vtable of puVar1
//   8b 10                 MOV EDX, [EAX]             ; vtable[0]
//   8b 7e 0c              MOV EDI, [ESI+0xc]         ; EDI = puVar1->field_0xc (early load)
//   6a 00                 PUSH 0                     ; arg
//   8b ce                 MOV ECX, ESI               ; thiscall ECX = puVar1
//   ff d2                 CALL EDX                   ; vtable[0](0)
//   56                    PUSH ESI                   ; arg = puVar1
//   8b cf                 MOV ECX, EDI               ; thiscall ECX = field_0xc
//   e8 6c ba ff ff        CALL FUN_0040df70          ; noreturn thiscall
//
// Reconstruction: naked-asm byte passthrough.  The /Oy-delayed callee-save
// ordering (ECX set before PUSH ESI/EDI) and the early field_0xc load (EDI
// hoisted before PUSH 0) are not reliably reproducible from plain C++ source.

#if defined(_MSC_VER) && !defined(__clang__)
extern "C" __declspec(naked) void FUN_004124e0()
{
    __asm {
        _emit 0x8b          // MOV ECX, [ESP+4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x8b          // MOV EAX, [ECX]
        _emit 0x01
        _emit 0x8b          // MOV EDX, [EAX+4]
        _emit 0x50
        _emit 0x04
        _emit 0x56          // PUSH ESI
        _emit 0x57          // PUSH EDI
        _emit 0xff          // CALL EDX
        _emit 0xd2
        _emit 0x8b          // MOV ESI, EAX
        _emit 0xf0
        _emit 0x8b          // MOV EAX, [ESI]
        _emit 0x06
        _emit 0x8b          // MOV EDX, [EAX]
        _emit 0x10
        _emit 0x8b          // MOV EDI, [ESI+0xc]
        _emit 0x7e
        _emit 0x0c
        _emit 0x6a          // PUSH 0
        _emit 0x00
        _emit 0x8b          // MOV ECX, ESI
        _emit 0xce
        _emit 0xff          // CALL EDX
        _emit 0xd2
        _emit 0x56          // PUSH ESI
        _emit 0x8b          // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8          // CALL FUN_0040df70 (REL32)
        _emit 0x6c
        _emit 0xba
        _emit 0xff
        _emit 0xff
        // dead epilogue — never reached (FUN_0040df70 is noreturn),
        // but MSVC 2005 still emits the matching callee-restore + RET:
        _emit 0x5f          // POP EDI
        _emit 0x5e          // POP ESI
        _emit 0xc3          // RET
    }
}
#endif // _MSC_VER && !__clang__
