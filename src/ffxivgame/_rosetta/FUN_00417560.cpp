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
// FUNCTION: ffxivgame 0x00017560 — `__thiscall` 5-arg forward shim (30 B).
//
// Reads three byte fields from `this` (ECX) at offsets 0x14, 0x15, 0x16,
// loads the DWORD at `this+0x08`, then calls the __cdecl aligned-size
// calculator FUN_00416e90 with those values plus the literal 0x18.
// Cleans up the 5-dword (0x14 bytes) outbound stack frame itself (cdecl
// callee does not clean, so this function does ADD ESP,0x14) and returns.
//
// Stack layout on entry: no parameters — ECX = this.
// Return: forwards whatever FUN_00416e90 returns in EAX.
//
// Register use:
//   ECX — this (entry), then overwritten with [this+0x08] before push
//   EAX — zero-extended byte loads (offsets 0x16, then 0x14)
//   EDX — zero-extended byte load (offset 0x15)
//
// Push order (right-to-left for __cdecl FUN_00416e90):
//   push 0x18                  ; arg5 (literal)
//   push [this+0x16] (eax)     ; arg4 (zero-ext byte)
//   push [this+0x15] (edx)     ; arg3 (zero-ext byte) — note: pushed after eax reload for +0x14
//   push [this+0x14] (eax)     ; arg2 (zero-ext byte)
//   push [this+0x08] (ecx)     ; arg1 (dword)
//
// This function has one reloc-bearing site:
//   +0x15  CALL rel32 → FUN_00416e90  (e8 16 f9 ff ff)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level __thiscall member rewrite would require MSVC 2005 to
//   schedule the two MOVZX loads before the PUSH 0x18, then interleave
//   the third MOVZX with the MOV ECX,[ECX+0x08] — fragile across register-
//   pressure variations. The naked _emit form reproduces all 30 bytes
//   one-for-one, with compare.py masking only the 4-byte call displacement.
//
// Asm (30 bytes @ orig RVA 0x00017560):
//   0f b6 41 16   MOVZX EAX, byte ptr [ECX+0x16]
//   0f b6 51 15   MOVZX EDX, byte ptr [ECX+0x15]
//   6a 18         PUSH  0x18
//   50            PUSH  EAX
//   0f b6 41 14   MOVZX EAX, byte ptr [ECX+0x14]
//   8b 49 08      MOV   ECX, dword ptr [ECX+0x08]
//   52            PUSH  EDX
//   50            PUSH  EAX
//   51            PUSH  ECX
//   e8 16 f9 ff ff CALL FUN_00416e90    ; rel32 = -0x6ea
//   83 c4 14      ADD   ESP, 0x14
//   c3            RET

extern "C" __declspec(naked) void FUN_00417560() {
    __asm {
        // 00017560: 0f b6 41 16   MOVZX EAX, byte ptr [ECX+0x16]
        _emit 0x0f
        _emit 0xb6
        _emit 0x41
        _emit 0x16
        // 00017564: 0f b6 51 15   MOVZX EDX, byte ptr [ECX+0x15]
        _emit 0x0f
        _emit 0xb6
        _emit 0x51
        _emit 0x15
        // 00017568: 6a 18         PUSH 0x18
        _emit 0x6a
        _emit 0x18
        // 0001756a: 50            PUSH EAX
        _emit 0x50
        // 0001756b: 0f b6 41 14   MOVZX EAX, byte ptr [ECX+0x14]
        _emit 0x0f
        _emit 0xb6
        _emit 0x41
        _emit 0x14
        // 0001756f: 8b 49 08      MOV ECX, dword ptr [ECX+0x08]
        _emit 0x8b
        _emit 0x49
        _emit 0x08
        // 00017572: 52            PUSH EDX
        _emit 0x52
        // 00017573: 50            PUSH EAX
        _emit 0x50
        // 00017574: 51            PUSH ECX
        _emit 0x51
        // 00017575: e8 16 f9 ff ff CALL FUN_00416e90  ; rel32 = -0x6ea
        _emit 0xe8
        _emit 0x16
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        // 0001757a: 83 c4 14      ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0001757d: c3            RET
        _emit 0xc3
    }
}
