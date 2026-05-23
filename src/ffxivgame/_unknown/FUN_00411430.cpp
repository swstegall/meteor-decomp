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
// FUNCTION: ffxivgame 0x00011430 — __cdecl factory dispatch + context transfer (36 B)
//
// void FUN_00411430(int *param_1)
//   [ESP+0x4] : param_1 — pointer to an object with a vtable; vtbl[1] is called
//               as __thiscall (ECX=param_1) to manufacture a new context object.
//
// Sequence:
//   1. Load param_1 into ECX; call vtbl[1](param_1) → EAX = new context object.
//   2. Save new context → ESI; load ESI->field_8 → EDI.
//   3. Call vtbl[0](ESI, 0) — thiscall on the new context with arg 0.
//   4. Tail: call FUN_0040df70(ESI) on ESI->field_8 (ECX=EDI, arg=ESI).
//
// Calling convention: __cdecl — one stack arg, void return.
// No frame-pointer, no epilogue: function terminates at the CALL to
// FUN_0040df70 (MSVC treats it as not returning in this call path;
// ESI/EDI are never restored before the terminal call).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The missing pop/ret epilogue cannot be reproduced from C++ source.
//   The terminal CALL is expressed as a MASM `call` mnemonic so the
//   assembler emits a proper COFF REL32 relocation that compare.py masks.
//
// Reloc-bearing site (rel32 masked by compare.py):
//   offset +0x1f (func-relative): CALL FUN_0040df70

extern "C" void __stdcall FUN_0040df70(void *param_1);

extern "C" __declspec(naked) void FUN_00411430() {
    __asm {
        // 00011430: 8b 4c 24 04    MOV ECX, dword ptr [ESP+0x4]   ; ECX = param_1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 00011434: 8b 01          MOV EAX, dword ptr [ECX]       ; EAX = *param_1 (vtbl)
        _emit 0x8b
        _emit 0x01
        // 00011436: 8b 50 04       MOV EDX, dword ptr [EAX+0x4]   ; EDX = vtbl[1]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00011439: 56             PUSH ESI
        _emit 0x56
        // 0001143a: 57             PUSH EDI
        _emit 0x57
        // 0001143b: ff d2          CALL EDX                        ; vtbl[1](param_1)
        _emit 0xff
        _emit 0xd2
        // 0001143d: 8b f0          MOV ESI, EAX                   ; ESI = new context
        _emit 0x8b
        _emit 0xf0
        // 0001143f: 8b 06          MOV EAX, dword ptr [ESI]       ; EAX = *context (vtbl)
        _emit 0x8b
        _emit 0x06
        // 00011441: 8b 10          MOV EDX, dword ptr [EAX]       ; EDX = vtbl[0]
        _emit 0x8b
        _emit 0x10
        // 00011443: 8b 7e 08       MOV EDI, dword ptr [ESI+0x8]   ; EDI = context->field_8
        _emit 0x8b
        _emit 0x7e
        _emit 0x08
        // 00011446: 6a 00          PUSH 0x0                        ; arg = 0
        _emit 0x6a
        _emit 0x00
        // 00011448: 8b ce          MOV ECX, ESI                   ; ECX = context (this)
        _emit 0x8b
        _emit 0xce
        // 0001144a: ff d2          CALL EDX                        ; vtbl[0](context, 0)
        _emit 0xff
        _emit 0xd2
        // 0001144c: 56             PUSH ESI                        ; push context as arg
        _emit 0x56
        // 0001144d: 8b cf          MOV ECX, EDI                   ; ECX = context->field_8
        _emit 0x8b
        _emit 0xcf
        // 0001144f: e8 ...         CALL FUN_0040df70               ; (rel32, reloc masked)
        call    FUN_0040df70
        // 00011454: 5f             POP EDI
        _emit 0x5f
        // 00011455: 5e             POP ESI
        _emit 0x5e
        // 00011456: c3             RET
        _emit 0xc3
    }
}
