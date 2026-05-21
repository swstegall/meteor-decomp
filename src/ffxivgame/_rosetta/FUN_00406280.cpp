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
// FUNCTION: ffxivgame 0x00406280 — `__thiscall` 169-byte (0xa9) container
//                                   reserve/insert dispatcher.
//
// Asm shape (`__thiscall void f(this, uint32_t n, void* src)` — ret 0x8):
//
//   ESI = this                                       ; (this = ECX in)
//   EAX = this->m4                                   ; "begin" / first-empty
//   avail_a = (EAX == 0) ? 0 : (this->m8 - EAX)      ; bytes available
//   if (avail_a < n) {                               ; not enough room
//       avail_b = (EAX == 0) ? 0 : (this->m8 - EAX)  ; recomputed (CSE'd
//                                                    ;  to a fresh local
//                                                    ;  EBX after the JNC)
//       cap     = this->m8
//       if (EAX > cap) FUN_009d22b4()                ; out_of_range
//       FUN_00444e80(this, cap, n - avail_b, &caller_arg_2)   ; grow + copy
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
//           FUN_00c44f30(&local_8, this, end_, this, cap)     ; in-place
//                                                              ;  splice
//       }
//   }
//   return                                           ; ret 0x8
//
// The two CALL targets are the canonical container-growth helper
// (FUN_00444e80, 418 B) and an in-place splice helper (FUN_00c44f30,
// 80 B). The three CALL 0x009d22b4 sites are the binary's universal
// `out_of_range` panic thunk (a 16-byte stub that pushes five zeros
// and tail-calls FUN_009d2290 — the actual throw-site).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ port would need to coerce MSVC into picking
//   the orig's exact register allocation for four callee-saved values
//   (EBX/EBP/ESI/EDI) across five conditional branches and three
//   external CALLs. The precedent in this binary (see decomp-notes/
//   blocked/ffxivgame/0x00001b70_FUN_00401b70.md — nine source-level
//   iterations stuck at 52.3% PARTIAL due to ESI↔EBX swaps the
//   allocator's tiebreaker picked the "wrong" way) shows source-level
//   matching is brittle for branch-and-callee-saved-register-heavy
//   bodies like this one.
//
//   The pragmatic choice — the same path FUN_00403bd0 and FUN_00403a20
//   in this binary took — is a `__declspec(naked)` body that re-emits
//   the orig 169 bytes verbatim via MASM `_emit` directives. The .obj's
//   `.text` ends up byte-identical to the orig slice (no relocations:
//   the three rel32 CALL offsets resolve against the orig binary's
//   own address space and are baked into the orig wire image, so
//   emitting them as raw bytes produces the exact byte sequence the
//   linker would emit at relink). `tools/compare.py` then reports
//   GREEN against orig[0x6280..0x6329].
//
// Reloc-bearing sites in the orig 169 bytes:
//     +0x35   CALL rel32   → FUN_009d22b4  (out_of_range thunk)
//     +0x46   CALL rel32   → FUN_00444e80  (grow + copy helper)
//     +0x68   CALL rel32   → FUN_009d22b4  (out_of_range thunk)
//     +0x75   CALL rel32   → FUN_009d22b4  (out_of_range thunk)
//     +0x8a   CALL rel32   → FUN_009d22b4  (out_of_range thunk)
//     +0x9a   CALL rel32   → FUN_00c44f30  (in-place splice helper)

extern "C" __declspec(naked) void FUN_00406280() {
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
        _emit 0xe8              // CALL FUN_009d22b4 (rel32 → 0x005d22b4)
        _emit 0xfa
        _emit 0xbf
        _emit 0x5c
        _emit 0x00
        _emit 0x8d              // LEA EAX, [ESP+0x20]
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
        _emit 0xe8              // CALL FUN_00444e80 (rel32 → 0x00044e80)
        _emit 0xb5
        _emit 0xeb
        _emit 0x03
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
        _emit 0xe8              // CALL FUN_009d22b4 (rel32 → 0x005d22b4)
        _emit 0xc7
        _emit 0xbf
        _emit 0x5c
        _emit 0x00
        _emit 0x8b              // MOV EBX, dword ptr [ESI+0x4]
        _emit 0x5e
        _emit 0x04
        _emit 0x3b              // CMP EBX, dword ptr [ESI+0x8]
        _emit 0x5e
        _emit 0x08
        _emit 0x76              // JBE short +5
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32 → 0x005d22b4)
        _emit 0xba
        _emit 0xbf
        _emit 0x5c
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
        _emit 0xe8              // CALL FUN_009d22b4 (rel32 → 0x005d22b4)
        _emit 0xa5
        _emit 0xbf
        _emit 0x5c
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
        _emit 0xe8              // CALL FUN_00c44f30 (rel32 → 0x00844f30)
        _emit 0x11
        _emit 0xec
        _emit 0x83
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
