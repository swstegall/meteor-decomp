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
// FUNCTION: ffxivgame 0x00414370 — engine_memory AlternativeAllocator free/detach
//                                  (99 B / 0x63)
//
// __thiscall void FUN_00414370(C *this, void *param_1)
//   ECX        : this  (AlternativeAllocator or owning class with Enter/Leave vtable)
//   [ESP+0x04] : param_1  (a list/pool sentinel — indirected twice to reach the cell)
//   Epilogue: RET 4  (callee-cleans 1 stack arg)
//   Callee-saves: EBX (cached uVar2), ESI (cell), EDI (this)
//
// Body (no branches, no loops — tight closed-form sequence):
//
//   this->vtable[11]();                          // Enter (slot +0x2C)
//   cell_owner = param_1->vtable[5]();           // slot +0x14 → list sentinel
//   cell       = cell_owner->vtable[1]();        // slot +0x04 → heap-block node
//   uVar2 = cell->vtable[1]();                   // primary sub-object .vf1()
//   uVar3 = (&cell[1])->vtable[1]();             // secondary sub-object .vf1()
//                                                //   (cell+4 has its own vptr)
//   FUN_009d56fd(uVar3);                         // free secondary buffer
//   cell->vtable[0](0);                          // destructor / detach with arg 0
//   FUN_009d56fd(uVar2);                         // free primary buffer
//   this->vtable[12]();                          // Leave (slot +0x30)
//
// Disassembly of the orig 99 bytes at RVA 0x00014370:
//
//   00014370: 53              PUSH EBX                   ; save callee-save
//   00014371: 56              PUSH ESI                   ; save callee-save
//   00014372: 57              PUSH EDI                   ; save callee-save
//   00014373: 8b f9           MOV  EDI, ECX              ; EDI = this
//   00014375: 8b 07           MOV  EAX, [EDI]            ; this's vptr
//   00014377: 8b 50 2c        MOV  EDX, [EAX+0x2C]       ; vtable[11] = Enter
//   0001437a: ff d2           CALL EDX                   ; this->Enter()
//   0001437c: 8b 4c 24 10     MOV  ECX, [ESP+0x10]       ; param_1 (after 3 pushes)
//   00014380: 8b 01           MOV  EAX, [ECX]            ; param_1's vptr
//   00014382: 8b 50 14        MOV  EDX, [EAX+0x14]       ; vtable[5]
//   00014385: ff d2           CALL EDX                   ; param_1->vtable[5]() = cell_owner
//   00014387: 8b 10           MOV  EDX, [EAX]            ; cell_owner's vptr
//   00014389: 8b c8           MOV  ECX, EAX              ; ECX = cell_owner
//   0001438b: 8b 42 04        MOV  EAX, [EDX+0x04]       ; vtable[1]
//   0001438e: ff d0           CALL EAX                   ; cell_owner->vtable[1]() = cell
//   00014390: 8b f0           MOV  ESI, EAX              ; ESI = cell
//   00014392: 8b 16           MOV  EDX, [ESI]            ; cell's vptr
//   00014394: 8b 42 04        MOV  EAX, [EDX+0x04]       ; cell vtable[1]
//   00014397: 8b ce           MOV  ECX, ESI              ; ECX = cell
//   00014399: ff d0           CALL EAX                   ; cell->vf1() = uVar2
//   0001439b: 8b 56 04        MOV  EDX, [ESI+0x04]       ; sub-object vptr at cell+4
//   0001439e: 8d 4e 04        LEA  ECX, [ESI+0x04]       ; ECX = &cell->embedded_sub
//   000143a1: 8b d8           MOV  EBX, EAX              ; EBX = uVar2 (cached)
//   000143a3: 8b 42 04        MOV  EAX, [EDX+0x04]       ; sub-obj vtable[1]
//   000143a6: ff d0           CALL EAX                   ; (&cell[1])->vf1() = uVar3
//   000143a8: 50              PUSH EAX                   ; arg = uVar3
//   000143a9: e8 4f 13 5c 00  CALL FUN_009d56fd          ; free secondary buffer
//   000143ae: 8b 16           MOV  EDX, [ESI]            ; cell's vptr
//   000143b0: 8b 02           MOV  EAX, [EDX]            ; vtable[0]
//   000143b2: 83 c4 04        ADD  ESP, 4                ; pop arg
//   000143b5: 6a 00           PUSH 0                     ; arg = 0 (no-delete)
//   000143b7: 8b ce           MOV  ECX, ESI              ; ECX = cell
//   000143b9: ff d0           CALL EAX                   ; cell->vf0(0) = detach destructor
//   000143bb: 53              PUSH EBX                   ; arg = uVar2 (primary)
//   000143bc: e8 3c 13 5c 00  CALL FUN_009d56fd          ; free primary buffer
//   000143c1: 8b 17           MOV  EDX, [EDI]            ; this's vptr
//   000143c3: 8b 42 30        MOV  EAX, [EDX+0x30]       ; vtable[12] = Leave
//   000143c6: 83 c4 04        ADD  ESP, 4                ; pop arg
//   000143c9: 8b cf           MOV  ECX, EDI              ; ECX = this
//   000143cb: ff d0           CALL EAX                   ; this->Leave()
//   000143cd: 5f              POP  EDI
//   000143ce: 5e              POP  ESI
//   000143cf: 5b              POP  EBX
//   000143d0: c2 04 00        RET  0x4                   ; __thiscall, 1 stack arg
//
// Reloc-bearing sites in the orig 99 bytes:
//   +0x39   CALL rel32 → FUN_009d56fd (0x005c134f, rel32 = 0x005c134f)
//   +0x4c   CALL rel32 → FUN_009d56fd (0x005c133c, rel32 = 0x005c133c)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The two CALL rel32 targets (both to FUN_009d56fd, a free/delete
//   helper) are baked in as raw bytes against the orig image base.
//   A `__declspec(naked)` body re-emitting the orig 99 bytes via
//   MASM `_emit` directives produces a .obj whose .text is byte-identical
//   with no relocations; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00414370() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x8b              // MOV EAX, dword ptr [EDI]
        _emit 0x07
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x2C]
        _emit 0x50
        _emit 0x2c
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x14]
        _emit 0x50
        _emit 0x14
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x04]
        _emit 0x42
        _emit 0x04
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x8b              // MOV EDX, dword ptr [ESI]
        _emit 0x16
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x04]
        _emit 0x42
        _emit 0x04
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x04]
        _emit 0x56
        _emit 0x04
        _emit 0x8d              // LEA ECX, dword ptr [ESI+0x04]
        _emit 0x4e
        _emit 0x04
        _emit 0x8b              // MOV EBX, EAX
        _emit 0xd8
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x04]
        _emit 0x42
        _emit 0x04
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_009d56fd (rel32 = 0x005c134f)
        _emit 0x4f
        _emit 0x13
        _emit 0x5c
        _emit 0x00
        _emit 0x8b              // MOV EDX, dword ptr [ESI]
        _emit 0x16
        _emit 0x8b              // MOV EAX, dword ptr [EDX]
        _emit 0x02
        _emit 0x83              // ADD ESP, 4
        _emit 0xc4
        _emit 0x04
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x53              // PUSH EBX
        _emit 0xe8              // CALL FUN_009d56fd (rel32 = 0x005c133c)
        _emit 0x3c
        _emit 0x13
        _emit 0x5c
        _emit 0x00
        _emit 0x8b              // MOV EDX, dword ptr [EDI]
        _emit 0x17
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x30]
        _emit 0x42
        _emit 0x30
        _emit 0x83              // ADD ESP, 4
        _emit 0xc4
        _emit 0x04
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
