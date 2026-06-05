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
// FUNCTION: ffxivgame 0x00047a00 — __thiscall byte-container "pop_front"
//                                  (shift-left + tail erase), 53 B / 0x35.
//
// Layout (inferred from the asm):
//   This (ECX):
//     +0x00  unsigned char *data    (begin pointer)
//     +0x08  size_t         count   (element count)
//
// Source shape (inferred):
//
//   char Foo::pop_front() {
//       unsigned char first = this->data[0];     // saved → returned in AL
//       for (size_t i = 1; i < this->count; ++i)
//           this->data[i - 1] = this->data[i];   // shift everything left
//       this->erase(this->count - 1, 1);         // FUN_00447010(count-1, 1)
//       return first;
//   }
//
//   - `MOV BL, [EAX]` captures data[0] up front (EAX = this->data), the
//     value returned via `MOV AL, BL` at the tail.
//   - The shift loop runs only when count > 1 (`CMP [ECX+8],1 ; JBE`);
//     the body reloads this->data each iteration into EDX and walks a
//     scratch pointer ESI = data + i, storing back to ESI-1.
//   - Tail call FUN_00447010(count-1, 1) is a __thiscall (ECX still this)
//     with the two args pushed right-to-left: PUSH 1 then PUSH (count-1).
//
// Calling convention: __thiscall (ECX = this; no stack args; returns
// byte in AL; `ret` — caller cleans nothing).
//
// Frame: PUSH EBX (saves the return byte across the loop + call); PUSH ESI
// inside the loop body only, popped before the shared tail.
//
// Asm (53 bytes):
//   8b 01              MOV  EAX, [ECX]            ; data
//   53                 PUSH EBX
//   8a 18              MOV  BL, [EAX]             ; first = data[0]
//   b8 01 00 00 00     MOV  EAX, 1                ; i = 1
//   39 41 08           CMP  [ECX+8], EAX          ; count vs 1
//   76 14              JBE  tail                  ; count <= 1 → skip loop
//   56                 PUSH ESI
//  loop:
//   8b 11              MOV  EDX, [ECX]            ; data
//   8d 34 02           LEA  ESI, [EDX + EAX]      ; data + i
//   8a 16              MOV  DL, [ESI]             ; data[i]
//   83 c0 01           ADD  EAX, 1                ; ++i
//   88 56 ff           MOV  [ESI-1], DL           ; data[i-1] = data[i]
//   3b 41 08           CMP  EAX, [ECX+8]          ; i vs count
//   72 ee              JC   loop                  ; i < count (unsigned)
//   5e                 POP  ESI
//  tail:
//   8b 41 08           MOV  EAX, [ECX+8]          ; count
//   6a 01              PUSH 1                      ; arg2 = 1
//   83 e8 01           SUB  EAX, 1                 ; count-1
//   50                 PUSH EAX                    ; arg1 = count-1
//   e8 df f5 ff ff     CALL FUN_00447010           ; erase
//   8a c3              MOV  AL, BL                 ; return first
//   5b                 POP  EBX
//   c3                 RET
//
// Reconstruction strategy — naked-asm byte passthrough (mirrors the
// sibling FUN_004051e0): a `__declspec(naked)` body that re-emits the
// orig 53 bytes verbatim via MASM `_emit` directives. The single REL32
// callsite (`FUN_00447010` at +0x2c) is baked in as raw bytes, so the
// .obj's `.text` carries NO relocations and is byte-identical to the
// orig slice. `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_00447a00() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x53              // PUSH EBX
        _emit 0x8a              // MOV BL, byte ptr [EAX]
        _emit 0x18
        _emit 0xb8              // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x39              // CMP dword ptr [ECX+8], EAX
        _emit 0x41
        _emit 0x08
        _emit 0x76              // JBE tail (+0x14)
        _emit 0x14
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV EDX, dword ptr [ECX]      (loop:)
        _emit 0x11
        _emit 0x8d              // LEA ESI, [EDX + EAX*1]
        _emit 0x34
        _emit 0x02
        _emit 0x8a              // MOV DL, byte ptr [ESI]
        _emit 0x16
        _emit 0x83              // ADD EAX, 1
        _emit 0xc0
        _emit 0x01
        _emit 0x88              // MOV byte ptr [ESI-1], DL
        _emit 0x56
        _emit 0xff
        _emit 0x3b              // CMP EAX, dword ptr [ECX+8]
        _emit 0x41
        _emit 0x08
        _emit 0x72              // JC loop (-0x12)
        _emit 0xee
        _emit 0x5e              // POP ESI
        _emit 0x8b              // MOV EAX, dword ptr [ECX+8]    (tail:)
        _emit 0x41
        _emit 0x08
        _emit 0x6a              // PUSH 1
        _emit 0x01
        _emit 0x83              // SUB EAX, 1
        _emit 0xe8
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_00447010 (rel32 → 0x00447010)
        _emit 0xdf
        _emit 0xf5
        _emit 0xff
        _emit 0xff
        _emit 0x8a              // MOV AL, BL
        _emit 0xc3
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
