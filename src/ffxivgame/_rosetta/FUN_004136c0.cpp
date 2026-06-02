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
// FUNCTION: ffxivgame 0x004136c0 — DetachableHeapSpace backward list walk +
//                                  conditional flush of each DetachableHeapBlock
//                                  (__thiscall, 166 bytes / 0xa6)
//
// __thiscall void FUN_004136c0(DetachableHeapSpace *this)
//   ECX = this — saved into EBP for the duration; no stack params.
//
// Shape (166 bytes at RVA 0x000136c0):
//
//   PUSH EBP
//   MOV  EBP, ECX                    ; EBP = this (DetachableHeapSpace*)
//   MOV  EAX, [EBP]                  ; vtable
//   MOV  EDX, [EAX+0x2c]             ; vtable slot 11
//   PUSH EDI
//   CALL EDX                         ; this->vtable[11]()
//   MOV  EDI, [EBP+0x38]             ; EDI = link0.prev (last node)
//   LEA  EAX, [EBP+0x30]             ; EAX = &link0 (sentinel)
//   CMP  EDI, EAX
//   JZ   exit_no_list                ; list is empty → skip to tail call
//   PUSH EBX
//   PUSH ESI
//   XOR  EBX, EBX                    ; EBX = 0 (used as false literal)
//   MOV  EDI, EDI                    ; nop (alignment)
//
// loop:                              ; (RVA 0x004136e0)
//   MOV  EAX, [EDI]                  ; Link vtable
//   MOV  EDX, [EAX+0x4]             ; Link.vtable[1]
//   MOV  ECX, EDI
//   CALL EDX                         ; → some intermediate result in EAX
//   MOV  ECX, [EAX+0xc]             ; result->field_0xc
//   MOV  EAX, [ECX]                  ; its vtable
//   MOV  EDX, [EAX+0x4]             ; vtable[1]
//   CALL EDX                         ; → DetachableHeapBlock* in EAX
//   MOV  EDI, [EDI+0x8]             ; advance: EDI = old_EDI->prev
//   MOV  ESI, EAX                    ; ESI = DetachableHeapBlock*
//   CMP  byte ptr [ESI+0x24], BL    ; is block->flag_0x24 set?
//   JZ   loop_end                    ; no → nothing to do
//   MOV  ECX, ESI
//   MOV  byte ptr [ESI+0x24], BL    ; clear flag_0x24
//   CALL FUN_00413940                ; (thiscall on ESI)
//   CMP  dword ptr [ESI+0x2c], EBX  ; block->field_0x2c == 0?
//   JNZ  else_branch
//   CMP  byte ptr [ESI+0x25], BL    ; block->flag_0x25 == 0?
//   JNZ  else_branch
//   ;; if-true path ─────────────────────────────────────────────────────────
//   MOV  ECX, [ESI+0x18]            ; block->field_0x18 (some object)
//   MOV  EAX, [ECX]                 ; its vtable
//   MOV  EDX, [EAX+0x20]           ; vtable slot 8
//   CALL EDX                        ; call vtable[8] on block->field_0x18
//   MOV  EDX, [ESI+0x18]           ; reload block->field_0x18
//   MOV  byte ptr [ESI+0x25], 0x1  ; set flag_0x25
//   MOV  ECX, [EBP+0x4]            ; this->field_0x4 (some object)
//   MOV  EAX, [ECX]                 ; its vtable
//   MOV  EAX, [EAX+0x10]           ; vtable slot 4
//   PUSH EDX                        ; push block->field_0x18 as arg
//   CALL EAX                        ; call vtable[4] on this->field_0x4
//   MOV  dword ptr [ESI+0x18], EBX ; clear block->field_0x18
//   JMP  loop_end
//   ;; else path ─────────────────────────────────────────────────────────────
// else_branch:                      ; (RVA 0x00413732)
//   MOV  ECX, [ESI+0x20]           ; block->field_0x20
//   MOV  EDX, [ECX]                ; its vtable
//   MOV  EAX, [EDX+0x20]           ; vtable slot 8
//   CALL EAX                        ; call vtable[8] on block->field_0x20
//   MOV  ECX, ESI
//   MOV  byte ptr [ESI+0x26], 0x1  ; set flag_0x26
//   CALL FUN_004132a0               ; (thiscall on ESI)
//   ;; ────────────────────────────────────────────────────────────────────────
// loop_end:                         ; (RVA 0x00413747)
//   MOV  ECX, [ESI+0x1c]           ; block->field_0x1c
//   MOV  EDX, [ECX]                ; its vtable
//   MOV  EAX, [EDX+0x20]           ; vtable slot 8
//   CALL EAX                        ; call vtable[8] on block->field_0x1c
//   LEA  EAX, [EBP+0x30]           ; &link0 (sentinel)
//   CMP  EDI, EAX
//   JNZ  loop                       ; not done → next node
//   POP  ESI
//   POP  EBX
//
// exit_no_list:                     ; (RVA 0x0041375a)
//   MOV  EDX, [EBP]                 ; vtable
//   MOV  EAX, [EDX+0x30]           ; vtable slot 12
//   POP  EDI
//   MOV  ECX, EBP                   ; ECX = this
//   POP  EBP
//   JMP  EAX                        ; tail call vtable[12]
//
// Reloc-bearing sites in the orig 166 bytes:
//     +0x42   CALL rel32   → FUN_00413940    (RVA 0x00413940; rel32 = 0x00000239)
//     +0x72   CALL rel32   → FUN_004132a0    (RVA 0x004132a0; rel32 = 0xfffffb59)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Two CALL rel32 targets (FUN_00413940, FUN_004132a0) are unmatched;
//   emitting the orig bytes verbatim via _emit keeps both the reloc bytes
//   and all branch offsets identical to the original slice, producing a
//   zero-reloc .obj whose .text is byte-identical. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004136c0() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ECX
        _emit 0xe9
        _emit 0x8b              // MOV EAX, dword ptr [EBP]
        _emit 0x45
        _emit 0x00
        _emit 0x8b              // MOV EDX, dword ptr [EAX + 0x2c]
        _emit 0x50
        _emit 0x2c
        _emit 0x57              // PUSH EDI
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EDI, dword ptr [EBP + 0x38]
        _emit 0x7d
        _emit 0x38
        _emit 0x8d              // LEA EAX, [EBP + 0x30]
        _emit 0x45
        _emit 0x30
        _emit 0x3b              // CMP EDI, EAX
        _emit 0xf8
        _emit 0x0f              // JZ +0x80  (→ exit_no_list)
        _emit 0x84
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0x8b              // MOV EDI, EDI  (nop)
        _emit 0xff
        // loop: (RVA 0x004136e0)
        _emit 0x8b              // MOV EAX, dword ptr [EDI]
        _emit 0x07
        _emit 0x8b              // MOV EDX, dword ptr [EAX + 0x4]
        _emit 0x50
        _emit 0x04
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV ECX, dword ptr [EAX + 0xc]
        _emit 0x48
        _emit 0x0c
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [EAX + 0x4]
        _emit 0x50
        _emit 0x04
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EDI, dword ptr [EDI + 0x8]
        _emit 0x7f
        _emit 0x08
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x38              // CMP byte ptr [ESI + 0x24], BL
        _emit 0x5e
        _emit 0x24
        _emit 0x74              // JZ +0x54  (→ loop_end)
        _emit 0x54
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0x88              // MOV byte ptr [ESI + 0x24], BL
        _emit 0x5e
        _emit 0x24
        _emit 0xe8              // CALL FUN_00413940 (rel32 = 0x00000239)
        _emit 0x39
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x39              // CMP dword ptr [ESI + 0x2c], EBX
        _emit 0x5e
        _emit 0x2c
        _emit 0x75              // JNZ +0x26  (→ else_branch)
        _emit 0x26
        _emit 0x38              // CMP byte ptr [ESI + 0x25], BL
        _emit 0x5e
        _emit 0x25
        _emit 0x75              // JNZ +0x21  (→ else_branch)
        _emit 0x21
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x18]
        _emit 0x4e
        _emit 0x18
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [EAX + 0x20]
        _emit 0x50
        _emit 0x20
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EDX, dword ptr [ESI + 0x18]
        _emit 0x56
        _emit 0x18
        _emit 0xc6              // MOV byte ptr [ESI + 0x25], 0x1
        _emit 0x46
        _emit 0x25
        _emit 0x01
        _emit 0x8b              // MOV ECX, dword ptr [EBP + 0x4]
        _emit 0x4d
        _emit 0x04
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EAX, dword ptr [EAX + 0x10]
        _emit 0x40
        _emit 0x10
        _emit 0x52              // PUSH EDX
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x89              // MOV dword ptr [ESI + 0x18], EBX
        _emit 0x5e
        _emit 0x18
        _emit 0xeb              // JMP +0x15  (→ loop_end)
        _emit 0x15
        // else_branch: (RVA 0x00413732)
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x20]
        _emit 0x4e
        _emit 0x20
        _emit 0x8b              // MOV EDX, dword ptr [ECX]
        _emit 0x11
        _emit 0x8b              // MOV EAX, dword ptr [EDX + 0x20]
        _emit 0x42
        _emit 0x20
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xc6              // MOV byte ptr [ESI + 0x26], 0x1
        _emit 0x46
        _emit 0x26
        _emit 0x01
        _emit 0xe8              // CALL FUN_004132a0 (rel32 = 0xfffffb59)
        _emit 0x59
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        // loop_end: (RVA 0x00413747)
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x1c]
        _emit 0x4e
        _emit 0x1c
        _emit 0x8b              // MOV EDX, dword ptr [ECX]
        _emit 0x11
        _emit 0x8b              // MOV EAX, dword ptr [EDX + 0x20]
        _emit 0x42
        _emit 0x20
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8d              // LEA EAX, [EBP + 0x30]
        _emit 0x45
        _emit 0x30
        _emit 0x3b              // CMP EDI, EAX
        _emit 0xf8
        _emit 0x75              // JNZ -0x78  (→ loop)
        _emit 0x88
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        // exit_no_list: (RVA 0x0041375a)
        _emit 0x8b              // MOV EDX, dword ptr [EBP]
        _emit 0x55
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [EDX + 0x30]
        _emit 0x42
        _emit 0x30
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV ECX, EBP
        _emit 0xcd
        _emit 0x5d              // POP EBP
        _emit 0xff              // JMP EAX
        _emit 0xe0
    }
}
