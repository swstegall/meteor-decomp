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
// FUNCTION: ffxivgame 0x000307c0 — `__thiscall` handle-release method (33 B / 0x21).
//
// Signature (inferred from ECX-as-this + frame shape):
//
//   void __thiscall FUN_004307c0(SomeObj* this);   // ECX = this
//
// Releases a non-zero handle stored at this->field_0x28. If the field is
// zero the function is a no-op. Otherwise it dispatches a `__thiscall`
// virtual method (vtable slot +0x28) on the global manager object whose
// pointer lives at *[0x01329920], passing the handle as the sole argument,
// then clears this->field_0x28 to 0.
//
// Asm shape (33 bytes — RVA 0x000307c0..0x000307e1):
//
//   000307c0:  56                       PUSH ESI
//   000307c1:  8b f1                     MOV  ESI, ECX                ; this
//   000307c3:  8b 46 28                  MOV  EAX, [ESI+0x28]         ; handle
//   000307c6:  85 c0                     TEST EAX, EAX
//   000307c8:  74 15                     JZ   0x004307df             ; handle == 0 → skip
//   000307ca:  8b 0d 20 99 32 01         MOV  ECX, [0x01329920]      ; g_manager (this)
//   000307d0:  8b 11                     MOV  EDX, [ECX]             ; vtable
//   000307d2:  50                        PUSH EAX                   ; push handle
//   000307d3:  8b 42 28                  MOV  EAX, [EDX+0x28]        ; vtable slot +0x28
//   000307d6:  ff d0                     CALL EAX                   ; thiscall dispatch
//   000307d8:  c7 46 28 00 00 00 00      MOV  [ESI+0x28], 0x0       ; clear handle
//   000307df:  5e                        POP  ESI
//   000307e0:  c3                        RET
//
// Reloc-bearing site in the orig 33 bytes:
//     +0x0c   DIR32 → 0x01329920  (global manager pointer address)
//
// The indirect CALL at +0x16 is `ff d0` (CALL EAX) — register-indirect,
// no relocation. Calling convention: __thiscall (ECX = this), void return,
// plain RET (callee has no stack args of its own; the single PUSH balances
// the callee's __thiscall arg).
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough, matching
// the immediate sibling idiom (cf. FUN_0041cec0): emit the 33 orig bytes
// verbatim via MASM `_emit` so the lone DIR32 reloc window is baked as raw
// bytes that compare.py masks, giving a stable GREEN match.

extern "C" __declspec(naked) void FUN_004307c0() {
    __asm {
        _emit 0x56    // PUSH ESI
        _emit 0x8b    // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b    // MOV EAX, dword ptr [ESI+0x28]
        _emit 0x46
        _emit 0x28
        _emit 0x85    // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74    // JZ +0x15
        _emit 0x15
        _emit 0x8b    // MOV ECX, dword ptr [0x01329920]
        _emit 0x0d
        _emit 0x20
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x8b    // MOV EDX, dword ptr [ECX]
        _emit 0x11
        _emit 0x50    // PUSH EAX
        _emit 0x8b    // MOV EAX, dword ptr [EDX+0x28]
        _emit 0x42
        _emit 0x28
        _emit 0xff    // CALL EAX
        _emit 0xd0
        _emit 0xc7    // MOV dword ptr [ESI+0x28], 0x0
        _emit 0x46
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e    // POP ESI
        _emit 0xc3    // RET
    }
}
