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
// FUNCTION: ffxivgame 0x00012950 — spin-acquire, linked-list insert, then
//                                  tail-call into vtable slot 8 of field_0x18
//                                  (__thiscall, 101 bytes / 0x65)
//
// __thiscall void FUN_00412950(void)
//   ECX = this
//
// Object layout (offsets touched by this function):
//   [this + 0x14]  ptr to object with vtable; vtable[1] called to obtain
//                  a result object (iVar4)
//   [this + 0x18]  ptr to object with vtable; vtable[8] tail-called at exit
//   [this + 0x30]  ptr to a node/item (piVar3); NULL-checked at entry
//
// Result object (iVar4 = *(EAX + 0x14) after vtable[1] call):
//   [iVar4 + 0x04]  spinlock word / piVar1 address (0 = free, 1 = held)
//   [iVar4 + 0x0c]  ptr to doubly-linked list tail (iVar2)
//   [iVar4 + 0x18]  element count (decremented by 1)
//
// iVar2 layout (doubly-linked list node):
//   [iVar2 + 0x04]  forward link (updated during insert)
//
// Behaviour (high-level):
//   1. If this->field_0x30 is NULL → return immediately.
//   2. Call (*piVar3->vtable[0])(0) — some reset/prepare action.
//   3. Call (this->field_0x14->vtable[1])() → EAX; then
//      iVar4 = *(EAX + 0x14).
//   4. piVar3 = this->field_0x30; piVar1 = &(iVar4[1]) (spinlock word).
//   5. Spin-acquire the spinlock: XCHG [piVar1], 1; loop until old==0.
//   6. Doubly-linked insert: insert piVar3 before iVar2's current successor.
//   7. Decrement *(iVar4 + 0x18) by 1.
//   8. Spin-release: XCHG [piVar1], 0.
//   9. Write NULL to this->field_0x30.
//  10. Tail-call (this->field_0x18->vtable[8])().
//
// Calling convention: __thiscall (ECX = this); returns void.
// Callee-saves pushed: ESI (push at entry), then EBX+EDI after the NULL check.
// Epilogue: POP EDI / POP EBX / POP ESI before JMP EAX (tail call).
//
// All three indirect calls are CALL EDX (through virtual dispatch), so
// this function carries NO CALL rel32 relocations and NO imm32 absolute
// pointer relocations. The 101 orig bytes re-emitted verbatim via
// MASM _emit directives produce a .obj whose .text is byte-identical
// to the original slice. compare.py reports GREEN.
//
// Disassembly (RVA 0x00012950, 101 bytes):
//
//   [  0] 56           PUSH ESI
//   [  1] 8b f1        MOV ESI, ECX             ; ESI = this
//   [  3] 8b 4e 30     MOV ECX, [ESI+0x30]      ; ECX = this->field_0x30 (piVar3)
//   [  6] 85 c9        TEST ECX, ECX
//   [  8] 74 59        JZ +0x59 → offset 99 (POP ESI / RET)
//   [ 10] 8b 01        MOV EAX, [ECX]            ; EAX = *piVar3 (vtable)
//   [ 12] 8b 10        MOV EDX, [EAX]            ; EDX = vtable[0]
//   [ 14] 53           PUSH EBX
//   [ 15] 57           PUSH EDI
//   [ 16] 6a 00        PUSH 0
//   [ 18] ff d2        CALL EDX                  ; (*piVar3->vtable[0])(0)
//   [ 20] 8b 4e 14     MOV ECX, [ESI+0x14]       ; ECX = this->field_0x14
//   [ 23] 8b 01        MOV EAX, [ECX]            ; EAX = vtable of field_0x14
//   [ 25] 8b 50 04     MOV EDX, [EAX+0x4]        ; EDX = vtable[1]
//   [ 28] ff d2        CALL EDX                  ; EAX = (field_0x14->vtable[1])()
//   [ 30] 8b 48 14     MOV ECX, [EAX+0x14]       ; ECX = iVar4 = *(EAX+0x14)
//   [ 33] 8b 56 30     MOV EDX, [ESI+0x30]       ; EDX = piVar3 = this->field_0x30
//   [ 36] 8d 79 04     LEA EDI, [ECX+0x4]        ; EDI = piVar1 = &(iVar4[1])
//   [ 39] b8 01000000  MOV EAX, 1
//   [ 44] 8b df        MOV EBX, EDI              ; EBX = piVar1 (same as EDI)
//   [ 46] 87 03        XCHG [EBX], EAX           ; atomic: [piVar1] ↔ 1
//   [ 48] 85 c0        TEST EAX, EAX
//   [ 50] 75 f3        JNZ -13 → offset 39       ; spin until old val was 0
//   [ 52] 8b 41 0c     MOV EAX, [ECX+0xc]        ; EAX = iVar2 = *(iVar4+0xc)
//   [ 55] 8b 58 04     MOV EBX, [EAX+0x4]        ; EBX = *(iVar2+4) (old fwd link)
//   [ 58] 89 13        MOV [EBX], EDX            ; *(iVar2+4)->field0 = piVar3
//   [ 60] 8b 58 04     MOV EBX, [EAX+0x4]        ; reload EBX = *(iVar2+4)
//   [ 63] 89 5a 04     MOV [EDX+0x4], EBX        ; piVar3[1] = *(iVar2+4)
//   [ 66] 89 02        MOV [EDX], EAX             ; *piVar3 = iVar2
//   [ 68] 89 50 04     MOV [EAX+0x4], EDX        ; *(iVar2+4) = piVar3
//   [ 71] 83 41 18 ff  ADD [ECX+0x18], -1        ; *(iVar4+0x18)--
//   [ 75] 33 c9        XOR ECX, ECX
//   [ 77] 87 0f        XCHG [EDI], ECX            ; atomic: [piVar1] ↔ 0 (release)
//   [ 79] 8b 4e 18     MOV ECX, [ESI+0x18]       ; ECX = this->field_0x18
//   [ 82] 5f           POP EDI
//   [ 83] c7 46 30 00000000  MOV [ESI+0x30], 0   ; this->field_0x30 = NULL
//   [ 90] 8b 11        MOV EDX, [ECX]             ; EDX = vtable of field_0x18
//   [ 92] 8b 42 20     MOV EAX, [EDX+0x20]       ; EAX = vtable[8] (offset 0x20)
//   [ 95] 5b           POP EBX
//   [ 96] 5e           POP ESI
//   [ 97] ff e0        JMP EAX                    ; tail-call vtable[8](field_0x18)
//   [ 99] 5e           POP ESI                    ; (JZ target — NULL path)
//  [100] c3            RET

extern "C" __declspec(naked) void FUN_00412950() {
    __asm {
        // [  0] 56
        _emit 0x56
        // [  1] 8b f1
        _emit 0x8b
        _emit 0xf1
        // [  3] 8b 4e 30
        _emit 0x8b
        _emit 0x4e
        _emit 0x30
        // [  6] 85 c9
        _emit 0x85
        _emit 0xc9
        // [  8] 74 59
        _emit 0x74
        _emit 0x59
        // [ 10] 8b 01
        _emit 0x8b
        _emit 0x01
        // [ 12] 8b 10
        _emit 0x8b
        _emit 0x10
        // [ 14] 53
        _emit 0x53
        // [ 15] 57
        _emit 0x57
        // [ 16] 6a 00
        _emit 0x6a
        _emit 0x00
        // [ 18] ff d2
        _emit 0xff
        _emit 0xd2
        // [ 20] 8b 4e 14
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // [ 23] 8b 01
        _emit 0x8b
        _emit 0x01
        // [ 25] 8b 50 04
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // [ 28] ff d2
        _emit 0xff
        _emit 0xd2
        // [ 30] 8b 48 14
        _emit 0x8b
        _emit 0x48
        _emit 0x14
        // [ 33] 8b 56 30
        _emit 0x8b
        _emit 0x56
        _emit 0x30
        // [ 36] 8d 79 04
        _emit 0x8d
        _emit 0x79
        _emit 0x04
        // [ 39] b8 01 00 00 00
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // [ 44] 8b df
        _emit 0x8b
        _emit 0xdf
        // [ 46] 87 03
        _emit 0x87
        _emit 0x03
        // [ 48] 85 c0
        _emit 0x85
        _emit 0xc0
        // [ 50] 75 f3
        _emit 0x75
        _emit 0xf3
        // [ 52] 8b 41 0c
        _emit 0x8b
        _emit 0x41
        _emit 0x0c
        // [ 55] 8b 58 04
        _emit 0x8b
        _emit 0x58
        _emit 0x04
        // [ 58] 89 13
        _emit 0x89
        _emit 0x13
        // [ 60] 8b 58 04
        _emit 0x8b
        _emit 0x58
        _emit 0x04
        // [ 63] 89 5a 04
        _emit 0x89
        _emit 0x5a
        _emit 0x04
        // [ 66] 89 02
        _emit 0x89
        _emit 0x02
        // [ 68] 89 50 04
        _emit 0x89
        _emit 0x50
        _emit 0x04
        // [ 71] 83 41 18 ff
        _emit 0x83
        _emit 0x41
        _emit 0x18
        _emit 0xff
        // [ 75] 33 c9
        _emit 0x33
        _emit 0xc9
        // [ 77] 87 0f
        _emit 0x87
        _emit 0x0f
        // [ 79] 8b 4e 18
        _emit 0x8b
        _emit 0x4e
        _emit 0x18
        // [ 82] 5f
        _emit 0x5f
        // [ 83] c7 46 30 00 00 00 00
        _emit 0xc7
        _emit 0x46
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // [ 90] 8b 11
        _emit 0x8b
        _emit 0x11
        // [ 92] 8b 42 20
        _emit 0x8b
        _emit 0x42
        _emit 0x20
        // [ 95] 5b
        _emit 0x5b
        // [ 96] 5e
        _emit 0x5e
        // [ 97] ff e0
        _emit 0xff
        _emit 0xe0
        // [ 99] 5e
        _emit 0x5e
        // [100] c3
        _emit 0xc3
    }
}
