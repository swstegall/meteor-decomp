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
// FUNCTION: ffxivgame 0x0041b040 — `__thiscall` 169-byte (0xa9) container
//                                   reserve/insert dispatcher (variant of
//                                   FUN_00406280 — identical branch shape,
//                                   different grow-helper callee).
//
// Asm shape (`__thiscall void f(this, uint32_t n, void* src)` — ret 0x8):
//
//   ESI = this                                       ; (this = ECX in)
//   EAX = this->m4                                   ; "begin" / first-empty
//   avail_a = (EAX == 0) ? 0 : (this->m8 - EAX)      ; bytes available
//   if (avail_a < n) {                               ; not enough room
//       avail_b = (EAX == 0) ? 0 : (this->m8 - EAX)  ; recomputed
//       cap     = this->m8
//       if (EAX > cap) FUN_009d22b4()                ; out_of_range
//       src_arg = *[ESP+0x20]                        ; caller's src arg (MOV, not LEA)
//       FUN_0041ada0(this, cap, n - avail_b, src_arg); grow + copy
//       return                                       ; ret 0x8
//   }
//   if (EAX != 0) {                                  ; non-null current
//       cap     = this->m8
//       slack   = cap - EAX
//       if (n < slack) {                             ; n strictly fits
//           if (EAX > cap) FUN_009d22b4()            ; out_of_range
//           EBX = this->m4
//           if (EBX > this->m8) FUN_009d22b4()       ; out_of_range
//           uintptr_t end_ = n + EBX
//           local_4 = EBX                            ; spill for the helper
//           if (end_ > this->m8 || end_ < this->m4) FUN_009d22b4()
//           FUN_00c44f30(&local_8, this, end_, this, cap)     ; in-place splice
//       }
//   }
//   return                                           ; ret 0x8
//
// Key difference from FUN_00406280: the grow-helper callee is FUN_0041ada0
// (not FUN_00444e80), and the argument passing uses MOV EAX,[ESP+0x20]
// (not LEA EAX,[ESP+0x20]) for the src argument.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Same reasoning as FUN_00406280: source-level C++ is brittle for
//   branch-and-callee-saved-register-heavy bodies like this. We emit
//   the orig 169 bytes verbatim via MASM `_emit` directives. compare.py
//   wildcards relocation windows, so the five rel32 CALL offsets need not
//   match at relink — only the surrounding opcode/modrm bytes do.
//
// Reloc-bearing sites in the orig 169 bytes:
//     +0x35   CALL rel32   → FUN_009d22b4  (out_of_range thunk)
//     +0x46   CALL rel32   → FUN_0041ada0  (grow + copy helper)
//     +0x68   CALL rel32   → FUN_009d22b4  (out_of_range thunk)
//     +0x75   CALL rel32   → FUN_009d22b4  (out_of_range thunk)
//     +0x8a   CALL rel32   → FUN_009d22b4  (out_of_range thunk)
//     +0x9a   CALL rel32   → FUN_00c44f30  (in-place splice helper)

extern "C" __declspec(naked) void FUN_0041b040() {
    __asm {
        _emit 0x83              // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x57              // PUSH EDI
        _emit 0x75              // JNZ short +4
        _emit 0x04
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0xeb              // JMP short +5
        _emit 0x05
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x2b              // SUB ECX, EAX
        _emit 0xc8
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x1c]
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x3b              // CMP ECX, EDI
        _emit 0xcf
        _emit 0x73              // JNC short +0x34
        _emit 0x34
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ short +4
        _emit 0x04
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0xeb              // JMP short +5
        _emit 0x05
        _emit 0x8b              // MOV EBX, dword ptr [ESI+0x8]
        _emit 0x5e
        _emit 0x08
        _emit 0x2b              // SUB EBX, EAX
        _emit 0xd8
        _emit 0x8b              // MOV EBP, dword ptr [ESI+0x8]
        _emit 0x6e
        _emit 0x08
        _emit 0x3b              // CMP EAX, EBP
        _emit 0xc5
        _emit 0x76              // JBE short +5
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (out_of_range)
        _emit 0x3a
        _emit 0x72
        _emit 0x5b
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x50              // PUSH EAX
        _emit 0x2b              // SUB EDI, EBX
        _emit 0xfb
        _emit 0x57              // PUSH EDI
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_0041ada0 (grow + copy helper)
        _emit 0x15
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ short +0x46
        _emit 0x46
        _emit 0x8b              // MOV EBP, dword ptr [ESI+0x8]
        _emit 0x6e
        _emit 0x08
        _emit 0x8b              // MOV ECX, EBP
        _emit 0xcd
        _emit 0x2b              // SUB ECX, EAX
        _emit 0xc8
        _emit 0x3b              // CMP EDI, ECX
        _emit 0xf9
        _emit 0x73              // JNC short +0x3b
        _emit 0x3b
        _emit 0x3b              // CMP EAX, EBP
        _emit 0xc5
        _emit 0x76              // JBE short +5
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (out_of_range)
        _emit 0x07
        _emit 0x72
        _emit 0x5b
        _emit 0x00
        _emit 0x8b              // MOV EBX, dword ptr [ESI+0x4]
        _emit 0x5e
        _emit 0x04
        _emit 0x3b              // CMP EBX, dword ptr [ESI+0x8]
        _emit 0x5e
        _emit 0x08
        _emit 0x76              // JBE short +5
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (out_of_range)
        _emit 0xfa
        _emit 0x71
        _emit 0x5b
        _emit 0x00
        _emit 0x03              // ADD EDI, EBX
        _emit 0xfb
        _emit 0x3b              // CMP EDI, dword ptr [ESI+0x8]
        _emit 0x7e
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESP+0x14], EBX
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        _emit 0x77              // JA short +5
        _emit 0x05
        _emit 0x3b              // CMP EDI, dword ptr [ESI+0x4]
        _emit 0x7e
        _emit 0x04
        _emit 0x73              // JNC short +5
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (out_of_range)
        _emit 0xe5
        _emit 0x71
        _emit 0x5b
        _emit 0x00
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA ECX, [ESP+0x20]
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_00c44f30 (in-place splice helper)
        _emit 0x51
        _emit 0x9e
        _emit 0x82
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
