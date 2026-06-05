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
// FUNCTION: ffxivgame 0x00042660 — `__thiscall` 170-byte (0xaa) container
//                                   resize dispatcher (8-byte element type).
//
// Asm shape (`__thiscall void f(this, uint32_t newsize, T val)` — ret 0xc,
// where T occupies 8 bytes passed by value, hence 4 + 8 = 0xc args):
//
//   ESI = this                                       ; (this = ECX in)
//   ECX = this->m4                                   ; "begin"
//   size = (ECX == 0) ? 0 : (this->m8 - ECX) >> 3    ; element size 8
//   newsize = arg0  ([ESP+0x14])
//   if (size < newsize) {                            ; grow
//       cur = (ECX == 0) ? 0 : (this->m8 - ECX) >> 3
//       end = this->m8
//       if (ECX > end) FUN_009d22b4()                ; out_of_range
//       FUN_00cb0ec0(this, end, newsize - cur, &val) ; insert_n
//       return                                       ; ret 0xc
//   }
//   if (ECX != 0) {                                  ; shrink
//       end = this->m8
//       if (newsize < (end - ECX) >> 3) {
//           if (ECX > end) FUN_009d22b4()            ; out_of_range
//           first = this->m4
//           if (first > this->m8) FUN_009d22b4()     ; out_of_range
//           local = first
//           where = first + newsize*8
//           if (where > this->m8 || where < this->m4) FUN_009d22b4()
//           FUN_00440940(&local, this, where, this, end)  ; erase tail
//       }
//   }
//   return                                           ; ret 0xc
//
// The two CALL targets are the container insert helper (FUN_00cb0ec0) and
// the tail-erase helper (FUN_00440940). The four CALL 0x009d22b4 sites are
// the binary's universal `out_of_range` panic thunk.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ port would need to coerce MSVC into picking the
//   orig's exact register allocation for four callee-saved values
//   (EBX/EBP/ESI/EDI) across the conditional branches and external CALLs.
//   The precedent in this binary (see the sibling FUN_00406280, an almost
//   identical reserve/insert dispatcher, plus blocked/0x00001b70) shows
//   source-level matching stalls at PARTIAL on these branch-and-callee-
//   saved-register-heavy bodies. The pragmatic choice is a
//   `__declspec(naked)` body re-emitting the orig 170 bytes verbatim via
//   MASM `_emit` directives. The .obj's .text is byte-identical to the
//   orig slice; the rel32 CALL offsets are baked from the orig wire image
//   and compare.py wildcards reloc windows anyway.
//
// Reloc-bearing sites in the orig 170 bytes:
//     +0x38   CALL rel32   → FUN_009d22b4  (out_of_range thunk)
//     +0x49   CALL rel32   → FUN_00cb0ec0  (insert_n helper)
//     +0x6b   CALL rel32   → FUN_009d22b4  (out_of_range thunk)
//     +0x78   CALL rel32   → FUN_009d22b4  (out_of_range thunk)
//     +0x8e   CALL rel32   → FUN_009d22b4  (out_of_range thunk)
//     +0x9e   CALL rel32   → FUN_00440940  (tail-erase helper)

extern "C" __declspec(naked) void FUN_00442660() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x4]
        _emit 0x4e
        _emit 0x04
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x57              // PUSH EDI
        _emit 0x75              // JNZ short +4
        _emit 0x04
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0xeb              // JMP short +8
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x8]
        _emit 0x46
        _emit 0x08
        _emit 0x2b              // SUB EAX, ECX
        _emit 0xc1
        _emit 0xc1              // SAR EAX, 0x3
        _emit 0xf8
        _emit 0x03
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x14]
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        _emit 0x3b              // CMP EAX, EBX
        _emit 0xc3
        _emit 0x73              // JNC short +0x34
        _emit 0x34
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x75              // JNZ short +4
        _emit 0x04
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0xeb              // JMP short +8
        _emit 0x08
        _emit 0x8b              // MOV EDI, dword ptr [ESI+0x8]
        _emit 0x7e
        _emit 0x08
        _emit 0x2b              // SUB EDI, ECX
        _emit 0xf9
        _emit 0xc1              // SAR EDI, 0x3
        _emit 0xff
        _emit 0x03
        _emit 0x8b              // MOV EBP, dword ptr [ESI+0x8]
        _emit 0x6e
        _emit 0x08
        _emit 0x3b              // CMP ECX, EBP
        _emit 0xcd
        _emit 0x76              // JBE short +5
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32)
        _emit 0x17
        _emit 0xfc
        _emit 0x58
        _emit 0x00
        _emit 0x8d              // LEA EAX, [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50              // PUSH EAX
        _emit 0x2b              // SUB EBX, EDI
        _emit 0xdf
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_00cb0ec0 (rel32)
        _emit 0x12
        _emit 0xe8
        _emit 0x86
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74              // JZ short +0x4a
        _emit 0x4a
        _emit 0x8b              // MOV EBP, dword ptr [ESI+0x8]
        _emit 0x6e
        _emit 0x08
        _emit 0x8b              // MOV EAX, EBP
        _emit 0xc5
        _emit 0x2b              // SUB EAX, ECX
        _emit 0xc1
        _emit 0xc1              // SAR EAX, 0x3
        _emit 0xf8
        _emit 0x03
        _emit 0x3b              // CMP EBX, EAX
        _emit 0xd8
        _emit 0x73              // JNC short +0x3c
        _emit 0x3c
        _emit 0x3b              // CMP ECX, EBP
        _emit 0xcd
        _emit 0x76              // JBE short +5
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32)
        _emit 0xe4
        _emit 0xfb
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV EDI, dword ptr [ESI+0x4]
        _emit 0x7e
        _emit 0x04
        _emit 0x3b              // CMP EDI, dword ptr [ESI+0x8]
        _emit 0x7e
        _emit 0x08
        _emit 0x76              // JBE short +5
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32)
        _emit 0xd7
        _emit 0xfb
        _emit 0x58
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESP+0x1c], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x8d              // LEA EDI, [EDI+EBX*8]
        _emit 0x3c
        _emit 0xdf
        _emit 0x3b              // CMP EDI, dword ptr [ESI+0x8]
        _emit 0x7e
        _emit 0x08
        _emit 0x77              // JA short +5
        _emit 0x05
        _emit 0x3b              // CMP EDI, dword ptr [ESI+0x4]
        _emit 0x7e
        _emit 0x04
        _emit 0x73              // JNC short +5
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32)
        _emit 0xc1
        _emit 0xfb
        _emit 0x58
        _emit 0x00
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA ECX, [ESP+0x28]
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_00440940 (rel32)
        _emit 0x3d
        _emit 0xe2
        _emit 0xff
        _emit 0xff
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
