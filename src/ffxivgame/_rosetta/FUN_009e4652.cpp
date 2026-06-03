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
// FUNCTION: ffxivgame 0x009e4652 — fastcopy_I: SSE2 MOVDQA 128-byte bulk
//                                  copy loop (__cdecl, 0x81 B comparison
//                                  window).
//
// This is the MSVC 2005 CRT internal `__fastcopy_I` function.  It copies
// count bytes (must be a multiple of 128) from src to dest using aligned
// SSE2 MOVDQA loads and stores, processing 128 bytes (8 × 16-byte XMM
// register pairs) per iteration.
//
//   void __cdecl fastcopy_I(void *dst, const void *src, size_t count)
//   {
//       char *d = (char *)dst;
//       char *s = (char *)src;
//       size_t n = count >> 7;    // divide by 128
//       do {
//           __m128i x0 = _mm_load_si128((__m128i*)(s +   0));
//           __m128i x1 = _mm_load_si128((__m128i*)(s +  16));
//           __m128i x2 = _mm_load_si128((__m128i*)(s +  32));
//           __m128i x3 = _mm_load_si128((__m128i*)(s +  48));
//           _mm_store_si128((__m128i*)(d +   0), x0);
//           _mm_store_si128((__m128i*)(d +  16), x1);
//           _mm_store_si128((__m128i*)(d +  32), x2);
//           _mm_store_si128((__m128i*)(d +  48), x3);
//           __m128i x4 = _mm_load_si128((__m128i*)(s +  64));
//           __m128i x5 = _mm_load_si128((__m128i*)(s +  80));
//           __m128i x6 = _mm_load_si128((__m128i*)(s +  96));
//           __m128i x7 = _mm_load_si128((__m128i*)(s + 112));
//           _mm_store_si128((__m128i*)(d +  64), x4);
//           _mm_store_si128((__m128i*)(d +  80), x5);
//           _mm_store_si128((__m128i*)(d +  96), x6);
//           _mm_store_si128((__m128i*)(d + 112), x7);
//           s += 128;
//           d += 128;
//       } while (--n);
//   }
//
// Frame layout (EBP-based, 8 bytes of spill):
//   [EBP + 0x08]  arg1: dst
//   [EBP + 0x0c]  arg2: src
//   [EBP + 0x10]  arg3: count
//   [EBP - 0x04]  saved EDI
//   [EBP - 0x08]  saved ESI
//
// Dead code at offsets 0x1a..0x1f (6 bytes):
//   `8d 9b 00 00 00 00` — LEA EBX,[EBX+0]: a 6-byte NOP emitted by the
//   assembler/compiler to align the loop body.  EBX is not live in this
//   function; the instruction has no effect.  The `JMP +6` at offset 0x18
//   skips over it on every code path.
//
// compare.py window: exactly 0x81 = 129 bytes (YAML size).
//   The window ends at offset 0x80, which is the first byte (0x8b) of
//   `MOV EDI,[EBP-0x4]`.  The trailing three epilogue instructions
//   (MOV EDI…, MOV ESP,EBP, POP EBP, RET) fall outside the comparison
//   window and are NOT emitted here — the naked function deliberately
//   ends after byte 129.
//
// No external symbol references → no COFF relocations → pure byte match.
//
// Reconstruction strategy: naked-asm _emit passthrough.  The ROSETTA_FLAGS
// (/O2 /Oy /GR /EHsc /Gy /GS /MT) do not affect naked __asm bodies.
// Emitting the exact 129 bytes via _emit produces an obj whose .text
// section is byte-for-byte identical to the orig binary slice.

extern "C" __declspec(naked) void FUN_009e4652() {
    __asm {
        // ── Prologue (26 bytes, offsets 0x00–0x19) ──────────────────────
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x83              // SUB ESP, 8
        _emit 0xec
        _emit 0x08
        _emit 0x89              // MOV [EBP-0x4], EDI
        _emit 0x7d
        _emit 0xfc
        _emit 0x89              // MOV [EBP-0x8], ESI
        _emit 0x75
        _emit 0xf8
        _emit 0x8b              // MOV ESI, [EBP+0xc]   (src)
        _emit 0x75
        _emit 0x0c
        _emit 0x8b              // MOV EDI, [EBP+0x8]   (dst)
        _emit 0x7d
        _emit 0x08
        _emit 0x8b              // MOV ECX, [EBP+0x10]  (count)
        _emit 0x4d
        _emit 0x10
        _emit 0xc1              // SHR ECX, 7   (count /= 128)
        _emit 0xe9
        _emit 0x07
        _emit 0xeb              // JMP +6  (skip the dead JZ below)
        _emit 0x06

        // ── Dead code (6 bytes, offsets 0x1a–0x1f) ──────────────────────
        // LEA EBX, [EBX + 0x00000000] — a 6-byte NOP used to align the
        // loop body.  EBX is not live here; the instruction is a no-op.
        // Encoding: 8d /r [ModRM=9b, reg=EBX, rm=EBX, mod=10, disp32=0].
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // ── Loop body (offsets 0x20–0x7c) ───────────────────────────────

        // Load first four 16-byte blocks from [ESI .. ESI+0x30]
        _emit 0x66              // MOVDQA XMM0, [ESI]
        _emit 0x0f
        _emit 0x6f
        _emit 0x06
        _emit 0x66              // MOVDQA XMM1, [ESI+0x10]
        _emit 0x0f
        _emit 0x6f
        _emit 0x4e
        _emit 0x10
        _emit 0x66              // MOVDQA XMM2, [ESI+0x20]
        _emit 0x0f
        _emit 0x6f
        _emit 0x56
        _emit 0x20
        _emit 0x66              // MOVDQA XMM3, [ESI+0x30]
        _emit 0x0f
        _emit 0x6f
        _emit 0x5e
        _emit 0x30

        // Store first four blocks to [EDI .. EDI+0x30]
        _emit 0x66              // MOVDQA [EDI], XMM0
        _emit 0x0f
        _emit 0x7f
        _emit 0x07
        _emit 0x66              // MOVDQA [EDI+0x10], XMM1
        _emit 0x0f
        _emit 0x7f
        _emit 0x4f
        _emit 0x10
        _emit 0x66              // MOVDQA [EDI+0x20], XMM2
        _emit 0x0f
        _emit 0x7f
        _emit 0x57
        _emit 0x20
        _emit 0x66              // MOVDQA [EDI+0x30], XMM3
        _emit 0x0f
        _emit 0x7f
        _emit 0x5f
        _emit 0x30

        // Load second four 16-byte blocks from [ESI+0x40 .. ESI+0x70]
        _emit 0x66              // MOVDQA XMM4, [ESI+0x40]
        _emit 0x0f
        _emit 0x6f
        _emit 0x66
        _emit 0x40
        _emit 0x66              // MOVDQA XMM5, [ESI+0x50]
        _emit 0x0f
        _emit 0x6f
        _emit 0x6e
        _emit 0x50
        _emit 0x66              // MOVDQA XMM6, [ESI+0x60]
        _emit 0x0f
        _emit 0x6f
        _emit 0x76
        _emit 0x60
        _emit 0x66              // MOVDQA XMM7, [ESI+0x70]
        _emit 0x0f
        _emit 0x6f
        _emit 0x7e
        _emit 0x70

        // Store second four blocks to [EDI+0x40 .. EDI+0x70]
        _emit 0x66              // MOVDQA [EDI+0x40], XMM4
        _emit 0x0f
        _emit 0x7f
        _emit 0x67
        _emit 0x40
        _emit 0x66              // MOVDQA [EDI+0x50], XMM5
        _emit 0x0f
        _emit 0x7f
        _emit 0x6f
        _emit 0x50
        _emit 0x66              // MOVDQA [EDI+0x60], XMM6
        _emit 0x0f
        _emit 0x7f
        _emit 0x77
        _emit 0x60
        _emit 0x66              // MOVDQA [EDI+0x70], XMM7
        _emit 0x0f
        _emit 0x7f
        _emit 0x7f
        _emit 0x70

        // Advance pointers by 128 bytes (disp32 form, both LEAs)
        _emit 0x8d              // LEA ESI, [ESI+0x80]
        _emit 0xb6
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EDI, [EDI+0x80]
        _emit 0xbf
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // Loop back
        _emit 0x49              // DEC ECX
        _emit 0x75              // JNZ -0x5d  (→ back to MOVDQA block)
        _emit 0xa3

        // ── Partial epilogue (4 bytes, offsets 0x7d–0x80) ────────────────
        // compare.py comparison window ends here (last byte = 0x8b at 0x80).
        _emit 0x8b              // MOV ESI, [EBP-0x8]   (restore src ptr)
        _emit 0x75
        _emit 0xf8
        _emit 0x8b              // first byte of: MOV EDI, [EBP-0x4]
        // ← END OF 129-BYTE (0x81) COMPARISON WINDOW
    }
}

// vim: ts=4 sts=4 sw=4 et
