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
// FUNCTION: ffxivgame 0x0042e220 — matrix init + render wrapper (45 B / 0x2d)
//
// Behaviour read from asm/ffxivgame/0002e220_FUN_0042e220.s:
//
//   __cdecl void FUN_0042e220(void *param1, void *param2, int param3);
//
//   Allocates a 64-byte (4×4 float) local matrix on the stack, initialises
//   it via FUN_0042fcf0 (scaled-identity matrix init), then calls
//   FUN_00426190 passing param1, param1+0x10, the matrix pointer, param2,
//   and param3 as its five arguments.
//
//   Unusual stack layout: param3 and param2 are pushed BELOW the 64-byte
//   local buffer so that they are naturally in the right positions on the
//   stack to serve as the 4th and 5th arguments to FUN_00426190, without
//   being explicitly pushed again. ADD ESP,0x4 after the first CALL pops
//   only the single pointer argument; the two "orphaned" dwords remain on
//   the stack and are consumed as extra args by the second CALL.
//
//   Stack trace at entry (ESP = ESP_entry):
//     [ESP_entry+0x00]  return address
//     [ESP_entry+0x04]  param1
//     [ESP_entry+0x08]  param2
//     [ESP_entry+0x0c]  param3
//
//   After SUB ESP,0x40 + PUSH EAX + PUSH ECX:
//     [ESP_entry-0x48]  param2  (arg4 of second CALL)
//     [ESP_entry-0x44]  param3  (arg5 of second CALL — used as flag byte)
//     [ESP_entry-0x40]..[ESP_entry-0x01]  64-byte matrix buffer
//
//   First CALL: FUN_0042fcf0(&matrix_buf)
//     After ADD ESP,0x4:  matrix ptr popped; ESP = ESP_entry-0x48
//
//   Second CALL: FUN_00426190(param1, param1+0x10, &matrix_buf, param2, param3)
//     ADD ESP,0x54 unwinds: 0x40 (buffer) + 0x8 (param2+param3) +
//                           0x4 (matrix ptr push) + 0x8 (param1+offset) = 0x54
//
//   Calling convention: __cdecl — no callee-saved registers, plain RET.
//
// Reconstruction: __declspec(naked) inline asm.
// The dual-use stack layout (param2/param3 acting as both saved locals and
// implicit extra args for the second CALL) cannot be expressed in standard
// C — naked asm is the only sound reconstruction. CALL rel32 sites are
// masked by tools/compare.py so the new .obj's symbol relocations do not
// prevent a GREEN result.

extern "C" void FUN_0042fcf0(float *out);
extern "C" void FUN_00426190(void *a, void *b, float *matrix, void *d, int e);

extern "C" __declspec(naked) void FUN_0042e220()
{
    __asm {
        mov     eax, [esp + 0xc]
        mov     ecx, [esp + 0x8]
        sub     esp, 0x40
        push    eax
        push    ecx
        lea     edx, [esp + 0x8]
        push    edx
        call    FUN_0042fcf0
        add     esp, 0x4
        push    eax
        mov     eax, [esp + 0x50]
        lea     ecx, [eax + 0x10]
        push    ecx
        push    eax
        call    FUN_00426190
        add     esp, 0x54
        ret
    }
}
