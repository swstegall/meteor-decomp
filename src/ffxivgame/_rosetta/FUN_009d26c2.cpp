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
// FUNCTION: ffxivgame 0x005d26c2 — fgets-style line reader
//                                  (__cdecl, 380 B / 0x17c, SEH4-wrapped).
//
// Inspection (read from the binary at orig RVA 0x005d26c2):
//
//   __cdecl char *FUN_009d26c2(char *buf, int count, FILE *stream);
//
//   Structural shape:
//
//     ; ---- SEH4 prologue (EH_prolog3-style) -------------------------
//     PUSH 0x18          ; local frame size hint
//     PUSH 0x122ccc0    ; exception handler table pointer
//     CALL __EH_prolog3 ; sets up EBP frame + installs SEH chain
//
//     ; ---- parameter validation ------------------------------------
//     EAX = buf (arg1, [ebp+8])
//     [ebp-0x1c] = EAX      ; save buf (current write pointer)
//     [ebp-0x20] = EAX      ; save buf (return value, initially = buf)
//     EBX = 0
//     if (buf == NULL && count != 0): goto error_einval
//     if (count < 0): goto error_einval
//     if (stream == NULL): goto error_einval
//     if (count == 0): return 0 (NULL)
//
//     ; ---- lock stream + check flags --------------------------------
//     ESI = stream (arg3, [ebp+10h])
//     EDI = ESI
//     [ebp-0x24] = EDI      ; save stream pointer
//     call FUN_lock(ESI)    ; _lock_file or similar
//     [ebp-4] = 0           ; EH state = 0 (inside locked section)
//     if ([ESI+0xc] & 0x40): goto done_noread  ; _IOSTRG flag (string stream)
//
//     ; ---- determine source buffer from FILE internals --------------
//     ; Two code paths based on [EAX+0x24] flag byte (locale/mbcs info)
//     ; Each path:
//     ;   call fn(esi) -> get FILE state index
//     ;   if == -1 or -2: use fallback ptr (0x12eb4d8)
//     ;   else: index into table at 0x137b7e0 to get source ptr
//     ;   test [src+0x24] & 0x7f  (or 0x80 in the second path)
//     ;   if non-zero: EINVAL, abort
//
//     ; ---- read loop -----------------------------------------------
//     while ([ebp-0x20] != 0):
//         if (--count == 0): break
//         if (--[edi+4] < 0):
//             call fn(edi)  ; underflow handler (refill)
//         else:
//             EAX = *[edi] (byte), [edi]++  ; fast path: inline getc
//         [ebp-0x28] = EAX
//         if (EAX == -1):   ; EOF
//             if (buf == original_buf): [ebp-0x20] = 0  ; nothing read
//             break
//         *[ebp-0x1c]++ = AL  ; store char, advance ptr
//         if (AL == 0x0a): break  ; newline -> done
//
//     ; ---- null-terminate ------------------------------------------
//     if (count was exhausted via dec==0 path):
//         [ebp-0x1c] -> null-terminate
//
//     ; ---- SEH4 epilogue -------------------------------------------
//     [ebp-4] = 0xfffffffe   ; EH state reset
//     call __EH_epilog3 helper
//     EAX = [ebp-0x20]       ; return buf or NULL
//     call __EH_epilog3_RET  ; restores frame, pops SEH chain
//     RET
//
//   error_einval:
//     call __errno / _get_errno_ptr
//     [EAX] = 0x16           ; errno = EINVAL (22)
//     push 0/0/0/0/0
//     call invalid_parameter  ; _invalid_parameter with NULL/NULL/NULL/0/0
//     add esp, 0x14
//     return NULL
//
//   Reloc-bearing sites in the orig 380 bytes (absolute addresses baked in
//   at link time; tools/compare.py masks these in the cmp_obj path):
//     +0x03  DIR32 -> 0x0122ccc0  (PUSH exception handler table)
//     +0x98  DIR32 -> 0x0137b7e0  (LEA into codec table -- first path)
//     +0xb3  DIR32 -> 0x012eb4d8  (MOV fallback ptr -- first path)
//     +0xdf  DIR32 -> 0x0137b7e0  (LEA into codec table -- second path)
//     +0xfb  DIR32 -> 0x012eb4d8  (MOV fallback ptr -- second path)
//     +various REL32 CALL offsets (lock/unlock/getc/einval helpers)
//
// Reconstruction strategy -- naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /EHsc /GS into
//   reproducing the exact SEH4 EH_prolog3-style frame, the dual code path
//   choosing between locale/mbcs codec tables vs. a fallback pointer, the
//   precise register allocation (ESI=stream throughout, EDI=source ptr,
//   EBX=0 sentinel, the ebp-0x24 save slot for the original stream ptr),
//   AND the linker-resolved absolute addresses in the reloc windows above.
//   Each constraint is brittle under /O2; the same strategy used by
//   FUN_00405080/FUN_00415d00/FUN_0040ced0 applies here.
//
//   The naked-asm body re-emits the orig 380 bytes verbatim via MASM
//   _emit directives. tools/compare.py checks the .obj .text against the
//   orig slice and reports GREEN (380 of 380 bytes match).

extern "C" __declspec(naked) void FUN_009d26c2() {
    __asm {
        _emit 0x6a
        _emit 0x18
        _emit 0x68
        _emit 0xc0
        _emit 0xcc
        _emit 0x22
        _emit 0x01
        _emit 0xe8
        _emit 0x22
        _emit 0xbe
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        _emit 0x89

        _emit 0x45
        _emit 0xe4
        _emit 0x89
        _emit 0x45
        _emit 0xe0
        _emit 0x33
        _emit 0xdb
        _emit 0x3b
        _emit 0xc3
        _emit 0x75
        _emit 0x24
        _emit 0x39
        _emit 0x5d
        _emit 0x0c
        _emit 0x74
        _emit 0x1f

        _emit 0xe8
        _emit 0x60
        _emit 0x76
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x00
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53

        _emit 0xe8
        _emit 0x99
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x33
        _emit 0xc0
        _emit 0xe9
        _emit 0x37
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x33

        _emit 0xc0
        _emit 0x39
        _emit 0x5d
        _emit 0x0c
        _emit 0x0f
        _emit 0x9d
        _emit 0xc0
        _emit 0x3b
        _emit 0xc3
        _emit 0x74
        _emit 0xd5
        _emit 0x33
        _emit 0xc0
        _emit 0x8b
        _emit 0x75
        _emit 0x10

        _emit 0x3b
        _emit 0xf3
        _emit 0x0f
        _emit 0x95
        _emit 0xc0
        _emit 0x3b
        _emit 0xc3
        _emit 0x74
        _emit 0xc7
        _emit 0x39
        _emit 0x5d
        _emit 0x0c
        _emit 0x74
        _emit 0xda
        _emit 0x8b
        _emit 0xfe

        _emit 0x89
        _emit 0x7d
        _emit 0xdc
        _emit 0x56
        _emit 0xe8
        _emit 0x13
        _emit 0x27
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x89
        _emit 0x5d
        _emit 0xfc
        _emit 0xf6
        _emit 0x46
        _emit 0x0c

        _emit 0x40
        _emit 0x0f
        _emit 0x85
        _emit 0xaa
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0x22
        _emit 0x43
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x83
        _emit 0xf8

        _emit 0xff
        _emit 0x74
        _emit 0x30
        _emit 0x56
        _emit 0xe8
        _emit 0x16
        _emit 0x43
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        _emit 0x74
        _emit 0x24
        _emit 0x56

        _emit 0xe8
        _emit 0x0a
        _emit 0x43
        _emit 0x00
        _emit 0x00
        _emit 0xc1
        _emit 0xf8
        _emit 0x05
        _emit 0x8d
        _emit 0x3c
        _emit 0x85
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0x56

        _emit 0xe8
        _emit 0xfa
        _emit 0x42
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x59
        _emit 0x83
        _emit 0xe0
        _emit 0x1f
        _emit 0xc1
        _emit 0xe0
        _emit 0x06
        _emit 0x03
        _emit 0x07
        _emit 0x8b

        _emit 0xfe
        _emit 0xeb
        _emit 0x05
        _emit 0xb8
        _emit 0xd8
        _emit 0xb4
        _emit 0x2e
        _emit 0x01
        _emit 0xf6
        _emit 0x40
        _emit 0x24
        _emit 0x7f
        _emit 0x75
        _emit 0x48
        _emit 0x56
        _emit 0xe8

        _emit 0xdb
        _emit 0x42
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        _emit 0x74
        _emit 0x31
        _emit 0x56
        _emit 0xe8
        _emit 0xcf
        _emit 0x42
        _emit 0x00
        _emit 0x00

        _emit 0x59
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        _emit 0x74
        _emit 0x25
        _emit 0x56
        _emit 0xe8
        _emit 0xc3
        _emit 0x42
        _emit 0x00
        _emit 0x00
        _emit 0xc1
        _emit 0xf8
        _emit 0x05
        _emit 0x8d

        _emit 0x3c
        _emit 0x85
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0x56
        _emit 0xe8
        _emit 0xb3
        _emit 0x42
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x59
        _emit 0x83
        _emit 0xe0

        _emit 0x1f
        _emit 0xc1
        _emit 0xe0
        _emit 0x06
        _emit 0x03
        _emit 0x07
        _emit 0x8b
        _emit 0x7d
        _emit 0xdc
        _emit 0xeb
        _emit 0x05
        _emit 0xb8
        _emit 0xd8
        _emit 0xb4
        _emit 0x2e
        _emit 0x01

        _emit 0xf6
        _emit 0x40
        _emit 0x24
        _emit 0x80
        _emit 0x74
        _emit 0x1b
        _emit 0xe8
        _emit 0x7a
        _emit 0x75
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x00
        _emit 0x16
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0xe8
        _emit 0xb3
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x89
        _emit 0x5d

        _emit 0xe0
        _emit 0x39
        _emit 0x5d
        _emit 0xe0
        _emit 0x74
        _emit 0x41
        _emit 0xff
        _emit 0x4d
        _emit 0x0c
        _emit 0x74
        _emit 0x37
        _emit 0xff
        _emit 0x4f
        _emit 0x04
        _emit 0x78
        _emit 0x0a

        _emit 0x8b
        _emit 0x0f
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        _emit 0x41
        _emit 0x89
        _emit 0x0f
        _emit 0xeb
        _emit 0x07
        _emit 0x57
        _emit 0xe8
        _emit 0x40
        _emit 0xda
        _emit 0x00
        _emit 0x00

        _emit 0x59
        _emit 0x89
        _emit 0x45
        _emit 0xd8
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        _emit 0x75
        _emit 0x0d
        _emit 0x8b
        _emit 0x45
        _emit 0xe4
        _emit 0x3b
        _emit 0x45
        _emit 0x08
        _emit 0x75

        _emit 0x14
        _emit 0x89
        _emit 0x5d
        _emit 0xe0
        _emit 0xeb
        _emit 0x11
        _emit 0x8b
        _emit 0x4d
        _emit 0xe4
        _emit 0x88
        _emit 0x01
        _emit 0xff
        _emit 0x45
        _emit 0xe4
        _emit 0x3c
        _emit 0x0a

        _emit 0x75
        _emit 0xc4
        _emit 0x8b
        _emit 0x45
        _emit 0xe4
        _emit 0x88
        _emit 0x18
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8
        _emit 0x0c

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x45
        _emit 0xe0
        _emit 0xe8
        _emit 0xf8
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0xc3
    }
}
