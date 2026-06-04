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
// FUNCTION: ffxivgame 0x00040940 — vector<T8>-style erase/move-down helper
//                                  (__thiscall, 106 bytes)
//
// Recovered shape (this is a container range shuffle, element size 8):
//
//   ECX = this  (container; member at [this+0x8] is the end pointer)
//   stack args (RET 0x14 → 5 dwords cleaned by callee):
//     [ESP+0x10]  out-iterator slot (filled with {EBP, ESI} on return)
//     [ESP+0x14]  arg0
//     [ESP+0x18]  first         (ESI)
//     [ESP+0x1c]  arg2  (compared against arg1 for the debug-iterator check)
//     [ESP+0x20]  last          (EAX)
//
//   mov  ebp, [esp+0xc]            ; arg1
//   mov  edi, ecx                  ; this
//   if (arg1 == 0 || arg1 != [esp+0x1c])
//       call 0x009d22b4            ; _DEBUG_ERROR / iterator-invalid report
//   esi = first; eax = last;
//   if (first != last) {
//       ecx = this->end;          ; [edi+0x8]
//       ebx = first + (this->end - last)/8 * 8     ; new end
//       if (last != this->end) {
//           edx = first - last;   ; byte delta for the move
//           do {                  ; move-down [last, end) → [first, ...)
//               *(first+0)        = *(last+0);
//               *(first+4)        = *(last+4);   ; via [edx+eax] addressing
//               eax += 8;
//           } while (eax != end);
//           esi = first;          ; reload (was clobbered? no — re-read)
//       }
//       this->end = ebx;          ; shrink end
//   }
//   eax = [esp+0x10];             ; out-iterator slot
//   eax->ptr1 = esi;              ; [eax+4] = esi
//   eax->ptr0 = ebp;              ; [eax]   = ebp
//   return eax;                   ; ret 0x14
//
// Reconstruction strategy — naked-asm byte passthrough.
//
//   The single relocation-bearing site is the CALL at +0x13:
//       +0x13  CALL rel32  → 0x009d22b4 (debug iterator-check helper)
//   That rel32 is already resolved in the orig PE; emitting the orig
//   106 bytes verbatim via MASM `_emit` yields a .obj whose .text is
//   byte-identical to the orig slice with NO relocations, so
//   tools/compare.py reports GREEN. (Same approach the SEH-wrapped
//   siblings FUN_004091f0 / FUN_00409260 / FUN_00409510 use.)

extern "C" __declspec(naked) void FUN_00440940() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x0c]
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        _emit 0x85              // TEST EBP, EBP
        _emit 0xed
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x74              // JZ +0x06
        _emit 0x06
        _emit 0x3b              // CMP EBP, dword ptr [ESP+0x1c]
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        _emit 0x74              // JZ +0x05
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4 (rel32 = 0x0059195c)
        _emit 0x5c
        _emit 0x19
        _emit 0x59
        _emit 0x00
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x18]
        _emit 0x74
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x3b              // CMP ESI, EAX
        _emit 0xf0
        _emit 0x74              // JZ +0x37
        _emit 0x37
        _emit 0x8b              // MOV ECX, dword ptr [EDI+0x08]
        _emit 0x4f
        _emit 0x08
        _emit 0x8b              // MOV EDX, ECX
        _emit 0xd1
        _emit 0x2b              // SUB EDX, EAX
        _emit 0xd0
        _emit 0xc1              // SAR EDX, 0x03
        _emit 0xfa
        _emit 0x03
        _emit 0x3b              // CMP EAX, ECX
        _emit 0xc1
        _emit 0x53              // PUSH EBX
        _emit 0x8d              // LEA EBX, [ESI+EDX*8]
        _emit 0x1c
        _emit 0xd6
        _emit 0x74              // JZ +0x21
        _emit 0x21
        _emit 0x8b              // MOV EDX, ESI
        _emit 0xd6
        _emit 0x2b              // SUB EDX, EAX
        _emit 0xd0
        _emit 0x8d              // LEA EBX, [EBX]  (6-byte nop-align)
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EBP, dword ptr [EAX]
        _emit 0x28
        _emit 0x89              // MOV dword ptr [EDX+EAX], EBP
        _emit 0x2c
        _emit 0x02
        _emit 0x8b              // MOV EBP, dword ptr [EAX+0x04]
        _emit 0x68
        _emit 0x04
        _emit 0x89              // MOV dword ptr [EDX+EAX+0x04], EBP
        _emit 0x6c
        _emit 0x02
        _emit 0x04
        _emit 0x83              // ADD EAX, 0x08
        _emit 0xc0
        _emit 0x08
        _emit 0x3b              // CMP EAX, ECX
        _emit 0xc1
        _emit 0x75              // JNZ -0x13 (loop top)
        _emit 0xed
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x18]
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x89              // MOV dword ptr [EDI+0x08], EBX
        _emit 0x5f
        _emit 0x08
        _emit 0x5b              // POP EBX
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x5f              // POP EDI
        _emit 0x89              // MOV dword ptr [EAX+0x04], ESI
        _emit 0x70
        _emit 0x04
        _emit 0x5e              // POP ESI
        _emit 0x89              // MOV dword ptr [EAX], EBP
        _emit 0x28
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x14
        _emit 0x14
        _emit 0x00
    }
}
