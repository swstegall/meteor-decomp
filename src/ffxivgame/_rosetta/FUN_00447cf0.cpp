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
// FUNCTION: ffxivgame 0x00047cf0 — string-like buffer append from C-string
//                                  (__thiscall, 72 B / 0x48, RET 0x4)
//
// __thiscall Str* FUN_00447cf0(Str *this, const char *str);
//
//   struct Str { char *data;  /* +0x00 */  int _;  /* +0x04 */  int size; /* +0x08 */ };
//
//   size includes the NUL terminator (i.e. size = strlen(data) + 1).
//   Computes strlen(str)+1 via an inline expansion, reserves room for the
//   merged length via FUN_00447010 (Resize, __thiscall), then memcpy's str
//   (including null terminator) onto the tail of this->data at the position
//   of the existing NUL. Returns `this`.
//
//     int old_size = this->size;               // EBX = [ESI+8]
//     int len      = strlen(str) + 1;          // EDI; inline expansion
//     this->Resize(old_size + len - 1, 1);     // FUN_00447010 (__thiscall)
//     memcpy(this->data + old_size - 1, str, len);  // 0x009d5110 (cdecl)
//     return this;
//
// Prologue interleave:  EBP (arg1) is loaded between PUSH EBX / PUSH EBP
// and PUSH ESI / PUSH EDI, which is a MSVC 2005 scheduler artifact.
// The inline strlen uses:
//     LEA EDX, [EAX+1]     ; EDX = str+1
//     loop: MOV CL,[EAX]; ADD EAX,1; TEST CL,CL; JNZ loop
//     SUB EAX, EDX         ; EAX = strlen
//     LEA EDI, [EAX+1]     ; EDI = strlen + 1
//
// Stack at entry:
//     [ESP+4]  const char *str   → EBP (loaded before PUSH ESI)
//   `this` arrives in ECX → ESI; EAX = ESI returned.
//
// Reloc-bearing sites (baked-in rel32, no COFF relocations emitted):
//     +0x29   CALL rel32 → FUN_00447010  (0xfffff2f2 relative)
//     +0x37   CALL rel32 → 0x009d5110   (0x0058d3e4 relative)
//
// Reconstruction strategy — naked-asm byte passthrough (same as sibling
// FUN_00447c80): prologue interleave and inline strlen are MSVC-specific;
// emit the orig 72 bytes verbatim via MASM _emit directives so
// tools/compare.py finds a byte-identical .text section.

extern "C" __declspec(naked) void FUN_00447cf0() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x0c]
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EBX, dword ptr [ESI+0x08]
        _emit 0x5e
        _emit 0x08
        _emit 0x8b              // MOV EAX, EBP
        _emit 0xc5
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA EDX, [EAX+0x01]
        _emit 0x50
        _emit 0x01
        // loop_top (+0x12 = 0x47d02):
        _emit 0x8a              // MOV CL, byte ptr [EAX]
        _emit 0x08
        _emit 0x83              // ADD EAX, 0x01
        _emit 0xc0
        _emit 0x01
        _emit 0x84              // TEST CL, CL
        _emit 0xc9
        _emit 0x75              // JNZ -0x09 (back to loop_top)
        _emit 0xf7
        _emit 0x2b              // SUB EAX, EDX
        _emit 0xc2
        _emit 0x8d              // LEA EDI, [EAX+0x01]
        _emit 0x78
        _emit 0x01
        _emit 0x6a              // PUSH 0x01
        _emit 0x01
        _emit 0x8d              // LEA EAX, [EBX+EDI*1-0x01]
        _emit 0x44
        _emit 0x3b
        _emit 0xff
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_00447010 (rel32=0xfffff2f2)
        _emit 0xf2
        _emit 0xf2
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ECX, dword ptr [ESI]
        _emit 0x0e
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA EDX, [ECX+EBX*1-0x01]
        _emit 0x54
        _emit 0x19
        _emit 0xff
        _emit 0x55              // PUSH EBP
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL 0x009d5110 (rel32=0x0058d3e4)
        _emit 0xe4
        _emit 0xd3
        _emit 0x58
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x0c
        _emit 0xc4
        _emit 0x0c
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x04
        _emit 0x04
        _emit 0x00
    }
}
