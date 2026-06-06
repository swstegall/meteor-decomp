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
// FUNCTION: ffxivgame 0x0046fb70 — Argument-transform forwarding thunk
//                                  (18 B / 0x12).
//
// Behaviour read from the disassembly at orig RVA 0x0006fb70:
//
//   This is one of a cluster of three adjacent, structurally identical
//   thunks (FUN_0046fb50 / FUN_0046fb70 / FUN_0046fb90) that each:
//     1. Load the first stack argument (a pointer) into a scratch register.
//     2. Double-dereference it with a fixed byte offset of 0x24 on the
//        second level (i.e. compute `(*arg1)[0x24/4]`).
//     3. Write the resulting pointer back over arg1 on the caller's stack.
//     4. Tail-call a downstream function with the modified argument list.
//
//   Register rotation across the cluster:
//     FUN_0046fb50  EAX ← [ESP+4],  ECX ← [EAX],    EDX ← [ECX+0x24];  JMP 0x00470c60
//     FUN_0046fb70  EDX ← [ESP+4],  EAX ← [EDX],    ECX ← [EAX+0x24];  JMP 0x0049e5a0
//     FUN_0046fb90  ECX ← [ESP+4],  EDX ← [ECX],    EAX ← [EDX+0x24];  JMP 0x00470c80
//
//   The rotated register allocation is a consequence of MSVC 2005 /O2
//   producing different scratch choices depending on the surrounding type
//   context; the instruction shape is otherwise byte-for-byte the same
//   template (5 instructions, no frame, no callee-saves).
//
//   Reconstruction strategy — naked asm:
//     As with other small no-frame thunks in this module (FUN_00401730,
//     FUN_00e58079) the exact register selection cannot be reliably
//     reproduced from any portable C++ source fragment.  A
//     `__declspec(naked)` body with MASM mnemonics generates the
//     required byte sequence directly.  The `jmp FUN_0049e5a0` encodes
//     as `e9 RR RR RR RR` where the four RR bytes are a COFF REL32
//     relocation; compare.py masks those four bytes in the diff so the
//     object-file offset (which differs from the orig PE's resolved
//     offset 0x0002ea1e) is never compared byte-for-byte.

extern "C" void FUN_0049e5a0();

extern "C" __declspec(naked) void FUN_0046fb70()
{
    __asm {
        mov edx, dword ptr [esp + 4]
        mov eax, dword ptr [edx]
        mov ecx, dword ptr [eax + 0x24]
        mov dword ptr [esp + 4], ecx
        jmp FUN_0049e5a0
    }
}
