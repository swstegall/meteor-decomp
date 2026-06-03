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
// FUNCTION: ffxivgame 0x00016810 — __fastcall DWORD-array copy with jump-table
//                                  unrolled cases for 1-4 elements and a counted
//                                  loop for count > 4 (101 B / 0x65).
//
// Signature (recovered from asm):
//
//   __fastcall void FUN_00416810(unsigned int* src, int count, unsigned int* dst)
//
//   ECX   = src   (source pointer, DWORD array)
//   EDX   = count (number of DWORDs to copy)
//   [ESP+4] = dst (destination pointer)
//
// Logic:
//
//   n = count - 1
//   switch (n):             ← JMP [ESI*4 + 0x416878]  (ESI = n)
//     0: copy 1 DWORD  (ECX → EAX)
//     1: copy 2 DWORDs (EDX/ECX → EAX)
//     2: copy 3 DWORDs (EDX/ECX → EAX)
//     3: copy 4 DWORDs (EDX/ECX → EAX)
//     default (n > 3):
//       if count > 0:
//         ECX = src - dst       ← compute offset so dst can be the sole iterator
//         loop: *dst = *(dst+offset); dst += 4; count--; until count == 0
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The indirect JMP at offset +9 embeds the absolute jump-table address
//   0x416878 (bytes 78 68 41 00) which is a linker-resolved relocation.
//   Emitting the 101 raw bytes via _emit lets compare.py mask that reloc
//   site and verify byte-for-byte identity. A source-level switch(count-1)
//   rewrite would need MSVC 2005 to produce identical switch layout, loop
//   shape, and register allocation — fragile under /O2 /GS.
//
// Relocation site in the 101-byte body:
//   offset +13..+16  (bytes 78 68 41 00) — jump-table base address 0x416878

extern "C" __declspec(naked) void FUN_00416810() {
    __asm {
        // 00016810: 8b 44 24 04  MOV EAX, dword ptr [ESP+4]   ; dst
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00016814: 56           PUSH ESI
        _emit 0x56
        // 00016815: 8d 72 ff     LEA ESI, [EDX-1]             ; n = count-1
        _emit 0x8d
        _emit 0x72
        _emit 0xff
        // 00016818: 83 fe 03     CMP ESI, 3
        _emit 0x83
        _emit 0xfe
        _emit 0x03
        // 0001681b: 77 43        JA +0x43  (→ 0x16860, loop path)
        _emit 0x77
        _emit 0x43
        // 0001681d: ff 24 b5 78 68 41 00  JMP dword ptr [ESI*4 + 0x416878]
        _emit 0xff
        _emit 0x24
        _emit 0xb5
        _emit 0x78  // ← reloc: 0x416878 (jump-table base)
        _emit 0x68
        _emit 0x41
        _emit 0x00
        // 00016824: case 0 — copy 1 DWORD
        // 8b 09  MOV ECX, [ECX]
        _emit 0x8b
        _emit 0x09
        // 89 08  MOV [EAX], ECX
        _emit 0x89
        _emit 0x08
        // 5e     POP ESI
        _emit 0x5e
        // c3     RET
        _emit 0xc3
        // 0001682a: case 1 — copy 2 DWORDs
        // 8b 11  MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 89 10  MOV [EAX], EDX
        _emit 0x89
        _emit 0x10
        // 8b 49 04  MOV ECX, [ECX+4]
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        // 89 48 04  MOV [EAX+4], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 5e        POP ESI
        _emit 0x5e
        // c3        RET
        _emit 0xc3
        // 00016836: case 2 — copy 3 DWORDs
        // 8b 11     MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 89 10     MOV [EAX], EDX
        _emit 0x89
        _emit 0x10
        // 8b 51 04  MOV EDX, [ECX+4]
        _emit 0x8b
        _emit 0x51
        _emit 0x04
        // 89 50 04  MOV [EAX+4], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x04
        // 8b 49 08  MOV ECX, [ECX+8]
        _emit 0x8b
        _emit 0x49
        _emit 0x08
        // 89 48 08  MOV [EAX+8], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 5e        POP ESI
        _emit 0x5e
        // c3        RET
        _emit 0xc3
        // 00016848: case 3 — copy 4 DWORDs
        // 8b 11     MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 89 10     MOV [EAX], EDX
        _emit 0x89
        _emit 0x10
        // 8b 51 04  MOV EDX, [ECX+4]
        _emit 0x8b
        _emit 0x51
        _emit 0x04
        // 89 50 04  MOV [EAX+4], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x04
        // 8b 51 08  MOV EDX, [ECX+8]
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        // 89 50 08  MOV [EAX+8], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 8b 49 0c  MOV ECX, [ECX+0xc]
        _emit 0x8b
        _emit 0x49
        _emit 0x0c
        // 89 48 0c  MOV [EAX+0xc], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x0c
        // 5e        POP ESI
        _emit 0x5e
        // c3        RET
        _emit 0xc3
        // 00016860: default — counted loop (count > 4, or count == 0)
        // 85 d2     TEST EDX, EDX
        _emit 0x85
        _emit 0xd2
        // 76 0f     JBE +0x0f  (→ 0x16873, past loop)
        _emit 0x76
        _emit 0x0f
        // 2b c8     SUB ECX, EAX   ; ECX = src - dst (src offset)
        _emit 0x2b
        _emit 0xc8
        // 8b 34 01  MOV ESI, [ECX + EAX*1]  ; ESI = *src
        _emit 0x8b
        _emit 0x34
        _emit 0x01
        // 89 30     MOV [EAX], ESI           ; *dst = ESI
        _emit 0x89
        _emit 0x30
        // 83 c0 04  ADD EAX, 4               ; dst += 4
        _emit 0x83
        _emit 0xc0
        _emit 0x04
        // 83 ea 01  SUB EDX, 1               ; count--
        _emit 0x83
        _emit 0xea
        _emit 0x01
        // 75 f3     JNZ -0xd  (→ 0x16866)
        _emit 0x75
        _emit 0xf3
        // 00016873: epilogue
        // 5e        POP ESI
        _emit 0x5e
        // c3        RET
        _emit 0xc3
    }
}
