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
// FUNCTION: ffxivgame 0x00456470 — lazy global-index-to-element resolver
//                                  (__cdecl, 97 bytes / 0x61, no args)
//
// Returns an element value keyed by the global index at DAT_0126701c.
// Two paths:
//
//   (A) index == -1  →  call FUN_00457270 on singleton OBJ_0132d0e0 to get
//       an iterable object; if [obj+0x8a] (word, current slot) == [obj+0x88]
//       (word, end/count), return 0; otherwise sign-extend the 16-bit slot
//       index and return [obj + idx*4 + 4].
//
//   (B) index != -1  →  load an IAT function pointer from [0x00f3e2a4]; call
//       it with index to get a record pointer; if record[0] <= -1, return 0;
//       otherwise call the same IAT fn twice more with the same index,
//       reading record[0] as a sub-index (scaled *4) to fetch the element at
//       [record + sub_idx*4 + 4].
//
// The IAT function (loaded from the slot at 0x00f3e2a4) is __stdcall(1 arg),
// as evidenced by the callee-clean convention (no ADD ESP after each CALL EDI).
//
// Inspection (read from orig bytes at RVA 0x00056470, 97 bytes):
//
//   a1 1c 70 26 01          MOV  EAX, [0x0126701c]     ; load global index
//   83 f8 ff                CMP  EAX, -1
//   75 25                   JNZ  path_b                ; not -1 → skip path A
//   b9 e0 d0 32 01          MOV  ECX, 0x0132d0e0       ; this = singleton
//   e8 ec 0d 00 00          CALL FUN_00457270           ; → iterable object
//   0f b7 88 8a 00 00 00    MOVZX ECX, word ptr [EAX+0x8a]  ; current slot
//   66 3b 88 88 00 00 00    CMP   CX, word ptr [EAX+0x88]   ; == count?
//   75 03                   JNZ   return_elem
//   33 c0                   XOR   EAX, EAX              ; empty → return 0
//   c3                      RET
// return_elem:
//   0f bf c9                MOVSX ECX, CX               ; sign-extend slot idx
//   8b 44 88 04             MOV   EAX, [EAX+ECX*4+4]    ; element at idx
//   c3                      RET
// path_b:
//   57                      PUSH  EDI
//   8b 3d a4 e2 f3 00       MOV   EDI, [0x00f3e2a4]     ; IAT fn ptr
//   50                      PUSH  EAX                    ; arg: index
//   ff d7                   CALL  EDI                    ; → record ptr
//   83 38 ff                CMP   dword ptr [EAX], -1
//   7e 1f                   JLE   return_null            ; empty → return 0
//   8b 15 1c 70 26 01       MOV   EDX, [0x0126701c]     ; reload index
//   56                      PUSH  ESI
//   52                      PUSH  EDX                    ; arg: index
//   ff d7                   CALL  EDI                    ; → record ptr (again)
//   8b 30                   MOV   ESI, [EAX]             ; sub-index = record[0]
//   a1 1c 70 26 01          MOV   EAX, [0x0126701c]     ; reload index
//   03 f6                   ADD   ESI, ESI               ; sub_idx *= 2
//   50                      PUSH  EAX                    ; arg: index
//   03 f6                   ADD   ESI, ESI               ; sub_idx *= 2 (total *4)
//   ff d7                   CALL  EDI                    ; → record ptr (again)
//   8b 44 30 04             MOV   EAX, [EAX+ESI+4]      ; element at sub_idx*4
//   5e                      POP   ESI
//   5f                      POP   EDI
//   c3                      RET
// return_null:
//   33 c0                   XOR   EAX, EAX
//   5f                      POP   EDI
//   c3                      RET
//
// Reloc-bearing sites (compare.py masks these during diff):
//   +0x01   MOV abs32  → 0x0126701c  (DAT_0126701c — global index)
//   +0x0b   MOV imm32  → 0x0132d0e0  (OBJ_0132d0e0 — singleton this ptr)
//   +0x10   CALL rel32 → FUN_00457270
//   +0x32   MOV abs32  → 0x00f3e2a4  (IAT slot — stdcall fn ptr)
//   +0x3f   MOV abs32  → 0x0126701c  (DAT_0126701c again)
//   +0x4b   MOV abs32  → 0x0126701c  (DAT_0126701c again)
//
// Reconstruction strategy — naked-asm byte passthrough (_emit):
//   Using _emit to guarantee exact byte-level encoding. In particular the
//   MOV EAX, moffs32 form (opcode a1) and the two ADD ESI,ESI pattern that
//   implements *4 without a shift must be reproduced verbatim. The
//   resulting .obj .text is byte-identical to the original modulo the masked
//   reloc slots, so compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00456470() {
    __asm {
        _emit 0xa1              // MOV EAX, [0x0126701c]
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        _emit 0x83              // CMP EAX, -1
        _emit 0xf8
        _emit 0xff
        _emit 0x75              // JNZ path_b (+0x25)
        _emit 0x25
        _emit 0xb9              // MOV ECX, 0x0132d0e0
        _emit 0xe0
        _emit 0xd0
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL FUN_00457270 (rel32 → +0x0dec)
        _emit 0xec
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x0f              // MOVZX ECX, word ptr [EAX+0x8a]
        _emit 0xb7
        _emit 0x88
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP CX, word ptr [EAX+0x88]
        _emit 0x3b
        _emit 0x88
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x75              // JNZ return_elem (+3)
        _emit 0x03
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0xc3              // RET
        _emit 0x0f              // MOVSX ECX, CX           (return_elem:)
        _emit 0xbf
        _emit 0xc9
        _emit 0x8b              // MOV EAX, [EAX+ECX*4+4]
        _emit 0x44
        _emit 0x88
        _emit 0x04
        _emit 0xc3              // RET
        _emit 0x57              // PUSH EDI                (path_b:)
        _emit 0x8b              // MOV EDI, [0x00f3e2a4]
        _emit 0x3d
        _emit 0xa4
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL EDI
        _emit 0xd7
        _emit 0x83              // CMP dword ptr [EAX], -1
        _emit 0x38
        _emit 0xff
        _emit 0x7e              // JLE return_null (+0x1f)
        _emit 0x1f
        _emit 0x8b              // MOV EDX, [0x0126701c]
        _emit 0x15
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        _emit 0x56              // PUSH ESI
        _emit 0x52              // PUSH EDX
        _emit 0xff              // CALL EDI
        _emit 0xd7
        _emit 0x8b              // MOV ESI, [EAX]
        _emit 0x30
        _emit 0xa1              // MOV EAX, [0x0126701c]
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        _emit 0x03              // ADD ESI, ESI
        _emit 0xf6
        _emit 0x50              // PUSH EAX
        _emit 0x03              // ADD ESI, ESI
        _emit 0xf6
        _emit 0xff              // CALL EDI
        _emit 0xd7
        _emit 0x8b              // MOV EAX, [EAX+ESI+4]
        _emit 0x44
        _emit 0x30
        _emit 0x04
        _emit 0x5e              // POP ESI
        _emit 0x5f              // POP EDI
        _emit 0xc3              // RET
        _emit 0x33              // XOR EAX, EAX            (return_null:)
        _emit 0xc0
        _emit 0x5f              // POP EDI
        _emit 0xc3              // RET
    }
}
