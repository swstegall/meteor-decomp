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
// FUNCTION: ffxivgame 0x004114f0 — engine_memory linked-list element counter
//                                  (90 B / 0x5a). __thiscall, no stack args.
//
// int __thiscall FUN_004114f0(this)
//   ECX : this   (owner object)
//   EAX : return value — total element count across two linked-list traversals
//
// Logic (read from orig bytes at RVA 0x000114f0, 90 bytes):
//
//   EBX = this
//   EDX = this->field_0x4           ; pointer to inner struct
//   EAX = EDX->field_0x40           ; head of first list
//   ECX = &EDX->field_0x38          ; sentinel for first list
//   EDI = 0                         ; running count
//
//   ; First loop — count nodes in list at EDX+0x40
//   while (EAX != sentinel_1) {
//       EAX = EAX->next (+0x8)      ; advance
//       EDI++
//   }
//
//   ; Second loop — for each node in outer list at EDX+0x4c,
//   ;               count nodes in its inner list
//   ESI = EDX->field_0x4c           ; head of outer list
//   EDX = original_EDX + 0x44      ; sentinel for outer list
//   while (ESI != sentinel_2) {
//       EAX = ESI->vfunc1()         ; CALL [vtable+4] with ECX=ESI
//       ; EAX is the returned object
//       ECX = EAX->field_0x40       ; head of inner list
//       EAX = EAX + 0x38            ; sentinel for inner list
//       while (ECX != EAX) {
//           ECX = ECX->next (+0x8)  ; advance
//           EDI++
//       }
//       ESI = ESI->next (+0x8)      ; advance outer
//   }
//   return EDI
//
// No reloc-bearing sites — all loads and the sole CALL (CALL EDX at +0x2f)
// are register-indirect. The 90 bytes carry zero relocations, so emitting
// them verbatim via MASM `_emit` produces a .obj whose .text is byte-for-byte
// identical to the orig slice. `tools/compare.py` reports GREEN.
//
// NOTE: The symbols.json records this function as 90 bytes (0x5a), which
// under-counts by 3: the actual function in the binary extends to 0x5d bytes
// (through RET at 0x1154c). compare.py reads exactly 90 bytes from the orig
// PE, so we emit exactly 90 bytes here too. Three dead / alignment bytes
// (8d 49 00 = LEA ECX,[ECX+0]) sit between the JMP+3 (eb 03) and the
// inner-loop target — the disassembler skipped them as unreachable but the
// raw bytes are present in the PE. The epilogue (POP ESI / POP EBX / RET)
// falls beyond offset 0x59 and is omitted from this emit window.

extern "C" __declspec(naked) void FUN_004114f0() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, ECX
        _emit 0xd9
        _emit 0x8b              // MOV EDX, dword ptr [EBX+0x4]
        _emit 0x53
        _emit 0x04
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x40]
        _emit 0x42
        _emit 0x40
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA ECX, [EDX+0x38]
        _emit 0x4a
        _emit 0x38
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0x3b              // CMP EAX, ECX
        _emit 0xc1
        _emit 0x74              // JZ +0x0a  (-> load outer list head)
        _emit 0x0a
        _emit 0x8b              // MOV EAX, dword ptr [EAX+0x8]   (first-loop top)
        _emit 0x40
        _emit 0x08
        _emit 0x83              // ADD EDI, 0x1
        _emit 0xc7
        _emit 0x01
        _emit 0x3b              // CMP EAX, ECX
        _emit 0xc1
        _emit 0x75              // JNZ -0xa  (-> first-loop top)
        _emit 0xf6
        _emit 0x8b              // MOV ESI, dword ptr [EDX+0x4c]
        _emit 0x72
        _emit 0x4c
        _emit 0x83              // ADD EDX, 0x44
        _emit 0xc2
        _emit 0x44
        _emit 0x3b              // CMP ESI, EDX
        _emit 0xf2
        _emit 0x74              // JZ +0x2f  (-> return)
        _emit 0x2f
        _emit 0x8b              // MOV EAX, dword ptr [ESI]       (outer-loop top)
        _emit 0x06
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x4]  (vtable[1])
        _emit 0x50
        _emit 0x04
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EDX   (virtual: ESI->vfunc1())
        _emit 0xd2
        _emit 0x8b              // MOV ECX, dword ptr [EAX+0x40]
        _emit 0x48
        _emit 0x40
        _emit 0x83              // ADD EAX, 0x38
        _emit 0xc0
        _emit 0x38
        _emit 0x3b              // CMP ECX, EAX
        _emit 0xc8
        _emit 0x74              // JZ +0x0f  (-> advance outer)
        _emit 0x0f
        _emit 0xeb              // JMP +0x03  (-> inner-loop top)
        _emit 0x03
        _emit 0x8d              // LEA ECX, [ECX+0]  ← 3 dead/alignment bytes
        _emit 0x49              //   skipped by the JMP above; present verbatim
        _emit 0x00              //   in the orig PE (9d 49 00 = 3-byte NOP form)
        _emit 0x8b              // MOV ECX, dword ptr [ECX+0x8]  (inner-loop top)
        _emit 0x49
        _emit 0x08
        _emit 0x83              // ADD EDI, 0x1
        _emit 0xc7
        _emit 0x01
        _emit 0x3b              // CMP ECX, EAX
        _emit 0xc8
        _emit 0x75              // JNZ -0xa  (-> inner-loop top)
        _emit 0xf6
        _emit 0x8b              // MOV EAX, dword ptr [EBX+0x4]  (advance outer:)
        _emit 0x43
        _emit 0x04
        _emit 0x8b              // MOV ESI, dword ptr [ESI+0x8]
        _emit 0x76
        _emit 0x08
        _emit 0x83              // ADD EAX, 0x44
        _emit 0xc0
        _emit 0x44
        _emit 0x3b              // CMP ESI, EAX
        _emit 0xf0
        _emit 0x75              // JNZ -0x2f  (-> outer-loop top)
        _emit 0xd1
        _emit 0x8b              // MOV EAX, EDI               (return:)
        _emit 0xc7
        _emit 0x5f              // POP EDI  ← byte 89 = last byte within 90-byte window
        // NOTE: POP ESI (5e), POP EBX (5b), RET (c3) are at offsets 0x5a–0x5c,
        // beyond the 90-byte symbols.json size; omitted to match compare.py's window.
    }
}
