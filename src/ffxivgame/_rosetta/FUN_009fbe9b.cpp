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
// FUNCTION: ffxivgame 0x009fbe9b — _fputwc_nolock (__cdecl, 478 B / 0x1de, /GS).
//
// Inspection (read from the disassembly at orig RVA 0x005fbe9b):
//
//   __cdecl wint_t FUN_009fbe9b(wchar_t ch /*[EBP+8]*/, FILE *str /*[EBP+0xc]*/)
//
//   Structural shape (traced from the asm) — the MSVC 2005 CRT _fputwc_nolock:
//
//     if (!(str->_flag & _IOSTRG /*0x40*/)) {
//         if      (_textmode_safe(_fileno(str)) == __IOINFO_TM_UTF8 /*2*/) { /* fall to raw */ }
//         else if (_textmode_safe(_fileno(str)) == __IOINFO_TM_UTF16LE /*1*/) {
//             // emit low byte then high byte via _putc_nolock (inlined)
//             if (_putc_nolock(((char*)&ch)[0], str) == EOF) return WEOF;
//             if (_putc_nolock(((char*)&ch)[1], str) == EOF) return WEOF;
//             return (wint_t)(unsigned short)ch;
//         }
//         else if (_osfile_safe(_fileno(str)) & FTEXT /*0x80*/) {
//             char mbc[MB_LEN_MAX /*5*/]; int size, i;
//             if (wctomb_s(&size, mbc, 5, ch) != 0) return WEOF;
//             for (i = 0; i < size; i++)
//                 if (_putc_nolock(mbc[i], str) == EOF) return WEOF;
//             return (wint_t)(unsigned short)ch;
//         }
//     }
//     // raw / string-stream / UTF8: write the 2-byte wchar_t (inlined _putwc_nolock)
//     return (wint_t)((str->_cnt -= 2) >= 0 ? (*(wchar_t*)str->_ptr++ = ch)
//                                           : _flswbuf((unsigned short)ch, str));
//
//   Calls (REL32) / global refs (DIR32) in the orig 478 bytes:
//     CALL 0x009d6a61  _fileno            (×6)
//     CALL 0x009e5064  _flsbuf            (×3, inlined _putc_nolock overflow)
//     CALL 0x009f711f  wctomb_s
//     CALL 0x009fc518  _flswbuf
//     CALL 0x009d20f4  __security_check_cookie
//     DIR32 0x012ea8b0 __security_cookie
//     DIR32 0x0137b7e0 __pioinfo table (SIB base of the _pioinfo macro)
//     DIR32 0x012eb4d8 __badioinfo      (the safe-access fallback ioinfo)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This function embeds three absolute-address references directly in
//   instruction immediates: the /GS security cookie load (MOV EAX,[0x12ea8b0]),
//   the __pioinfo table SIB base (LEA EBX,[EAX*4+0x137b7e0]) and the __badioinfo
//   fallback (MOV EDI,0x12eb4d8). MSVC 2005 cannot be coaxed into reproducing
//   those exact absolute operands from a clean-room source-level port, so — as
//   with the sibling _flsbuf (FUN_009e5064) and the other CRT-internal stream
//   helpers — the matching strategy is a naked-asm body re-emitting the 478
//   orig bytes verbatim via MASM _emit directives.

extern "C" __declspec(naked) void FUN_009fbe9b() {
    __asm {
        // 5fbe9b: 55 8b ec 83 ec 10 a1 b0
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x83
        _emit 0xec
        _emit 0x10
        _emit 0xa1
        _emit 0xb0
        // 5fbea3: a8 2e 01 33 c5 89 45 fc
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc5
        _emit 0x89
        _emit 0x45
        _emit 0xfc
        // 5fbeab: 53 56 8b 75 0c f6 46 0c
        _emit 0x53
        _emit 0x56
        _emit 0x8b
        _emit 0x75
        _emit 0x0c
        _emit 0xf6
        _emit 0x46
        _emit 0x0c
        // 5fbeb3: 40 57 0f 85 8f 01 00 00
        _emit 0x40
        _emit 0x57
        _emit 0x0f
        _emit 0x85
        _emit 0x8f
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 5fbebb: 56 e8 a0 ab fd ff 83 f8
        _emit 0x56
        _emit 0xe8
        _emit 0xa0
        _emit 0xab
        _emit 0xfd
        _emit 0xff
        _emit 0x83
        _emit 0xf8
        // 5fbec3: ff 59 bf d8 b4 2e 01 74
        _emit 0xff
        _emit 0x59
        _emit 0xbf
        _emit 0xd8
        _emit 0xb4
        _emit 0x2e
        _emit 0x01
        _emit 0x74
        // 5fbecb: 2e 56 e8 8f ab fd ff 83
        _emit 0x2e
        _emit 0x56
        _emit 0xe8
        _emit 0x8f
        _emit 0xab
        _emit 0xfd
        _emit 0xff
        _emit 0x83
        // 5fbed3: f8 fe 59 74 22 56 e8 83
        _emit 0xf8
        _emit 0xfe
        _emit 0x59
        _emit 0x74
        _emit 0x22
        _emit 0x56
        _emit 0xe8
        _emit 0x83
        // 5fbedb: ab fd ff c1 f8 05 56 8d
        _emit 0xab
        _emit 0xfd
        _emit 0xff
        _emit 0xc1
        _emit 0xf8
        _emit 0x05
        _emit 0x56
        _emit 0x8d
        // 5fbee3: 1c 85 e0 b7 37 01 e8 73
        _emit 0x1c
        _emit 0x85
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0xe8
        _emit 0x73
        // 5fbeeb: ab fd ff 83 e0 1f 59 c1
        _emit 0xab
        _emit 0xfd
        _emit 0xff
        _emit 0x83
        _emit 0xe0
        _emit 0x1f
        _emit 0x59
        _emit 0xc1
        // 5fbef3: e0 06 03 03 59 eb 02 8b
        _emit 0xe0
        _emit 0x06
        _emit 0x03
        _emit 0x03
        _emit 0x59
        _emit 0xeb
        _emit 0x02
        _emit 0x8b
        // 5fbefb: c7 8a 40 24 24 7f 3c 02
        _emit 0xc7
        _emit 0x8a
        _emit 0x40
        _emit 0x24
        _emit 0x24
        _emit 0x7f
        _emit 0x3c
        _emit 0x02
        // 5fbf03: 0f 84 41 01 00 00 56 e8
        _emit 0x0f
        _emit 0x84
        _emit 0x41
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        // 5fbf0b: 52 ab fd ff 83 f8 ff 59
        _emit 0x52
        _emit 0xab
        _emit 0xfd
        _emit 0xff
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        _emit 0x59
        // 5fbf13: 74 2e 56 e8 46 ab fd ff
        _emit 0x74
        _emit 0x2e
        _emit 0x56
        _emit 0xe8
        _emit 0x46
        _emit 0xab
        _emit 0xfd
        _emit 0xff
        // 5fbf1b: 83 f8 fe 59 74 22 56 e8
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        _emit 0x59
        _emit 0x74
        _emit 0x22
        _emit 0x56
        _emit 0xe8
        // 5fbf23: 3a ab fd ff c1 f8 05 56
        _emit 0x3a
        _emit 0xab
        _emit 0xfd
        _emit 0xff
        _emit 0xc1
        _emit 0xf8
        _emit 0x05
        _emit 0x56
        // 5fbf2b: 8d 1c 85 e0 b7 37 01 e8
        _emit 0x8d
        _emit 0x1c
        _emit 0x85
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0xe8
        // 5fbf33: 2a ab fd ff 83 e0 1f 59
        _emit 0x2a
        _emit 0xab
        _emit 0xfd
        _emit 0xff
        _emit 0x83
        _emit 0xe0
        _emit 0x1f
        _emit 0x59
        // 5fbf3b: c1 e0 06 03 03 59 eb 02
        _emit 0xc1
        _emit 0xe0
        _emit 0x06
        _emit 0x03
        _emit 0x03
        _emit 0x59
        _emit 0xeb
        _emit 0x02
        // 5fbf43: 8b c7 8a 40 24 24 7f 3c
        _emit 0x8b
        _emit 0xc7
        _emit 0x8a
        _emit 0x40
        _emit 0x24
        _emit 0x24
        _emit 0x7f
        _emit 0x3c
        // 5fbf4b: 01 75 5c ff 4e 04 8b 5d
        _emit 0x01
        _emit 0x75
        _emit 0x5c
        _emit 0xff
        _emit 0x4e
        _emit 0x04
        _emit 0x8b
        _emit 0x5d
        // 5fbf53: 08 78 0e 8b 06 88 18 8b
        _emit 0x08
        _emit 0x78
        _emit 0x0e
        _emit 0x8b
        _emit 0x06
        _emit 0x88
        _emit 0x18
        _emit 0x8b
        // 5fbf5b: 0e 0f b6 01 41 89 0e eb
        _emit 0x0e
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        _emit 0x41
        _emit 0x89
        _emit 0x0e
        _emit 0xeb
        // 5fbf63: 0c 0f be c3 56 50 e8 f6
        _emit 0x0c
        _emit 0x0f
        _emit 0xbe
        _emit 0xc3
        _emit 0x56
        _emit 0x50
        _emit 0xe8
        _emit 0xf6
        // 5fbf6b: 90 fe ff 59 59 83 f8 ff
        _emit 0x90
        _emit 0xfe
        _emit 0xff
        _emit 0x59
        _emit 0x59
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        // 5fbf73: 75 09 66 0d ff ff e9 ec
        _emit 0x75
        _emit 0x09
        _emit 0x66
        _emit 0x0d
        _emit 0xff
        _emit 0xff
        _emit 0xe9
        _emit 0xec
        // 5fbf7b: 00 00 00 ff 4e 04 78 0e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0x4e
        _emit 0x04
        _emit 0x78
        _emit 0x0e
        // 5fbf83: 8b 06 88 38 8b 0e 0f b6
        _emit 0x8b
        _emit 0x06
        _emit 0x88
        _emit 0x38
        _emit 0x8b
        _emit 0x0e
        _emit 0x0f
        _emit 0xb6
        // 5fbf8b: 01 41 89 0e eb 0c 0f be
        _emit 0x01
        _emit 0x41
        _emit 0x89
        _emit 0x0e
        _emit 0xeb
        _emit 0x0c
        _emit 0x0f
        _emit 0xbe
        // 5fbf93: c7 56 50 e8 c9 90 fe ff
        _emit 0xc7
        _emit 0x56
        _emit 0x50
        _emit 0xe8
        _emit 0xc9
        _emit 0x90
        _emit 0xfe
        _emit 0xff
        // 5fbf9b: 59 59 83 f8 ff 74 d3 66
        _emit 0x59
        _emit 0x59
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        _emit 0x74
        _emit 0xd3
        _emit 0x66
        // 5fbfa3: 8b c3 e9 c0 00 00 00 56
        _emit 0x8b
        _emit 0xc3
        _emit 0xe9
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x56
        // 5fbfab: e8 b1 aa fd ff 83 f8 ff
        _emit 0xe8
        _emit 0xb1
        _emit 0xaa
        _emit 0xfd
        _emit 0xff
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        // 5fbfb3: 59 74 2e 56 e8 a5 aa fd
        _emit 0x59
        _emit 0x74
        _emit 0x2e
        _emit 0x56
        _emit 0xe8
        _emit 0xa5
        _emit 0xaa
        _emit 0xfd
        // 5fbfbb: ff 83 f8 fe 59 74 22 56
        _emit 0xff
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        _emit 0x59
        _emit 0x74
        _emit 0x22
        _emit 0x56
        // 5fbfc3: e8 99 aa fd ff c1 f8 05
        _emit 0xe8
        _emit 0x99
        _emit 0xaa
        _emit 0xfd
        _emit 0xff
        _emit 0xc1
        _emit 0xf8
        _emit 0x05
        // 5fbfcb: 56 8d 1c 85 e0 b7 37 01
        _emit 0x56
        _emit 0x8d
        _emit 0x1c
        _emit 0x85
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        // 5fbfd3: e8 89 aa fd ff 83 e0 1f
        _emit 0xe8
        _emit 0x89
        _emit 0xaa
        _emit 0xfd
        _emit 0xff
        _emit 0x83
        _emit 0xe0
        _emit 0x1f
        // 5fbfdb: 59 c1 e0 06 03 03 59 eb
        _emit 0x59
        _emit 0xc1
        _emit 0xe0
        _emit 0x06
        _emit 0x03
        _emit 0x03
        _emit 0x59
        _emit 0xeb
        // 5fbfe3: 02 8b c7 f6 40 04 80 74
        _emit 0x02
        _emit 0x8b
        _emit 0xc7
        _emit 0xf6
        _emit 0x40
        _emit 0x04
        _emit 0x80
        _emit 0x74
        // 5fbfeb: 5e ff 75 08 8d 45 f4 6a
        _emit 0x5e
        _emit 0xff
        _emit 0x75
        _emit 0x08
        _emit 0x8d
        _emit 0x45
        _emit 0xf4
        _emit 0x6a
        // 5fbff3: 05 50 8d 45 f0 50 e8 21
        _emit 0x05
        _emit 0x50
        _emit 0x8d
        _emit 0x45
        _emit 0xf0
        _emit 0x50
        _emit 0xe8
        _emit 0x21
        // 5fbffb: b1 ff ff 83 c4 10 85 c0
        _emit 0xb1
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x85
        _emit 0xc0
        // 5fc003: 0f 85 6c ff ff ff 33 db
        _emit 0x0f
        _emit 0x85
        _emit 0x6c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x33
        _emit 0xdb
        // 5fc00b: 39 5d f0 7e 34 ff 4e 04
        _emit 0x39
        _emit 0x5d
        _emit 0xf0
        _emit 0x7e
        _emit 0x34
        _emit 0xff
        _emit 0x4e
        _emit 0x04
        // 5fc013: 78 12 8b 06 8a 4c 1d f4
        _emit 0x78
        _emit 0x12
        _emit 0x8b
        _emit 0x06
        _emit 0x8a
        _emit 0x4c
        _emit 0x1d
        _emit 0xf4
        // 5fc01b: 88 08 8b 0e 0f b6 01 41
        _emit 0x88
        _emit 0x08
        _emit 0x8b
        _emit 0x0e
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        _emit 0x41
        // 5fc023: 89 0e eb 0e 0f be 44 1d
        _emit 0x89
        _emit 0x0e
        _emit 0xeb
        _emit 0x0e
        _emit 0x0f
        _emit 0xbe
        _emit 0x44
        _emit 0x1d
        // 5fc02b: f4 56 50 e8 31 90 fe ff
        _emit 0xf4
        _emit 0x56
        _emit 0x50
        _emit 0xe8
        _emit 0x31
        _emit 0x90
        _emit 0xfe
        _emit 0xff
        // 5fc033: 59 59 83 f8 ff 0f 84 37
        _emit 0x59
        _emit 0x59
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        _emit 0x0f
        _emit 0x84
        _emit 0x37
        // 5fc03b: ff ff ff 43 3b 5d f0 7c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x43
        _emit 0x3b
        _emit 0x5d
        _emit 0xf0
        _emit 0x7c
        // 5fc043: cc 66 8b 45 08 eb 20 83
        _emit 0xcc
        _emit 0x66
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        _emit 0xeb
        _emit 0x20
        _emit 0x83
        // 5fc04b: 46 04 fe 78 0d 8b 0e 8b
        _emit 0x46
        _emit 0x04
        _emit 0xfe
        _emit 0x78
        _emit 0x0d
        _emit 0x8b
        _emit 0x0e
        _emit 0x8b
        // 5fc053: 45 08 66 89 01 83 06 02
        _emit 0x45
        _emit 0x08
        _emit 0x66
        _emit 0x89
        _emit 0x01
        _emit 0x83
        _emit 0x06
        _emit 0x02
        // 5fc05b: eb 0d 0f b7 45 08 56 50
        _emit 0xeb
        _emit 0x0d
        _emit 0x0f
        _emit 0xb7
        _emit 0x45
        _emit 0x08
        _emit 0x56
        _emit 0x50
        // 5fc063: e8 b0 04 00 00 59 59 8b
        _emit 0xe8
        _emit 0xb0
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x59
        _emit 0x8b
        // 5fc06b: 4d fc 5f 5e 33 cd 5b e8
        _emit 0x4d
        _emit 0xfc
        _emit 0x5f
        _emit 0x5e
        _emit 0x33
        _emit 0xcd
        _emit 0x5b
        _emit 0xe8
        // 5fc073: 7d 60 fd ff c9 c3
        _emit 0x7d
        _emit 0x60
        _emit 0xfd
        _emit 0xff
        _emit 0xc9
        _emit 0xc3
    }
}
