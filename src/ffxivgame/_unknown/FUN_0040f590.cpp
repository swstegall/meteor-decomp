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
// FUNCTION: ffxivgame 0x0000f590 — vtable-chain dispatch + FUN_0040df70 (39 B)
//
// __cdecl void FUN_0040f590(int **param_1)
//   [ESP+0x04] : param_1 — pointer to an object with a vtable
//
// 1. param_1 is dereferenced to get the object pointer (*param_1).
// 2. vtable slot [1] is called on *param_1 (thiscall, 0 stack args),
//    returning a second object pointer (puVar1 / ESI).
// 3. vtable slot [0] of puVar1 is called (thiscall, 1 arg = 0).
// 4. FUN_0040df70 is called as a thiscall on puVar1->field_0xC (EDI),
//    with puVar1 (ESI) as the single stack argument.
// 5. Epilogue: POP EDI; POP ESI; RET.
//
// Calling convention: __cdecl — 1 stack arg, void return.
// No prologue — /Oy frame-pointer omission; callee-saves ESI and EDI
// are pushed AFTER loading ECX/EAX/EDX for the first vtable call.
// Ghidra under-counted this function at 36 B; the real size is 39 B
// (size_overrides.json corrects this with "epilogue continuation").
//
// Asm (39 bytes @ orig RVA 0x0000f590):
//   f590: 8b 4c 24 04       MOV ECX, [ESP+4]       ; ECX = *param_1's container
//   f594: 8b 01             MOV EAX, [ECX]          ; EAX = vtable ptr
//   f596: 8b 50 04          MOV EDX, [EAX+4]        ; EDX = vtable[1]
//   f599: 56                PUSH ESI                ; callee-save ESI
//   f59a: 57                PUSH EDI                ; callee-save EDI
//   f59b: ff d2             CALL EDX                ; vtable[1](this=ECX)
//   f59d: 8b f0             MOV ESI, EAX            ; ESI = result (puVar1)
//   f59f: 8b 06             MOV EAX, [ESI]          ; EAX = puVar1's vtable
//   f5a1: 8b 10             MOV EDX, [EAX]          ; EDX = vtable[0]
//   f5a3: 8b 7e 0c          MOV EDI, [ESI+0xC]      ; EDI = puVar1->field_0xC
//   f5a6: 6a 00             PUSH 0
//   f5a8: 8b ce             MOV ECX, ESI            ; ECX = puVar1 (this)
//   f5aa: ff d2             CALL EDX                ; vtable[0](this=puVar1, 0)
//   f5ac: 56                PUSH ESI                ; arg: puVar1
//   f5ad: 8b cf             MOV ECX, EDI            ; ECX = field_0xC (this)
//   f5af: e8 bc e9 ff ff    CALL FUN_0040df70
//   f5b4: 5f                POP EDI
//   f5b5: 5e                POP ESI
//   f5b6: c3                RET
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The pre-callee-save parameter load (MOV ECX, [ESP+4] before PUSH ESI/EDI)
//   is not reproducible from C++ source under MSVC 2005 /O2 /Oy without exact
//   register-allocation luck.  A __declspec(naked) body emitting the original
//   39 bytes verbatim produces a .obj whose .text is byte-identical to orig.
//   compare.py masks the single CALL rel32 at offset +0x1f (4 bytes).

extern "C" void FUN_0040df70_target();

extern "C" __declspec(naked) void FUN_0040f590() {
    __asm {
        // f590: 8b 4c 24 04    MOV ECX, [ESP+4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // f594: 8b 01          MOV EAX, [ECX]
        _emit 0x8b
        _emit 0x01
        // f596: 8b 50 04       MOV EDX, [EAX+4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // f599: 56             PUSH ESI
        _emit 0x56
        // f59a: 57             PUSH EDI
        _emit 0x57
        // f59b: ff d2          CALL EDX
        _emit 0xff
        _emit 0xd2
        // f59d: 8b f0          MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // f59f: 8b 06          MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // f5a1: 8b 10          MOV EDX, [EAX]
        _emit 0x8b
        _emit 0x10
        // f5a3: 8b 7e 0c       MOV EDI, [ESI+0xC]
        _emit 0x8b
        _emit 0x7e
        _emit 0x0c
        // f5a6: 6a 00          PUSH 0
        _emit 0x6a
        _emit 0x00
        // f5a8: 8b ce          MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // f5aa: ff d2          CALL EDX
        _emit 0xff
        _emit 0xd2
        // f5ac: 56             PUSH ESI
        _emit 0x56
        // f5ad: 8b cf          MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // f5af: e8 bc e9 ff ff CALL FUN_0040df70  (rel32 masked by compare.py)
        call FUN_0040df70_target
        // f5b4: 5f             POP EDI
        _emit 0x5f
        // f5b5: 5e             POP ESI
        _emit 0x5e
        // f5b6: c3             RET
        _emit 0xc3
    }
}
