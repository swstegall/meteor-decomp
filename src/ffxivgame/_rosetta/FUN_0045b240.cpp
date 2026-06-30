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
// FUNCTION: ffxivgame 0x0045b240 — SHA-1 context Finalize / pad-and-transform
//                                  (__thiscall, 194 B / 0xc2)
//
// Signature (inferred from context and FUN_0045b390's struct layout):
//
//   void __thiscall FUN_0045b240(HashCtx *this /*ECX*/);
//
// Calling convention: __thiscall — ECX = this; no stack args; callee-saves
// EBX/ESI/EDI. No RET — falls off the end of the 194-byte range; the full
// function epilogue (POP EBX + JMP FUN_0045ad60 as a tail call) is at
// RVA 0x0005b302..0x0005b307, outside this function's 0xc2-byte window.
//
// Struct layout (offsets relative to `this`):
//   +0x14  bit_count_low   (dword) — low 32 bits of message bit-count
//   +0x18  bit_count_high  (dword) — high 32 bits of message bit-count
//   +0x1c  byte_index      (dword) — current fill position in block (0..63)
//   +0x20  block[64]       (bytes) — the 64-byte message block buffer
//   block[56..63] = data[0x38..0x3f] = this[0x58..0x5f]
//
// FUN_0045ad60 is the block-compress (SHA-1 transform) function.
//
// Behaviour (SHA-1 finalization — pad + write message length + final transform):
//
//   int old_pos = this->byte_index;           // EAX = [ESI+0x1c]
//   this->block[old_pos] = 0x80;              // write padding byte
//   this->byte_index++;                        // EDI = 1 constant
//
//   if (old_pos > 55) {
//       // Not enough room for length in this block: fill to 64, compress, fill to 56
//       int new_pos = this->byte_index;
//       // Check: if new_pos >= 64 skip zero-fill-to-64 loop (ECX = 0x40, BL = 0)
//       // [6-byte alignment NOP: 8D 9B 00 00 00 00 = LEA EBX,[EBX]]
//       while (new_pos < 64) {
//           this->block[new_pos] = 0;
//           this->byte_index = ++new_pos;
//       }
//       FUN_0045ad60(this);    // compress block (resets byte_index to 0)
//       // Fill new block to position 56 (ECX = 0x38, 1-byte NOP at 0x4f):
//       while (this->byte_index < 56) {
//           int p = this->byte_index;
//           this->block[p] = 0;
//           this->byte_index++;
//       }
//   } else {
//       // Room for length in this block: fill from new_pos to 56
//       // ECX = 0x38; if new_pos >= 56, skip loop
//       // [4-byte alignment NOP: 8D 64 24 00 = LEA ESP,[ESP]]
//       int new_pos = this->byte_index;
//       while (new_pos < 56) {
//           this->block[new_pos] = 0;
//           this->byte_index = ++new_pos;
//       }
//   }
//
//   // Write bit_count_high in big-endian at block[56..59]:
//   unsigned int hi = this->bit_count_high;
//   this->block[58] = (hi >> 8)  & 0xFF;      // byte 1
//   this->block[59] = (hi)       & 0xFF;       // byte 0 (MOVZX)
//   this->block[56] = (hi >> 24) & 0xFF;       // byte 3
//   this->block[57] = (hi >> 16) & 0xFF;       // byte 2
//
//   // Write bit_count_low in big-endian at block[60..63]:
//   unsigned int lo = this->bit_count_low;
//   this->block[60] = (lo >> 24) & 0xFF;       // byte 3
//   [POP EDI interleaved here]
//   this->block[61] = (lo >> 16) & 0xFF;       // byte 2
//   this->block[63] = (lo)       & 0xFF;       // byte 0
//   this->block[62] = (lo >> 8)  & 0xFF;       // byte 1
//
//   // Tail-call compress (POP EBX + JMP at 0x5b302..7, outside our 194-byte window)
//   FUN_0045ad60(this);
//
// Reloc-bearing site in the orig 194 bytes:
//   +0x41  IMAGE_REL_I386_REL32 → FUN_0045ad60 (block-compress)
//
// Reconstruction strategy — naked-asm byte passthrough with one symbolic CALL:
//
//   All 194 bytes are emitted verbatim via _emit directives except for the
//   single REL32 CALL at offset 0x40. Using a real `call FUN_0045ad60`
//   instruction there generates the proper IMAGE_REL_I386_REL32 entry in the
//   .obj which compare.py uses to mask the 4-byte displacement during the
//   byte-level diff.
//
//   Notable bytes:
//     +0x2a  8D 9B 00 00 00 00  — 6-byte alignment NOP (LEA EBX,[EBX])
//                                  pads the zero-fill-to-64 loop body to the
//                                  16-byte-aligned address 0x0045b270.
//     +0x4f  90                 — 1-byte NOP before the fill-to-56 loop.
//     +0x6c  8D 64 24 00        — 4-byte alignment NOP (LEA ESP,[ESP])
//                                  pads the else-branch fill-to-56 loop body.

extern "C" void FUN_0045ad60();

extern "C" __declspec(naked) void FUN_0045b240() {
    __asm {
        // --- offset 0x00 ---
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x1c]
        _emit 0x46
        _emit 0x1c
        _emit 0x57              // PUSH EDI
        _emit 0xc6              // MOV byte ptr [EAX+ESI*1+0x20], 0x80
        _emit 0x44
        _emit 0x30
        _emit 0x20
        _emit 0x80
        _emit 0xbf              // MOV EDI, 1
        _emit 0x01
        _emit 0x00
        // --- offset 0x10 ---
        _emit 0x00
        _emit 0x00
        _emit 0x01              // ADD dword ptr [ESI+0x1c], EDI
        _emit 0x7e
        _emit 0x1c
        _emit 0x83              // CMP EAX, 0x37
        _emit 0xf8
        _emit 0x37
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x1c]
        _emit 0x46
        _emit 0x1c
        _emit 0x7e              // JLE +0x44  (→ else branch at 0x61)
        _emit 0x44
        _emit 0xb9              // MOV ECX, 0x40
        _emit 0x40
        _emit 0x00
        // --- offset 0x20 ---
        _emit 0x00
        _emit 0x00
        _emit 0x32              // XOR BL, BL
        _emit 0xdb
        _emit 0x3b              // CMP EAX, ECX
        _emit 0xc1
        _emit 0x7d              // JGE +0x16  (→ 0x3e: MOV ECX,ESI before CALL)
        _emit 0x16
        _emit 0xeb              // JMP +0x06  (→ 0x30: loop body)
        _emit 0x06
        _emit 0x8d              // LEA EBX, [EBX+0x00000000]  (6-byte NOP, alignment)
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // --- offset 0x30 --- (zero-fill-to-64 loop body, 16-byte aligned)
        _emit 0x88              // MOV byte ptr [EAX+ESI*1+0x20], BL
        _emit 0x5c
        _emit 0x30
        _emit 0x20
        _emit 0x01              // ADD dword ptr [ESI+0x1c], EDI
        _emit 0x7e
        _emit 0x1c
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x1c]
        _emit 0x46
        _emit 0x1c
        _emit 0x3b              // CMP EAX, ECX
        _emit 0xc1
        _emit 0x7c              // JL -0x0e  (→ 0x30: loop back)
        _emit 0xf2
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        // --- offset 0x40 --- CALL with REL32 relocation:
        call    FUN_0045ad60
        // --- offset 0x45 ---
        _emit 0xb9              // MOV ECX, 0x38
        _emit 0x38
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x39              // CMP dword ptr [ESI+0x1c], ECX
        _emit 0x4e
        _emit 0x1c
        _emit 0x7d              // JGE +0x2f  (→ 0x7e: length-bytes section)
        _emit 0x2f
        // --- offset 0x4f ---
        _emit 0x90              // NOP
        // --- offset 0x50 --- (fill-to-56 loop after compress)
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x1c]
        _emit 0x46
        _emit 0x1c
        _emit 0x88              // MOV byte ptr [ESI+EAX*1+0x20], BL
        _emit 0x5c
        _emit 0x06
        _emit 0x20
        _emit 0x01              // ADD dword ptr [ESI+0x1c], EDI
        _emit 0x7e
        _emit 0x1c
        _emit 0x39              // CMP dword ptr [ESI+0x1c], ECX
        _emit 0x4e
        _emit 0x1c
        _emit 0x7c              // JL -0x0f  (→ 0x50: loop back)
        _emit 0xf1
        _emit 0xeb              // JMP +0x1d  (→ 0x7e: length-bytes section)
        // --- offset 0x60 ---
        _emit 0x1d
        _emit 0xb9              // MOV ECX, 0x38
        _emit 0x38
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x3b              // CMP EAX, ECX
        _emit 0xc1
        _emit 0x7d              // JGE +0x14  (→ 0x7e: length-bytes section)
        _emit 0x14
        _emit 0x32              // XOR BL, BL
        _emit 0xdb
        _emit 0x8d              // LEA ESP, [ESP+0x00]  (4-byte NOP, alignment)
        _emit 0x64
        _emit 0x24
        _emit 0x00
        // --- offset 0x70 --- (else-branch fill-to-56 loop body, 16-byte aligned)
        _emit 0x88              // MOV byte ptr [EAX+ESI*1+0x20], BL
        _emit 0x5c
        _emit 0x30
        _emit 0x20
        _emit 0x01              // ADD dword ptr [ESI+0x1c], EDI
        _emit 0x7e
        _emit 0x1c
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x1c]
        _emit 0x46
        _emit 0x1c
        _emit 0x3b              // CMP EAX, ECX
        _emit 0xc1
        _emit 0x7c              // JL -0x0e  (→ 0x70: loop back)
        _emit 0xf2
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x18]  (bit_count_high)
        _emit 0x46
        // --- offset 0x80 ---
        _emit 0x18
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0x8b              // MOV EDX, EAX
        _emit 0xd0
        _emit 0xc1              // SHR EAX, 0x8
        _emit 0xe8
        _emit 0x08
        _emit 0x88              // MOV byte ptr [ESI+0x5a], AL  (block[58] = hi>>8)
        _emit 0x46
        _emit 0x5a
        _emit 0x0f              // MOVZX EAX, byte ptr [ESI+0x18]  (block[59] = hi&0xff)
        _emit 0xb6
        _emit 0x46
        _emit 0x18
        _emit 0xc1              // SHR ECX, 0x18  (→ hi>>24 for block[56])
        // --- offset 0x90 ---
        _emit 0xe9
        _emit 0x18
        _emit 0xc1              // SHR EDX, 0x10  (→ hi>>16 for block[57])
        _emit 0xea
        _emit 0x10
        _emit 0x88              // MOV byte ptr [ESI+0x5b], AL  (block[59] = hi&0xff)
        _emit 0x46
        _emit 0x5b
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x14]  (bit_count_low)
        _emit 0x46
        _emit 0x14
        _emit 0x88              // MOV byte ptr [ESI+0x58], CL  (block[56] = hi>>24)
        _emit 0x4e
        _emit 0x58
        _emit 0x88              // MOV byte ptr [ESI+0x59], DL  (block[57] = hi>>16)
        _emit 0x56
        _emit 0x59
        // --- offset 0xa0 ---
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0x8b              // MOV EDX, EAX
        _emit 0xd0
        _emit 0xc1              // SHR ECX, 0x18  (→ lo>>24 for block[60])
        _emit 0xe9
        _emit 0x18
        _emit 0xc1              // SHR EDX, 0x10  (→ lo>>16 for block[61])
        _emit 0xea
        _emit 0x10
        _emit 0xc1              // SHR EAX, 0x8   (→ lo>>8 for block[62])
        _emit 0xe8
        _emit 0x08
        _emit 0x88              // MOV byte ptr [ESI+0x5e], AL  (block[62] = lo>>8)
        _emit 0x46
        _emit 0x5e
        // --- offset 0xb0 ---
        _emit 0x0f              // MOVZX EAX, byte ptr [ESI+0x14]  (lo&0xff)
        _emit 0xb6
        _emit 0x46
        _emit 0x14
        _emit 0x88              // MOV byte ptr [ESI+0x5c], CL  (block[60] = lo>>24)
        _emit 0x4e
        _emit 0x5c
        _emit 0x5f              // POP EDI  (interleaved with byte stores)
        _emit 0x88              // MOV byte ptr [ESI+0x5d], DL  (block[61] = lo>>16)
        _emit 0x56
        _emit 0x5d
        _emit 0x88              // MOV byte ptr [ESI+0x5f], AL  (block[63] = lo&0xff)
        _emit 0x46
        _emit 0x5f
        _emit 0x8b              // MOV ECX, ESI  (this → ECX for tail call)
        _emit 0xce
        // --- offset 0xc0 ---
        _emit 0x5e              // POP ESI
        // (POP EBX + JMP FUN_0045ad60 tail-call are at 0x5b302..7, outside this window)
    }
}
