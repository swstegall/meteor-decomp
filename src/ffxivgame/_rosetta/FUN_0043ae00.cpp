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
// FUNCTION: ffxivgame 0x0043ae00 — copy-elements loop with SEH frame
//                                   (__thiscall, 167 bytes / 0xa7)
//
// __thiscall void* FUN_0043ae00(T *this, U *src)
//   ECX        : this  (the destination object)
//   [ESP+0x04] : src   (source object; read back from [ESP+0x28] after
//                       the 9-push SEH prologue shifts ESP by 36 bytes)
//
// Behaviour (167 bytes at RVA 0x0003ae00):
//
//   SEH prologue:
//     PUSH -1                     ; exception-state sentinel
//     PUSH 0xe56508               ; pointer into __except_handler3 table
//     MOV EAX, FS:[0]             ; save previous SEH head
//     PUSH EAX
//     PUSH ECX                    ; save 'this'
//     PUSH EBX/EBP/ESI/EDI        ; callee-saved registers
//     MOV EAX, [0x012ea8b0]       ; /GS security cookie
//     XOR EAX, ESP
//     PUSH EAX                    ; push scrambled cookie
//     LEA EAX, [ESP+0x18]         ; point at the prev-FS:[0] slot
//     MOV FS:[0], EAX             ; link SEH frame
//
//   Body:
//     MOV ESI, ECX                ; ESI = this
//     MOV [ESP+0x14], ESI         ; spill this into saved-ECX slot
//     PUSH 0x43aa50               ; arg5
//     PUSH 0x43a6b0               ; arg4
//     PUSH 0xd                    ; arg3 = 13
//     PUSH 0x1c                   ; arg2 = 28 (sizeof element)
//     PUSH ESI                    ; arg1 = this
//     CALL 0x009d61d6             ; __stdcall vtable/ctor init helper
//     MOV EBP, [ESP+0x28]         ; EBP = src (param after SEH prologue)
//     XOR EDI, EDI                ; EDI = 0 (loop counter)
//     MOV [ESI+0x16c], EDI        ; this->count = 0
//     CMP [EBP+0x16c], EDI        ; src->count == 0?
//     MOV [ESP+0x20], EDI         ; SEH exception-state = 0
//     JBE  exit                   ; skip loop if src empty
//     MOV EBX, EBP                ; EBX = current src element ptr
//     LEA ESP, [ESP]              ; 7-byte loop-alignment NOP
//   loop:
//     MOV EAX, [ESI+0x16c]        ; EAX = this->count (current)
//     LEA ECX, [EAX*8+0]          ; ECX = EAX * 8
//     SUB ECX, EAX                ; ECX = EAX * 7 (dwords; * 4 = EAX * 28)
//     ADD EAX, 1                  ; new count
//     LEA ECX, [ESI+ECX*4]        ; ECX = &this->items[old_count]
//     PUSH EBX                    ; arg: src element ptr
//     MOV [ESI+0x16c], EAX        ; this->count = new count
//     CALL FUN_0043abc0           ; element copy constructor
//     ADD EDI, 1
//     ADD EBX, 0x1c               ; advance src by sizeof(T)=28
//     CMP EDI, [EBP+0x16c]        ; done?
//     JC  loop
//   exit:
//     MOV EAX, ESI                ; return this
//
//   SEH epilogue:
//     MOV ECX, [ESP+0x18]         ; recover saved prev-FS:[0]
//     MOV FS:[0], ECX
//     POP ECX                     ; discard cookie
//     POP EDI / ESI / EBP / EBX
//     ADD ESP, 0x10               ; reclaim saved-ECX + 3 SEH frame dwords
//     RET 0x4                     ; __thiscall: callee cleans 1 param dword
//
// Reloc-bearing sites in the orig 167 bytes:
//     +0x02   PUSH imm32   → 0xe56508       (SEH except-handler table)
//     +0x08   MOV  EAX     → FS:[0x0]       (FS-segment override)
//     +0x14   MOV  EAX     → [0x012ea8b0]   (security cookie)
//     +0x20   MOV  FS:[0]  → EAX            (FS-segment override)
//     +0x2c   PUSH imm32   → 0x0043aa50     (arg to init helper)
//     +0x31   PUSH imm32   → 0x0043a6b0     (arg to init helper)
//     +0x3b   CALL rel32   → 0x009d61d6     (init helper)
//     +0x7d   CALL rel32   → 0x0043abc0     (element copy ctor)
//     +0x96   MOV  FS:[0]  → ECX            (FS-segment override)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function body contains multiple absolute-address immediates and
//   FS-segment-override instructions that produce reloc entries at link
//   time. Emitting as raw bytes via __declspec(naked) + _emit gives a
//   .obj whose .text is byte-identical to the orig slice (compare.py
//   masks reloc bytes from the diff), which is the same approach used
//   by siblings FUN_004063c0, FUN_004071b0, FUN_00404d60.

extern "C" __declspec(naked) void FUN_0043ae00() {
    __asm {
        // --- SEH / security-cookie prologue ---
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0xe56508
        _emit 0x08
        _emit 0x65
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX  (save prev SEH head)
        _emit 0x51              // PUSH ECX  (save 'this')
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, [0x012ea8b0]  (security cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX  (push scrambled cookie)
        _emit 0x8d              // LEA EAX, [ESP + 0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x64              // MOV FS:[0x0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // --- body ---
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x89              // MOV dword ptr [ESP + 0x14], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x68              // PUSH 0x43aa50
        _emit 0x50
        _emit 0xaa
        _emit 0x43
        _emit 0x00
        _emit 0x68              // PUSH 0x43a6b0
        _emit 0xb0
        _emit 0xa6
        _emit 0x43
        _emit 0x00
        _emit 0x6a              // PUSH 0xd
        _emit 0x0d
        _emit 0x6a              // PUSH 0x1c
        _emit 0x1c
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL 0x009d61d6  (rel32 = 0x0059b397)
        _emit 0x97
        _emit 0xb3
        _emit 0x59
        _emit 0x00
        _emit 0x8b              // MOV EBP, dword ptr [ESP + 0x28]
        _emit 0x6c
        _emit 0x24
        _emit 0x28
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0x89              // MOV dword ptr [ESI + 0x16c], EDI
        _emit 0xbe
        _emit 0x6c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x39              // CMP dword ptr [EBP + 0x16c], EDI
        _emit 0xbd
        _emit 0x6c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESP + 0x20], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x76              // JBE +0x38  (→ exit)
        _emit 0x38
        _emit 0x8b              // MOV EBX, EBP
        _emit 0xdd
        _emit 0x8d              // LEA ESP, [ESP]  (7-byte NOP for loop alignment)
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // --- loop: ---
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x16c]
        _emit 0x86
        _emit 0x6c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [EAX*8 + 0x0]
        _emit 0x0c
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB ECX, EAX
        _emit 0xc8
        _emit 0x83              // ADD EAX, 0x1
        _emit 0xc0
        _emit 0x01
        _emit 0x8d              // LEA ECX, [ESI + ECX*4]
        _emit 0x0c
        _emit 0x8e
        _emit 0x53              // PUSH EBX
        _emit 0x89              // MOV dword ptr [ESI + 0x16c], EAX
        _emit 0x86
        _emit 0x6c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_0043abc0  (rel32 = 0xfffffd3f)
        _emit 0x3f
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD EDI, 0x1
        _emit 0xc7
        _emit 0x01
        _emit 0x83              // ADD EBX, 0x1c
        _emit 0xc3
        _emit 0x1c
        _emit 0x3b              // CMP EDI, dword ptr [EBP + 0x16c]
        _emit 0xbd
        _emit 0x6c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x72              // JC -0x2f  (→ loop)
        _emit 0xd1
        // --- exit: ---
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x64              // MOV FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX  (discard cookie)
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
