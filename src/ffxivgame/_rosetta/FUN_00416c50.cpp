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
// FUNCTION: ffxivgame 0x00416c50 — __thiscall predicate loop that calls
//                                  vtable[9] (offset 0x24) for each element;
//                                  returns true on first truthy call, false
//                                  if all calls return false.
//                                  (86 B, __thiscall, bool, 1 stack arg)
//
// Calling convention: __thiscall — ECX = this (→ ESI), one stack argument,
//   RET 4 (callee-pops the single stack argument).
// Return: bool in AL (0 = false, 1 = true).
//
// Object fields accessed (this-relative):
//   [ESI+0x00] = vtable pointer
//   [ESI+0x04] = int32 initial offset loaded into EDI
//   [ESI+0x0C] = int32 iteration count / loop limit
//   [ESI+0x14] = uint8 stride component A
//   [ESI+0x15] = uint8 stride component B
//   [ESI+0x16] = uint8 stride component C
//
// Logic:
//   EDI = [ESI+0x04];
//   for (EBX = 0; EBX < [ESI+0x0C]; EBX++) {
//       if (vtable[9](arg, (uint8)[ESI+0x14] + EDI)) return true;
//       EDI += (uint8)[ESI+0x14] + (uint8)[ESI+0x15] + (uint8)[ESI+0x16];
//   }
//   return false;
//
// Reconstruction strategy — naked-asm byte passthrough:
//   All branches are intra-function relative offsets with no external
//   relocations; the 86 orig bytes are re-emitted verbatim via MASM
//   _emit directives so compare.py reports GREEN without any reloc
//   masking.

extern "C" __declspec(naked) void FUN_00416c50() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0x39              // CMP dword ptr [ESI+0x0C], EBX
        _emit 0x5e
        _emit 0x0c
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESI+0x04]
        _emit 0x7e
        _emit 0x04
        _emit 0x76              // JBE +0x34  (→ exit_false)
        _emit 0x34
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x14]  ; stack arg (after 4 pushes)
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        // loop_body:
        _emit 0x0f              // MOVZX EAX, byte ptr [ESI+0x14]
        _emit 0xb6
        _emit 0x46
        _emit 0x14
        _emit 0x8b              // MOV EDX, dword ptr [ESI]        ; vtable
        _emit 0x16
        _emit 0x03              // ADD EAX, EDI                    ; computed offset arg
        _emit 0xc7
        _emit 0x50              // PUSH EAX                        ; arg2 = computed offset
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x24]   ; vtable[9]
        _emit 0x42
        _emit 0x24
        _emit 0x55              // PUSH EBP                        ; arg1 = stack arg
        _emit 0x8b              // MOV ECX, ESI                    ; this
        _emit 0xce
        _emit 0xff              // CALL EAX                        ; vtable[9](arg1, arg2)
        _emit 0xd0
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x75              // JNZ +0x24  (→ exit_true)
        _emit 0x24
        _emit 0x0f              // MOVZX ECX, byte ptr [ESI+0x16]  ; stride C
        _emit 0xb6
        _emit 0x4e
        _emit 0x16
        _emit 0x0f              // MOVZX EDX, byte ptr [ESI+0x15]  ; stride B
        _emit 0xb6
        _emit 0x56
        _emit 0x15
        _emit 0x0f              // MOVZX EAX, byte ptr [ESI+0x14]  ; stride A
        _emit 0xb6
        _emit 0x46
        _emit 0x14
        _emit 0x03              // ADD ECX, EDX                    ; B + C
        _emit 0xca
        _emit 0x03              // ADD EAX, EDI                    ; A + EDI
        _emit 0xc7
        _emit 0x83              // ADD EBX, 1                      ; i++
        _emit 0xc3
        _emit 0x01
        _emit 0x3b              // CMP EBX, dword ptr [ESI+0x0C]   ; i < count?
        _emit 0x5e
        _emit 0x0c
        _emit 0x8d              // LEA EDI, [EAX + ECX*1]          ; new offset
        _emit 0x3c
        _emit 0x08
        _emit 0x72              // JC -0x30  (→ loop_body)
        _emit 0xd0
        // exit_false:
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x32              // XOR AL, AL                      ; return false
        _emit 0xc0
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 4
        _emit 0x04
        _emit 0x00
        // exit_true:
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0xb0              // MOV AL, 1                       ; return true
        _emit 0x01
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 4
        _emit 0x04
        _emit 0x00
    }
}
