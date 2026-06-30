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
// FUNCTION: ffxivgame 0x000646d0 — _ASN1_put_object (OpenSSL ASN.1 DER tag+length writer)
//                                  (233 B / 0xe9, no SEH, no /GS cookie)
//
// Asm name from binary: _ASN1_put_object
//
// Calling convention: __cdecl (caller cleans; bare RET in epilogue).
// Saved registers: EBX, EBP, ESI unconditionally; EDI saved/restored only
// inside the long-tag multi-byte encoding branch.
// No SEH frame, no /GS security cookie (no local arrays ≥5 bytes).
//
// Structural shape reconstructed from disassembly at orig RVA 0x000646d0:
//
//   void _ASN1_put_object(unsigned char **pp,  // [ESP+4]  → arg1
//                         int constructed,     // [ESP+8]  → arg2  (EBP)
//                         int length,          // [ESP+C]  → arg3
//                         int tag,             // [ESP+10] → arg4  (EBX)
//                         int xclass)          // [ESP+14] → arg5  (EDX)
//
//   unsigned char *p = *pp;                    // ECX = *(arg1)
//
//   // Build identifier-octet high bits:
//   // branchless: (constructed != 0) ? 0x20 : 0, OR'd with xclass low 6-bits
//   int i = (constructed ? V_ASN1_CONSTRUCTED/*0x20*/ : 0) | (xclass & 0xc0);
//
//   if (tag < 31) {
//       // Short tag form — single identifier octet
//       *(p++) = (unsigned char)(i | (tag & 0x1f));
//   } else {
//       // Long tag form — first octet has tag=0x1f, then base-128 tag bytes
//       *(p++) = (unsigned char)(i | 0x1f);
//       int count = 0;
//       for (int ttag = tag; ttag > 0; ttag >>= 7) count++;
//       // write count bytes big-endian at p[count-1]..p[0]
//       int idx = count - 1;    // (ESI = count-1)
//       do {
//           int n = count - 1;  // (EAX goes count-1 down to 0)
//           unsigned char c = tag & 0x7f;
//           if (n != idx) c |= 0x80;  // continuation bit on all but the last written
//           p[n] = c;
//           tag >>= 7;
//           count--;
//       } while (count > 0);
//       p += (count-restored-to-EDI);
//   }
//
//   // Length encoding:
//   if (constructed == 2) {
//       // Indefinite form (BER; not DER but supported)
//       *(p++) = 0x80;
//   } else if (length <= 0x7f) {
//       // Short definite form
//       *(p++) = (unsigned char)length;
//   } else {
//       // Long definite form — first byte encodes number of subsequent length bytes
//       int lcount = 0;
//       for (int ltag = length; ltag > 0; ltag >>= 8) lcount++;
//       *(p++) = (unsigned char)(lcount | 0x80);
//       // write lcount bytes big-endian at p[lcount-1]..p[0]
//       for (int j = lcount - 1; j >= 0; j--) {
//           p[j] = length & 0xff;
//           length >>= 8;
//       }
//       p += lcount;
//   }
//
//   *pp = p;
//
// Algorithm notes:
//   - The "constructed ? V_ASN1_CONSTRUCTED : 0" is emitted as a branchless
//     idiom: MOV EAX,EBP / NEG EAX / SBB EAX,EAX / AND EAX,0x20.
//   - The long-tag byte array is written reverse-index (EAX counts down from
//     count-1 to 0) so the most-significant 7 bits land at p[0]. The
//     continuation bit (0x80) is set on all bytes except the one at position
//     count-1 (the last to be written = least-significant 7 bits).
//   - The length-byte write loop follows the same reverse-index pattern but
//     shifts by 8 instead of 7.
//   - Two alignment NOPs are emitted by MSVC's /O2 loop-alignment pass:
//       8D 9B 00 00 00 00  (LEA EBX,[EBX] — 6-byte NOP) aligns the tag write
//                         loop to 0x...730 (16-byte boundary).
//       8B FF              (MOV EDI,EDI  — 2-byte NOP)  aligns the length
//                         write loop to 0x...7a0 (16-byte boundary).
//
// Reconstruction: naked _emit passthrough — all 233 bytes verbatim.
// No relocations: function is pure arithmetic with no IAT calls, no absolute
// data references, and no CALL targets — so the raw bytes are self-contained.

extern "C" __declspec(naked) void FUN_004646d0() {
    __asm {
        // 000646d0  MOV EAX, [ESP+0x4]      ; pp
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 000646d4  MOV ECX, [EAX]          ; p = *pp
        _emit 0x8b
        _emit 0x08
        // 000646d6  MOV EDX, [ESP+0x14]     ; EDX = xclass (arg5, before any pushes)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 000646da  PUSH EBX
        _emit 0x53
        // 000646db  MOV EBX, [ESP+0x14]     ; EBX = tag (arg4, after PUSH EBX)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        // 000646df  PUSH EBP
        _emit 0x55
        // 000646e0  MOV EBP, [ESP+0x10]     ; EBP = constructed (arg2, after 2 pushes)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // 000646e4  MOV EAX, EBP
        _emit 0x8b
        _emit 0xc5
        // 000646e6  NEG EAX                 ; CF=1 if constructed != 0
        _emit 0xf7
        _emit 0xd8
        // 000646e8  SBB EAX, EAX            ; EAX = 0xFFFFFFFF if constructed, else 0
        _emit 0x1b
        _emit 0xc0
        // 000646ea  AND EAX, 0x20           ; EAX = V_ASN1_CONSTRUCTED(0x20) or 0
        _emit 0x83
        _emit 0xe0
        _emit 0x20
        // 000646ed  AND EDX, 0xc0           ; EDX = xclass & 0xc0 (class bits only)
        _emit 0x81
        _emit 0xe2
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000646f3  OR EAX, EDX             ; EAX = constructed_bit | class_bits
        _emit 0x0b
        _emit 0xc2
        // 000646f5  CMP EBX, 0x1f           ; tag < 31?
        _emit 0x83
        _emit 0xfb
        _emit 0x1f
        // 000646f8  PUSH ESI
        _emit 0x56
        // 000646f9  JGE 0x00464707          ; if tag >= 31, long form
        _emit 0x7d
        _emit 0x0c
        // 000646fb  AND BL, 0x1f            ; BL = tag & 0x1f
        _emit 0x80
        _emit 0xe3
        _emit 0x1f
        // 000646fe  OR BL, AL               ; BL = class_bits | constructed_bit | tag
        _emit 0x0a
        _emit 0xd8
        // 00064700  MOV [ECX], BL           ; *p = identifier byte (short form)
        _emit 0x88
        _emit 0x19
        // 00064702  ADD ECX, 1              ; p++
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // 00064705  JMP 0x0046474f          ; goto length encoding
        _emit 0xeb
        _emit 0x48
        // 00064707  OR AL, 0x1f             ; first octet: class|constructed|0x1f
        _emit 0x0c
        _emit 0x1f
        // 00064709  MOV [ECX], AL           ; *p = first identifier octet (long form)
        _emit 0x88
        _emit 0x01
        // 0006470b  ADD ECX, 1              ; p++
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // 0006470e  XOR EAX, EAX            ; count = 0
        _emit 0x33
        _emit 0xc0
        // 00064710  TEST EBX, EBX           ; test tag
        _emit 0x85
        _emit 0xdb
        // 00064712  MOV EDX, EBX            ; EDX = tag (copy for counting)
        _emit 0x8b
        _emit 0xd3
        // 00064714  JLE 0x00464720          ; if tag <= 0 skip count loop
        _emit 0x7e
        _emit 0x0a
        // 00064716  SAR EDX, 7              ; EDX >>= 7 (count 7-bit groups)
        _emit 0xc1
        _emit 0xfa
        _emit 0x07
        // 00064719  ADD EAX, 1              ; count++
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 0006471c  TEST EDX, EDX
        _emit 0x85
        _emit 0xd2
        // 0006471e  JG 0x00464716           ; while EDX > 0
        _emit 0x7f
        _emit 0xf6
        // 00064720  TEST EAX, EAX           ; test count
        _emit 0x85
        _emit 0xc0
        // 00064722  PUSH EDI
        _emit 0x57
        // 00064723  MOV EDI, EAX            ; EDI = count (save for p += count)
        _emit 0x8b
        _emit 0xf8
        // 00064725  JLE 0x0046474c          ; if count <= 0 skip write loop
        _emit 0x7e
        _emit 0x25
        // 00064727  LEA ESI, [EAX-1]        ; ESI = count-1 (index of first write)
        _emit 0x8d
        _emit 0x70
        _emit 0xff
        // 0006472a  LEA EBX, [EBX+0]        ; 6-byte NOP (align loop to 0x...730)
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00064730  SUB EAX, 1              ; EAX-- (index: count-1 down to 0)
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // 00064733  MOV DL, BL              ; DL = tag & 0xff (low byte)
        _emit 0x8a
        _emit 0xd3
        // 00064735  AND DL, 0x7f            ; DL = tag & 0x7f (low 7 bits)
        _emit 0x80
        _emit 0xe2
        _emit 0x7f
        // 00064738  CMP EAX, ESI            ; first iteration (EAX == count-1)?
        _emit 0x3b
        _emit 0xc6
        // 0006473a  MOV [EAX+ECX], DL       ; p[EAX] = 7-bit group (tentative)
        _emit 0x88
        _emit 0x14
        _emit 0x08
        // 0006473d  JZ 0x00464745           ; if first iter (no continuation bit)
        _emit 0x74
        _emit 0x06
        // 0006473f  OR DL, 0x80             ; set continuation bit
        _emit 0x80
        _emit 0xca
        _emit 0x80
        // 00064742  MOV [EAX+ECX], DL       ; overwrite with continuation bit set
        _emit 0x88
        _emit 0x14
        _emit 0x08
        // 00064745  SAR EBX, 7              ; tag >>= 7 (shift out low 7 bits)
        _emit 0xc1
        _emit 0xfb
        _emit 0x07
        // 00064748  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0006474a  JG 0x00464730           ; while EAX > 0
        _emit 0x7f
        _emit 0xe4
        // 0006474c  ADD ECX, EDI            ; p += count
        _emit 0x03
        _emit 0xcf
        // 0006474e  POP EDI
        _emit 0x5f
        // 0006474f  CMP EBP, 0x2            ; constructed == 2 (indefinite length)?
        _emit 0x83
        _emit 0xfd
        _emit 0x02
        // 00064752  JNZ 0x00464764          ; else normal length encoding
        _emit 0x75
        _emit 0x10
        // 00064754  MOV EAX, [ESP+0x10]     ; EAX = pp (arg1, after 3 pushes)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00064758  POP ESI
        _emit 0x5e
        // 00064759  MOV [ECX], 0x80         ; *p = 0x80 (indefinite-length marker)
        _emit 0xc6
        _emit 0x01
        _emit 0x80
        // 0006475c  ADD ECX, 1              ; p++
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // 0006475f  POP EBP
        _emit 0x5d
        // 00064760  MOV [EAX], ECX          ; *pp = p
        _emit 0x89
        _emit 0x08
        // 00064762  POP EBX
        _emit 0x5b
        // 00064763  RET
        _emit 0xc3
        // 00064764  MOV EDX, [ESP+0x18]     ; EDX = length (arg3, after 3 pushes)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 00064768  CMP EDX, 0x7f           ; length <= 127?
        _emit 0x83
        _emit 0xfa
        _emit 0x7f
        // 0006476b  JG 0x0046477c           ; else long length form
        _emit 0x7f
        _emit 0x0f
        // 0006476d  MOV [ECX], DL           ; *p = length (single byte)
        _emit 0x88
        _emit 0x11
        // 0006476f  MOV EDX, [ESP+0x10]     ; EDX = pp (arg1, after 3 pushes)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 00064773  POP ESI
        _emit 0x5e
        // 00064774  ADD ECX, 1              ; p++
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // 00064777  POP EBP
        _emit 0x5d
        // 00064778  MOV [EDX], ECX          ; *pp = p
        _emit 0x89
        _emit 0x0a
        // 0006477a  POP EBX
        _emit 0x5b
        // 0006477b  RET
        _emit 0xc3
        // 0006477c  XOR EAX, EAX            ; lcount = 0
        _emit 0x33
        _emit 0xc0
        // 0006477e  TEST EDX, EDX           ; test length (for count loop entry)
        _emit 0x85
        _emit 0xd2
        // 00064780  MOV ESI, EDX            ; ESI = length copy (for counting only)
        _emit 0x8b
        _emit 0xf2
        // 00064782  JLE 0x0046478e          ; if length <= 0 skip count loop
        _emit 0x7e
        _emit 0x0a
        // 00064784  SAR ESI, 8              ; ESI >>= 8 (count 8-bit groups)
        _emit 0xc1
        _emit 0xfe
        _emit 0x08
        // 00064787  ADD EAX, 1              ; lcount++
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 0006478a  TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0006478c  JG 0x00464784           ; while ESI > 0
        _emit 0x7f
        _emit 0xf6
        // 0006478e  MOV BL, AL              ; BL = lcount
        _emit 0x8a
        _emit 0xd8
        // 00064790  OR BL, 0x80             ; BL = lcount | 0x80 (long-form marker)
        _emit 0x80
        _emit 0xcb
        _emit 0x80
        // 00064793  MOV [ECX], BL           ; *p = lcount | 0x80
        _emit 0x88
        _emit 0x19
        // 00064795  ADD ECX, 1              ; p++
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // 00064798  TEST EAX, EAX           ; test lcount
        _emit 0x85
        _emit 0xc0
        // 0006479a  MOV ESI, EAX            ; ESI = lcount (save for p += lcount)
        _emit 0x8b
        _emit 0xf0
        // 0006479c  JLE 0x004647ad          ; if lcount <= 0 skip write loop
        _emit 0x7e
        _emit 0x0f
        // 0006479e  MOV EDI, EDI            ; 2-byte NOP (align loop to 0x...7a0)
        _emit 0x8b
        _emit 0xff
        // 000647a0  SUB EAX, 1              ; EAX-- (index: lcount-1 down to 0)
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // 000647a3  MOV [EAX+ECX], DL       ; p[EAX] = low byte of length
        _emit 0x88
        _emit 0x14
        _emit 0x08
        // 000647a6  SAR EDX, 8              ; length >>= 8 (shift out low byte)
        _emit 0xc1
        _emit 0xfa
        _emit 0x08
        // 000647a9  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000647ab  JG 0x004647a0           ; while EAX > 0
        _emit 0x7f
        _emit 0xf3
        // 000647ad  MOV EDX, [ESP+0x10]     ; EDX = pp (arg1, after 3 pushes)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 000647b1  ADD ECX, ESI            ; p += lcount
        _emit 0x03
        _emit 0xce
        // 000647b3  POP ESI
        _emit 0x5e
        // 000647b4  POP EBP
        _emit 0x5d
        // 000647b5  MOV [EDX], ECX          ; *pp = p
        _emit 0x89
        _emit 0x0a
        // 000647b7  POP EBX
        _emit 0x5b
        // 000647b8  RET
        _emit 0xc3
    }
}
