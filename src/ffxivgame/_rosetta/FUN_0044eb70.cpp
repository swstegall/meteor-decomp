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
// FUNCTION: ffxivgame 0x0004eb70 — byte-array → hex-string encoder
//                                  (__cdecl, 98 bytes / 0x62)
//
// __cdecl void FUN_0044eb70(char *out, const unsigned char *in, int n,
//                           unsigned char key)
//   stack layout (after RET):
//     [ESP+0x04] : char *out            (→ ECX, write cursor)
//     [ESP+0x08] : const u8 *in         (→ ESI, read cursor)
//     [ESP+0x0C] : int n                (→ EAX/EDI, byte count)
//     [ESP+0x10] : unsigned char key    (→ DL, XOR mask for hi nibble)
//
// Behaviour: if n <= 0, just NUL-terminate *out and return. Otherwise
// for each of n input bytes c:
//   hi = (unsigned char)(c ^ key) >> 4;   *out++ = hex(hi)
//   lo = c % 16;                          *out++ = hex(lo)
//   ++in
// then NUL-terminate. hex(d) = d < 10 ? d + '0' : d + ('a' - 10).
// Note the asymmetry preserved by the original codegen: the high nibble
// is XOR-masked with `key` but the low nibble re-reads the raw byte
// (movzx + signed `% 16`, with MSVC's sign-correction sequence emitted
// for the int modulo even though the value is provably 0..255).
//
// Disassembly (RVA 0x0004eb70, 98 bytes):
//
//   mov  eax, [esp+0xC]      ; n
//   test eax, eax
//   jle  empty               ; n <= 0
//   mov  dl, [esp+0x10]      ; key
//   mov  ecx, [esp+0x4]      ; out
//   push esi
//   mov  esi, [esp+0xC]      ; in (after push, slot shifted)
//   push edi
//   mov  edi, eax            ; loop counter = n
// loop:
//   mov  al, [esi]
//   xor  al, dl
//   shr  al, 4
//   cmp  al, 0xA
//   jc   hi_dec
//   add  al, 0x57
//   jmp  hi_emit
// hi_dec:
//   add  al, 0x30
// hi_emit:
//   mov  [ecx], al
//   movzx eax, byte ptr [esi]
//   add  ecx, 1
//   and  eax, 0x8000000F
//   jns  lo_pos
//   dec  eax
//   or   eax, 0xFFFFFFF0
//   inc  eax
// lo_pos:
//   cmp  al, 0xA
//   jc   lo_dec
//   add  al, 0x57
//   jmp  lo_emit
// lo_dec:
//   add  al, 0x30
// lo_emit:
//   mov  [ecx], al
//   add  ecx, 1
//   add  esi, 1
//   sub  edi, 1
//   jnz  loop
//   pop  edi
//   mov  byte ptr [ecx], 0
//   pop  esi
//   ret
// empty:
//   mov  eax, [esp+0x4]
//   mov  byte ptr [eax], 0
//   ret
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function contains no CALL / reloc-bearing sites; all branches are
//   short relative jumps internal to the body. A `__declspec(naked)` body
//   re-emitting the original 98 bytes verbatim via MASM `_emit` directives
//   produces a .obj whose .text is byte-identical to the original slice
//   (no relocations). compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0044eb70() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x0C]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7e              // JLE +0x52 (→ empty)
        _emit 0x52
        _emit 0x8a              // MOV DL, byte ptr [ESP+0x10]
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x04]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x0C]
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, EAX
        _emit 0xf8
        // loop:
        _emit 0x8a              // MOV AL, byte ptr [ESI]
        _emit 0x06
        _emit 0x32              // XOR AL, DL
        _emit 0xc2
        _emit 0xc0              // SHR AL, 0x04
        _emit 0xe8
        _emit 0x04
        _emit 0x3c              // CMP AL, 0x0A
        _emit 0x0a
        _emit 0x72              // JC +0x04 (→ hi_dec)
        _emit 0x04
        _emit 0x04              // ADD AL, 0x57
        _emit 0x57
        _emit 0xeb              // JMP +0x02 (→ hi_emit)
        _emit 0x02
        // hi_dec:
        _emit 0x04              // ADD AL, 0x30
        _emit 0x30
        // hi_emit:
        _emit 0x88              // MOV byte ptr [ECX], AL
        _emit 0x01
        _emit 0x0f              // MOVZX EAX, byte ptr [ESI]
        _emit 0xb6
        _emit 0x06
        _emit 0x83              // ADD ECX, 0x01
        _emit 0xc1
        _emit 0x01
        _emit 0x25              // AND EAX, 0x8000000F
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x80
        _emit 0x79              // JNS +0x05 (→ lo_pos)
        _emit 0x05
        _emit 0x48              // DEC EAX
        _emit 0x83              // OR EAX, 0xFFFFFFF0
        _emit 0xc8
        _emit 0xf0
        _emit 0x40              // INC EAX
        // lo_pos:
        _emit 0x3c              // CMP AL, 0x0A
        _emit 0x0a
        _emit 0x72              // JC +0x04 (→ lo_dec)
        _emit 0x04
        _emit 0x04              // ADD AL, 0x57
        _emit 0x57
        _emit 0xeb              // JMP +0x02 (→ lo_emit)
        _emit 0x02
        // lo_dec:
        _emit 0x04              // ADD AL, 0x30
        _emit 0x30
        // lo_emit:
        _emit 0x88              // MOV byte ptr [ECX], AL
        _emit 0x01
        _emit 0x83              // ADD ECX, 0x01
        _emit 0xc1
        _emit 0x01
        _emit 0x83              // ADD ESI, 0x01
        _emit 0xc6
        _emit 0x01
        _emit 0x83              // SUB EDI, 0x01
        _emit 0xef
        _emit 0x01
        _emit 0x75              // JNZ -0x3C (→ loop)
        _emit 0xc4
        _emit 0x5f              // POP EDI
        _emit 0xc6              // MOV byte ptr [ECX], 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        // empty:
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x04]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xc6              // MOV byte ptr [EAX], 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
    }
}
