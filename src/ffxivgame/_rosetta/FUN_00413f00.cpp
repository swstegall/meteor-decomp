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
// FUNCTION: ffxivgame 0x00413f00 — engine_memory DetachableHeapBlock
//           adjustor-thiscall: walk linked list, accumulate sizes, dispatch
//           (116 B / 0x74).
//
// Called via the secondary sub-object vftable at +0x04.  ECX on entry
// points to the secondary vftable slot (this+4), so the first thing the
// function does is adjust back to the primary this:
//
//   LEA EAX, [ECX-4]    ; EAX = real this
//   MOV ECX, EAX        ; ECX = real this
//
// Phase 1 — walk [ECX+0x2c] linked list to find the tail node:
//
//   MOV EDX, [ECX+0x2c]
//   TEST EDX, EDX
//   JZ   <found_tail>
//   LEA  ESP, [ESP]         ; 4-byte alignment NOP
//   .loop:
//     MOV  ECX, EDX
//     MOV  EDX, [ECX+0x2c]
//     TEST EDX, EDX
//     JNZ  .loop
//   <found_tail>:           ; ECX = tail node
//
// Phase 2 — save ESI, XOR ESI,ESI (accumulator = 0)
//
// Phase 3 — branch on byte flag at [ECX+0x25]:
//
//   if ([ECX+0x25] == 0):
//     if (EAX != 0):
//       LEA ESP,[ESP]        ; 7-byte NOP
//       .loop2:
//         ESI += [EAX+0x28]
//         EAX  = [EAX+0x2c]
//         JNZ .loop2
//     ECX = [ECX+0x18]
//     EAX = *ECX            ; vtable ptr
//     EDX = [EAX+4]         ; vtable[1]
//     CALL EDX              ; virtual call on the object at [tail+0x18]
//     EAX += ESI
//     POP ESI
//     RET
//
//   else ([ECX+0x25] != 0):
//     if (EAX == 0):
//       XOR EAX, EAX         ; return 0
//       POP ESI
//       RET
//     LEA ESP,[ESP]          ; 7-byte NOP
//     .loop3:
//       if ([EAX+0x26] != 0):  → jump to vtable path
//       ESI += [EAX+0x28]
//       EAX  = [EAX+0x2c]
//       JNZ .loop3
//     XOR EAX, EAX
//     POP ESI
//     RET
//
//   vtable_path:
//     EAX = [EAX+0x20]
//     EDX = *EAX              ; vtable ptr
//     ECX = EAX               ; this
//     EAX = [EDX+4]           ; vtable[1]
//     CALL EAX
//     EAX += ESI
//     POP ESI
//     RET
//
// No relocations: all CALLs are through registers (CALL EDX / CALL EAX).
// Raw-byte passthrough produces a byte-identical .obj with zero fixups.

extern "C" __declspec(naked) void FUN_00413f00() {
    __asm {
        _emit 0x8d              // LEA EAX, [ECX - 0x4]
        _emit 0x41
        _emit 0xfc
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0x8b              // MOV EDX, dword ptr [ECX + 0x2c]
        _emit 0x51
        _emit 0x2c
        _emit 0x85              // TEST EDX, EDX
        _emit 0xd2
        _emit 0x74              // JZ +0x0d -> found_tail
        _emit 0x0d
        _emit 0x8d              // LEA ESP, [ESP]  (4-byte NOP)
        _emit 0x64
        _emit 0x24
        _emit 0x00
        _emit 0x8b              // MOV ECX, EDX
        _emit 0xca
        _emit 0x8b              // MOV EDX, dword ptr [ECX + 0x2c]
        _emit 0x51
        _emit 0x2c
        _emit 0x85              // TEST EDX, EDX
        _emit 0xd2
        _emit 0x75              // JNZ -0x09 -> loop
        _emit 0xf7
        _emit 0x56              // PUSH ESI
        _emit 0x33              // XOR ESI, ESI
        _emit 0xf6
        _emit 0x80              // CMP byte ptr [ECX + 0x25], 0x0
        _emit 0x79
        _emit 0x25
        _emit 0x00
        _emit 0x75              // JNZ +0x23 -> flag_set
        _emit 0x23
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x8b              // MOV ECX, dword ptr [ECX + 0x18]
        _emit 0x49
        _emit 0x18
        _emit 0x74              // JZ +0x11 -> skip_loop2
        _emit 0x11
        _emit 0x8d              // LEA ESP, [ESP]  (7-byte NOP)
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x03              // ADD ESI, dword ptr [EAX + 0x28]
        _emit 0x70
        _emit 0x28
        _emit 0x8b              // MOV EAX, dword ptr [EAX + 0x2c]
        _emit 0x40
        _emit 0x2c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ -0x0a -> loop2
        _emit 0xf6
        _emit 0x8b              // MOV EAX, dword ptr [ECX]    (skip_loop2:)
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [EAX + 0x4]
        _emit 0x50
        _emit 0x04
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x03              // ADD EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        _emit 0x85              // TEST EAX, EAX              (flag_set:)
        _emit 0xc0
        _emit 0x74              // JZ +0x17 -> zero_return
        _emit 0x17
        _emit 0x8d              // LEA ESP, [ESP]  (7-byte NOP)
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80              // CMP byte ptr [EAX + 0x26], 0x0
        _emit 0x78
        _emit 0x26
        _emit 0x00
        _emit 0x75              // JNZ +0x0e -> vtable_path
        _emit 0x0e
        _emit 0x03              // ADD ESI, dword ptr [EAX + 0x28]
        _emit 0x70
        _emit 0x28
        _emit 0x8b              // MOV EAX, dword ptr [EAX + 0x2c]
        _emit 0x40
        _emit 0x2c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ -0x10 -> loop3
        _emit 0xf0
        _emit 0x33              // XOR EAX, EAX               (zero_return:)
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        _emit 0x8b              // MOV EAX, dword ptr [EAX + 0x20]  (vtable_path:)
        _emit 0x40
        _emit 0x20
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0x8b              // MOV EAX, dword ptr [EDX + 0x4]
        _emit 0x42
        _emit 0x04
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x03              // ADD EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
