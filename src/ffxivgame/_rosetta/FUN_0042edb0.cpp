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
// FUNCTION: ffxivgame 0x0042edb0 — __thiscall "transform 4 vectors into dest"
//                                  (97 bytes / 0x61, RET 0x8)
//
// void __thiscall Matrix4::transformInto(SomeClass *dest, const VecStruct *src)
//   ECX         : this   (Matrix4 pointer — saved to ESI)
//   [ESP+0x04]  : dest   (target object whose 4 slots receive the results)
//   [ESP+0x08]  : src    (source struct with Vec4 fields at +0x00, +0x10, +0x20, +0x30)
//
// Behaviour: calls this->transform() four times on the four 16-byte Vec4
// fields of *src (fields at +0x30, +0x20, +0x10, and +0x00/src itself),
// writing each result into a 16-byte local buffer.  The four local buffers
// (plus the accumulated return values r1..r4 held on the stack between
// calls) are then handed off to dest->set4(r4, r3, r2, r1) in one shot.
// Returns dest.
//
// Call sequence:
//   r1 = this->FUN_0042ec50(&local0, &src->field_0x30)   [RET 8]
//   r2 = this->FUN_0042ec50(&local1, &src->field_0x20)   [r1 held on stack]
//   r3 = this->FUN_0042ec50(&local2, &src->field_0x10)   [r1,r2 held]
//   r4 = this->FUN_0042ec50(&local3,  src)               [r1,r2,r3 held]
//   dest->FUN_004201a0(r4, r3, r2, r1)                   [RET 0x10 cleans all four]
//   return dest;
//
// Stack frame (64 bytes of local storage split into four 16-byte Vec4 slots
// past the two callee-saved registers pushed in the prologue):
//   local0 : [SP+0x08] after prologue  (output of transform(src+0x30))
//   local1 : [SP+0x18]                 (output of transform(src+0x20))
//   local2 : [SP+0x28]                 (output of transform(src+0x10))
//   local3 : [SP+0x38]                 (output of transform(src+0x00))

extern "C" {
    void FUN_0042ec50();
    void FUN_004201a0();
}

extern "C" __declspec(naked) void FUN_0042edb0() {
    __asm {
        sub  esp, 0x40
        push esi
        push edi
        mov  edi, dword ptr [esp + 0x50]   // EDI = src (arg2)
        mov  esi, ecx                       // ESI = this (Matrix4*)

        // Call 1: transform src->field_0x30 → local0 at [SP+8]
        lea  eax, [edi + 0x30]
        push eax                            // push &src->field_0x30
        lea  ecx, [esp + 0xc]              // &local0 (SP was just decremented)
        push ecx
        mov  ecx, esi
        call FUN_0042ec50                   // r1 = this->transform(&local0, &src->field_0x30)

        // r1 stays on stack; call 2: transform src->field_0x20 → local1
        push eax                            // push r1
        lea  edx, [edi + 0x20]
        push edx                            // push &src->field_0x20
        lea  eax, [esp + 0x20]            // &local1
        push eax
        mov  ecx, esi
        call FUN_0042ec50                   // r2 = this->transform(&local1, &src->field_0x20)

        // r1,r2 stay on stack; call 3: transform src->field_0x10 → local2
        push eax                            // push r2
        lea  ecx, [edi + 0x10]
        push ecx                            // push &src->field_0x10
        lea  edx, [esp + 0x34]            // &local2
        push edx
        mov  ecx, esi
        call FUN_0042ec50                   // r3 = this->transform(&local2, &src->field_0x10)

        // r1,r2,r3 stay on stack; call 4: transform src itself → local3
        push eax                            // push r3
        push edi                            // push src
        lea  eax, [esp + 0x48]            // &local3
        push eax
        mov  ecx, esi
        call FUN_0042ec50                   // r4 = this->transform(&local3, src)

        // Now load dest (arg1) into ESI, push r4, call dest->set4(r4,r3,r2,r1)
        mov  esi, dword ptr [esp + 0x58]   // ESI = dest (arg1); [r1,r2,r3] still on stack
        push eax                            // push r4
        mov  ecx, esi
        call FUN_004201a0                   // dest->set4(r4, r3, r2, r1) — RET 0x10

        // Epilogue: restore callee-saves, collapse frame, return dest in EAX
        pop  edi
        mov  eax, esi
        pop  esi
        add  esp, 0x40
        ret  0x8
    }
}
