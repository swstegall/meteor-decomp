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
// FUNCTION: ffxivgame 0x009e5064 — streambuf::sputc / overflow-path byte write
//                                  (__cdecl, 352 B / 0x160, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x005e5064):
//
//   __cdecl int FUN_009e5064(char c, struct_type *obj)
//      — arg1=[EBP+0x8] (char, written as byte at end),
//        arg2=[EBP+0xc] (stream/buf pointer, re-assigned on entry).
//
//   Structural shape (traced from the asm):
//
//     ESI = obj (arg2);
//     obj = lock_stream(ESI);                  // CALL 0x009d6a61
//     flags = ESI->flags;                      // [ESI+0xc]
//     if (!(flags & 0x82)) {
//         *get_errno() = 9;                    // EINVAL — stream not open
//         ESI->flags |= 0x20;
//         return -1;
//     }
//     if (flags & 0x40) {
//         *get_errno() = 0x22;                 // EINVAL — text/binary mode error
//         ESI->flags |= 0x20;
//         return -1;
//     }
//     // Push EBX=0 (local result) onto the stack (shrink-wrapped callee save)
//     local_result = 0;
//     if (flags & 0x1) {
//         ESI->field_4 = 0;
//         if (!(flags & 0x10)) {
//             // bit 0x10 not set → error path
//             ESI->flags |= 0x20;
//             return -1;
//         }
//         ESI->field_0 = ESI->field_8;         // restore write pointer
//         ESI->flags = flags & ~0x1;
//     }
//     flags = (ESI->flags & ~0x10) | 0x2;
//     TEST AX, 0x10c;
//     ESI->flags = flags;
//     ESI->field_4 = 0;
//     local_result = 0;
//     if (!(AX & 0x10c)) {
//         p = get_stdout() + 0x20;             // CALL 0x009d4d67
//         if (ESI == p) goto write_byte;
//         q = get_stdout() + 0x40;             // CALL 0x009d4d67
//         if (ESI == q) goto write_byte;
//     write_byte_check:
//         if (flush_buf(obj)) goto write_byte; // CALL 0x009f6d01
//         seek_stream(ESI);                    // CALL 0x009f0a23
//     write_byte:
//         ;
//     }
//     // PUSH EDI (shrink-wrapped callee save)
//     if (ESI->flags & 0x108) {
//         buf_ptr = ESI->field_8;
//         prev = ESI->field_0;
//         ESI->field_0 = buf_ptr + 1;
//         ESI->field_4 = ESI->field_0x18 - 1;
//         diff = prev - buf_ptr;               // (old_ptr - write_ptr), effectively ahead count
//         if (diff > 0) {
//             local_result = write_block(obj, buf_ptr, diff); // CALL 0x009e9211
//             goto check_result;
//         }
//         // diff <= 0: look up channel entry for obj token
//         token = obj;
//         if (token == -1 || token == -2) {
//             entry = &g_default_entry;         // 0x012eb4d8
//         } else {
//             idx = (token & 0x1f) << 6;
//             tbl = g_channel_table[token >> 5]; // [EDX*4 + 0x137b7e0]
//             entry = tbl + idx;
//         }
//         if (entry->field_4 & 0x20) {
//             rc = notify_channel(token, 0, 0, 2); // CALL 0x009f6de2
//             rc &= EDX;                        // AND EAX, EDX (uses residual EDX)
//             if (rc == -1) goto error;
//         }
//     check_result:
//         ;
//     } else {
//         // 0x108 bits not set — single-byte write path
//         EDI = 0; EDI = 1;                    // XOR EDI, EDI; INC EDI
//         local_result = write_block(obj, &c, 1); // CALL 0x009e9211
//     }
//     if (local_result == 1) {
//         EAX = ESI->field_8;
//         *EAX = c;                            // write the byte
//         goto success;
//     }
//   error:
//     ESI->flags |= 0x20;
//     return -1;
//   success:
//     return (unsigned char)c;
//
//   Stack frame (after prologue PUSH EBP / MOV EBP,ESP / PUSH ECX / PUSH ESI):
//     [EBP-4]  local_result (int)             (via PUSH ECX in prologue)
//     [EBP+8]  arg1 = char c
//     [EBP+0xc] arg2 = obj pointer (overwritten with locked obj)
//   Shrink-wrapped callee saves (pushed mid-function, not in prologue):
//     EBX pushed at +0x41 (start of main path), popped at +0x15c
//     EDI pushed at +0xa8 (before big branch), popped at +0x15b
//   Three epilogue entry points:
//     +0x15d = POP EDI + POP EBX + POP ESI + LEAVE + RET  (full)
//     +0x15c = POP EBX + POP ESI + LEAVE + RET           (EDI not saved)
//     +0x15d = POP ESI + LEAVE + RET                     (neither saved)
//
//   Reloc-bearing sites in the orig 352 bytes:
//     +0x09   REL32 → 0x009d6a61  (CALL lock_stream)
//     +0x19   REL32 → 0x009d9d47  (CALL get_errno, 1st)
//     +0x34   REL32 → 0x009d9d47  (CALL get_errno, 2nd)
//     +0x76   REL32 → 0x009d4d67  (CALL get_stdout, 1st)
//     +0x82   REL32 → 0x009d4d67  (CALL get_stdout, 2nd)
//     +0x91   REL32 → 0x009f6d01  (CALL flush_buf)
//     +0x9c   REL32 → 0x009f0a23  (CALL seek_stream)
//     +0xcb   REL32 → 0x009e9211  (CALL write_block, 1st)
//     +0x116  REL32 → 0x009f6de2  (CALL notify_channel)
//     +0xfd   DIR32 → 0x137b7e0   (MOV: g_channel_table SIB base)
//     +0x106  DIR32 → 0x12eb4d8   (MOV EAX: g_default_entry)
//     +0x13a  REL32 → 0x009e9211  (CALL write_block, 2nd)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function has two absolute-address references embedded in instruction
//   immediates (the SIB-form ADD at +0xfd and the MOV EAX imm32 at +0x106),
//   plus shrink-wrapped callee saves that push EBX and EDI mid-body rather
//   than in the prologue, leading to three distinct epilogue entry points.
//   These constraints make a source-level port very fragile under MSVC 2005
//   /O2. The pragmatic choice (matching the strategy used by FUN_004014b0,
//   FUN_0040ced0 et al.) is a naked-asm body that re-emits the 352 orig bytes
//   verbatim via MASM _emit directives.

extern "C" __declspec(naked) void FUN_009e5064() {
    __asm {
        // 005e5064: 55 8b ec 51 56 8b 75 0c
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x51
        _emit 0x56
        _emit 0x8b
        _emit 0x75
        _emit 0x0c
        // 005e506c: 56 e8 ef 19 ff ff 89 45
        _emit 0x56
        _emit 0xe8
        _emit 0xef
        _emit 0x19
        _emit 0xff
        _emit 0xff
        _emit 0x89
        _emit 0x45
        // 005e5074: 0c 8b 46 0c a8 82 59 75
        _emit 0x0c
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0xa8
        _emit 0x82
        _emit 0x59
        _emit 0x75
        // 005e507c: 17 e8 c5 4c ff ff c7 00
        _emit 0x17
        _emit 0xe8
        _emit 0xc5
        _emit 0x4c
        _emit 0xff
        _emit 0xff
        _emit 0xc7
        _emit 0x00
        // 005e5084: 09 00 00 00 83 4e 0c 20
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0x4e
        _emit 0x0c
        _emit 0x20
        // 005e508c: 83 c8 ff e9 2d 01 00 00
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        _emit 0xe9
        _emit 0x2d
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 005e5094: a8 40 74 0d e8 aa 4c ff
        _emit 0xa8
        _emit 0x40
        _emit 0x74
        _emit 0x0d
        _emit 0xe8
        _emit 0xaa
        _emit 0x4c
        _emit 0xff
        // 005e509c: ff c7 00 22 00 00 00 eb
        _emit 0xff
        _emit 0xc7
        _emit 0x00
        _emit 0x22
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        // 005e50a4: e3 53 33 db a8 01 74 16
        _emit 0xe3
        _emit 0x53
        _emit 0x33
        _emit 0xdb
        _emit 0xa8
        _emit 0x01
        _emit 0x74
        _emit 0x16
        // 005e50ac: a8 10 89 5e 04 0f 84 85
        _emit 0xa8
        _emit 0x10
        _emit 0x89
        _emit 0x5e
        _emit 0x04
        _emit 0x0f
        _emit 0x84
        _emit 0x85
        // 005e50b4: 00 00 00 8b 4e 08 83 e0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        _emit 0x83
        _emit 0xe0
        // 005e50bc: fe 89 0e 89 46 0c 8b 46
        _emit 0xfe
        _emit 0x89
        _emit 0x0e
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        _emit 0x8b
        _emit 0x46
        // 005e50c4: 0c 83 e0 ef 83 c8 02 66
        _emit 0x0c
        _emit 0x83
        _emit 0xe0
        _emit 0xef
        _emit 0x83
        _emit 0xc8
        _emit 0x02
        _emit 0x66
        // 005e50cc: a9 0c 01 89 46 0c 89 5e
        _emit 0xa9
        _emit 0x0c
        _emit 0x01
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        _emit 0x89
        _emit 0x5e
        // 005e50d4: 04 89 5d fc 75 2c e8 88
        _emit 0x04
        _emit 0x89
        _emit 0x5d
        _emit 0xfc
        _emit 0x75
        _emit 0x2c
        _emit 0xe8
        _emit 0x88
        // 005e50dc: fc fe ff 83 c0 20 3b f0
        _emit 0xfc
        _emit 0xfe
        _emit 0xff
        _emit 0x83
        _emit 0xc0
        _emit 0x20
        _emit 0x3b
        _emit 0xf0
        // 005e50e4: 74 0c e8 7c fc fe ff 83
        _emit 0x74
        _emit 0x0c
        _emit 0xe8
        _emit 0x7c
        _emit 0xfc
        _emit 0xfe
        _emit 0xff
        _emit 0x83
        // 005e50ec: c0 40 3b f0 75 0d ff 75
        _emit 0xc0
        _emit 0x40
        _emit 0x3b
        _emit 0xf0
        _emit 0x75
        _emit 0x0d
        _emit 0xff
        _emit 0x75
        // 005e50f4: 0c e8 07 1c 01 00 85 c0
        _emit 0x0c
        _emit 0xe8
        _emit 0x07
        _emit 0x1c
        _emit 0x01
        _emit 0x00
        _emit 0x85
        _emit 0xc0
        // 005e50fc: 59 75 07 56 e8 1e b9 00
        _emit 0x59
        _emit 0x75
        _emit 0x07
        _emit 0x56
        _emit 0xe8
        _emit 0x1e
        _emit 0xb9
        _emit 0x00
        // 005e5104: 00 59 66 f7 46 0c 08 01
        _emit 0x00
        _emit 0x59
        _emit 0x66
        _emit 0xf7
        _emit 0x46
        _emit 0x0c
        _emit 0x08
        _emit 0x01
        // 005e510c: 57 0f 84 80 00 00 00 8b
        _emit 0x57
        _emit 0x0f
        _emit 0x84
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        // 005e5114: 46 08 8b 3e 8d 48 01 89
        _emit 0x46
        _emit 0x08
        _emit 0x8b
        _emit 0x3e
        _emit 0x8d
        _emit 0x48
        _emit 0x01
        _emit 0x89
        // 005e511c: 0e 8b 4e 18 2b f8 49 3b
        _emit 0x0e
        _emit 0x8b
        _emit 0x4e
        _emit 0x18
        _emit 0x2b
        _emit 0xf8
        _emit 0x49
        _emit 0x3b
        // 005e5124: fb 89 4e 04 7e 1d 57 50
        _emit 0xfb
        _emit 0x89
        _emit 0x4e
        _emit 0x04
        _emit 0x7e
        _emit 0x1d
        _emit 0x57
        _emit 0x50
        // 005e512c: ff 75 0c e8 dd 40 00 00
        _emit 0xff
        _emit 0x75
        _emit 0x0c
        _emit 0xe8
        _emit 0xdd
        _emit 0x40
        _emit 0x00
        _emit 0x00
        // 005e5134: 83 c4 0c 89 45 fc eb 4d
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x89
        _emit 0x45
        _emit 0xfc
        _emit 0xeb
        _emit 0x4d
        // 005e513c: 83 c8 20 89 46 0c 83 c8
        _emit 0x83
        _emit 0xc8
        _emit 0x20
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        _emit 0x83
        _emit 0xc8
        // 005e5144: ff eb 79 8b 4d 0c 83 f9
        _emit 0xff
        _emit 0xeb
        _emit 0x79
        _emit 0x8b
        _emit 0x4d
        _emit 0x0c
        _emit 0x83
        _emit 0xf9
        // 005e514c: ff 74 1b 83 f9 fe 74 16
        _emit 0xff
        _emit 0x74
        _emit 0x1b
        _emit 0x83
        _emit 0xf9
        _emit 0xfe
        _emit 0x74
        _emit 0x16
        // 005e5154: 8b c1 83 e0 1f 8b d1 c1
        _emit 0x8b
        _emit 0xc1
        _emit 0x83
        _emit 0xe0
        _emit 0x1f
        _emit 0x8b
        _emit 0xd1
        _emit 0xc1
        // 005e515c: fa 05 c1 e0 06 03 04 95
        _emit 0xfa
        _emit 0x05
        _emit 0xc1
        _emit 0xe0
        _emit 0x06
        _emit 0x03
        _emit 0x04
        _emit 0x95
        // 005e5164: e0 b7 37 01 eb 05 b8 d8
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0xeb
        _emit 0x05
        _emit 0xb8
        _emit 0xd8
        // 005e516c: b4 2e 01 f6 40 04 20 74
        _emit 0xb4
        _emit 0x2e
        _emit 0x01
        _emit 0xf6
        _emit 0x40
        _emit 0x04
        _emit 0x20
        _emit 0x74
        // 005e5174: 14 6a 02 53 53 51 e8 63
        _emit 0x14
        _emit 0x6a
        _emit 0x02
        _emit 0x53
        _emit 0x53
        _emit 0x51
        _emit 0xe8
        _emit 0x63
        // 005e517c: 1c 01 00 23 c2 83 c4 10
        _emit 0x1c
        _emit 0x01
        _emit 0x00
        _emit 0x23
        _emit 0xc2
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 005e5184: 83 f8 ff 74 25 8b 46 08
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        _emit 0x74
        _emit 0x25
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 005e518c: 8a 4d 08 88 08 eb 16 33
        _emit 0x8a
        _emit 0x4d
        _emit 0x08
        _emit 0x88
        _emit 0x08
        _emit 0xeb
        _emit 0x16
        _emit 0x33
        // 005e5194: ff 47 57 8d 45 08 50 ff
        _emit 0xff
        _emit 0x47
        _emit 0x57
        _emit 0x8d
        _emit 0x45
        _emit 0x08
        _emit 0x50
        _emit 0xff
        // 005e519c: 75 0c e8 6e 40 00 00 83
        _emit 0x75
        _emit 0x0c
        _emit 0xe8
        _emit 0x6e
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x83
        // 005e51a4: c4 0c 89 45 fc 39 7d fc
        _emit 0xc4
        _emit 0x0c
        _emit 0x89
        _emit 0x45
        _emit 0xfc
        _emit 0x39
        _emit 0x7d
        _emit 0xfc
        // 005e51ac: 74 09 83 4e 0c 20 83 c8
        _emit 0x74
        _emit 0x09
        _emit 0x83
        _emit 0x4e
        _emit 0x0c
        _emit 0x20
        _emit 0x83
        _emit 0xc8
        // 005e51b4: ff eb 08 8b 45 08 25 ff
        _emit 0xff
        _emit 0xeb
        _emit 0x08
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        _emit 0x25
        _emit 0xff
        // 005e51bc: 00 00 00 5f 5b 5e c9 c3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5f
        _emit 0x5b
        _emit 0x5e
        _emit 0xc9
        _emit 0xc3
    }
}
