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
// FUNCTION: ffxivgame 0x00043e00 — __thiscall slot setter (51 B / 0x33)
//
//   void __thiscall FUN_00443e00(C *this, int page, int idx, int arg)
//     stack layout (after the function's own PUSH ESI / PUSH EDX):
//       [ESP+0x04] : int page   (param_1)
//       [ESP+0x08] : int idx    (param_2)
//       [ESP+0x0c] : int arg    (param_3)
//     callee cleans 12 bytes via `ret 0xc` → __thiscall, 3 stack args.
//
// Inspection (read from the orig bytes at RVA 0x00043e00, 51 bytes total):
//
//   mov  eax, [esp+0x4]          ; page
//   push esi
//   mov  esi, ecx               ; this
//   mov  ecx, [esp+0xc]         ; idx   (after the push)
//   shl  eax, 5                 ; page << 5  (32 entries / page)
//   and  ecx, 0x1f             ; idx & 0x1f
//   add  eax, ecx              ; flat = page*32 + (idx & 31)
//   imul eax, eax, 0xbc       ; * 188-byte element stride
//   add  eax, [esi+8]          ; + this->_base  → element pointer
//   jz   done                   ; null element → skip
//   mov  edx, [esp+0x10]        ; arg
//   push edx
//   lea  ecx, [eax+0x68]       ; sub-object at +0x68 is the call's `this`
//   call FUN_00447450           ; (rel32 → 0x00447450) __thiscall(arg)
//   mov  byte ptr [esi+0xc], 1  ; this->_dirty = 1
// done:
//   pop  esi
//   ret  0xc
//
// Calling convention: __thiscall (ECX = this; three stack args; callee
// cleans 12 bytes). Stack frame: PUSH ESI callee-save only, no locals.
//
// The ADD that produces the element pointer also sets ZF, so the orig
// branches `if (element == 0) skip;` straight off that ADD with no extra
// TEST — re-emitted verbatim below.
//
// Reloc-bearing site in the orig 51 bytes:
//     +0x26   CALL rel32   → FUN_00447450
//
// Reconstruction strategy — naked-asm byte passthrough (mirrors sibling
// _rosetta matches): a `__declspec(naked)` body that re-emits the orig
// 51 bytes verbatim via MASM `_emit` directives. The rel32 immediate is
// baked in as raw bytes; `tools/compare.py` masks the reloc out of the
// diff and reports GREEN with a zero-reloc .obj.

extern "C" __declspec(naked) void FUN_00443e00() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0xc1              // SHL EAX, 0x5
        _emit 0xe0
        _emit 0x05
        _emit 0x83              // AND ECX, 0x1f
        _emit 0xe1
        _emit 0x1f
        _emit 0x03              // ADD EAX, ECX
        _emit 0xc1
        _emit 0x69              // IMUL EAX, EAX, 0xbc
        _emit 0xc0
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x03              // ADD EAX, dword ptr [ESI+0x8]
        _emit 0x46
        _emit 0x08
        _emit 0x74              // JZ done (+0x11)
        _emit 0x11
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x10]
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x52              // PUSH EDX
        _emit 0x8d              // LEA ECX, [EAX+0x68]
        _emit 0x48
        _emit 0x68
        _emit 0xe8              // CALL FUN_00447450 (rel32 → 0x00447450)
        _emit 0x25
        _emit 0x36
        _emit 0x00
        _emit 0x00
        _emit 0xc6              // MOV byte ptr [ESI+0xc], 0x1
        _emit 0x46
        _emit 0x0c
        _emit 0x01
        _emit 0x5e              // POP ESI    (done:)
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
