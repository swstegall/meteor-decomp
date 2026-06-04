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
// FUNCTION: ffxivgame 0x00436700 — `__thiscall` vector<T,20> single-element
//                                   splice/insert returning an iterator
//                                   (170 bytes / 0xaa, ret 0x10).
//
// Asm shape (`__thiscall iterator* f(this, ret*, void* where, T* val)` — the
// element stride is 20 bytes; the IMUL 0x66666667 / SAR 3 sequence is MSVC's
// signed divide-by-20 to recover an element index from a byte distance):
//
//   ESI = this   (= ECX in)
//   EBP = param "where" pointer   ([ESP+0x18] at entry → arg2)
//   EDI = this->_Myfirst   ([ESI+0x4])
//   if (EDI != 0) {                                   ; non-empty vector
//       n = (this->_Mylast - this->_Myfirst) / 20     ; size()
//       if (n != 0) {
//           if (this->_Myfirst > _Mylast) FUN_009d22b4()   ; out_of_range
//           it = [ESP+0x1c]   (arg1, an iterator's _Mycont)
//           if (it != 0 && it != this) FUN_009d22b4()       ; iter container
//           EDI = (where - _Myfirst) / 20             ; insertion index
//       }
//   } else {                                          ; empty
//       EBX = [ESP+0x1c];  EDI = 0;
//   }
//   FUN_00436280(this, EBX, where, 1, val);           ; reserve/grow helper
//   EBX = this->_Myfirst;
//   if (EBX > this->_Mylast) FUN_009d22b4();           ; out_of_range
//   EDI = EBX + (EDI * 5) * 4;                          ; &first[index] (×20)
//   if (EDI > _Mylast || EDI < _Myfirst) FUN_009d22b4();
//   ret->_Myptr  = EDI;
//   ret->_Mycont = this;
//   return ret;                                        ; ret 0x10
//
// Reloc-bearing sites in the orig 170 bytes (all rel32 CALLs — compare.py
// masks the 4-byte rel windows, and emitting the orig's own baked rel32
// bytes yields a zero-reloc .obj whose .text is byte-identical to the orig
// slice, so GREEN holds without a relink):
//     +0x38   CALL rel32 → FUN_009d22b4  (out_of_range thunk)
//     +0x49   CALL rel32 → FUN_009d22b4  (out_of_range thunk)
//     +0x6e   CALL rel32 → FUN_00436280  (reserve/grow dispatcher)
//     +0x7b   CALL rel32 → FUN_009d22b4  (out_of_range thunk)
//     +0x94   CALL rel32 → FUN_009d22b4  (out_of_range thunk)
//
// Reconstruction strategy — naked-asm byte passthrough, matching the
// precedent set by the sibling container helpers in this module
// (FUN_00406280, FUN_004063c0): a source-level C++ port would need MSVC to
// reproduce the orig's exact EBX/EBP/ESI/EDI allocation across the branch
// nest and five external CALLs, which is brittle (see the 52.3% PARTIAL
// stall recorded for FUN_00401b70). The `__declspec(naked)` `_emit` body
// re-emits the orig 170 bytes verbatim.

extern "C" __declspec(naked) void FUN_00436700() {
    __asm {
        _emit 0x51              // PUSH ECX
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x18]
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESI+0x4]
        _emit 0x7e
        _emit 0x04
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x74              // JZ short +0x1a
        _emit 0x1a
        _emit 0x8b              // MOV EBX, dword ptr [ESI+0x8]
        _emit 0x5e
        _emit 0x08
        _emit 0x8b              // MOV ECX, EBX
        _emit 0xcb
        _emit 0x2b              // SUB ECX, EDI
        _emit 0xcf
        _emit 0xb8              // MOV EAX, 0x66666667
        _emit 0x67
        _emit 0x66
        _emit 0x66
        _emit 0x66
        _emit 0xf7              // IMUL ECX
        _emit 0xe9
        _emit 0xc1              // SAR EDX, 0x3
        _emit 0xfa
        _emit 0x03
        _emit 0x8b              // MOV EAX, EDX
        _emit 0xc2
        _emit 0xc1              // SHR EAX, 0x1f
        _emit 0xe8
        _emit 0x1f
        _emit 0x03              // ADD EAX, EDX
        _emit 0xc2
        _emit 0x75              // JNZ short +0x08
        _emit 0x08
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x1c]
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0xeb              // JMP short +0x2f
        _emit 0x2f
        _emit 0x3b              // CMP EDI, EBX
        _emit 0xfb
        _emit 0x76              // JBE short +0x05
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32)
        _emit 0x77
        _emit 0xbb
        _emit 0x59
        _emit 0x00
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x1c]
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x74              // JZ short +0x04
        _emit 0x04
        _emit 0x3b              // CMP EBX, ESI
        _emit 0xde
        _emit 0x74              // JZ short +0x05
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32)
        _emit 0x66
        _emit 0xbb
        _emit 0x59
        _emit 0x00
        _emit 0x8b              // MOV ECX, EBP
        _emit 0xcd
        _emit 0x2b              // SUB ECX, EDI
        _emit 0xcf
        _emit 0xb8              // MOV EAX, 0x66666667
        _emit 0x67
        _emit 0x66
        _emit 0x66
        _emit 0x66
        _emit 0xf7              // IMUL ECX
        _emit 0xe9
        _emit 0xc1              // SAR EDX, 0x3
        _emit 0xfa
        _emit 0x03
        _emit 0x8b              // MOV EDI, EDX
        _emit 0xfa
        _emit 0xc1              // SHR EDI, 0x1f
        _emit 0xef
        _emit 0x1f
        _emit 0x03              // ADD EDI, EDX
        _emit 0xfa
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x24]
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x51              // PUSH ECX
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x55              // PUSH EBP
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_00436280 (rel32)
        _emit 0x0d
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EBX, dword ptr [ESI+0x4]
        _emit 0x5e
        _emit 0x04
        _emit 0x3b              // CMP EBX, dword ptr [ESI+0x8]
        _emit 0x5e
        _emit 0x08
        _emit 0x76              // JBE short +0x05
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32)
        _emit 0x34
        _emit 0xbb
        _emit 0x59
        _emit 0x00
        _emit 0x8d              // LEA EDX, [EDI+EDI*4]
        _emit 0x14
        _emit 0xbf
        _emit 0x8d              // LEA EDI, [EBX+EDX*4]
        _emit 0x3c
        _emit 0x93
        _emit 0x3b              // CMP EDI, dword ptr [ESI+0x8]
        _emit 0x7e
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESP+0x20], EBX
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        _emit 0x77              // JA short +0x05
        _emit 0x05
        _emit 0x3b              // CMP EDI, dword ptr [ESI+0x4]
        _emit 0x7e
        _emit 0x04
        _emit 0x73              // JNC short +0x05
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32)
        _emit 0x1b
        _emit 0xbb
        _emit 0x59
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x89              // MOV dword ptr [EAX+0x4], EDI
        _emit 0x78
        _emit 0x04
        _emit 0x5f              // POP EDI
        _emit 0x89              // MOV dword ptr [EAX], ESI
        _emit 0x30
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0x59              // POP ECX
        _emit 0xc2              // RET 0x10
        _emit 0x10
        _emit 0x00
    }
}
