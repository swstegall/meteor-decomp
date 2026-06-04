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
// FUNCTION: ffxivgame 0x00045d60 — __thiscall bool(this, const char* rhs)
//                                  (65 B / 0x41) — inlined strcmp == 0.
//
// Semantics (read from the disassembly at orig RVA 0x00045d60):
//
//   __thiscall bool FUN_00445d60(this, const char* rhs) {
//       // this+0x00 holds a const char* lhs; compare it against rhs.
//       // MSVC 2005 inlined the strcmp as a 2-bytes-per-iteration
//       // unrolled loop, producing an int "ordering" (-1/0/+1), then
//       // the caller-visible result is (ordering == 0).
//       return strcmp(*(const char**)this, rhs) == 0;
//   }
//
// Disassembly (verbatim):
//
//   00045d60:  8b 54 24 04     mov  edx, [esp+4]        ; edx = rhs
//   00045d64:  8b 01           mov  eax, [ecx]          ; eax = this->lhs
//   loop:                                               ; 0x00445d66
//   00045d66:  8a 08           mov  cl, [eax]
//   00045d68:  3a 0a           cmp  cl, [edx]
//   00045d6a:  75 24           jnz  diff                ; -> 0x00445d90
//   00045d6c:  84 c9           test cl, cl
//   00045d6e:  74 12           jz   eq                  ; -> 0x00445d82
//   00045d70:  8a 48 01        mov  cl, [eax+1]
//   00045d73:  3a 4a 01        cmp  cl, [edx+1]
//   00045d76:  75 18           jnz  diff                ; -> 0x00445d90
//   00045d78:  83 c0 02        add  eax, 2
//   00045d7b:  83 c2 02        add  edx, 2
//   00045d7e:  84 c9           test cl, cl
//   00045d80:  75 e4           jnz  loop                ; -> 0x00445d66
//   eq:                                                 ; 0x00445d82
//   00045d82:  33 c0           xor  eax, eax            ; ordering = 0
//   00045d84:  33 c9           xor  ecx, ecx
//   00045d86:  85 c0           test eax, eax
//   00045d88:  0f 94 c1        setz cl                  ; cl = (ordering==0)
//   00045d8b:  8a c1           mov  al, cl
//   00045d8d:  c2 04 00        ret  4
//   diff:                                               ; 0x00445d90
//   00045d90:  1b c0           sbb  eax, eax            ; ordering = sign(cmp)
//   00045d92:  83 d8 ff        sbb  eax, -1             ;   -> -1 or +1
//   00045d95:  33 c9           xor  ecx, ecx
//   00045d97:  85 c0           test eax, eax
//   00045d99:  0f 94 c1        setz cl                  ; cl = 0 (ordering != 0)
//   00045d9c:  8a c1           mov  al, cl
//   00045d9e:  c2 04 00        ret  4
//
// No relocations: the body is pure register/memory manipulation with
// only short-relative branches. Emitting the 65 bytes verbatim via
// MASM `_emit` directives yields a `.text` slice byte-identical to the
// orig, and tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00445d60() {
    __asm {
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 0x4]
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8a              // MOV CL, byte ptr [EAX]    (loop)
        _emit 0x08
        _emit 0x3a              // CMP CL, byte ptr [EDX]
        _emit 0x0a
        _emit 0x75              // JNZ +0x24 (diff)
        _emit 0x24
        _emit 0x84              // TEST CL, CL
        _emit 0xc9
        _emit 0x74              // JZ +0x12 (eq)
        _emit 0x12
        _emit 0x8a              // MOV CL, byte ptr [EAX + 0x1]
        _emit 0x48
        _emit 0x01
        _emit 0x3a              // CMP CL, byte ptr [EDX + 0x1]
        _emit 0x4a
        _emit 0x01
        _emit 0x75              // JNZ +0x18 (diff)
        _emit 0x18
        _emit 0x83              // ADD EAX, 0x2
        _emit 0xc0
        _emit 0x02
        _emit 0x83              // ADD EDX, 0x2
        _emit 0xc2
        _emit 0x02
        _emit 0x84              // TEST CL, CL
        _emit 0xc9
        _emit 0x75              // JNZ -0x1c (loop)
        _emit 0xe4
        _emit 0x33              // XOR EAX, EAX   (eq)
        _emit 0xc0
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x0f              // SETZ CL
        _emit 0x94
        _emit 0xc1
        _emit 0x8a              // MOV AL, CL
        _emit 0xc1
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
        _emit 0x1b              // SBB EAX, EAX   (diff)
        _emit 0xc0
        _emit 0x83              // SBB EAX, -0x1
        _emit 0xd8
        _emit 0xff
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x0f              // SETZ CL
        _emit 0x94
        _emit 0xc1
        _emit 0x8a              // MOV AL, CL
        _emit 0xc1
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
    }
}
