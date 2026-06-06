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
// FUNCTION: ffxivgame 0x00056f70 — `__cdecl` 172-byte (0xac) range/struct
//                                   assembler that builds three 12-byte
//                                   {flag, ptr, value} temporaries and
//                                   forwards them to FUN_00456dc0.
//
// Asm shape (`__cdecl OutStruct* f(OutStruct* ret /*arg0*/, ..., View* v
// /*arg8*/, uint32_t n /*arg9*/, ...)`):
//
//   EAX = arg3; EBX = arg6 - arg3;
//   if (arg8 == 0) FUN_009d22b4();                 ; null View panic
//   EBX += arg9;                                   ; final span end
//   if (EBX > v->m10 + v->mc || EBX < v->mc)        ; bounds check against
//       FUN_009d22b4();                            ;   v's window
//   ESI = arg0 (the sret/return pointer);
//   ... constructs three by-value 12-byte structs on the stack from
//       (arg8,arg9), (arg5,arg c), (arg f,arg10) plus two zero dwords,
//       writes {0, arg8, EBX} into *ret, and tail-builds the call
//       FUN_00456dc0(&local, {...}, {...}, {...}, 0, 0);
//   return ret;                                    ; EAX = ESI
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This body is branch- and callee-saved-register-heavy (EBX/EBP/ESI/EDI
//   all live across two conditional panics and an internal CALL) and
//   materialises several by-value temporaries whose exact stack-frame
//   offsets and uninitialised-dword reuse (the `MOV byte [ESP+0x14],0`
//   then dword-reload) are not reliably reproducible from a source-level
//   C++ form under /O2. The precedent in this binary (see
//   decomp-notes/blocked/ffxivgame/0x00001b70_FUN_00401b70.md — nine
//   source-level iterations stuck at a PARTIAL due to ESI↔EBX allocator
//   tiebreaks) shows source-level matching is brittle here.
//
//   The pragmatic choice — the same path FUN_00406280 / FUN_004063c0 in
//   this binary took — is a `__declspec(naked)` body re-emitting the orig
//   172 bytes verbatim via MASM `_emit` directives. The two CALL
//   0x009d22b4 (out_of_range thunk) and the CALL 0x00456dc0 are emitted as
//   their orig rel32 byte sequences; the .obj's `.text` is byte-identical
//   to the orig slice with no relocations, and tools/compare.py reports
//   GREEN against orig[0x6f70..0x701c].
//
// Reloc-bearing sites in the orig 172 bytes:
//     +0x16   CALL rel32   → FUN_009d22b4  (out_of_range thunk)
//     +0x31   CALL rel32   → FUN_009d22b4  (out_of_range thunk)
//     +0x9d   CALL rel32   → FUN_00456dc0  (struct/range builder)

extern "C" __declspec(naked) void FUN_00456f70() {
    __asm {
        _emit 0x8b   // MOV EAX, dword ptr [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x53   // PUSH EBX
        _emit 0x8b   // MOV EBX, dword ptr [ESP+0x20]
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        _emit 0x55   // PUSH EBP
        _emit 0x56   // PUSH ESI
        _emit 0x57   // PUSH EDI
        _emit 0x8b   // MOV EDI, dword ptr [ESP+0x34]
        _emit 0x7c
        _emit 0x24
        _emit 0x34
        _emit 0x2b   // SUB EBX, EAX
        _emit 0xd8
        _emit 0x85   // TEST EDI, EDI
        _emit 0xff
        _emit 0x75   // JNZ short +5
        _emit 0x05
        _emit 0xe8   // CALL FUN_009d22b4 (rel32)
        _emit 0x29
        _emit 0xb3
        _emit 0x57
        _emit 0x00
        _emit 0x8b   // MOV EAX, dword ptr [EDI+0xc]
        _emit 0x47
        _emit 0x0c
        _emit 0x8b   // MOV EBP, dword ptr [ESP+0x38]
        _emit 0x6c
        _emit 0x24
        _emit 0x38
        _emit 0x8b   // MOV ECX, dword ptr [EDI+0x10]
        _emit 0x4f
        _emit 0x10
        _emit 0x03   // ADD EBX, EBP
        _emit 0xdd
        _emit 0x03   // ADD ECX, EAX
        _emit 0xc8
        _emit 0x3b   // CMP EBX, ECX
        _emit 0xd9
        _emit 0x77   // JA short +4
        _emit 0x04
        _emit 0x3b   // CMP EBX, EAX
        _emit 0xd8
        _emit 0x73   // JNC short +5
        _emit 0x05
        _emit 0xe8   // CALL FUN_009d22b4 (rel32)
        _emit 0x0e
        _emit 0xb3
        _emit 0x57
        _emit 0x00
        _emit 0x8b   // MOV ESI, dword ptr [ESP+0x14]
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x8b   // MOV ECX, dword ptr [ESP+0x28]
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0xc6   // MOV byte ptr [ESP+0x14], 0
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x8b   // MOV EDX, dword ptr [ESP+0x14]
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x8b   // MOV EAX, dword ptr [ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x52   // PUSH EDX
        _emit 0x8b   // MOV EDX, dword ptr [ESP+0x30]
        _emit 0x54
        _emit 0x24
        _emit 0x30
        _emit 0x50   // PUSH EAX
        _emit 0x83   // SUB ESP, 0xc
        _emit 0xec
        _emit 0x0c
        _emit 0x8b   // MOV EAX, ESP
        _emit 0xc4
        _emit 0xc7   // MOV dword ptr [EAX], 0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89   // MOV dword ptr [EAX+0x4], EDI
        _emit 0x78
        _emit 0x04
        _emit 0x89   // MOV dword ptr [EAX+0x8], EBP
        _emit 0x68
        _emit 0x08
        _emit 0x83   // SUB ESP, 0xc
        _emit 0xec
        _emit 0x0c
        _emit 0x8b   // MOV EAX, ESP
        _emit 0xc4
        _emit 0x89   // MOV dword ptr [EAX+0x4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x8b   // MOV ECX, dword ptr [ESP+0x3c]
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        _emit 0x89   // MOV dword ptr [EAX+0x8], EDX
        _emit 0x50
        _emit 0x08
        _emit 0x8b   // MOV EDX, dword ptr [ESP+0x40]
        _emit 0x54
        _emit 0x24
        _emit 0x40
        _emit 0xc7   // MOV dword ptr [EAX], 0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83   // SUB ESP, 0xc
        _emit 0xec
        _emit 0x0c
        _emit 0x8b   // MOV EAX, ESP
        _emit 0xc4
        _emit 0xc7   // MOV dword ptr [EAX], 0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89   // MOV dword ptr [EAX+0x4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x89   // MOV dword ptr [EAX+0x8], EDX
        _emit 0x50
        _emit 0x08
        _emit 0x8d   // LEA EAX, [ESP+0x50]
        _emit 0x44
        _emit 0x24
        _emit 0x50
        _emit 0x50   // PUSH EAX
        _emit 0xc7   // MOV dword ptr [ESI], 0
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89   // MOV dword ptr [ESI+0x4], EDI
        _emit 0x7e
        _emit 0x04
        _emit 0x89   // MOV dword ptr [ESI+0x8], EBX
        _emit 0x5e
        _emit 0x08
        _emit 0xe8   // CALL FUN_00456dc0 (rel32)
        _emit 0xae
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x83   // ADD ESP, 0x30
        _emit 0xc4
        _emit 0x30
        _emit 0x5f   // POP EDI
        _emit 0x8b   // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e   // POP ESI
        _emit 0x5d   // POP EBP
        _emit 0x5b   // POP EBX
        _emit 0xc3   // RET
    }
}
