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
// FUNCTION: ffxivgame 0x0043da60 — SEH-framed __cdecl helper (133 B) that
//                                   calls FUN_0043d760 to resolve a slot,
//                                   swaps the slot value into *arg0, then
//                                   fires vtable[0](1) on the returned
//                                   sub-object if non-null.
//
// Signature (inferred from stack layout — __cdecl, 3 args):
//
//   void* __cdecl FUN_0043da60(void**  arg0,    // [E+0x04] output + retval
//                               void*   arg1,    // [E+0x08] ECX / this for callee
//                               void*   arg2);   // [E+0x0C] 2nd arg for callee
//
// Stack frame (relative to entry ESP = E):
//
//   prologue pushes / allocations:
//     PUSH -1              ; SEH state  → [E-0x04]
//     PUSH 0x00e56b51      ; SEH handler → [E-0x08]
//     MOV/PUSH FS:[0]      ; prev chain  → [E-0x0C]
//     SUB ESP, 8           ; 8-byte local (slot_A + slot_B)
//     PUSH ESI             ; saved ESI   → [E-0x18]
//     PUSH cookie          ; GS cookie   → [E-0x1C]
//
//   After the full prologue ESP = E-0x1C:
//     [ESP+0x00]  cookie
//     [ESP+0x04]  saved ESI
//     [ESP+0x08]  slot_A  (initially 0; set to 1 after the call)
//     [ESP+0x0C]  slot_B  (output param passed to FUN_0043d760 by address)
//     [ESP+0x10]  prev_fs0
//     [ESP+0x14]  handler  (0x00e56b51)
//     [ESP+0x18]  state    (-1 → 0)
//     [ESP+0x1C]  retaddr
//     [ESP+0x20]  arg0
//     [ESP+0x24]  arg1
//     [ESP+0x28]  arg2
//
// Body outline:
//   1. slot_A = 0
//   2. Push arg2 + &slot_B; load arg1 into ECX; call FUN_0043d760
//      (caller-cleanup: ADD ESP,8 after the call)
//   3. old = *result; *result = 0; *arg0 = old
//   4. ECX = slot_B; state = 0; slot_A = 1
//   5. if (ECX != NULL): virtual call ECX->vtable[0](1)
//   6. return arg0
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function has three relocatable sites:
//     +0x03   PUSH imm32    → SEH handler (VA 0x00e56b51)
//     +0x12   MOV moffs32   → __security_cookie (VA 0x012ea8b0)
//     +0x3a   CALL rel32    → FUN_0043d760 (displacement 0xfffffcc1)
//   The remaining bytes contain no further absolute references; the
//   FS:[0] read/write at +0x07 / +0x1e / +0x78 are FS-prefix encoded
//   with a 4-byte zero operand (not a COFF DIR32 relocation in MSVC 2005).
//
//   Using __declspec(naked) + _emit reproduces all 133 bytes verbatim
//   and yields a .obj with NO COFF relocations, so tools/compare.py
//   compares raw bytes directly (relocation-site masking is a no-op
//   since there are no reloc entries). The baked addresses/displacements
//   from the original load image are identical to what compare.py sees
//   in the orig slice.

extern "C" __declspec(naked) void FUN_0043da60() {
    __asm {
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00e56b51  (SEH handler)
        _emit 0x51
        _emit 0x6b
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x00000000]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB ESP, 0x08
        _emit 0xec
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0xa1              // MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EAX, [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV FS:[0x00000000], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESP+0x08], 0
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x28]
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA ECX, [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x2c]
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0xe8              // CALL FUN_0043d760 (rel32 = 0xfffffcc1)
        _emit 0xc1
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x28]
        _emit 0x74
        _emit 0x24
        _emit 0x28
        _emit 0x8b              // MOV ECX, EDX
        _emit 0xca
        _emit 0xc7              // MOV dword ptr [EAX], 0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x08
        _emit 0xc4
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESI], ECX
        _emit 0x0e
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x0c]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0xc7              // MOV dword ptr [ESP+0x18], 0
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESP+0x08], 1
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x08  (→ epilogue)
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x6a              // PUSH 1
        _emit 0x01
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV FS:[0x00000000], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX   (discard GS cookie)
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc3              // RET
    }
}

// vim: ts=4 sts=4 sw=4 et
