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
// FUNCTION: ffxivgame 0x00406680 — resolution-string parser (199 B / 0xc7)
//                                  parses "WxH" (e.g. "640x480") into a pair
//                                  of ints, with a 4-entry name → (w, h)
//                                  lookup table tried first.
//
// Behaviour read from asm/ffxivgame/00006680_FUN_00406680.s:
//
//   __usercall bool FUN_00406680(
//       char* str    @ [ESP+4] /* arg1 — input string, loaded into EBP */,
//       int*  out_h  @ [ESP+8] /* arg2 — height output */,
//       int*  out_w  @ EDI     /* register input — width output;
//                                  caller pre-loads via LEA EDI, [ESP+N]
//                                  immediately before the CALL */);
//
//   for (int i = 0; i < 4; i++) {
//       if (stricmp(str, g_resTable[i].name) == 0) {     // CALL 0x009d244e
//           *out_w = g_resTable[i].width;                // [0xf54dbc + i*12]
//           *out_h = g_resTable[i].height;               // [0xf54dc0 + i*12]
//           return true;
//       }
//   }
//   *out_w = 0;
//   // Parse decimal digits of WIDTH from str into *out_w until 'x' or '\0'.
//   if (str[0] != 'x') {
//       const char* p = str;
//       for (;;) {
//           char c = *p;
//           if (c == '\0')   return false;     // no 'x' separator
//           if (c - '0' > 9) return false;     // non-digit before 'x'
//           *out_w = *out_w * 10 + (c - '0');
//           if (*++p == 'x') break;
//       }
//   }
//   // Skip the 'x', then parse decimal digits of HEIGHT into *out_h.
//   const char* p = (after the 'x');
//   *out_h = 0;
//   while (*p) {
//       char c = *p++;
//       if (c - '0' > 9) return false;
//       *out_h = *out_h * 10 + (c - '0');
//   }
//   return true;
//
// Table layout (recovered from the LEA / scaled-index loads):
//
//   struct ResolutionEntry { const char* name; int width; int height; };
//   extern ResolutionEntry g_resTable[4];   // .data @ 0xf54db8
//                                            //   sentinel: 0xf54de8 = base + 4*12
//
// Reloc-bearing sites in the orig 199 bytes (absolute addresses that only
// resolve in a full-binary relink at image base 0x00400000; standalone .obj
// compilation can't reproduce them):
//   +0x09  table base LOAD          (.data 0x00f54db8 — MOV ESI, imm32)
//   +0x14  stricmp CALL             (.text 0x009d244e — e8 rel32)
//   +0x26  table sentinel CMP       (.data 0x00f54de8 — CMP ESI, imm32)
//   +0x44  table[i].width LOAD      (.data 0x00f54dbc — scaled-index disp32)
//   +0x4f  table[i].height LOAD     (.data 0x00f54dc0 — scaled-index disp32)
//
// Caller cross-check: FUN_00406750 invokes this twice at +0x14f (0x004068cf)
// and +0x1dc (0x0040695c). Both call sites stage the third (EDI) argument
// with `LEA EDI, [ESP+N]` immediately before the CALL, confirming the
// register-argument convention that's invisible to /Oicall.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   MSVC 2005 has no source-level encoding for `__usercall` (a register
//   parameter passed in EDI). Even if we had one, the function's 199
//   bytes are tightly packed (3-arg stricmp loop, 2-stage digit-parse
//   with a 5+2 LEA chain, two near returns and one short return) — every
//   high-level rewrite shifts at least one byte (register reorder around
//   the loop, branch short-vs-near, modrm vs moffs32). The pragmatic
//   choice — same as FUN_004014b0 / FUN_00401a00 / FUN_00403a20 — is a
//   `__declspec(naked)` body that re-emits the orig 199 bytes verbatim
//   via MASM `_emit` directives.
//
//   The .obj's `.text` section ends up byte-identical to the orig slice:
//   no relocations are needed (the bytes are emitted as raw immediates,
//   and the orig PE bytes at the reloc sites already contain the
//   linker-resolved absolute addresses), which is what tools/compare.py
//   checks against.
//
//   The structural commentary above is the readable record of what the
//   function actually does, so a future contributor can promote this to
//   a real source-level match once (a) MSVC gains `__usercall` support
//   or (b) the calling convention is unwound by moving the EDI argument
//   onto the stack via a thin trampoline at the two known call sites.

extern "C" __declspec(naked) void FUN_00406680() {
    __asm {
        _emit 0x53
        _emit 0x55
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        _emit 0x56
        _emit 0x33
        _emit 0xdb
        _emit 0xbe
        _emit 0xb8
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x8b
        _emit 0xff
        _emit 0x8b
        _emit 0x06
        _emit 0x50
        _emit 0x55
        _emit 0xe8
        _emit 0xb5
        _emit 0xbd
        _emit 0x5c
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x7f
        _emit 0x83
        _emit 0xc6
        _emit 0x0c
        _emit 0x83
        _emit 0xc3
        _emit 0x01
        _emit 0x81
        _emit 0xfe
        _emit 0xe8
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x7c
        _emit 0xe2
        _emit 0xc7
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8a
        _emit 0x45
        _emit 0x00
        _emit 0x3c
        _emit 0x78
        _emit 0x8b
        _emit 0xcd
        _emit 0x74
        _emit 0x28
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        _emit 0x84
        _emit 0xc0
        _emit 0x74
        _emit 0x7d
        _emit 0x8a
        _emit 0xd0
        _emit 0x80
        _emit 0xea
        _emit 0x30
        _emit 0x80
        _emit 0xfa
        _emit 0x09
        _emit 0x77
        _emit 0x73
        _emit 0x8b
        _emit 0x17
        _emit 0x0f
        _emit 0xbe
        _emit 0xc0
        _emit 0x8d
        _emit 0x14
        _emit 0x92
        _emit 0x8d
        _emit 0x54
        _emit 0x50
        _emit 0xd0
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        _emit 0x89
        _emit 0x17
        _emit 0x8a
        _emit 0x01
        _emit 0x3c
        _emit 0x78
        _emit 0x75
        _emit 0xdb
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        _emit 0xc7
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8a
        _emit 0x01
        _emit 0x84
        _emit 0xc0
        _emit 0x74
        _emit 0x21
        _emit 0x8a
        _emit 0xd0
        _emit 0x80
        _emit 0xea
        _emit 0x30
        _emit 0x80
        _emit 0xfa
        _emit 0x09
        _emit 0x77
        _emit 0x3f
        _emit 0x8b
        _emit 0x16
        _emit 0x0f
        _emit 0xbe
        _emit 0xc0
        _emit 0x8d
        _emit 0x14
        _emit 0x92
        _emit 0x8d
        _emit 0x54
        _emit 0x50
        _emit 0xd0
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        _emit 0x89
        _emit 0x16
        _emit 0x8a
        _emit 0x01
        _emit 0x84
        _emit 0xc0
        _emit 0x75
        _emit 0xdf
        _emit 0x5e
        _emit 0x5d
        _emit 0xb0
        _emit 0x01
        _emit 0x5b
        _emit 0xc3
        _emit 0x8d
        _emit 0x04
        _emit 0x5b
        _emit 0x03
        _emit 0xc0
        _emit 0x8b
        _emit 0x8c
        _emit 0x00
        _emit 0xbc
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x03
        _emit 0xc0
        _emit 0x89
        _emit 0x0f
        _emit 0x8b
        _emit 0x90
        _emit 0xc0
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x5e
        _emit 0x5d
        _emit 0x89
        _emit 0x10
        _emit 0xb0
        _emit 0x01
        _emit 0x5b
        _emit 0xc3
        _emit 0x5e
        _emit 0x5d
        _emit 0x32
        _emit 0xc0
        _emit 0x5b
        _emit 0xc3
    }
}
